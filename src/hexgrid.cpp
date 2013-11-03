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

QList<HexGrid::Coord>
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


const QList<HexGrid::Move>
HexGrid::possibleMoves(Coord c) const
{
    if (isEmpty(c)) return QList<Move> {};

    QList<Move> moves = dfs(c);

    foreach (Move move, moves) {
        qDebug() << move.path;
    }

    if (!moves.empty()) return moves;

    foreach (Coord n, neighbours(c)) {
        if (isEmpty(n)) {
            Move m {c};
            m.path << n;

            moves << m;
        }
    }

    return moves;
}

QList<HexGrid::Move>
HexGrid::dfs(Coord c, Move move) const
{
    static QList<Move> best_moves;
    static Color col;
    if (move.from == Coord { -1, -1}) {
        best_moves.clear();
        move.from = c;
        col = color(c);
    }

    foreach (Coord n, neighbours(c)) {
        // different colors
        if (col == -color(n) && !move.taken.contains(n)) {
            Coord j = c + (n - c) * 2;
            // skip if !(onGrid && (empty || startingPos))
            if (!grid.contains(j) || !isEmpty(j) && move.from != j) continue;

            Move newMove(move);

            newMove.path << j;
            newMove.taken << n;

            if (best_moves.empty() || newMove.path.size() == best_moves.at(0).path.size()) {
                best_moves << newMove;
            } else if (newMove.path.size() > best_moves.at(0).path.size()) {
                best_moves.clear();
                best_moves << newMove;
            }

            dfs(j, newMove);
        }
    }
    return best_moves;
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
