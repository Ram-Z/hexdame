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

#include "player.h"

#include <QtGlobal>

HexdameGame::HexdameGame(QObject *parent)
    : QObject(parent)
{
    setWhitePlayer(new HumanPlayer(this, White));
    setBlackPlayer(new HumanPlayer(this, Black));

    connect(this, SIGNAL(playerMoved()), SLOT(startNextTurn()));
}

bool
HexdameGame::currentPlayerIsHuman() const
{
    return currentPlayer()->type() == AbstractPlayer::Human;
}

void
HexdameGame::debugRightClick(Coord c)
{
    if (!debug()) return;

    switch (_grid.at(c)) {
        case BlackKing: _grid[c] = BlackPawn; break;
        case BlackPawn: _grid[c] = Empty    ; break;
        case Empty:     _grid[c] = WhitePawn; break;
        case WhitePawn: _grid[c] = WhiteKing; break;
        case WhiteKing: _grid[c] = BlackKing; break;
    }
    // this will compute all moves
    setDebugMode(true);
    emit boardChanged();
}

void
HexdameGame::makeMove(const Coord &from, const Coord &to)
{
    if (!_grid.isEmpty(to)) return;

    if (debug()) {
        _grid.move(from, to);

        emit boardChanged();

        return;
    }

    Move move = _grid.validMoves(from).value(to);
    makeMove(move);
}

void
HexdameGame::makeMove(const Move &move, bool partial)
{
    _grid.makeMove(move, partial);
    emit boardChanged();
    if (!partial)
        emit playerMoved();
}


void
HexdameGame::makePartialMove(const Coord &from, const Coord &to)
{
    if (!_grid.isEmpty(to)) return;

    if (debug()) {
        _grid.move(from, to);

        emit boardChanged();

        return;
    }

    QMultiHash<Coord, Move> tmpHash = _grid.validMoves(from);
    Move m;
    foreach (Move move, tmpHash.values()) {
        // not a partial moves
        if (move.to() == to) continue;

        m = Move();

        m.path << move.path.takeFirst();

        while (!move.path.empty() && move.path.first() != to) {
            m.path << move.path.takeFirst();
            m.taken << move.taken.takeFirst();
        }

        if (!move.path.empty()) {
            m.path << move.path.first();
            m.taken << move.taken.first();
            break;
        }
    }

    makeMove(m, true);
}

void
HexdameGame::setBlackPlayer(AbstractPlayer *player)
{
    if (_black) {
        disconnect(_black, SIGNAL(move(Move)), this, SLOT(makeMove(Move)));
        delete _black;
    }
    _black = player;
    connect(_black, SIGNAL(move(Move)), this, SLOT(makeMove(Move)));
    if (currentColor() == Black)
        _black->play();
}

void
HexdameGame::setWhitePlayer(AbstractPlayer *player)
{
    if (_white) {
        disconnect(_white, SIGNAL(move(Move)), this, SLOT(makeMove(Move)));
        delete _white;
    }
    _white = player;
    connect(_white, SIGNAL(move(Move)), this, SLOT(makeMove(Move)));
    if (currentColor() == White)
        _white->play();
}

void
HexdameGame::startNextTurn()
{
    if (!_grid.gameOver()) {
        if (_currentColor == White) {
            _currentColor = Black;
        } else {
            _currentColor = White;
        }

        currentPlayer()->play();
    }
}

QDebug
operator<<(QDebug dbg, const Coord &coord)
{
    dbg.nospace() << "Coord(" << coord.x << "," << coord.y << ")";
    return dbg.space();
}
