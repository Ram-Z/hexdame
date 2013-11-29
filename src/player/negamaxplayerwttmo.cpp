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

#include "negamaxplayerwttmo.h"

#include "heuristic.h"
#include "hexdamegame.h"

#include <qmath.h>
#include <QTime>
#include <QCoreApplication>

#include <QtDebug>

NegaMaxPlayerWTtMo::NegaMaxPlayerWTtMo(HexdameGame *game, Color color, AbstractHeuristic *heuristic)
    : AbstractPlayer(AI, game, color)
    , _heuristic(heuristic)
{
    qsrand(QTime::currentTime().second());
    ttable.setMaxCost(50000000);
}

NegaMaxPlayerWTtMo::~NegaMaxPlayerWTtMo()
{
    delete _heuristic;

    mutex.lock();
    abort = true;
    mutex.unlock();

    wait();
}

void
NegaMaxPlayerWTtMo::play()
{
    QTime tic;
    tic.start();
    nodeCnt = 0;
    int bestValue = INT_MIN;
    QList<Move> bestMoves;
    QList<Move> moves = orderMoves(_game->grid());
    int depth = 6;
    // iterate in reverse since QMap is sorted asc order and we want the best
    Move mm;
    for (int i = moves.size() - 1; i >= 0; --i) {
        mm = moves.at(i);
        if (abort) return;

        nodeCnt++;
        HexdameGrid child(_game->grid());
        child.makeMove(mm);
        int val = -negamax(child, depth - 1, -INT_MAX, INT_MAX, -_color);

        if (bestValue <= val) {
            if (bestValue < val) {
                bestValue = val;
                bestMoves.clear();
            }
            bestMoves << mm;
        }
    }
    qDebug("%10s %5s %2d %8d %10d", "NMPwTtMo", _color == White ? "white" : "black", depth, nodeCnt, tic.elapsed());

    emit move(bestMoves.at(qrand() % bestMoves.size()));
}

int
NegaMaxPlayerWTtMo::negamax(const HexdameGrid &node, int depth, int alpha, int beta, int color)
{
    if (abort) return 0x42;

    nodeCnt++;
    int alphaOrig = alpha;

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
            qDebug() << "COLLISION";
        }
    }
    if (depth == 0 || node.winner() != None) {
        return _heuristic->value(node, color);
    }

    int bestValue = INT_MIN;

    QList<Move> moves = orderMoves(node);
    Move mm;
    HexdameGrid bestGrid;
    for (int i = moves.size() - 1; i >= 0; --i) {
        Move mm = moves.at(i);
        HexdameGrid child(node);
        child.makeMove(mm);
        int val = -negamax(child, depth-1, -beta, -alpha, -color);
        if (bestValue < val) {
            bestValue = val;
            bestGrid = child;
        }
        alpha = qMax(alpha, val);
        if (alpha >= beta) break;
    }

    TTentry *new_ttentry = new TTentry();
    new_ttentry->value = bestValue;
    new_ttentry->zobrist_key = node.zobristHash();
    new_ttentry->grid = node;
    if (bestValue <= alphaOrig) {
        new_ttentry->flag = FLAG_UPPER;
    } else if (bestValue >= beta) {
        new_ttentry->flag = FLAG_LOWER;
    } else {
        new_ttentry->flag = FLAG_EXACT;
    }
    new_ttentry->depth = depth;
    ttable.insert(node.zobristHash(), new_ttentry);

    return bestValue;
}

QList<Move>
NegaMaxPlayerWTtMo::orderMoves(const HexdameGrid &grid) const
{
    QMap<int, Move> orderedMoves;
    QHash<Coord, QMultiHash<Coord, Move>> moves = grid.validMoves();
    foreach (auto m, moves) {
        foreach (Move mm, m) {
            HexdameGrid copy(grid);
            copy.makeMove(mm);
            TTentry *ttentry = ttable.object(copy.zobristHash());
            if (ttentry) {
                orderedMoves.insertMulti(-ttentry->value, mm);
            } else {
                // add unknown moves with score 0
                orderedMoves.insertMulti(0, mm);
            }
        }
    }
    return orderedMoves.values();
}


void
NegaMaxPlayerWTtMo::run()
{
    play();
}
