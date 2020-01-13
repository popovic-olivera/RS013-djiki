#include "drawgraph.hpp"
#include "ui_drawgraph.h"
#include "../drawingFunctions.hpp"

#include <limits>
#include <iostream>

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QPainter>
#include <QInputDialog>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>


DrawGraph::DrawGraph(QWidget* parent)
    : QWidget(parent)
      , ui(new Ui::drawGraph)
{
    ui->setupUi(this);

    initializeScene();
    finished = false;
}

void DrawGraph::initializeScene()
{
    Drawing::setScene(ui->graphicsView, this);

    clearItem = Drawing::createBoxBtnOrLabel<QPushButton>(ui->graphicsView, "Clear", QPointF(width()-130, 20), this);
    helpItem = Drawing::createBoxBtnOrLabel<QLabel>(ui->graphicsView, "Help", QPointF(width()-100, 70), this);
    doneItem = Drawing::createBoxBtnOrLabel<QCheckBox>(ui->graphicsView, "Done drawing graph", QPointF(20, 20), this);
    doneItem->setEnabled(false);

    codeItem = Drawing::createBoxBtnOrLabel<QLabel>(ui->graphicsView, "", QPointF(0, 0), this);
    codeItem->setVisible(false);
    codeItem->setZValue(100);   // active pseudocode will be above graph

    QPushButton* clearBtn = static_cast<QPushButton*>(clearItem->widget());
    QLabel* helpLabel = static_cast<QLabel*>(helpItem->widget());
    QCheckBox* doneBox = static_cast<QCheckBox*>(doneItem->widget());

    QObject::connect(clearBtn, &QPushButton::clicked,
                     this, &DrawGraph::onClearGraph);

    QObject::connect(doneBox, &QPushButton::clicked,
                     this, &DrawGraph::onDoneDrawing);

    QString instructions = "Click anywhere to create nodes and click and drag to move them.\n\n"
                           "Right click on node to delete it.\n\n"
                           "Create directed edges by clicking on two nodes, holding control key.\n\n"
                           "Create node loop by clicking twice on the same node.\n\n"
                           "After that enter node weight (Integer value or 'Inf').\n\n"
                           "Double click on edge to change it's weight.\n\n"
                           "Right click on edge to delete it.\n\n"
                           "When you finish click 'Done drawing graph'.\n\n"
                           "You can start over from scratch by clicking Clear button.\n";

    directions = Drawing::drawDirections(ui->graphicsView, instructions);
    helpLabel->setToolTip(instructions);
    helpLabel->setToolTipDuration(3000);
}

void DrawGraph::mousePressEvent(QMouseEvent* event)
{
    if (directions->isVisible())
        directions->setVisible(false);

    if (event->buttons() == Qt::LeftButton &&
        QGuiApplication::keyboardModifiers().testFlag(Qt::NoModifier) && !finished)
    {
        QPointF mapped = ui->graphicsView->mapToScene(event->pos());
        Node* newNode = new Node(mapped.x(), mapped.y());
        nodes.push_back(newNode);

        QObject::connect(newNode, &Node::drawNeighbour,
                         this, &DrawGraph::drawEdge);

        QObject::connect(newNode, &Node::nodeDeleted,
                         this, &DrawGraph::deleteFromNeighbours);

        ui->graphicsView->scene()->addItem(newNode);
    }

    /* If there are no nodes, then algorithms don't work at all, so done button is disabled
       until first node is drawn */
    if (!finished && nodes.size() > 0)
        doneItem->setEnabled(true);
}

void DrawGraph::resizeEvent(QResizeEvent *)
{
    clearItem->setPos(QPointF(width()-130, 20));
    helpItem->setPos(QPointF(width()-100, 70));

    int x = (width() - 600) / 200;
    QFont font("Times", 12+x);
    directions->setFont(font);

    qreal textWidth = directions->boundingRect().width();
    qreal textHeight = directions->boundingRect().height();
    directions->setPos(QPointF(width()/2 - textWidth/2, height()/2 - textHeight/2 + 50));

    QLabel* codeLbl = static_cast<QLabel*>(codeItem->widget());
    int textW = codeLbl->fontMetrics().boundingRect(codeLbl->text()).width();

    codeLbl->setGeometry(0, 0, textW+10, 40);
    codeItem->setPos(QPointF((width() - textW)/2, 90));
}

DrawGraph::~DrawGraph()
{
    /* This has to be reinitialized because user can go back to main menu and
       again start drawing */
    Node::numberOfNodes = 0;
    Node::deletedNumbers.clear();

    ui->graphicsView->scene()->clear();

    delete ui;
}

