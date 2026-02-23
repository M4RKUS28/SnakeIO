#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include <QObject>
#include <qgraphicsscene.h>

#include <QGraphicsItemGroup>
#include <QPainterPath>

#include "src/game.h"
#include "startdialog.h"



class GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    GraphicsView(StartSettings s, QWidget* parent, QComboBox* mutAlgo,
                 double speed_game, bool isPvE = false);
    ~GraphicsView();

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

    QGraphicsPathItem   * snake_enemy;
    QGraphicsPathItem   * head_enemy;

    QGraphicsPathItem   * rays;
    QGraphicsRectItem   * border;
    QGraphicsEllipseItem * apple;

    void setShowRays(bool newShowRays);
    void setHiddenApple(bool enabled);

    void setRreconnect(bool newRreconnect);

private:
    int ai_count;
    bool showRays;
    bool hiddenApple = false;  // show apple only when a ray crosses it
    bool rreconnect;

    int connected_to;
    bool isPvE;
    QMutex reconnectMutex;

    QPoint lastHeadGrid;  // head position in grid coords, updated each snake_moved
    QPoint lastFoodGrid;  // food position in grid coords, updated each apple_pos_changed

signals:
    void textUpdateNeeded();
    void fokus_changed(unsigned id);

private slots:
    void snake_moved(QPolygon newPos, int id, bool isFirstCall = true);
    void apple_pos_changed(QPoint newPos, int id);
    void reconnect(int id);
    void setNewFokusToBest(int id, int, int);

    void updateAppleVisibility();  // shows/hides apple according to hiddenApple+ray check



protected:
    virtual void keyPressEvent(QKeyEvent *event);
};





#endif // GRAPHICSVIEW_H
