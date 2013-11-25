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

#include <limits>
#include <random>

#include <QtDebug>

QHash<Coord, quint8> HexdameGrid::_coordToIdx;
quint64 HexdameGrid::_zobrist_idx[61][4];

HexdameGrid::HexdameGrid()
{
    _coordToIdx.reserve(67);

    static const int SIZE = 9;

    zobristInit();

    quint8 idx = 0;
    for (int x = 0; x < SIZE; ++x) {
        for (int y = 0; y < SIZE; ++y) {
            static const int s = SIZE / 2;
            if (qAbs(x - y) <= s) {
                _coordToIdx[Coord{x,y}] = idx;
                if (x < s && y < s) {
                    _whitePawns |= 1 << idx;
                    _zobrist_hash ^= _zobrist_idx[idx][2];
                    _cntWhite++;
                } else if (x > s && y > s) {
                    _blackPawns |= (quint64) 1 << idx;
                    _zobrist_hash ^= _zobrist_idx[idx][1];
                    _cntBlack++;
                }
                idx++;
            }
        }
    }

    _coordToIdx.squeeze();

    computeValidMoves(White);
}

HexdameGrid::HexdameGrid(const HexdameGrid &other)
    : _whitePawns(other._whitePawns)
    , _whiteKings(other._whiteKings)
    , _blackPawns(other._blackPawns)
    , _blackKings(other._blackKings)
    , _cntWhite(other._cntWhite)
    , _cntBlack(other._cntBlack)
    , _validMoves(other._validMoves)
    , _maxTaken(other._maxTaken)
    , _zobrist_hash(other._zobrist_hash)
{

}

HexdameGrid &
HexdameGrid::operator=(const HexdameGrid &other)
{
    if (this != &other) {
        _whitePawns = other._whitePawns;
        _whiteKings = other._whiteKings;
        _blackPawns = other._blackPawns;
        _blackKings = other._blackKings;
        _cntWhite = other._cntWhite;
        _cntBlack = other._cntBlack;
        _validMoves = other._validMoves;
        _maxTaken = other._maxTaken;
        _zobrist_hash = other._zobrist_hash;
    }
    return *this;
}

bool
HexdameGrid::operator==(const HexdameGrid &other) const
{
    return _whitePawns == other._whitePawns
        && _blackPawns == other._blackPawns
        && _whiteKings == other._whiteKings
        && _blackKings == other._blackKings;
}

Piece
HexdameGrid::at(const Coord& c) const
{
    quint64 mask = (quint64) 1 << _coordToIdx.value(c);

    if (_whitePawns & mask) return WhitePawn;
    if (_blackPawns & mask) return BlackPawn;
    if (_whiteKings & mask) return WhiteKing;
    if (_blackKings & mask) return BlackKing;
    return Empty;
}

void
HexdameGrid::set(const Coord& c, Piece p)
{
    quint64 mask = (quint64) 1 << _coordToIdx.value(c);

    _whitePawns &= ~mask;
    _blackPawns &= ~mask;
    _whiteKings &= ~mask;
    _blackKings &= ~mask;

    switch (p) {
        case WhitePawn: _whitePawns |= mask; break;
        case BlackPawn: _blackPawns |= mask; break;
        case WhiteKing: _whiteKings |= mask; break;
        case BlackKing: _blackKings |= mask; break;
    }
}

Color
HexdameGrid::winner() const
{
    //TODO check for draws
    if (_cntBlack == 0)
        return White;
    if (_cntWhite == 0)
        return Black;

    return None;
}

void
HexdameGrid::kingPiece(Coord c)
{
    if (color(c) == None) return;

    if (color(c) == White) {
        if (c.x == 8 || c.y == 8) {
            set(c, WhiteKing);
            _zobrist_hash ^= zobristString(c, WhitePawn);
            _zobrist_hash ^= zobristString(c, WhiteKing);
            return;
        }
    }

    if (color(c) == Black) {
        if (c.x == 0 || c.y == 0) {
            set(c, BlackKing);
            _zobrist_hash ^= zobristString(c, BlackPawn);
            _zobrist_hash ^= zobristString(c, BlackKing);
            return;
        }
    }
}

