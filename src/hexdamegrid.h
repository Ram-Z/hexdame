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

#ifndef HEXDAMEGRID_H
#define HEXDAMEGRID_H

#include "commondefs.h"

#include <QtDebug> // needed for Q_ASSERT

using namespace Hexdame;

class HexdameGrid
{
public:
    HexdameGrid();
    HexdameGrid(const HexdameGrid &other);
    HexdameGrid &operator=(const HexdameGrid &other);
    bool operator==(const HexdameGrid &other);

    static const int SIZE = 9;

    const Piece at(const Coord &c) const { Q_ASSERT(_grid.contains(c)); return _grid.value(c); }
    Piece &operator[](const Coord &c) { return _grid[c]; }
    const Piece operator[](const Coord &c) const { return at(c); }

    QList<Coord> coords() const { return _grid.keys(); }

    const QHash<Coord, QMultiHash<Coord, Move>> &validMoves() const { return _validMoves; }
    const QMultiHash<Coord, Move> validMoves(Coord c) const { return _validMoves.value(c); }

    // convenience functions
    inline bool isWhite(const Coord &c) const { return Hexdame::isWhite(at(c)); }
    inline bool isBlack(const Coord &c) const { return Hexdame::isBlack(at(c)); }
    inline bool isEmpty(const Coord &c) const { return Hexdame::isEmpty(at(c)); }
    inline bool  isPawn(const Coord &c) const { return Hexdame::isPawn(at(c)); }
    inline bool  isKing(const Coord &c) const { return Hexdame::isKing(at(c)); }
    inline Color  color(const Coord &c) const { return Hexdame::color(at(c)); }

    void makeMove(const Move &move, bool partial = false);
    // does not check validity and calculates all valid moves, use for debug
    void move(const Coord &from, const Coord &to);

    bool gameOver() const;

private:
    QHash<Coord, QMultiHash<Coord, Move>> computeValidMoves(Color col);
    void dfs(const Coord &from, Move move = Move());

    void kingPiece(Coord c);

    QHash<Coord, Piece> _grid;

    QHash<Coord, QMultiHash<Coord, Move>> _validMoves;
    mutable int _maxTaken;

    int _cntWhite = 0;
    int _cntBlack = 0;
};

#endif // HEXDAMEGRID_H
