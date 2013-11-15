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

QHash<Coord, QList<Move>>
HexdameGame::computeValidMoves(Color col)
{
    QHash<Coord, QList<Move>> validMoves;
    if (col == None) {
        return validMoves;
    }

    int maxMoves = 0;

    foreach (Coord from, coords()) {
        if (color(from) != col) continue;

        //FIXME
        if (true || isPawn(from)) {
            QList<Move> moves = possibleMoves(from);
            if (moves.empty()) continue;

            if (moves.at(0).taken.size() > maxMoves) {
                maxMoves = moves.at(0).taken.size();
                validMoves.clear();
                validMoves[from] = moves;
            } else if (moves.at(0).taken.size() == maxMoves) {
                validMoves[from] = moves;
            }
        }
    }

    _validMoves = validMoves;

#if 0
    foreach (QList<Move> vl, validMoves) {
        QHash<QPair<Coord,Coord>, Move> arf;
        foreach (Move v, vl) {
            QPair<Coord,Coord> meh = qMakePair(v.from,v.to());
            qSort(v.taken.begin(), v.taken.end());
            if (!arf.contains(meh)) {
                arf[meh] = v;
                qDebug() << v.from << v.taken << v.to();
            } else {
                if (!qEqual(arf.value(meh).taken.begin(),arf.value(meh).taken.end(),
                           v.taken.begin())) {
                    qDebug() << "FUCK";
                    qDebug() << v.from << v.path << v.to();
                    qDebug() << arf.value(meh).from << arf.value(meh).path << arf.value(meh).to();
                }
            }
        }
    }
#endif

    return validMoves;
}

bool
HexdameGame::currentPlayerIsHuman() const
{
    return currentPlayer()->type() == AbstractPlayer::Human;
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
    if (oldCoord == newCoord) return;
    if (!isEmpty(newCoord)) return;

    Move move;
    bool valid = false;
    foreach (move, _validMoves.value(oldCoord)) {
        if (move.to() == newCoord) {
            valid = true;
            break;
        }
    }

    //FIXME
    if (!valid) {
        rat(newCoord) = at(oldCoord);
        rat(oldCoord) = Empty;

        emit boardChanged();
        return;
    }

    makeMove(move);
}

void
HexdameGame::makeMove(const Move &move)
{
    rat(move.to()) = at(move.from);
    rat(move.from) = Empty;

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

const QList<Move>
HexdameGame::possibleMoves(const Coord &from) const
{
    if (isEmpty(from)) return QList<Move> {};

    QList<Move> moves = dfs(from);

    if (!moves.empty()) return moves;

    // don't change the order  |<----------Whites moves---------->|<-------------Blacks moves------------->|
    const static QList<Coord> l{Coord{1,0}, Coord{0,1}, Coord{1,1}, Coord{0,-1}, Coord{-1,0}, Coord{-1,-1}};
    for (int i = 0; i < l.size(); ++i) {
        if (i <  3 && isPawn(from) && isBlack(from)) i = 3; // jump to Blacks moves
        if (i >= 3 && isPawn(from) && isWhite(from)) break; // ignore the rest

        for (int j = 1; j < 9; ++j) {
            Coord to = from + j*l.at(i);

            if (!_grid.contains(to)) break;
            if (!isEmpty(to)) break;

            // create Move and add to list
            Move m{from};
            m.path << to;
            moves << m;

            if (isPawn(from)) break; // Pawns can't jump further than 1
        }
    }

    return moves;
}

QList<Move>
HexdameGame::dfs(const Coord &c, Move move) const
{
    static QList<Move> best_moves;
    static Color col;
    static bool king;
    if (move.from == Coord { -1, -1}) {
        best_moves.clear();
        move.from = c;
        col = color(c);
        king = isKing(c);
    }

    const static QList<Coord> l{Coord{1,0}, Coord{-1,0}, Coord{0,1}, Coord{0,-1}, Coord{1,1}, Coord{-1,-1}};
    foreach (Coord lv, l) {
        for (int i = 1; i < 9; ++i) {
            Coord n = c + i*lv;

            if (!_grid.contains(n)) break;                // not on the grid
            if (move.from != n && col == color(n)) break; // same colour but not starting position
            if (move.taken.contains(n)) break;            // already took piece

            if (col == -color(n)) {
                while (++i < 9) {
                    Coord to = c + i*lv;
                    if (!_grid.contains(to) || (move.from != to && !isEmpty(to))) { i = 9; break; } // non-empty cell after jumping

                    // create a new Move
                    Move newMove(move);
                    newMove.taken << n;
                    newMove.path << to;

                    // update/reset best_moves list
                    if (best_moves.empty()) {
                        best_moves << newMove;
                    } else if (newMove.path.size() == best_moves.at(0).path.size()) {
                        bool dup = false;
                        foreach (Move oldMove, best_moves) {
                            if (dup = QSet<Coord>::fromList(oldMove.taken) == QSet<Coord>::fromList(newMove.taken)) {
                                // moves are equivalent
                                break;
                            }
                        }
                        if (!dup) best_moves << newMove;
                    } else if (newMove.path.size() > best_moves.at(0).path.size()) {
                        best_moves.clear();
                        best_moves << newMove;
                    }

                    // recursive call
                    dfs(to, newMove);

                    if (!king) break; // Pawns can't jump further than 1
                }
            }
            if (!king) break; // Pawns can't jump further than 1
        }
    }

    return best_moves;
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
