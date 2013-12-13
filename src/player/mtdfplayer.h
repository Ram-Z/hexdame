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

#ifndef MTDFPLAYER_H
#define MTDFPLAYER_H

#define FLAG_EXACT 0
#define FLAG_LOWER 1
#define FLAG_UPPER 2

#include "player/abstractplayer.h"
#include <QCache>

class HexdameGrid;
class AbstractHeuristic;
class QTime;

class MTDfPlayer : public AbstractPlayer
{
    Q_OBJECT

public:
    MTDfPlayer(HexdameGame *game, Color color, AbstractHeuristic *heuristic);
    virtual ~MTDfPlayer();

    virtual void play();

protected:
    void run();

private slots:
    void timesUp() { abort = true; }

private:
    QList<MoveBit> iterativeDeepening(const HexdameGrid& root, QTime tic);
    int mtdf(const HexdameGrid& node, int f, int depth);
    int negamax(const HexdameGrid& node, int depth, int alpha, int beta, int color);

    struct TTentry {
        quint64 zobrist_key;
        quint8 depth;
        quint8 flag;
        qint16 value;
        MoveBit bestMove;
    };
    QCache<quint64, TTentry> ttable;

    quint8 _depth = 0;
    QTime *startTime;

    AbstractHeuristic *_heuristic;
    int nodeCnt;
};

#endif // MTDFPLAYER_H
