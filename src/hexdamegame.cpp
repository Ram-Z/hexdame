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
#include "player/heuristic.h"

#include <QtGlobal>

HexdameGame::HexdameGame(QObject *parent)
    : QObject(parent)
{
    connect(this, SIGNAL(playerMoved()), SLOT(startNextTurn()));
    qRegisterMetaType<Move>("Move");
    qRegisterMetaType<MoveBit>("MoveBit");
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
        case BlackKing: _grid.set(c, BlackPawn); break;
        case BlackPawn: _grid.set(c, Empty    ); break;
        case Empty:     _grid.set(c, WhitePawn); break;
        case WhitePawn: _grid.set(c, WhiteKing); break;
        case WhiteKing: _grid.set(c, BlackKing); break;
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

    Move move = _grid.computeValidMoves(currentColor()).value(from).value(to);
    makeMove(move);
}

void
HexdameGame::makeMove(const MoveBit &move)
{
    if (move.empty()) return;

    _grid.makeMoveBit(move);
    emit boardChanged();
    emit playerMoved();
}

void
HexdameGame::makeMove(const Move &move, bool partial)
{
    if (move.empty()) return;

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

    QMultiHash<Coord, Move> tmpHash = _grid.computeValidMoves(currentColor()).value(from);
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
        disconnect(_black, SIGNAL(moveBit(MoveBit)), this, SLOT(makeMove(MoveBit)));
        _black->deleteLater();
    }
    _black = player;
    if (_black) {
        connect(_black, SIGNAL(move(Move)), this, SLOT(makeMove(Move)));
        connect(_black, SIGNAL(moveBit(MoveBit)), this, SLOT(makeMove(MoveBit)));
    }
    if (currentColor() == Black)
        _black->start();
}

void
HexdameGame::setWhitePlayer(AbstractPlayer *player)
{
    if (_white) {
        disconnect(_white, SIGNAL(move(Move)), this, SLOT(makeMove(Move)));
        disconnect(_white, SIGNAL(moveBit(MoveBit)), this, SLOT(makeMove(MoveBit)));
        _white->deleteLater();
    }
    _white = player;
    if (_white) {
        connect(_white, SIGNAL(move(Move)), this, SLOT(makeMove(Move)));
        connect(_white, SIGNAL(moveBit(MoveBit)), this, SLOT(makeMove(MoveBit)));
    }
    if (currentColor() == White)
        _white->start();
}

void
HexdameGame::startNextTurn()
{
    if (_grid.winner() == None) {
        if (_currentColor == White) {
            _currentColor = Black;
        } else {
            _currentColor = White;
        }

        currentPlayer()->start();
    }
}

QDebug
operator<<(QDebug dbg, const Coord &coord)
{
    dbg.nospace() << "Coord(" << coord.x << "," << coord.y << ")";
    return dbg.maybeSpace();
}

QDebug
operator<<(QDebug dbg, const Move &move)
{
    dbg.nospace() << "Path: " << move.path;
    dbg.nospace() << "Take: " << move.taken;
    return dbg.maybeSpace();
}
