#ifndef EDGE_HPP
#define EDGE_HPP

#include "node.hpp"

#include <QGraphicsItem>
#include <QInputDialog>

class Edge : public QObject, public QGraphicsItem {
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    Edge(Node* s, Node* e, int w);

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    Node* start;
    Node* end;

    int nodeWeight;
};

#endif // EDGE_HPP
