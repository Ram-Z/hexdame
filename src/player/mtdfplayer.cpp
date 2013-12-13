/*
 * hexdame: a draughts game played on a hexagonal grid.
 * Copyright (C) 2013  Samir Benmendil <samir.benmendil@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mtdfplayer.h"

#include "heuristic.h"
#include "hexdamegame.h"

#include <qmath.h>
#include <QTime>
#include <QCoreApplication>

#include <QtDebug>
#include <QTimer>

MTDfPlayer::MTDfPlayer(HexdameGame *game, Color color, AbstractHeuristic *heuristic)
    : AbstractPlayer(AI, game, color)
    , _heuristic(heuristic)
{
    //FIXME this does not prealocate the underlying hashtable :@
    // do I really need to subclass QCache?
    ttable.setMaxCost(500000000);
}

MTDfPlayer::~MTDfPlayer()
{
    delete _heuristic;

    mutex.lock();
    abort = true;
    mutex.unlock();

    wait();
}

void
MTDfPlayer::play()
{
    QTime tic;
    tic.start();
    nodeCnt = 0;
    QList<MoveBit> bestMoves = iterativeDeepening(_game->grid(), tic);
    qDebug("%10s %5s %2d %8d %10d", "MTDf", _color == White ? "white" : "black", _depth, nodeCnt, tic.elapsed());
    qDebug() << ttable.totalCost() << ttable.maxCost();

    emit moveBit(bestMoves.at(qrand() % bestMoves.size()));
}

QList<MoveBit>
MTDfPlayer::iterativeDeepening(const HexdameGrid& root, QTime tic)
{
    static const int MAX_DEPTH = 25;

    QList<MoveBit> bestMoves;
    int firstguess = 0;
    for (int d = 0; d <= MAX_DEPTH; ++d) {
        int bestValue = INT_MIN;
        QList<MoveBit> moves = root.computeValidMoveBits(_color);
        foreach (MoveBit m, moves) {
            nodeCnt++;
            HexdameGrid child(root);
            child.makeMoveBit(m);

            firstguess = _color * mtdf(child, _color*firstguess, d);

            if (firstguess >= bestValue) {
                if (firstguess > bestValue) {
                    bestValue = firstguess;
                    bestMoves.clear();
                }
                bestMoves << m;
            }
        }
        _depth = d;
        if (tic.elapsed() >= 6000) break;
    }
    return bestMoves;
}

int
MTDfPlayer::mtdf(const HexdameGrid& node, int f, int depth)
{
    int g = f;
    int upperBound = INT_MAX;
    int lowerBound = -INT_MAX;
    while (lowerBound < upperBound) {
        int beta = g == lowerBound ? g+1 : g;
        g = -_color * negamax(node, depth, beta-1, beta, -_color);
        (g < beta ? upperBound : lowerBound) = g;
    }

    return g;
}

int
MTDfPlayer::negamax(const HexdameGrid &node, int depth, int alpha, int beta, int color)
{
    // return an actuall score +-INF or alpha/beta bounds
    //if (abort) return 0x42;

    int alphaOrig = alpha;
    nodeCnt++;

    TTentry *ttentry = ttable.object(node.zobristHash());
    if (ttentry && ttentry->depth >= depth) {
        if (ttentry->zobrist_key == node.zobristHash()) {
            //qDebug() << ttentry->zobrist_key << ttentry->depth << ttentry->value << ttentry->flag;
            if (ttentry->flag == FLAG_EXACT)
                return ttentry->value;
            else if (ttentry->flag == FLAG_LOWER)
                alpha = qMax<int>(alpha, ttentry->value);
            else if (ttentry->flag == FLAG_UPPER)
                beta = qMin<int>(beta, ttentry->value);

            if (alpha >= beta)
                return ttentry->value;
        } else {
            qWarning() << "COLLISION";
        }
    }

    if (depth == 0 || node.winner() != None) {
        return _heuristic->value(node, color);
    }

    int bestValue = INT_MIN;
    MoveBit bestMove;

    QList<MoveBit> moves = node.computeValidMoveBits((Color) color);
    // move ordering?
    if (ttentry) {
        moves.prepend(ttentry->bestMove);
    }
    foreach (MoveBit m, moves) {
        HexdameGrid child(node);
        child.makeMoveBit(m);
        int val = -negamax(child, depth-1, -beta, -alpha, -color);
        if (val > bestValue) {
            bestValue = val;
            bestMove = m;
        }
        alpha = qMax(alpha, val);
        if (alpha >= beta) break;
    }

    TTentry *new_ttentry = new TTentry();
    new_ttentry->value = bestValue;
    new_ttentry->zobrist_key = node.zobristHash();
    if (bestValue <= alphaOrig) {
        new_ttentry->flag = FLAG_UPPER;
    } else if (bestValue >= beta) {
        new_ttentry->flag = FLAG_LOWER;
    } else {
        new_ttentry->flag = FLAG_EXACT;
    }
    new_ttentry->depth = depth;
    new_ttentry->bestMove = bestMove;
    ttable.insert(node.zobristHash(), new_ttentry);

    return bestValue;
}

void
MTDfPlayer::run()
{
    abort = false;
    qsrand(QDateTime::currentMSecsSinceEpoch());
    play();
}
