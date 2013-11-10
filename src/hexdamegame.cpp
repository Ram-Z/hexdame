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

#include "hexdamegame.h"

HexdameGame::HexdameGame()
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
HexdameGame::canJump(int x, int y) const
{
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            Piece dc = at(x + dx, y + dx);

            qDebug() << x + dx << y + dy << ": " << dc;
        }
    }
}

QHash<Coord, QList<Move>>
HexdameGame::computeValidMoves(Color col)
{
    QHash<Coord, QList<Move>> validMoves;
    if (col == None) {
        validMoves.unite(computeValidMoves(White));
        validMoves.unite(computeValidMoves(Black));
        return validMoves;
    }
    int maxMoves = 0;

    foreach (Coord from, coords()) {
        if (color(from) != col) continue;

        if (isPawn(from)) {
            QList<Move> moves = possibleMoves(from);
            if (moves.empty()) continue;

            if (moves.at(0).taken.size() > maxMoves) {
                maxMoves = moves.at(0).taken.size();
                validMoves.clear();
                validMoves[from] = moves;
            } else if (moves.at(0).taken.size() == maxMoves) {
                validMoves[from] = moves;
            }
        }
    }

    foreach (QList<Move> vl, validMoves) {
        foreach (Move v, vl) {
            qDebug() << v.from << v.path;
        }
    }

    return validMoves;
}

bool
HexdameGame::movePiece(const Coord &oldCoord, const Coord &newCoord)
{
    //TODO check if valid move
    if (oldCoord != newCoord) {
        rat(newCoord) = at(oldCoord);
        rat(oldCoord) = Empty;
    }
}

QList<Coord>
HexdameGame::neighbours(const Coord &c) const
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


const QList<Move>
HexdameGame::possibleMoves(const Coord &c) const
{
    if (isEmpty(c)) return QList<Move> {};

    QList<Move> moves = dfs(c);

//    foreach (Move move, moves) {
//        qDebug() << move.path;
//    }

    if (!moves.empty()) return moves;

    QList<Coord> tos;
    if (color(c) == Black) {
        tos << Coord { -1, 0} << Coord { -1, -1} << Coord {0, -1};
    } else if (color(c) == White) {
        tos << Coord {1, 0} << Coord {1, 1} << Coord {0, 1};
    }

    foreach (Coord n, tos) {
        Coord to = c + n;
        if (grid.contains(to) && isEmpty(to)) {
            Move m {c};
            m.path << to;

            moves << m;
        }
    }

    return moves;
}

QList<Move>
HexdameGame::dfs(const Coord &c, Move move) const
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
operator<<(QDebug dbg, const Coord &coord)
{
    dbg.nospace() << "Coord(" << coord.x << "," << coord.y << ")";
    return dbg.space();
}

