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

#ifndef BOARD_H
#define BOARD_H

#include "hexgrid.h"

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>

class Board : public QGraphicsView
{
    Q_OBJECT

    static const quint8 radius = 50;

    enum Type {
        HexItem = QGraphicsItem::UserType + 1,
        PieceItem
    };

    class Hex : public QGraphicsPolygonItem
    {
    public:
        enum { Type = UserType + 1 };
        explicit Hex(HexGrid::Coord c, QGraphicsItem *parent = 0);
        inline const HexGrid::Coord &coord() const { return _coord; }

        int type() const { return Type; }
    private:
        HexGrid::Coord _coord;
    };

    class Piece : public QGraphicsEllipseItem
    {
    public:
        enum { Type = UserType + 2 };
        explicit Piece(HexGrid::Piece c, QGraphicsItem *parent);
        inline const HexGrid::Coord &coord() const {
            return qgraphicsitem_cast<Hex*>(parentItem())->coord();
        }

        int type() const { return Type; }
    };

public:
    explicit Board();

signals:
    void playerMoved(HexGrid::Coord oldCoord, HexGrid::Coord newCoord);

protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    QGraphicsScene scene;

    QHash<HexGrid::Coord, Hex *> map;
    Hex *hexFrom = 0;
    Piece *selectedPiece = 0;
    QGraphicsItemGroup *lines;
    QList<Hex*> dests;

    HexGrid grid;
    QHash< HexGrid::Coord, QList<HexGrid::Move> > validMoves;
};

#endif
