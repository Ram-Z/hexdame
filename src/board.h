// Samir Benmendil. Copyright (C) 2013. GPL-3.0.

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
        explicit Hex(QGraphicsItem *parent = 0);
        int type() const { return HexItem; }
    };

    class Piece : public QGraphicsEllipseItem
    {
    public:
        explicit Piece(HexGrid::Piece c, QGraphicsItem *parent = 0);
        int type() const { return PieceItem; }
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

    QHash<QGraphicsItem *, HexGrid::Coord> map;
    QGraphicsItem *selectedPiece = 0;

    HexGrid grid;
};

#endif
