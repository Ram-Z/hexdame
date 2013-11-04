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
