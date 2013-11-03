// Samir Benmendil. Copyright (C) 2013. GPL-3.0.

#include "hexgrid.h"

HexGrid::HexGrid()
    : QObject()
    , _size(9)
{
    grid.reserve(_size * _size);

    for (int x = 0; x < _size; ++x) {
        for (int y = 0; y < _size; ++y) {
            int s = size() / 2;
            if (qAbs(x - y) <= s)
                grid[Coord {x, y}] = Empty;
            if (x < s && y < s)
                grid[Coord {x, y}] = WhitePawn;
            if (x > s && y > s)
                grid[Coord {x, y}] = BlackPawn;
        }
    }
}

bool
HexGrid::canJump(int x, int y) const
{
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            Piece dc = at(x + dx, y + dx);

            qDebug() << x + dx << y + dy << ": " << dc;
        }
    }
}

void
HexGrid::computeValidMoves(Color col)
{
    static int jump = 0;
    foreach (Coord from, coords()) {
        Piece p = at(from);
        if (color(p) != col) continue;

        if (isPawn(p)) {
            foreach (Coord to, neighbours(from)) {
                if (!jump) {
                    QList<Coord> forward;
                    forward << Coord {1, 0} << Coord {1, 1} << Coord {0, 1};
                    if (at(to) == Empty) {
                    }
                }

                if (isBlack(p) && isWhite(at(to))) {

                }
            }
        }
    }
}

bool
HexGrid::movePiece(HexGrid::Coord oldCoord, HexGrid::Coord newCoord)
{
    //TODO check if valid move
    if (oldCoord != newCoord) {
        rat(newCoord) = at(oldCoord);
        rat(oldCoord) = Empty;
    }

    computeValidMoves(color(at(newCoord)));
}

QList< HexGrid::Coord >
HexGrid::neighbours(HexGrid::Coord c) const
{
    static QHash<Coord, QList<Coord>> cache;
    if (cache.contains(c)) return cache.value(c);

    QList<Coord> list, retval;
    list << Coord {1, 0} << Coord {1, 1} << Coord {0, 1}
         << Coord { -1, 0} << Coord { -1, -1} << Coord {0, -1};

    foreach (Coord dc, list) {
        if (grid.contains(c + dc))
            retval << c + dc;
    }
    cache[c] = retval;

    return retval;
}


const QList< HexGrid::Coord >
HexGrid::possibleMoves(int x, int y) const
{
    QList<HexGrid::Coord> list;
    if (x + 1 < size()) list << Coord {x + 1, y};
    if (y + 1 < size()) list << Coord {x, y + 1};
    if (x + 1 < size() && y + 1 < size()) list << Coord {x + 1, y + 1};

    return list;
}

QDebug
operator<<(QDebug dbg, const HexGrid::Coord &coord)
{
    dbg.nospace() << "Coord(" << coord.x << "," << coord.y << ")";
    return dbg.space();
}

uint
qHash(const HexGrid::Coord &c)
{
    uint h1 = qHash(c.x);
    uint h2 = qHash(c.y);
    return ((h1 << 16) | (h1 >> 16)) ^ h2;
}
