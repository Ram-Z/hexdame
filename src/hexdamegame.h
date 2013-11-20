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

#include "hexdamegrid.h"

#include <QtDebug> // needed for Q_ASSERT

class AbstractPlayer;

using namespace Hexdame;

class HexdameGame : public QObject
{
    Q_OBJECT

public:
    HexdameGame(QObject *parent);

    Color currentColor() const { return _currentColor; }
    bool currentPlayerIsHuman() const;

    inline bool debug() const { return _debug; }

public slots:
    void makeMove(const Coord &oldCoord, const Coord &newCoord);
    void makeMove(const Move &move, bool partial = false);
    void makePartialMove(const Coord &oldCoord, const Coord &newCoord);

    void startNextTurn();

    void setBlackPlayer(AbstractPlayer *player);
    void setWhitePlayer(AbstractPlayer *player);

    void setDebugMode(bool debug) { _debug = debug; }
    void debugRightClick(Coord c);
    const HexdameGrid &grid() const { return _grid; }

signals:
    void boardChanged();
    //void gameOver();
    void moveFinished();
    void playerMoved();
    void currentHumanPlayer(Color);

private:
    AbstractPlayer *currentPlayer() const { return _currentColor == White ? _white : _black; }

    HexdameGrid _grid;

    AbstractPlayer *_white = 0;
    AbstractPlayer *_black = 0;
    Color _currentColor = None;

    bool _debug = false;
};

#endif