void DrawGraph::drawEdge(Node* node)
{
    selectedNodes.push_back(node);

    if (selectedNodes.size() < 2)
        return;
    else if (selectedNodes.size() > 2)
    {
        selectedNodes.clear();
        return;
    }

    Node* start = std::move(selectedNodes[0]);
    Node* end = std::move(selectedNodes[1]);
    selectedNodes.clear();

    if (start->isNeighbour(end))
        return;

    auto [status, value] = Drawing::getWeightFromUser(start, end, this);
    if (!status)
        return;

    bool curve = false;
    if (start == end)
        curve = true;

    if (end->isNeighbour(start))
        for (auto& e : edges)
            if (e->getStart() == end && e->getEnd() == start)
            {
                if (value != e->getEdgeWeight())
                    curve = true;

                break;
            }

    start->addNeighbour(end);

    Edge* newEdge = new Edge(start, end, value, curve, this);

    QObject::connect(newEdge, &Edge::edgeDeleted,
                     this, &DrawGraph::removeEdge);

    QObject::connect(start, &Node::nodeMoved,
                     newEdge, &Edge::nodeMoved);

    QObject::connect(end, &Node::nodeMoved,
                     newEdge, &Edge::nodeMoved);

    edges.push_back(newEdge);
    ui->graphicsView->scene()->addItem(newEdge);
}

void DrawGraph::onClearGraph()
{
    if (directions->isVisible())
        directions->setVisible(false);

    for (auto& e : edges)
    {
        e->setVisible(false);
        ui->graphicsView->scene()->removeItem(e);
        e->deleteLater();
    }
    edges.clear();

    for (auto& n : nodes)
    {
        n->setVisible(false);
        ui->graphicsView->scene()->removeItem(n);
        n->deleteLater();
    }
    nodes.clear();
    selectedNodes.clear();

    Node::numberOfNodes = 0;
    Node::deletedNumbers.clear();
}

void DrawGraph::updateBox(QString line)
{
    activeLine = cleanPseudocodeLine(line);

    QLabel* codeLbl = static_cast<QLabel*>(codeItem->widget());
    codeLbl->setText(activeLine);
    int textWidth = codeLbl->fontMetrics().boundingRect(codeLbl->text()).width();

    if (textWidth > width()-350)
    {
        activeLine = splitLine(activeLine);
        codeLbl->setText(activeLine);

        codeLbl->setGeometry(0, 0, textWidth+10, 40);
        codeItem->setPos(QPointF((width() - textWidth/2)/2, 90));
    }
    else
    {
        codeLbl->setGeometry(0, 0, textWidth+10, 40);
        codeItem->setPos(QPointF((width() - textWidth)/2, 90));
    }

    codeLbl->setStyleSheet("background-color: rgba(0,0,0,0%)");
}

QString& DrawGraph::cleanPseudocodeLine(QString &line)
{
    while (line.indexOf("\t") != -1 || line.indexOf("\n") != -1)
    {
        line = QString::fromStdString(line.toStdString().substr(1));
    }

    return line;
}

QString DrawGraph::splitLine(QString line)
{
    /* Line is split on space which is aproximately in the middle of the line */
    QVector<int> indexes;
    for (int i = 0; i < line.size(); i++)
    {
        if (line[i] == ' ')
            indexes.push_back(i);
    }

    int n = indexes.size();
    if (n == 0)
        return line;

    int half = n % 2 == 0 ? n/2 : (n-1)/2;
    int splitOn = indexes[half];

    line = line.left(splitOn+1) + "\n" + line.right(line.size()-splitOn-1);

    return line;
}

void DrawGraph::onDoneDrawing()
{
    /* Disable scene, but leave scrollbars enabled ( before this->setEnabled(false) ) */
    finished = true;
    doneItem->setEnabled(false);
    clearItem->setEnabled(false);
    helpItem->setVisible(false);

    for (auto& n: nodes)
        n->setEnabled(false);

    for (auto& e: edges)
        e->setEnabled(false);

    animationTimer = new QTimer(this);
    QObject::connect(animationTimer, &QTimer::timeout, ui->graphicsView->scene(), &QGraphicsScene::advance);
    animationTimer->start(200);

    Graph* g = new Graph(&nodes, &edges);
    emit doneDrawingGraph(g);
}

void DrawGraph::deleteFromNeighbours(Node* n)
{
    for (auto& node: nodes)
        node->removeNeighbour(n);

    auto pos = std::find(std::begin(nodes), std::end(nodes), n);
    if (pos != std::end(nodes))
        nodes.erase(pos);

    for (auto& edge: edges)
        if (edge->getStart() == n || edge->getEnd() == n)
        {
            ui->graphicsView->scene()->removeItem(edge);
            edge->deleteLater();
        }

    const auto edgeEnd = [n](Edge* e){ return e->getStart() == n || e->getEnd() == n; };
    edges.erase(std::remove_if(std::begin(edges), std::end(edges), edgeEnd), std::end(edges));

    n->deleteLater();
}

void DrawGraph::removeEdge(Edge* e)
{
    auto it = std::find(std::begin(edges), std::end(edges), e);
    if (it != edges.end())
        edges.erase(it);

    e->deleteLater();
}

QVector<Node*> DrawGraph::getNodes() const
{
    return nodes;
}

QVector<Edge*> DrawGraph::getEdges() const
{
    return edges;
}

QTimer* DrawGraph::getAnimationTimer() const
{
    return animationTimer;
}

Ui::drawGraph* DrawGraph::getUi() const
{
    return ui;
}
