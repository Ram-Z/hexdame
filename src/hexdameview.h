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

#ifndef HEXDAMEVIEW_H
#define HEXDAMEVIEW_H

#include "hexdamegame.h"

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>

class HexdameView : public QGraphicsView
{
    Q_OBJECT

    static const quint8 radius = 50;

    enum Type {
        HexItem = QGraphicsItem::UserType + 1,
        PieceItem
    };

    class GraphicsHexItem : public QGraphicsPolygonItem
    {
    public:
        enum { Type = UserType + 1 };
        explicit GraphicsHexItem(Coord c, QGraphicsItem *parent = 0);
        inline const Coord &coord() const { return _coord; }

        int type() const { return Type; }
    private:
        Coord _coord;
    };

    class GraphicsPieceItem : public QGraphicsEllipseItem
    {
    public:
        enum { Type = UserType + 2 };
        explicit GraphicsPieceItem(Color c, QGraphicsItem *parent);
        inline const Coord &coord() const {
            return qgraphicsitem_cast<GraphicsHexItem*>(parentItem())->coord();
        }

        int type() const { return Type; }
    };

public:
    explicit HexdameView();

signals:
    void playerMoved(Coord oldCoord, Coord newCoord);

protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    QGraphicsScene scene;

    QHash<Coord, GraphicsHexItem *> map;
    GraphicsHexItem *hexFrom = 0;
    GraphicsPieceItem *selectedPiece = 0;
    QGraphicsItemGroup *lines;
    QList<GraphicsHexItem*> dests;

    HexdameGame grid;
    QHash< Coord, QList<Move> > validMoves;
};

#endif
