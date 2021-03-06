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

#include "negamaxplayer.h"

#include "heuristic.h"
#include "hexdamegame.h"

#include <qmath.h>
#include <QTime>
#include <QCoreApplication>

#include <QtDebug>

int NegaMaxPlayer::cnt = 0;
int NegaMaxPlayer::total = 0;

NegaMaxPlayer::NegaMaxPlayer(HexdameGame *game, Color color, AbstractHeuristic *heuristic)
    : AbstractPlayer(AI, game, color)
    , _heuristic(heuristic)
{
    qsrand(QTime::currentTime().second());
}

NegaMaxPlayer::~NegaMaxPlayer()
{
    delete _heuristic;

    mutex.lock();
    abort = true;
    mutex.unlock();

    wait();
}

void
NegaMaxPlayer::play()
{
    QTime tic;
    tic.start();

    nodeCnt = 0;
    int bestValue = INT_MIN;
    QList<Move> bestMoves;
    QHash<Coord, QMultiHash<Coord, Move>> moves = _game->grid().computeValidMoves(_color);
    int depth = 4;
    foreach (auto m, moves.values()) {
        foreach (Move mm, m.values()) {
            if (abort) return;

            nodeCnt++;
            HexdameGrid child(_game->grid());
            child.makeMove(mm);
            int val = -negamax(child, depth-1, -INT_MAX, INT_MAX, -_color);

            if (bestValue <= val) {
                if (bestValue < val) {
                    bestValue = val;
                    bestMoves.clear();
                }
                bestMoves << mm;
            }
        }
    }
    //qDebug("%7s %5s %2d %8d %10d", "NMP", _color == White ? "white" : "black", depth, nodeCnt, tic.elapsed());
    qDebug() << cnt/total+1;

    emit move(bestMoves.at(qrand() % bestMoves.size()));
}

int
NegaMaxPlayer::negamax(const HexdameGrid &node, int depth, int alpha, int beta, int color)
{
    if (abort) return 0x42;

    nodeCnt++;
    if (depth == 0 || node.winner() != None) {
        return _heuristic->value(node, color);
    }
    int bestValue = INT_MIN;

    QHash<Coord, QMultiHash<Coord, Move>> moves = node.computeValidMoves((Color) color);
    QMultiHash<Coord, Move> m;
    ++total;
    foreach (m, moves.values()) {
        foreach (Move mm, m.values()) {
            ++cnt;
            HexdameGrid child(node);
            child.makeMove(mm);
            int val = -negamax(child, depth-1, -beta, -alpha, -color);
            bestValue = qMax(bestValue, val);
            alpha = qMax(alpha, val);
            if (alpha >= beta) break;
        }
        if (alpha >= beta) break;
    }
    return bestValue;
}

void
NegaMaxPlayer::run()
{
    play();
}
