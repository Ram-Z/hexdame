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

#ifndef HEXDAMEGRID_H
#define HEXDAMEGRID_H

#include "commondefs.h"

#include <QVector>

using namespace Hexdame;

class HexdameGrid
{
public:
    HexdameGrid();
    HexdameGrid(const HexdameGrid &other);
    HexdameGrid &operator=(const HexdameGrid &other);
    bool operator==(const HexdameGrid &other) const;

    Piece at(quint8 idx) const;

    Piece at(const Coord &c) const { return at(_coordToIdx.value(c)); }
    void set(const Coord &c, Piece p);

    static QList<Coord> coords() { return _coordToIdx.keys(); }
    static bool contains(const Coord &c) { return _coordToIdx.contains(c); }

    const QHash<Coord, QMultiHash<Coord, Move>> &validMoves() const { return _validMoves; }
    const QMultiHash<Coord, Move> validMoves(Coord c) const { return _validMoves.value(c); }

    // convenience functions
    inline bool isWhite(quint8 idx) const { return _white.test(idx); }
    inline bool isBlack(quint8 idx) const { return _black.test(idx); }
    inline bool isEmpty(quint8 idx) const { return !(_white | _black).test(idx); }
    inline bool  isPawn(quint8 idx) const { return ((_white | _black) & ~_kings).test(idx); }
    inline bool  isKing(quint8 idx) const { return _kings.test(idx); }
    inline Color  color(quint8 idx) const
        { if (isWhite(idx)) return White;
          if (isBlack(idx)) return Black;
          return None; }

    inline bool isWhite(const Coord &c) const { return isWhite(_coordToIdx[c]); }
    inline bool isBlack(const Coord &c) const { return isBlack(_coordToIdx[c]); }
    inline bool isEmpty(const Coord &c) const { return isEmpty(_coordToIdx[c]); }
    inline bool  isPawn(const Coord &c) const { return  isPawn(_coordToIdx[c]); }
    inline bool  isKing(const Coord &c) const { return  isKing(_coordToIdx[c]); }
    inline Color  color(const Coord &c) const { return   color(_coordToIdx[c]); }

    void makeMove(const Move &move, bool partial = false);
    void makeMoveBit(const MoveBit &move);
    // does not check validity and calculates all valid moves, use for debug
    void move(const Coord &from, const Coord &to);

    Color winner() const;
    QHash<Coord, QMultiHash<Coord, Move>> computeValidMoves(Color col);
    QList<MoveBit> computeValidMoveBits(Color col) const;

    quint64 zobristHash() const { return _zobrist_hash; }

    static QHash<Coord, quint8> _coordToIdx;

private:
    enum Direction {
        NorthWest = 0,
        North,
        NorthEast,
        SouthEast,
        South,
        SouthWest
    };

    void dfs(const Coord &from, Move move = Move());
    void dfs(const quint8 &from, MoveBit move = MoveBit()) const;
    void kingPiece();

    static bool initialized;
    static const int SIZE = 9;
    static void zobristInit();
    static quint64 zobristString(quint8 idx, const Piece &p);
    static quint64 zobristString(const Coord &c, const Piece &p);
    static quint64 _zobrist_idx[61][4];
    static quint64 _zobrist_turn;
    quint64 _zobrist_hash;


    BitBoard _white = 0;
    BitBoard _black = 0;
    BitBoard _kings = 0;

    QHash<Coord, QMultiHash<Coord, Move>> _validMoves;
    mutable QList<MoveBit> _validMoveBits;
    mutable quint8 _maxTaken;

    static void moveInit();
    static BitBoard _neighbourMasks[61];
    static BitBoard _northMasks[61];
    static BitBoard _southMasks[61];
    static BitBoard _pawnJumpMasks[61][6];
    static BitBoard _kingJumpMasks[61][6];
};

#endif // HEXDAMEGRID_H
