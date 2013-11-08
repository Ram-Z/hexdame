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

#ifndef HEXGRID_H
#define HEXGRID_H

#include <QtDebug> // needed for Q_ASSERT

class HexGrid : public QObject
{
    Q_OBJECT

public:
    enum Piece {
        BlackKing = -2,
        BlackPawn = -1,
        Empty  = 0,
        WhitePawn = 1,
        WhiteKing = 2
    };

    enum Color {
        Black = -1,
        None = 0,
        White = 1
    };

    struct Coord {
        int x, y;

        const bool operator==(Coord c) const { return x == c.x && y == c.y; }
        const bool operator!=(Coord c) const { return !(*this == c); }

        const Coord operator+(Coord c) const { return Coord {x + c.x, y + c.y}; }
        Coord &operator+=(Coord c) { x += c.x; y += c.y; return *this; }

        const Coord operator-(Coord c) const { return Coord {x - c.x, y - c.y}; }
        Coord &operator-=(Coord c) { x -= c.x; y -= c.y; return *this; }

        friend const Coord operator*(int i, Coord c) { return Coord {i * c.x, i * c.y}; }
        friend const Coord operator*(Coord c, int i) { return i * c; }
        Coord &operator*=(int i) { x *= i; y *= i; return *this; }

        friend QDebug operator<<(QDebug dbg, const HexGrid::Coord &coord);

        friend uint qHash(const HexGrid::Coord &c);
    };

    struct Move {
        Coord from;
        QList<Coord> path;
        QList<Coord> taken;

        inline const Coord &to() const { return path.last(); }
    };

    inline static bool isWhite(Piece p) { return p > 0; }
    inline static bool isWhite(Color c) { return c > 0; }
    inline bool isWhite(Coord c) const { return isWhite(at(c)); }
    inline static bool isBlack(Piece p) { return p < 0; }
    inline static bool isBlack(Color c) { return c < 0; }
    inline bool isBlack(Coord c) const { return isBlack(at(c)); }
    inline static bool isEmpty(Piece p) { return !p; }
    inline static bool isEmpty(Color c) { return !c; }
    inline bool isEmpty(Coord c) const { return isEmpty(at(c)); }
    inline static bool isPawn(Piece p) { return qAbs<int>(p) == 1; }
    inline bool isPawn(Coord c) const { return isPawn(at(c)); }
    inline static bool isKing(Piece p) { return qAbs<int>(p) == 2; }
    inline bool isKing(Coord c) const { return isKing(at(c)); }

    inline static Color color(Piece p) { return p > 0 ? White : p < 0 ? Black : None; }
    inline Color color(Coord c) const { return color(at(c)); }

    HexGrid();

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
    const QList<Move> possibleMoves(HexGrid::Coord c) const;

    QHash< Coord, QList< Move > > computeValidMoves(HexGrid::Color col = None);
public slots:
    bool movePiece(HexGrid::Coord oldCoord, HexGrid::Coord newCoord);

private:
    QList<Move> dfs(HexGrid::Coord c, HexGrid::Move move = Move { -1, -1}) const;


    int _size = 0;
    QHash<Coord, Piece> grid;

    QHash<Coord, QList<Move>> validMoves;
};

#endif
