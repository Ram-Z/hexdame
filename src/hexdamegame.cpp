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
    , _size(9)
    , _currentColor(Black) // it'll swith on the first turn
{
    _grid.reserve(_size * _size);

    for (int x = 0; x < _size; ++x) {
        for (int y = 0; y < _size; ++y) {
            int s = size() / 2;
            if (qAbs(x - y) <= s) {
                _grid[Coord {x, y}] = Empty;
            }
            if (x < s && y < s) {
                _grid[Coord {x, y}] = WhitePawn;
                ++cntWhite;
            } else if (x > s && y > s) {
                _grid[Coord {x, y}] = BlackPawn;
                ++cntBlack;
            }
        }
    }

    setWhitePlayer(new HumanPlayer(this, White));
    setBlackPlayer(new HumanPlayer(this, Black));

    connect(this, SIGNAL(playerMoved()), SLOT(startNextTurn()));
}

bool
HexdameGame::canJump(int x, int y) const
{
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            Piece dc = at(x + dx, y + dx);
        }
    }
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

    switch (at(c)) {
        case BlackKing: rat(c) = BlackPawn; break;
        case BlackPawn: rat(c) = Empty    ; break;
        case Empty:     rat(c) = WhitePawn; break;
        case WhitePawn: rat(c) = WhiteKing; break;
        case WhiteKing: rat(c) = BlackKing; break;
    }
    // this will compute all moves
    setDebugMode(true);
    emit boardChanged();
}

bool
HexdameGame::gameOver() const
{
    //TODO check for draws
    return !cntBlack || !cntWhite;
}

void
HexdameGame::kingPiece(Coord c)
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
HexdameGame::makeMove(const Coord &oldCoord, const Coord &newCoord)
{
    if (!isEmpty(newCoord)) return;

    if (debug()) {
        if (newCoord == oldCoord) return;

        rat(newCoord) = at(oldCoord);
        rat(oldCoord) = Empty;

        emit boardChanged();

        // probably not the right place but who cares
        setDebugMode(true);

        return;
    }

    //FIXME use partial moves
    Move move;
    foreach (move, _validMoves.value(oldCoord).values(newCoord)) {
        if (move.to() == newCoord) {
            break;
        }
    }

    makeMove(move);
}

void
HexdameGame::makeMove(const Move &move)
{
    rat(move.to()) = at(move.from());
    rat(move.from()) = Empty;

    foreach (Coord c, move.taken) {
        rat(c) = Empty;
        if (_currentColor == White) {
            --cntBlack;
        } else if (_currentColor == Black) {
            --cntWhite;
        }
    }

    kingPiece(move.to());

    emit boardChanged();
    emit playerMoved();
}


void
HexdameGame::setDebugMode(bool debug)
{
    _debug = debug;
    if (debug) {
        computeValidMoves(None);
    } else {
        computeValidMoves(currentColor());
    }
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
    if (!gameOver()) {
        if (_currentColor == White) {
            _currentColor = Black;
        } else {
            _currentColor = White;
        }

        computeValidMoves(_currentColor);

        currentPlayer()->play();
    }
}

QDebug
operator<<(QDebug dbg, const Coord &coord)
{
    dbg.nospace() << "Coord(" << coord.x << "," << coord.y << ")";
    return dbg.space();
}

QHash<Coord, QMultiHash<Coord, Move>>
HexdameGame::computeValidMoves(Color col)
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
HexdameGame::dfs(const Coord &from, Move move)
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
                        foreach (Move oldMove, _validMoves.value(from)) {
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
