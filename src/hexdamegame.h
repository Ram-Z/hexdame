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

#ifndef HEXDAMEGAME_H
#define HEXDAMEGAME_H

#include "commondefs.h"

#include <QtDebug> // needed for Q_ASSERT

class AbstractPlayer;

using namespace Hexdame;

class HexdameGame : public QObject
{
    Q_OBJECT

public:
    HexdameGame(QObject *parent);

    inline const int size() const { return _size; }

    const Piece at(int x, int y) const { return at(Coord {x, y}); }
    const Piece at(const Coord &c) const { Q_ASSERT(_grid.contains(c)); return _grid.value(c); }
    Piece &rat(int x, int y) { return _grid[Coord {x, y}]; }
    Piece &rat(const Coord &c) { return _grid[c]; }

    QList<Coord> coords() const { return _grid.keys(); }

    bool canJump(int x, int y) const;
    bool canJump(const Coord &c) const { return canJump(c.x, c.y); }

    QList<Move> validMoves(const Coord &c) const { return _validMoves.value(c); }

    QHash< Coord, QList< Move > > computeValidMoves(Color col);

    Color currentColor() const { return _currentColor; }
    bool currentPlayerIsHuman() const;

    bool gameOver() const;

    // convenience functions
    inline bool isWhite(const Coord &c) const { return Hexdame::isWhite(at(c)); }
    inline bool isBlack(const Coord &c) const { return Hexdame::isBlack(at(c)); }
    inline bool isEmpty(const Coord &c) const { return Hexdame::isEmpty(at(c)); }
    inline bool  isPawn(const Coord &c) const { return Hexdame::isPawn(at(c)); }
    inline bool  isKing(const Coord &c) const { return Hexdame::isKing(at(c)); }

    inline Color color(const Coord &c) const { return Hexdame::color(at(c)); }

    inline bool debug() const { return _debug; }

public slots:
    void makeMove(const Coord &oldCoord, const Coord &newCoord);
    void makeMove(const Move &move);

    void startNextTurn();

    void setBlackPlayer(AbstractPlayer *player);
    void setWhitePlayer(AbstractPlayer *player);

    void setDebugMode(bool debug);
    void debugRightClick(Coord c);

signals:
    void boardChanged();
    //void gameOver();
    void moveFinished();
    void playerMoved();
    void currentHumanPlayer(Color);

private:
    const QList<Move> possibleMoves(int x, int y) const { return possibleMoves(Coord {x, y}); }
    const QList<Move> possibleMoves(const Coord &c) const;
    QList<Move> dfs(const Coord &c, Move move = Move { -1, -1}) const;

    void kingPiece(Coord c);

    AbstractPlayer *currentPlayer() const { return _currentColor == White ? _white : _black; }

    int _size = 0;
    QHash<Coord, Piece> _grid;
    int cntWhite = 0;
    int cntBlack = 0;

    QHash<Coord, QList<Move>> _validMoves;

    AbstractPlayer *_white = 0;
    AbstractPlayer *_black = 0;
    Color _currentColor;

    bool _debug = false;
};

#endif
