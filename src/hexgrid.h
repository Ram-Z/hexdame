// Samir Benmendil. Copyright (C) 2013. GPL-3.0.

#ifndef HEXGRID_H
#define HEXGRID_H

#include <QtDebug> // needed for Q_ASSERT

class HexGrid : public QObject
{
    Q_OBJECT

public:
    enum Piece {
        BlackKing = -2,
        Black = -1,
        None  = 0,
        White = 1,
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
    };

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

    const QList<Coord> possibleMoves(int x, int y) const;
    const QList<Coord> possibleMoves(Coord c) const { return possibleMoves(c.x, c.y); }

public slots:
    bool movePiece(HexGrid::Coord oldCoord, HexGrid::Coord newCoord);

private:
    void computeValidMoves();

    int _size = 0;
    QHash<Coord, Piece> grid;

    QHash<Coord, QList<Coord>> validMoves;
};

QDebug operator<<(QDebug dbg, const HexGrid::Coord &coord);
uint qHash(const HexGrid::Coord &c);

#endif
