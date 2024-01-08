#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include <QObject>
#include <qgraphicsscene.h>

#include <QGraphicsItemGroup>
#include <QPainterPath>


#include "game.h"
#include "startdialog.h"



class GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    GraphicsView(StartSettings s, QWidget * parent, QComboBox *mutAlgo);
    ~GraphicsView();

    void init(int ai_count);


    Game * game;
    Snake * currentSnake();
    Net * currentNet();

    int getAi_count() const;
    int getConnected_to() const;
    void connectToSnake(int id);


    QGraphicsScene * scene;
    QGraphicsItemGroup * grid;

    QGraphicsPathItem   * snake;
    QGraphicsPathItem   * head;
    QGraphicsPathItem   * rays;
    QGraphicsRectItem * border;
    QGraphicsEllipseItem * apple;

    void setShowRays(bool newShowRays);

    void setRreconnect(bool newRreconnect);

private:
    int ai_count;
    bool showRays;
    bool rreconnect;

    int connected_to;

    QMutex reconnectMutex;

signals:
    void textUpdateNeeded();
    void fokus_changed(unsigned id);

private slots:
    void snake_moved(QPolygon newPos, int id);
    void apple_pos_changed(QPoint newPos, int id);
    void reconnect(int id);
    void setNewFokusToBest(int id, int, int);



protected:
    virtual void keyPressEvent(QKeyEvent *event);
};





#endif // GRAPHICSVIEW_H
