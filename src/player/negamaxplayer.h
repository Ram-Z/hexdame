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

#ifndef NEGAMAX_H
#define NEGAMAX_H

#include "player/abstractplayer.h"

class HexdameGrid;
class AbstractHeuristic;

class NegaMaxPlayer : public AbstractPlayer
{
    Q_OBJECT

public:
    NegaMaxPlayer(HexdameGame *game, Color color, AbstractHeuristic *heuristic);

    virtual void play();

private:
    int negamax(const HexdameGrid& node, int depth, int alpha, int beta, int color);

    AbstractHeuristic *_heuristic;
};

#endif // NEGAMAX_H
