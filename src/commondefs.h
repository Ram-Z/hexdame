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

#ifndef COMMONDEFS_H
#define COMMONDEFS_H

#include <QList>
#include <QSet>
#include <QtGlobal>

class QDebug;

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

namespace Hexdame {
inline Color color(const Piece &p) { return p > 0 ? White : p < 0 ? Black : None; }

inline bool isWhite(const Piece &p) { return p > 0; }
inline bool isWhite(const Color &c) { return c > 0; }
inline bool isBlack(const Piece &p) { return p < 0; }
inline bool isBlack(const Color &c) { return c < 0; }
inline bool isEmpty(const Piece &p) { return !p; }
inline bool isEmpty(const Color &c) { return !c; }
inline bool  isPawn(const Piece &p) { return qAbs<int>(p) == 1; }
inline bool  isKing(const Piece &p) { return qAbs<int>(p) == 2; }
}

struct Coord {
    qint8 x, y;

    Coord(qint8 x, qint8 y) : x(x), y(y) { }
    bool operator==(const Coord &c) const { return x == c.x && y == c.y; }
    bool operator!=(const Coord &c) const { return !(*this == c); }

    Coord operator+(const Coord &c) const { return Coord(x + c.x, y + c.y); }
    Coord &operator+=(const Coord &c) { x += c.x; y += c.y; return *this; }

    Coord operator-(const Coord &c) const { return Coord(x - c.x, y - c.y); }
    Coord &operator-=(const Coord &c) { x -= c.x; y -= c.y; return *this; }

    friend Coord operator*(qint8 i, const Coord &c) { return Coord(i * c.x, i * c.y); }
    friend Coord operator*(const Coord &c, qint8 i) { return i * c; }
    Coord &operator*=(qint8 i) { x *= i; y *= i; return *this; }

    friend QDebug operator<<(QDebug dbg, const Coord &coord);

    friend uint qHash(const Coord &c) { return ((c.x << 16) | (c.x >> 16)) ^ c.y; }
};

struct Move {
    QList<Coord> path;
    QList<Coord> taken;

    inline const Coord &from() const { return path.first(); }
    inline const Coord &to()   const { return path.last(); }
    inline bool empty() const { return path.empty(); }
    bool operator==(const Move &m) const {
        return this->from() == m.from() && this->to() == m.to()
            && QSet<Coord>::fromList(this->taken) == QSet<Coord>::fromList(m.taken);
    }

    friend QDebug operator<<(QDebug dbg, const Move &move);
};

#endif // COMMONDEFS_H
