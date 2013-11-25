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

#ifndef NEGAMAXPLAYERWTT_H
#define NEGAMAXPLAYERWTT_H

#define FLAG_EXACT 0
#define FLAG_LOWER 1
#define FLAG_UPPER 2

#include "player/abstractplayer.h"
#include <QCache>

class HexdameGrid;
class AbstractHeuristic;

class NegaMaxPlayerWTt : public AbstractPlayer
{
    Q_OBJECT

public:
    NegaMaxPlayerWTt(HexdameGame *game, Color color, AbstractHeuristic *heuristic);

    virtual void play();

private:
    int negamax(const HexdameGrid& node, int depth, int alpha, int beta, int color);

    struct TTentry {
        quint64 zobrist_key;
        quint8 depth;
        quint8 flag;
        qint16 value;
        Move move;
    };
    QCache<quint64, TTentry> ttable;

    AbstractHeuristic *_heuristic;
    int nodeCnt;
};

#endif // NEGAMAXPLAYERWTT_H
