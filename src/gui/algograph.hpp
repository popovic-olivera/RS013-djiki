#ifndef ALGOGRAPH_HPP
#define ALGOGRAPH_HPP

#include <QWidget>
#include <QKeyEvent>
#include "../backend/graphalgorithm.hpp"

namespace Ui {
class AlgoGraph;
}

class AlgoGraph : public QWidget
{
    Q_OBJECT

public:
    explicit AlgoGraph(QWidget *parent = 0);
    ~AlgoGraph();
    QString getAlgoName();
    GraphAlgorithm* getAlgorithmInstance();

private slots:
    void on_pushButtonAStar_clicked();
    void on_pushButtonDijkstra_clicked();
    void on_pushButtonDFS_clicked();
    void on_pushButtonBFS_clicked();
    void on_pushButtonFW_clicked();
    void on_pushButtonAdd_clicked();

private:
    Ui::AlgoGraph *ui;
    QString algoName;
    GraphAlgorithm* algorithmInstance;
};

#endif // ALGOGRAPH_HPP
