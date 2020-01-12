#include "drawing.hpp"

#include <iostream>

#include <QApplication>
#include <QScreen>

QFont Drawing::font = QFont("Times", 12);

std::pair<qreal, qreal> Drawing::getWindowSize()
{
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int width = screenGeometry.width();
    int height = screenGeometry.height();

    return std::make_pair<qreal, qreal>(width, height);
}

void Drawing::setScene(QGraphicsView *view, QWidget* parent)
{
    std::pair<qreal, qreal> screenSize = getWindowSize();
    QGraphicsScene* scene = new QGraphicsScene(0, 0,
                                               screenSize.first, screenSize.second,
                                               parent);
    view->setScene(scene);
    view->setRenderHint(QPainter::Antialiasing);
}

QGraphicsTextItem* Drawing::drawDirections(QGraphicsView* view, QString instructions)
{
    font.setBold(true);
    QGraphicsTextItem* directions = view->scene()->addText(instructions, font);
    qreal textWidth = directions->boundingRect().width();
    qreal textHeight = directions->boundingRect().height();
    directions->setPos(QPointF(view->width()/2 - textWidth/2, view->height()/2 - textHeight/2 + 50));

    directions->setDefaultTextColor("#5599ff");

    return directions;
}