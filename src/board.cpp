// Samir Benmendil. Copyright (C) 2013. GPL-3.0.

#include "board.h"

#include <qmath.h>
#include <QMouseEvent>

#include <QtDebug>

Board::Hex::Hex(QGraphicsItem *parent)
    : QGraphicsPolygonItem(parent)
{
    QPolygonF p;
    for (qreal theta = 0.0; theta < 2 * M_PI; theta += M_PI / 3) {
        p << QPointF(radius * qCos(theta), radius * qSin(theta));
    }
    setPolygon(p);
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
    static const qreal delta_x = radius + radius * qCos(M_PI / 3);
    static const qreal delta_y = radius * qSin(M_PI / 3);

    foreach (HexGrid::Coord c, grid.coords()) {
        qreal new_x = c.x * delta_x - c.y * delta_x;
        qreal new_y = (c.x + c.y) * delta_y;

        Hex *h = new Hex();
        h->setPos(new_x, new_y);

#if 1   // add text coords
        QGraphicsTextItem *t = scene.addText(QString("{%1,%2}").arg(c.x).arg(c.y));
        t->setPos(new_x, new_y);
        t->setZValue(20);
        t->rotate(180);
#endif

        map[h] = c;

        HexGrid::Piece piece = grid.at(c);
        if (piece != HexGrid::Empty) {
            Piece *p = new Piece(piece, h);
        }

        scene.addItem(h);
    }

    rotate(180);
    setScene(&scene);

    // connect signals
    connect(this, SIGNAL(playerMoved(HexGrid::Coord, HexGrid::Coord)),
            &grid, SLOT(movePiece(HexGrid::Coord, HexGrid::Coord)));
}

void
Board::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);

    QGraphicsItem *hex = 0;
    QGraphicsItem *piece = 0;
    foreach (QGraphicsItem * item, items(event->pos())) {
        if (item->type() == PieceItem)
            selectedPiece = item;
        if (item->type() == HexItem)
            hex = item;
    }
}

void
Board::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);

    QGraphicsItem *hex = 0;
    QGraphicsItem *piece = 0;
    foreach (QGraphicsItem * item, items(event->pos())) {
        if (item->type() == PieceItem)
            if (selectedPiece != item)
                piece = item;
        if (item->type() == HexItem)
            hex = item;
    }

    if (selectedPiece) {
        if (hex && !piece) {
            HexGrid::Coord oldCoord = map.value(selectedPiece->parentItem());
            HexGrid::Coord newCoord = map.value(hex);

            selectedPiece->setParentItem(hex);

            emit playerMoved(oldCoord, newCoord);
        }

        selectedPiece->setPos(0.0, 0.0);

        selectedPiece = 0;
    }
}
