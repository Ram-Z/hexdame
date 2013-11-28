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

#ifndef ABSTRACTPLAYER_H
#define ABSTRACTPLAYER_H

#include "commondefs.h"

#include <QThread>
#include <QMutex>

class HexdameGame;
class AbstractPlayer : public QThread
{
    Q_OBJECT

public:
    enum PlayerType {
        Human,
        AI
    };

    AbstractPlayer(PlayerType type, HexdameGame *game, Color color);
    virtual ~AbstractPlayer();

    PlayerType type() { return _type; }

public slots:
    virtual void play() = 0;

signals:
    void move(const Move &);

protected:
    virtual void run() = 0;

    QMutex mutex;
    bool abort = false;

    const PlayerType _type;

    HexdameGame *_game;
    const Color _color;
};

#endif // ABSTRACTPLAYER_H
