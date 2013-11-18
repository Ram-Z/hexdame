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

#include "hexdamegame.h"

#include <qmath.h>
#include <QMouseEvent>

#include <QtGlobal>
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

HexdameView::GraphicsPieceItem::GraphicsPieceItem(Piece state, QGraphicsItem *parent)
    : QGraphicsEllipseItem(parent)
{
    static const float r = radius * 0.6;

    setState(state);

    setRect(-r, -r, 2 * r, 2 * r);
    setFlags(ItemIsSelectable | ItemIsMovable);
}

void HexdameView::GraphicsPieceItem::setState(const Piece &state)
{
    _state = state;
    if (state == WhitePawn) {
        setBrush(QBrush(Qt::gray, Qt::SolidPattern));
        setVisible(true);
    } else if (state == WhiteKing) {
        setBrush(QBrush(Qt::gray, Qt::CrossPattern));
        setVisible(true);
    } else if (state == BlackPawn) {
        setBrush(QBrush(Qt::black, Qt::SolidPattern));
        setVisible(true);
    } else if (state == BlackKing) {
        setBrush(QBrush(Qt::black, Qt::CrossPattern));
        setVisible(true);
    } else if (state == Empty) {
        setVisible(false);
    }
}

HexdameView::HexdameView(HexdameGame *game, QWidget *parent)
    : QGraphicsView(parent)
    , _game(game)
{
    foreach (Coord c, _game->coords()) {
        GraphicsHexItem *h = new GraphicsHexItem(c);
        coordToHex[c] = h;

        Piece piece = _game->at(c);
        GraphicsPieceItem *p = new GraphicsPieceItem(piece, h);
        coordToPiece[c] = p;

#if 1   // add text coords
        QGraphicsTextItem *t = new QGraphicsTextItem(QString("{%1,%2}").arg(c.x).arg(c.y), h);
        t->setZValue(1);
        t->rotate(180);
#endif

        scene.addItem(h);
    }

    rotate(180);
    setScene(&scene);

    // connect signals
    connect(this, SIGNAL(playerMoved(Coord, Coord)),
            _game, SLOT(makeMove(Coord,Coord)));
    connect(this, SIGNAL(partialMove(Coord,Coord)),
            _game, SLOT(makePartialMove(Coord,Coord)));
    connect(_game, SIGNAL(boardChanged()),
            this, SLOT(updateBoard()));
    connect(this, SIGNAL(rightClicked(Coord)),
            _game, SLOT(debugRightClick(Coord)));
}

void
HexdameView::mousePressEvent(QMouseEvent *event)
{
    if (!_game->debug() && event->button() == Qt::RightButton) return;

    GraphicsHexItem *hex = 0;
    GraphicsPieceItem *piece = 0;
    foreach (QGraphicsItem *item, items(event->pos())) {
        if (item->type() == PieceItem) {
            piece = qgraphicsitem_cast<GraphicsPieceItem *>(item);
        }
        if (item->type() == HexItem) {
            hex = qgraphicsitem_cast<GraphicsHexItem *>(item);
        }
    }

    if (_game->debug()) {
        if (event->button() == Qt::RightButton) {
            emit rightClicked(hex->coord());
            return;
        }
    }

    // not selected a piece
    if (!piece) return;

    if (!_game->debug()) {
        if (!_game->currentPlayerIsHuman()) return;
        if (color(piece->state()) != _game->currentColor()) return;
    }

    QGraphicsView::mousePressEvent(event);

    hexFrom = hex;
    selectedPiece = piece;

    // draw it on top
    hexFrom->setZValue(1);

    QMultiHash<Coord, Move> moves = _game->validMoves(hexFrom->coord());

    // partial move
    QList<Coord> partDests;
    foreach (const Coord &to, moves.uniqueKeys()) {
        QList<Move> m = moves.values(to);
        // not a partial move
        if (m.size() == 1) continue;

        bool same = true, first = true;
        Coord common = m.first().path.at(0);
        QList<Coord> path, taken;
        path << m.first().path.first();
        int i;
        for (i = 1; i < m.first().path.size(); ++i) {
            common = m.first().path.at(i);
            if (same) {
                foreach (Move mm, m) {
                    same &= mm.path.at(i) == common;
                    if (!same) { break; }
                }
            }
            if (!same) break;
        }
        for (int k = 0; k < m.size(); ++k) {
            Move newMove = m.at(k);
            moves.remove(to, m.at(k));
            for (int j = i+1; j < m.at(k).path.size(); ++j) {
                newMove.path.removeLast();
                newMove.taken.removeLast();
            }
            qDebug() << newMove.path;
            partDests << newMove.to();
            moves.insert(newMove.to(), newMove);
        }
    }

    lines = new QGraphicsItemGroup();
    for (auto move = moves.constBegin(); move != moves.constEnd(); ++move) {
        QPointF from = hexFrom->pos();
        QPointF to;
        QColor col(qrand() % 255, qrand() % 255, qrand() % 255);
        foreach (Coord c, move.value().path) {
            to = coordToHex.value(c)->pos();

            QGraphicsLineItem *line = new QGraphicsLineItem(QLineF(from, to));
            line->setPen(col);

            lines->addToGroup(line);

            from = coordToHex.value(c)->pos();

        }
        GraphicsHexItem *dest = qgraphicsitem_cast<GraphicsHexItem *>(coordToHex.value(move.key()));
        if (partDests.contains(move.key())) {
            dest->setBrush(Qt::Dense3Pattern);
            _partialDests << dest;
        } else {
            dest->setBrush(Qt::Dense1Pattern);
            _dests << dest;
        }
    }
    scene.addItem(lines);
}

void
HexdameView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    if (event->button() == Qt::RightButton) return;
    if (!selectedPiece) return;

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

    selectedPiece->setPos(0.0, 0.0);

    // restore zValue
    hexFrom->setZValue(0);

    scene.removeItem(lines);
    foreach (GraphicsHexItem* h, _dests) {
        h->setBrush(Qt::NoBrush);
    }
    foreach (GraphicsHexItem* h, _partialDests) {
        h->setBrush(Qt::NoBrush);
    }
    delete lines;

    if (hex && !piece) {
        Coord oldCoord = hexFrom->coord();
        Coord newCoord = hex->coord();

        if (_partialDests.contains(hex))
            emit partialMove(oldCoord, newCoord);
        else if (_game->debug() || _dests.contains(hex))
            emit playerMoved(oldCoord, newCoord);
    }

    _dests.clear();
    _partialDests.clear();
    hexFrom = 0;
    selectedPiece = 0;
}

void HexdameView::updateBoard()
{
    foreach (Coord c, _game->coords()) {
        Piece piece = _game->at(c);
        coordToPiece[c]->setState(piece);
    }
}
