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

#ifndef HEXDAMEGAME_H
#define HEXDAMEGAME_H

#include "commondefs.h"

#include <QtDebug> // needed for Q_ASSERT

class HexdameGame : public QObject
{
    Q_OBJECT

public:
    HexdameGame();

    // convenience functions (some of those could be static)
    inline bool isWhite(const Piece &p) const { return p > 0; }
    inline bool isWhite(const Color &c) const { return c > 0; }
    inline bool isWhite(const Coord &c) const { return isWhite(at(c)); }
    inline bool isBlack(const Piece &p) const { return p < 0; }
    inline bool isBlack(const Color &c) const { return c < 0; }
    inline bool isBlack(const Coord &c) const { return isBlack(at(c)); }
    inline bool isEmpty(const Piece &p) const { return !p; }
    inline bool isEmpty(const Color &c) const { return !c; }
    inline bool isEmpty(const Coord &c) const { return isEmpty(at(c)); }
    inline bool  isPawn(const Piece &p) const { return qAbs<int>(p) == 1; }
    inline bool  isPawn(const Coord &c) const { return isPawn(at(c)); }
    inline bool  isKing(const Piece &p) const { return qAbs<int>(p) == 2; }
    inline bool  isKing(const Coord &c) const { return isKing(at(c)); }

    inline Color color(const Piece &p) const { return p > 0 ? White : p < 0 ? Black : None; }
    inline Color color(const Coord &c) const { return color(at(c)); }

    inline const int size() const { return _size; }

    const Piece at(int x, int y) const { return at(Coord {x, y}); }
    const Piece at(Coord c) const { Q_ASSERT(grid.contains(c)); return grid.value(c); }
    Piece &rat(int x, int y) { return grid[Coord {x, y}]; }
    Piece &rat(Coord c) { return grid[c]; }

    QList<Coord> coords() { return grid.keys(); }

    bool canJump(int x, int y) const;
    bool canJump(Coord c) const { return canJump(c.x, c.y); }

    QList<Coord> neighbours(int x, int y) const { return neighbours(Coord {x, y}); }
    QList<Coord> neighbours(Coord c) const;

    const QList<Move> possibleMoves(int x, int y) const { return possibleMoves(Coord {x, y}); }
    const QList<Move> possibleMoves(Coord c) const;

    QHash< Coord, QList< Move > > computeValidMoves(Color col = None);
public slots:
    bool movePiece(Coord oldCoord, Coord newCoord);

private:
    QList<Move> dfs(Coord c, Move move = Move { -1, -1}) const;

    int _size = 0;
    QHash<Coord, Piece> grid;

    QHash<Coord, QList<Move>> validMoves;
};

#endif
