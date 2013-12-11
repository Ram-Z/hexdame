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
quint64 HexdameGrid::_zobrist_turn;

BitBoard HexdameGrid::_neighbourMasks[61];
BitBoard HexdameGrid::_northMasks[61];
BitBoard HexdameGrid::_southMasks[61];
BitBoard HexdameGrid::_pawnJumpMasks[61][6];
BitBoard HexdameGrid::_kingJumpMasks[61][6];

bool HexdameGrid::initialized = false;

HexdameGrid::HexdameGrid()
{
    if (!initialized) {
        zobristInit();
        _coordToIdx.reserve(67);
    }
    _zobrist_hash = 0;
    quint8 idx = 0;
    for (quint8 x = 0; x < SIZE; ++x) {
        for (quint8 y = 0; y < SIZE; ++y) {
            static const int s = SIZE / 2;
            if (qAbs(x - y) <= s) {
                if (!initialized)
                    _coordToIdx[Coord(x,y)] = idx;

                if (x < s && y < s) {
                    _white.set(idx);
                    _zobrist_hash ^= _zobrist_idx[idx][2];
                } else if (x > s && y > s) {
                    _black.set(idx);
                    _zobrist_hash ^= _zobrist_idx[idx][1];
                }
                idx++;
            }
        }
    }

    if (!initialized) {
        _coordToIdx.squeeze();
        moveInit();
        initialized = true;
    }
}

HexdameGrid::HexdameGrid(const HexdameGrid &other)
    : _white(other._white)
    , _black(other._black)
    , _kings(other._kings)
    , _validMoves(other._validMoves)
    , _maxTaken(other._maxTaken)
    , _zobrist_hash(other._zobrist_hash)
{

}

HexdameGrid &
HexdameGrid::operator=(const HexdameGrid &other)
{
    if (this != &other) {
        _white = other._white;
        _black = other._black;
        _kings = other._kings;
        _validMoves = other._validMoves;
        _maxTaken = other._maxTaken;
        _zobrist_hash = other._zobrist_hash;
    }
    return *this;
}

bool
HexdameGrid::operator==(const HexdameGrid &other) const
{
    return _white == other._white
        && _black == other._black
        && _kings == other._kings;
}

Piece
HexdameGrid::at(quint8 idx) const
{
    if (_kings.test(idx))
        return _white.test(idx) ? WhiteKing : BlackKing;
    if (_white.test(idx)) return WhitePawn;
    if (_black.test(idx)) return BlackPawn;
    return Empty;
}

void
HexdameGrid::set(const Coord& c, Piece p)
{
    quint8 idx = _coordToIdx.value(c);

    _white.reset(idx);
    _black.reset(idx);
    _kings.reset(idx);

    switch (p) {
        case WhitePawn: _white.set(idx); break;
        case BlackPawn: _black.set(idx); break;
        case WhiteKing: _white.set(idx); _kings.set(idx); break;
        case BlackKing: _black.set(idx); _kings.set(idx); break;
    }
}

Color
HexdameGrid::winner() const
{
    //TODO check for draws
    if (_black.none())
        return White;
    if (_white.none())
        return Black;

    return None;
}

void
HexdameGrid::kingPiece()
{
    static const BitBoard white(0x1f82040400000000ULL);
    static const BitBoard black(0x000000000404083fULL);

    BitBoard mask = _black & black;
    if (mask.any()) {
        _kings |= mask;

//        // use this while I still pass a Coord
//        _zobrist_hash ^= zobristString(c, BlackPawn);
//        _zobrist_hash ^= zobristString(c, BlackKing);
//        return;

        // use this if for some reason I don't pass a Coord anymore
        quint8 idx = 0;
        // is this really faster?
        while (idx < 26 && !mask[idx]) ++idx;
        _zobrist_hash ^= _zobrist_idx[idx][0]; //blackKing
        _zobrist_hash ^= _zobrist_idx[idx][1]; //blackPawn
        return;
    }

    mask = _white & white;
    if (mask.any()) {
        _kings |= mask;

//        // use this while I still pass a Coord
//        _zobrist_hash ^= zobristString(c, WhitePawn);
//        _zobrist_hash ^= zobristString(c, WhiteKing);
//        return;

        // use this if for some reason I don't pass a Coord anymore
        quint8 idx = 34;
        // is this really faster?
        while (idx < 61 && !mask[idx]) ++idx;
        _zobrist_hash ^= _zobrist_idx[idx][2]; //whitePawn
        _zobrist_hash ^= _zobrist_idx[idx][3]; //whiteKing
        return;
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
        _zobrist_hash ^= zobristString(c, at(c));
        set(c, Empty);
    }

    if (partial) {
        _validMoves.clear();
        _maxTaken = 0;
        dfs(move.to());
    } else {
        kingPiece();
        _zobrist_hash ^= _zobrist_turn;
        if (color(move.to()) == White)
            computeValidMoves(Black);
        if (color(move.to()) == Black)
            computeValidMoves(White);
    }
}

