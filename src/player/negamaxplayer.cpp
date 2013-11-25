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

NegaMaxPlayer::NegaMaxPlayer(HexdameGame *game, Color color, AbstractHeuristic *heuristic)
    : AbstractPlayer(AI, game, color)
    , _heuristic(heuristic)
{
    qsrand(QTime::currentTime().second());
}

void
NegaMaxPlayer::play()
{
    // wait a bit before next move
    QTime wait = QTime::currentTime().addMSecs(10);
    while (QTime::currentTime() < wait)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    nodeCnt = 0;
    int bestValue = INT_MIN;
    QList<Move> bestMoves;
    QHash<Coord, QMultiHash<Coord, Move>> moves = _game->grid().validMoves();
    foreach (auto m, moves.values()) {
        foreach (Move mm, m.values()) {
            nodeCnt++;
            HexdameGrid child(_game->grid());
            child.makeMove(mm);
            int val = -negamax(child, 4, -INT_MAX, INT_MAX, -_color);

            if (bestValue <= val) {
                if (bestValue < val) {
                    bestValue = val;
                    bestMoves.clear();
                }
                bestMoves << mm;
            }
        }
    }
    qDebug() << nodeCnt;

    emit move(bestMoves.at(qrand() % bestMoves.size()));
}

int
NegaMaxPlayer::negamax(const HexdameGrid &node, int depth, int alpha, int beta, int color)
{
    nodeCnt++;
    if (depth == 0 || node.winner() != None) {
        return _heuristic->value(node, color);
    }
    int bestValue = INT_MIN;

    QHash<Coord, QMultiHash<Coord, Move>> moves = node.validMoves();
    QMultiHash<Coord, Move> m;
    foreach (m, moves.values()) {
        foreach (Move mm, m.values()) {
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
