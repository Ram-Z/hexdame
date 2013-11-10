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

#include "hexdameview.h"

#include <qmath.h>
#include <QMouseEvent>

#include <QtDebug>

HexdameView::GraphicsHexItem::GraphicsHexItem(Coord c, QGraphicsItem *parent)
    : QGraphicsPolygonItem(parent)
    , _coord(c)
{
    QPolygonF p;
    for (qreal theta = 0.0; theta < 2 * M_PI; theta += M_PI / 3) {
        p << QPointF(radius * qCos(theta), radius * qSin(theta));
    }
    setPolygon(p);

    static const qreal delta_x = radius + radius * qCos(M_PI / 3);
    static const qreal delta_y = radius * qSin(M_PI / 3);

    qreal new_x = c.x * delta_x - c.y * delta_x;
    qreal new_y = (c.x + c.y) * delta_y;

    setPos(new_x,new_y);
}

HexdameView::GraphicsPieceItem::GraphicsPieceItem(Color c, QGraphicsItem *parent)
    : QGraphicsEllipseItem(parent)
{
    static const float r = radius * 0.6;

    if (c == White)
        setBrush(QBrush(Qt::gray));
    else if (c == Black)
        setBrush(QBrush(Qt::black));

    setRect(-r, -r, 2 * r, 2 * r);
    setFlags(ItemIsSelectable | ItemIsMovable);
}

HexdameView::HexdameView()
    : QGraphicsView()
{
    foreach (Coord c, grid.coords()) {
        GraphicsHexItem *h = new GraphicsHexItem(c);
        map[c] = h;

        Piece piece = grid.at(c);
        if (piece != Empty) {
            GraphicsPieceItem *p = new GraphicsPieceItem(color(piece), h);
        }

#if 1   // add text coords
        QGraphicsTextItem *t = new QGraphicsTextItem(QString("{%1,%2}").arg(c.x).arg(c.y), h);
        t->setZValue(1);
        t->rotate(180);
#endif

        scene.addItem(h);
    }

    rotate(180);
    setScene(&scene);

    validMoves = grid.computeValidMoves();

    // connect signals
    connect(this, SIGNAL(playerMoved(Coord, Coord)),
            &grid, SLOT(movePiece(Coord, Coord)));
}

void
HexdameView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
    if (event->button() == Qt::RightButton) return;

    GraphicsHexItem *hex = 0;
    GraphicsPieceItem *piece = 0;
    foreach (QGraphicsItem * item, items(event->pos())) {
        if (item->type() == PieceItem) {
            piece = qgraphicsitem_cast<GraphicsPieceItem *>(item);
        }
        if (item->type() == HexItem) {
            hex = qgraphicsitem_cast<GraphicsHexItem *>(item);
        }
    }

    if (piece) {
        hexFrom = hex;
        selectedPiece = piece;

        // draw it on top
        hexFrom->setZValue(1);

        QList<Move> moves = validMoves.value(hexFrom->coord());
        lines = new QGraphicsItemGroup();
        foreach (Move move, moves) {
            QPointF from = map.value(move.from)->pos();
            QPointF to;
            QColor col(qrand() % 255, qrand() % 255, qrand() % 255);
            foreach (Coord c, move.path) {
                to = map.value(c)->pos();

                QGraphicsLineItem *line = new QGraphicsLineItem(QLineF(from, to));
                line->setPen(col);

                lines->addToGroup(line);

                from = map.value(c)->pos();

                GraphicsHexItem *dest = qgraphicsitem_cast<GraphicsHexItem *>(map.value(move.path.last()));
                dest->setBrush(Qt::Dense1Pattern);
                dests << dest;
            }
        }
        scene.addItem(lines);
    }
}

void
HexdameView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    if (event->button() == Qt::RightButton) return;

    GraphicsHexItem *hex = 0;
    GraphicsPieceItem *piece = 0;
    foreach (QGraphicsItem * item, items(event->pos())) {
        if (item->type() == PieceItem) {
            if (selectedPiece != item)
                piece = qgraphicsitem_cast<GraphicsPieceItem *>(item);
        }
        if (item->type() == HexItem) {
            hex = qgraphicsitem_cast<GraphicsHexItem *>(item);
        }
    }

    if (selectedPiece) {
        if (hex && !piece) {
#define ALL_MOVES
#ifndef ALL_MOVES
            if (dests.contains(hex)) {
#endif
                Coord oldCoord = hexFrom->coord();
                Coord newCoord = hex->coord();

                selectedPiece->setParentItem(hex);

                emit playerMoved(oldCoord, newCoord);

                validMoves = grid.computeValidMoves();
#ifndef ALL_MOVES
            }
#endif
        }

        selectedPiece->setPos(0.0, 0.0);

        // restore zValue
        hexFrom->setZValue(0);

        hexFrom = 0;
        selectedPiece = 0;

        scene.removeItem(lines);
        foreach (GraphicsHexItem* h, dests) {
            h->setBrush(Qt::NoBrush);
        }
        dests.clear();
        delete lines;
    }
}
