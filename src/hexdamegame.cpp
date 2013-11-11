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

#include "humanplayer.h"
#include "randomplayer.h"
#include <QDate>
#include <qcoreapplication.h>

HexdameGame::HexdameGame()
    : QObject()
    , _size(9)
    , _white(new RandomPlayer(this, White))
    , _black(new RandomPlayer(this, Black))
    , _currentPlayer(White)
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
    //FIXME remove this at some point
    QHash<Coord, QList<Move>> validMoves;
    if (col == None) {
        validMoves.unite(computeValidMoves(White));
        validMoves.unite(computeValidMoves(Black));
        _validMoves = validMoves;
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

bool HexdameGame::gameOver() const
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
        computeValidMoves();
        return;
    }

    makeMove(move);

    computeValidMoves();
}

void
HexdameGame::makeMove(const Move &move)
{
    rat(move.to()) = at(move.from);
    rat(move.from) = Empty;

    foreach (Coord c, move.taken) {
        rat(c) = Empty;
        if (_currentPlayer == White) {
            --cntBlack;
        } else if (_currentPlayer == Black) {
            --cntWhite;
        }
    }

    kingPiece(move.to());

    emit boardChanged();
}

QList<Coord>
HexdameGame::neighbours(const Coord &c) const
{
    static QHash<Coord, QList<Coord>> cache;
    if (cache.contains(c)) return cache.value(c);

    QList<Coord> list, retval;
    list << Coord {1, 0} << Coord {1, 1} << Coord {0, 1}
         << Coord { -1, 0} << Coord { -1, -1} << Coord {0, -1};

    foreach (Coord dc, list) {
        if (_grid.contains(c + dc))
            retval << c + dc;
    }
    cache[c] = retval;

    return retval;
}

const QList<Move>
HexdameGame::possibleMoves(const Coord &c) const
{
    if (isEmpty(c)) return QList<Move> {};

    QList<Move> moves = dfs(c);

    if (isKing(c)) {
        foreach (Move tmp, moves) {
            qDebug() << tmp.path;
        }
    }

    if (!moves.empty()) return moves;

    if (isPawn(c)) {
        QList<Coord> tos;
        if (isBlack(c)) {
            tos << Coord { -1, 0} << Coord { -1, -1} << Coord {0, -1};
        } else if (isWhite(c)) {
            tos << Coord {1, 0} << Coord {1, 1} << Coord {0, 1};
        }

        foreach (Coord n, tos) {
            Coord to = c + n;
            if (_grid.contains(to) && isEmpty(to)) {
                Move m {c};
                m.path << to;

                moves << m;
            }
        }
    } else if (isKing(c)) {
        const static QList<Coord> l{Coord{1,0}, Coord{-1,0}, Coord{0,1}, Coord{0,-1}, Coord{1,1}, Coord{-1,-1}};
        foreach (Coord lv, l) {
            for (int i = 1; i < 9; ++i) {
                Coord n = c + i*lv;

                if (!_grid.contains(n)) break;
                if (!isEmpty(n)) break;

                Move m{c};
                m.path << n;
                moves << m;
            }
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

    QList<Move> moves;
    if (king) {
        const static QList<Coord> l{Coord{1,0}, Coord{-1,0}, Coord{0,1}, Coord{0,-1}, Coord{1,1}, Coord{-1,-1}};
        foreach (Coord lv, l) {
            bool jump = false;
            Move newMove, tmpMove;
            for (int i = 1; i < 9; i++) {
                Coord n = c + i*lv;
                // not on the grid
                if (!_grid.contains(n)) break;
                // same colour but not same piece
                if (move.from != n && col == color(n)) break;
                // non-empty cell after jumping
                if (jump && move.from != n && !isEmpty(n)) break;
                // already jumped
                if (move.taken.contains(n)) break;

                // not yet jumping but different colour
                if (!jump && col == -color(n)) {
                    jump = true;
                    tmpMove = move;
                    tmpMove.taken << n;
                    continue;
                }
                if (jump) {
                    newMove = tmpMove;
                    newMove.path << n;
                    moves << newMove;
                }
            }
        }
    } else {
        foreach (Coord n, neighbours(c)) {
            // different colors
            if (col == -color(n) && !move.taken.contains(n)) {
                Coord j = c + (n - c) * 2;
                // skip if !(onGrid && (empty || startingPos))
                if (!_grid.contains(j) || !isEmpty(j) && move.from != j) continue;

                Move newMove(move);

                newMove.path << j;
                newMove.taken << n;

                moves << newMove;
            }
        }
    }

    foreach (Move newMove, moves) {
        if (best_moves.empty() || newMove.path.size() == best_moves.at(0).path.size()) {
            best_moves << newMove;
        } else if (newMove.path.size() > best_moves.at(0).path.size()) {
            best_moves.clear();
            best_moves << newMove;
        }

        dfs(newMove.to(), newMove);
    }
    return best_moves;
}

void
HexdameGame::startNextTurn()
{
    while (!gameOver()) {
        if (_currentPlayer == White) {
            _currentPlayer = Black;
        } else {
            _currentPlayer = White;
        }

//        Move move = currentPlayer()->play();
//        makeMove(move);

        // wait 2 seconds before next move
        QTime wait = QTime::currentTime().addSecs(2);
        while (QTime::currentTime() < wait)
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

QDebug
operator<<(QDebug dbg, const Coord &coord)
{
    dbg.nospace() << "Coord(" << coord.x << "," << coord.y << ")";
    return dbg.space();
}

