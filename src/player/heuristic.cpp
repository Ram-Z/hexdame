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

#include "heuristic.h"

#include "hexdamegrid.h"

using namespace Hexdame;

int SomeHeuristic::value(const HexdameGrid &grid, const int &c) const
{
    if (grid.winner() ==  c) return  1000;
    if (grid.winner() == -c) return -1000;

    int value = 0;
    foreach (Coord coord, grid.coords()) {
        Piece p = grid.at(coord);

        if (isPawn(p) && color(p) ==  c) value+=10;
        if (isKing(p) && color(p) ==  c) value+=30;

        if (isPawn(p) && color(p) == -c) value-=10;
        if (isKing(p) && color(p) == -c) value-=30;

//        if (isPawn(p) && color(p) ==  c) value += qAbs(coord.x - coord.y)/2;
//        if (isPawn(p) && color(p) == -c) value -= qAbs(coord.x - coord.y)/2;

    }
    return value;
}