void
HexdameGrid::makeMove(const Move &move, bool partial)
{
    if (move.from() != move.to()) {
        _zobrist_hash ^= zobristString(move.to(), at(move.from()));
        _zobrist_hash ^= zobristString(move.from(), at(move.from()));

        set(move.to(), at(move.from()));
        set(move.from(), Empty);
    }

    foreach (Coord c, move.taken) {
        if (color(c) == Black) {
            _cntBlack--;
        } else if (color(c) == White) {
            _cntWhite--;
        }
        _zobrist_hash ^= zobristString(c, at(c));
        set(c, Empty);
    }

    if (partial) {
        _validMoves.clear();
        _maxTaken = 0;
        dfs(move.to());
    } else {
        kingPiece(move.to());
        if (color(move.to()) == White)
            computeValidMoves(Black);
        if (color(move.to()) == Black)
            computeValidMoves(White);
    }
}

void
HexdameGrid::move(const Coord &from, const Coord &to)
{
    if (!isEmpty(to)) return;

    if (to == from) return;

    _zobrist_hash ^= zobristString(to, at(from));
    _zobrist_hash ^= zobristString(from, at(from));

    set(to, at(from));
    set(from, Empty);

    computeValidMoves(None);
}


QHash<Coord, QMultiHash<Coord, Move>>
HexdameGrid::computeValidMoves(Color col)
{
    _maxTaken = 0;
    _validMoves.clear();

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

    if (col == White)
        _validMoves.reserve(2*_cntWhite);
    else if (col == Black)
        _validMoves.reserve(2*_cntBlack);

    // don't change the order  |<----------Whites moves---------->|<-------------Blacks moves------------->|
    const static QList<Coord> l{Coord{1,0}, Coord{0,1}, Coord{1,1}, Coord{0,-1}, Coord{-1,0}, Coord{-1,-1}};
    foreach(Coord from, coords()) {
        if (color(from) != col) continue;
        for (int i = 0; i < l.size(); ++i) {
            if (i <  3 && isPawn(from) && isBlack(from)) i = 3; // jump to Blacks moves
            if (i >= 3 && isPawn(from) && isWhite(from)) break; // ignore the rest

            for (int j = 1; j < 9; ++j) {
                Coord to = from + j*l.at(i);

                if (!contains(to)) break;
                if (!isEmpty(to)) break;

                // create Move and add to list
                Move m;
                m.path << from << to;
                _validMoves[from].insert(to, m);

                if (isPawn(from)) break; // Pawns can't jump further than 1
            }
        }
    }

    return _validMoves;
}

void
HexdameGrid::dfs(const Coord &from, Move move)
{
    static Color col;
    static bool king;
    if (move.empty()) {
        move.path << from;
        col = color(from);
        king = isKing(from);
    }

    const static QList<Coord> l{Coord{1,0}, Coord{-1,0}, Coord{0,1}, Coord{0,-1}, Coord{1,1}, Coord{-1,-1}};
    foreach (Coord lv, l) {
        for (int i = 1; i < 9; ++i) {
            Coord over = from + i*lv;

            if (!contains(over)) break;           // not on the grid
            if (col == color(over)) break;        // same colour
            if (move.taken.contains(over)) break; // already took piece

            if (col == -color(over)) {
                while (++i < 9) {
                    Coord to = from + i*lv;
                    // non-empty cell after jumping
                    if (!contains(to) || (move.from() != to && !isEmpty(to))) { i = 9; break; }

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

void
HexdameGrid::zobristInit()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<quint64> dis(std::numeric_limits<quint64>::min(), std::numeric_limits<quint64>::max());

    _zobrist_hash = 0;

    for (int i = 0; i < 61; ++i) {
        for (int j = 0; j < 4; ++j) {
            _zobrist_idx[i][j] = dis(gen);
        }
    }

}

quint64
HexdameGrid::zobristString(const Coord& c, const Piece& p)
{
    int i = _coordToIdx.value(c);
    int j;

    switch (p) {
        case BlackKing: j = 0; break;
        case BlackPawn: j = 1; break;
        case WhitePawn: j = 2; break;
        case WhiteKing: j = 3; break;
    }

    return _zobrist_idx[i][j];
}
