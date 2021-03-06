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

#include "abstractplayer.h"
#include <QtDebug>

AbstractPlayer::AbstractPlayer(PlayerType type, HexdameGame *game, Color color)
    : QThread((QObject *) game)
    , _game(game)
    , _color(color)
    , _type(type)
{
}

AbstractPlayer::~AbstractPlayer()
{
    mutex.lock();
    abort = true;
    mutex.unlock();

    wait();
}
