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

#include "randomplayer.h"
#include "hexdamegame.h"

#include <ctime>

RandomPlayer::RandomPlayer(HexdameGame *game, Color color)
    : AbstractPlayer(game, color)
{
}

Move RandomPlayer::play()
{
    QHash<Coord, QList<Move>> moves = _game->computeValidMoves(_color);

    qsrand(std::time(0));
    int rand = qrand() % moves.size();
    Coord randCoord = moves.keys().at(rand);
    rand = qrand() % moves.value(randCoord).size();
    Move randMove = moves.value(randCoord).at(rand);
    qDebug() << randMove.from << randMove.path;

    emit move(randMove);

    return randMove;
}
