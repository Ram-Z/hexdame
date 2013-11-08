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

#include "board.h"

#include <qmath.h>
#include <QMouseEvent>

#include <QtDebug>

Board::Hex::Hex(HexGrid::Coord c, QGraphicsItem *parent)
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

Board::Piece::Piece(HexGrid::Piece c, QGraphicsItem *parent)
    : QGraphicsEllipseItem(parent)
{
    static const float r = radius * 0.6;

    if (c == HexGrid::WhitePawn)
        setBrush(QBrush(Qt::gray));
    else if (c == HexGrid::BlackPawn)
        setBrush(QBrush(Qt::black));

    setRect(-r, -r, 2 * r, 2 * r);
    setFlags(ItemIsSelectable | ItemIsMovable);
}

Board::Board()
    : QGraphicsView()
{
    foreach (HexGrid::Coord c, grid.coords()) {
        Hex *h = new Hex(c);
        map[c] = h;

        HexGrid::Piece piece = grid.at(c);
        if (piece != HexGrid::Empty) {
            Piece *p = new Piece(piece, h);
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
    connect(this, SIGNAL(playerMoved(HexGrid::Coord, HexGrid::Coord)),
            &grid, SLOT(movePiece(HexGrid::Coord, HexGrid::Coord)));
}

void
Board::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
    if (event->button() == Qt::RightButton) return;

    Hex *hex = 0;
    Piece *piece = 0;
    foreach (QGraphicsItem * item, items(event->pos())) {
        if (item->type() == PieceItem) {
            piece = qgraphicsitem_cast<Piece *>(item);
        }
        if (item->type() == HexItem) {
            hex = qgraphicsitem_cast<Hex *>(item);
        }
    }

    if (piece) {
        hexFrom = hex;
        selectedPiece = piece;

        // draw it on top
        hexFrom->setZValue(1);

        QList<HexGrid::Move> moves = validMoves.value(hexFrom->coord());
        lines = new QGraphicsItemGroup();
        foreach (HexGrid::Move move, moves) {
            QPointF from = map.value(move.from)->pos();
            QPointF to;
            QColor col(qrand() % 255, qrand() % 255, qrand() % 255);
            foreach (HexGrid::Coord c, move.path) {
                to = map.value(c)->pos();

                QGraphicsLineItem *line = new QGraphicsLineItem(QLineF(from, to));
                line->setPen(col);

                lines->addToGroup(line);

                from = map.value(c)->pos();

                Hex *dest = qgraphicsitem_cast<Hex *>(map.value(move.path.last()));
                dest->setBrush(Qt::Dense1Pattern);
                dests << dest;
            }
        }
        scene.addItem(lines);
    }
}

void
Board::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    if (event->button() == Qt::RightButton) return;

    Hex *hex = 0;
    Piece *piece = 0;
    foreach (QGraphicsItem * item, items(event->pos())) {
        if (item->type() == PieceItem) {
            if (selectedPiece != item)
                piece = qgraphicsitem_cast<Piece *>(item);
        }
        if (item->type() == HexItem) {
            hex = qgraphicsitem_cast<Hex *>(item);
        }
    }

    if (selectedPiece) {
        if (hex && !piece) {
#define ALL_MOVES
#ifndef ALL_MOVES
            if (dests.contains(hex)) {
#endif
                HexGrid::Coord oldCoord = hexFrom->coord();
                HexGrid::Coord newCoord = hex->coord();

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
        foreach (Hex* h, dests) {
            h->setBrush(Qt::NoBrush);
        }
        dests.clear();
        delete lines;
    }
}
