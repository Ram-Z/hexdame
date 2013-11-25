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

    Piece at(const Coord &c) const;
    void set(const Coord &c, Piece p);

    QList<Coord> coords() const { return _coordToIdx.keys(); }
    bool contains(const Coord &c) const { return _coordToIdx.contains(c); }

    const QHash<Coord, QMultiHash<Coord, Move>> &validMoves() const { return _validMoves; }
    const QMultiHash<Coord, Move> validMoves(Coord c) const { return _validMoves.value(c); }

    // convenience functions
    inline bool isWhite(const Coord &c) const
        { return (_whitePawns | _whiteKings) & ((quint64) 1 << _coordToIdx[c]); }
    inline bool isBlack(const Coord &c) const
        { return (_blackPawns | _blackKings) & ((quint64) 1 << _coordToIdx[c]); }
    inline bool isEmpty(const Coord &c) const
        { return !((_whitePawns | _whiteKings | _blackPawns | _blackKings) & ((quint64) 1 << _coordToIdx[c])); }
    inline bool  isPawn(const Coord &c) const
        { return (_whitePawns | _blackPawns) & ((quint64) 1 << _coordToIdx[c]); }
    inline bool  isKing(const Coord &c) const
        { return (_whiteKings | _blackKings) & ((quint64) 1 << _coordToIdx[c]); }
    inline Color  color(const Coord &c) const
        { return Hexdame::color(at(c)); }

    void makeMove(const Move &move, bool partial = false);
    // does not check validity and calculates all valid moves, use for debug
    void move(const Coord &from, const Coord &to);

    Color winner() const;
    QHash<Coord, QMultiHash<Coord, Move>> computeValidMoves(Color col);

    quint64 zobristHash() const { return _zobrist_hash; }

private:
    void dfs(const Coord &from, Move move = Move());
    void kingPiece(Coord c);

    void zobristInit();
    static quint64 zobristString(const Coord &c, const Piece &p);
    static quint64 _zobrist_idx[61][4];
    quint64 _zobrist_hash;

    static QHash<Coord, quint8> _coordToIdx;

    quint64 _whitePawns = 0;
    quint64 _whiteKings = 0;
    quint64 _blackPawns = 0;
    quint64 _blackKings = 0;

    QHash<Coord, QMultiHash<Coord, Move>> _validMoves;
    quint8 _maxTaken;

    quint8 _cntWhite = 0;
    quint8 _cntBlack = 0;
};

#endif // HEXDAMEGRID_H