void
HexdameGrid::makeMoveBit(const MoveBit &move)
{
    Q_ASSERT(move.path.count() == 2);
    Piece p;
    quint8 from, to;
    for (int i = 0; i < 61; ++i) {
        if (move.path[i]) {
             if (at(i) != Empty) {
                 from = i;
                 p = at(i);
             } else {
                 to = i;
             }
        }
        if (move.taken[i]) _zobrist_hash ^= zobristString(i, at(i));
    }
    _zobrist_hash ^= zobristString(from, p);
    _zobrist_hash ^= zobristString(to, p);

    if ((_white & move.path).any()) {
        _white ^= move.path;
        _black &= ~move.taken;
    } else {
        _black ^= move.path;
        _white &= ~move.taken;
    }
    if ((_kings & move.path).any()) {
        _kings ^= move.path;
    }
    _kings &= ~move.taken;

    kingPiece();
    _zobrist_hash ^= _zobrist_turn;
   // if (color(p) == White)
   //     computeValidMoves(Black);
   // if (color(p) == Black)
   //     computeValidMoves(White);
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
        _validMoves.reserve(2*_white.count());
    else if (col == Black)
        _validMoves.reserve(2*_black.count());

    // don't change the order  |<----------Whites moves---------->|<-------------Blacks moves------------->|
    const static QList<Coord> l{Coord(1,0), Coord(0,1), Coord(1,1), Coord(0,-1), Coord(-1,0), Coord(-1,-1)};
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

QList<MoveBit>
HexdameGrid::computeValidMoveBits(Color col) const
{
    _maxTaken = 0;
    _validMoveBits.clear();

    if (col == None) {
        QList<MoveBit> meh;
        meh.append(computeValidMoveBits(White));
        meh.append(computeValidMoveBits(Black));
        _validMoveBits = meh;
        return _validMoveBits;
    }

    for (int i = 0; i < 61; ++i) {
        if (color(i) != col) continue;
        dfs(i);
    }

    if (!_validMoveBits.empty()) return _validMoveBits;

    if (col == White)
        _validMoveBits.reserve(2*_white.count());
    else if (col == Black)
        _validMoveBits.reserve(2*_black.count());

    for (int from = 0; from < 61; ++from) {
        if (color(from) != col) continue;

        BitBoard dests;
        if (col == White) {
            dests = ~(_white | _black) & _northMasks[from];
        } else {
            dests = ~(_white | _black) & _southMasks[from];
        }

        for (int to = 0; to < 61; ++to) {
            if (!dests[to]) continue;
            MoveBit m;
            m.path.set(from);
            m.path.set(to);
            _validMoveBits << m;
        }
    }

    return _validMoveBits;
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

    const static QList<Coord> l{Coord(1,0), Coord(-1,0), Coord(0,1), Coord(0,-1), Coord(1,1), Coord(-1,-1)};
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
HexdameGrid::dfs(const quint8 &from, MoveBit move) const
{
    static Color col;
    static bool king;
    if (move.empty()) {
        move.path.set(from);
        col = color(from);
        king = isKing(from);
    }

    const BitBoard &cur = col == White ? _white : _black;
    const BitBoard &opp = col == White ? _black : _white;


    if (!king) {
        // may not jump
        if ((opp & _neighbourMasks[from]).none()) return;

        for (int d = 0; d < 6; ++d) {
            if ((opp & _pawnJumpMasks[from][d]).none()) continue; // can't jump in direction d
            if ((move.taken & _pawnJumpMasks[from][d]).any()) continue; // already took piece
            size_t to = 0;
            {
                BitBoard tmp = _pawnJumpMasks[from][d] & ~_neighbourMasks[from];
                while (!(tmp).test(to)) ++to;
            }
            if ((cur | opp).test(to)) continue; // dest is not free

            MoveBit newMove(move);
            // reset from only if we're in a multi-jump
            if (newMove.path.count() == 2)
                newMove.path.reset(from);
            newMove.path.set(to);
            newMove.taken |= _pawnJumpMasks[from][d] & _neighbourMasks[from];
            Q_ASSERT(newMove.path.count() == 2);

            size_t s = newMove.taken.count();
            if (s >= _maxTaken) {
                if (s > _maxTaken) {
                    _validMoveBits.clear();
                    _maxTaken = s;
                }
                if(!_validMoveBits.contains(newMove))
                    _validMoveBits << newMove;
            }

            dfs(to, newMove);
        }
    } else {
        for (int d = 0; d < 6; ++d) {
            if ((opp & _kingJumpMasks[from][d]).none()) continue; // can't jump in direction d

            BitBoard dests;
            quint8 takenIdx = -1;

            quint8 t_min, t_max, t_inc;
            if (d < 3) { // going North
                t_min = 0;
                t_max = 61;
                t_inc = 1;
            } else { // going South
                t_min = 60;
                t_max = -1;
                t_inc = -1;
            }

            bool canJump = false;
            quint8 t = t_min;
            // I don't like this loop
            while (t != t_max) {
                if ((cur & _kingJumpMasks[from][d]).test(t)) break;

                if (canJump && _kingJumpMasks[from][d].test(t)) {
                    if (isEmpty(t))
                        dests.set(t);
                    else
                        break;
                }

                if ((opp & _kingJumpMasks[from][d]).test(t)) {
                    canJump = true;
                    takenIdx = t;
                }

                t+=t_inc;
            }

            if (takenIdx > 60) continue; // we're trying to jump out of the board

            if (move.taken.test(takenIdx)) continue; // already took piece
            size_t to = 0;
            for (int to = 0; to < 61; ++to) {
                if (!dests.test(to)) continue;

                MoveBit newMove(move);
                // reset from only if we're in a multi-jump
                if (newMove.path.count() == 2)
                    newMove.path.reset(from);
                newMove.path.set(to);
                newMove.taken.set(takenIdx);
                Q_ASSERT(newMove.path.count() == 2);

                size_t s = newMove.taken.count();
                if (s >= _maxTaken) {
                    if (s > _maxTaken) {
                        _validMoveBits.clear();
                        _maxTaken = s;
                    }
                    if(!_validMoveBits.contains(newMove))
                        _validMoveBits << newMove;
                }

                dfs(to, newMove);
            }
        }
    }
}

void
HexdameGrid::zobristInit()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<quint64> dis(std::numeric_limits<quint64>::min(), std::numeric_limits<quint64>::max());

    for (int i = 0; i < 61; ++i) {
        for (int j = 0; j < 4; ++j) {
            _zobrist_idx[i][j] = dis(gen);
        }
    }

    _zobrist_turn = dis(gen);
}

quint64
HexdameGrid::zobristString(quint8 idx, const Piece& p)
{
    Q_ASSERT(p != Empty);
    int j;

    switch (p) {
        case BlackKing: j = 0; break;
        case BlackPawn: j = 1; break;
        case WhitePawn: j = 2; break;
        case WhiteKing: j = 3; break;
    }

    return _zobrist_idx[idx][j];
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


void print(BitBoard grid) {
    qDebug() << QString::number(grid.to_ullong(), 2).rightJustified(64, '0');
    QString str;
    for (int i = 63; i >= 0; --i) {
        if (i % 10 == 0)
            str += QString::number(i/10);
        else
            str += QString::number(i%10);
    }
    qDebug() << str;
}

void
HexdameGrid::moveInit()
{
    // don't change order:     |<-NorthWest->|<---North--->|<-NorthEast->|<-SouthEast->|<---South--->|<-SouthWest->|
    const static QList<Coord> l{Coord( 1, 0), Coord( 1, 1), Coord( 0, 1), Coord( 0,-1), Coord(-1,-1), Coord(-1, 0)};
    foreach (Coord d, _coordToIdx.keys()) {
        quint8 idx = _coordToIdx.value(d);
        qDebug() << "idx: " << idx;
        for (int c = 0; c < l.size(); ++c) {
            Coord one = d + 1*l.at(c);
            if (!contains(one)) continue; // not on the grid

            if (c < 3) { // going north
                _northMasks[idx].set(_coordToIdx.value(one));
            } else { // going south
                _southMasks[idx].set(_coordToIdx.value(one));
            }

            Coord two = d + 2*l.at(c);
            if (!contains(two)) continue; // not on the grid

            _pawnJumpMasks[idx][c].set(_coordToIdx.value(one));
            _pawnJumpMasks[idx][c].set(_coordToIdx.value(two));

            _neighbourMasks[idx].set(_coordToIdx.value(one));
            qDebug() << "c: " << c;
            print(_pawnJumpMasks[idx][c]);

            _kingJumpMasks[idx][c].set(_coordToIdx.value(one));
            _kingJumpMasks[idx][c].set(_coordToIdx.value(two));
            for (int i = 3; i < 9; ++i) {
                Coord next = d + i*l.at(c);
                if (!contains(next)) break; // not on the grid

                _kingJumpMasks[idx][c].set(_coordToIdx.value(next));
            }
        }
        print(_neighbourMasks[idx]);
    }
}
