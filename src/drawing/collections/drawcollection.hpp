#ifndef DRAWCOLLECTION_HPP
#define DRAWCOLLECTION_HPP

#include "item.hpp"
#include "connection.hpp"

#include <QWidget>

namespace Ui {
class DrawCollection;
}

class DrawCollection : public QWidget
{
    Q_OBJECT

public:
    explicit DrawCollection(QWidget *parent = nullptr);

    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    QGraphicsProxyWidget* clearItem;
    QGraphicsProxyWidget* helpItem;
    QGraphicsProxyWidget* doneItem;
    QGraphicsTextItem* directions;

    ~DrawCollection() override;

public Q_SLOTS:
    /* After user deletes last drawn item this slot receives signal */
    void deleteItemFromCollection(Item* item);
    /* After user clicks 'Done drawing collection' this slot receives signal */
    void onDoneDrawing();
    /* After uses clicks 'Clear' button this slot receives signal */
    void onClearCollection();

private:
    Ui::DrawCollection *ui;
    void initializeScene();

    bool finished;  // used to disable creating new item on click after drawing is finished

    QVector<Item*> collectionItems;
    QVector<Connection*> connections;
};

#endif // DRAWCOLLECTION_HPP