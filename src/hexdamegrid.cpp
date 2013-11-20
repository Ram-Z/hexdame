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

#include "hexdamegrid.h"

HexdameGrid::HexdameGrid()
{
    _grid.reserve(SIZE * SIZE);

    for (int x = 0; x < SIZE; ++x) {
        for (int y = 0; y < SIZE; ++y) {
            int s = SIZE / 2;
            if (qAbs(x - y) <= s) {
                _grid[Coord {x, y}] = Empty;
            }
            if (x < s && y < s) {
                _grid[Coord {x, y}] = WhitePawn;
                _cntWhite++;
            } else if (x > s && y > s) {
                _grid[Coord {x, y}] = BlackPawn;
                _cntBlack++;
            }
        }
    }
}

HexdameGrid::HexdameGrid(const HexdameGrid &other)
    : _grid(other._grid)
    , _cntWhite(other._cntWhite)
    , _cntBlack(other._cntBlack)
{

}

HexdameGrid &HexdameGrid::operator=(const HexdameGrid &other)
{
    if (this != &other) {
        _grid = other._grid;
        _cntWhite = other._cntWhite;
        _cntBlack = other._cntBlack;
        _validMoves.clear();
        _maxTaken = 0;
    }
    return *this;
}

bool
HexdameGrid::operator==(const HexdameGrid &other)
{
    return _grid == other._grid;
}

bool
HexdameGrid::gameOver() const
{
    //TODO check for draws
    return !_cntBlack || !_cntWhite;
}

void
HexdameGrid::kingPiece(Coord c)
{
    if (color(c) == None) return;

    if (color(c) == White) {
        if (c.x == 8 || c.y == 8) {
            _grid[c] = WhiteKing;
            return;
        }
    }

    if (color(c) == Black) {
        if (c.x == 0 || c.y == 0) {
            _grid[c] = BlackKing;
            return;
        }
    }
}

void
HexdameGrid::makeMove(const Move &move, bool partial)
{
    _grid[move.to()] = at(move.from());
    _grid[move.from()] = Empty;

    foreach (Coord c, move.taken) {
        if (color(c) == Black) {
            _cntBlack--;
        } else if (color(c) == White) {
            _cntWhite--;
        }
        _grid[c] = Empty;
    }

    if (!partial)
        kingPiece(move.to());
}

QHash<Coord, QMultiHash<Coord, Move>>
HexdameGrid::computeValidMoves(Color col) const
{
    _validMoves.clear();
    _maxTaken = 0;
    if (col == None) {
        QHash<Coord, QMultiHash<Coord, Move>> meh;
        meh.unite(computeValidMoves(White));
        meh.unite(computeValidMoves(Black));
        _validMoves = meh;
        return _validMoves;
    }

    foreach (Coord from, coords()) {
        if (color(from) != col) continue;

        dfs(from);
    }

    if (!_validMoves.empty()) return _validMoves;

    // don't change the order  |<----------Whites moves---------->|<-------------Blacks moves------------->|
    const static QList<Coord> l{Coord{1,0}, Coord{0,1}, Coord{1,1}, Coord{0,-1}, Coord{-1,0}, Coord{-1,-1}};
    foreach(Coord from, coords()) {
        if (color(from) != col) continue;
        for (int i = 0; i < l.size(); ++i) {
            if (i <  3 && isPawn(from) && isBlack(from)) i = 3; // jump to Blacks moves
            if (i >= 3 && isPawn(from) && isWhite(from)) break; // ignore the rest

            for (int j = 1; j < 9; ++j) {
                Coord to = from + j*l.at(i);

                if (!_grid.contains(to)) break;
                if (!isEmpty(to)) break;

                // create Move and add to list
                Move m(from);
                m.path << to;
                _validMoves[from].insert(to, m);

                if (isPawn(from)) break; // Pawns can't jump further than 1
            }
        }
    }

    return _validMoves;
}

void
HexdameGrid::dfs(const Coord &from, Move move) const
{
    static Color col;
    static bool king;
    if (move.from() == Coord { -1, -1}) {
        move = Move(from);
        col = color(from);
        king = isKing(from);
    }

    const static QList<Coord> l{Coord{1,0}, Coord{-1,0}, Coord{0,1}, Coord{0,-1}, Coord{1,1}, Coord{-1,-1}};
    foreach (Coord lv, l) {
        for (int i = 1; i < 9; ++i) {
            Coord over = from + i*lv;

            if (!_grid.contains(over)) break;     // not on the grid
            if (col == color(over)) break;        // same colour
            if (move.taken.contains(over)) break; // already took piece

            if (col == -color(over)) {
                while (++i < 9) {
                    Coord to = from + i*lv;
                    // non-empty cell after jumping
                    if (!_grid.contains(to) || (move.from() != to && !isEmpty(to))) { i = 9; break; }

                    // create a new Move
                    Move newMove(move);
                    newMove.taken << over;
                    newMove.path << to;

                    // update/reset validMoves list
                    if (newMove.taken.size() >= _maxTaken) {
                        if (newMove.taken.size() > _maxTaken) {
                            _validMoves.clear();
                            _maxTaken = newMove.taken.size();
                        }
                        bool dup = false;
                        foreach (Move oldMove, _validMoves.value(move.from())) {
                            if (dup = oldMove == newMove) break; // moves are equivalent
                        }
                        if (!dup) _validMoves[newMove.from()].insert(to, newMove);
                    }

                    // recursive call
                    dfs(to, newMove);

                    if (!king) break; // Pawns can't jump further than 1
                }
            }
            if (!king) break; // Pawns can't jump further than 1
        }
    }
}
