#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include <QObject>
#include <qgraphicsscene.h>

#include <QGraphicsItemGroup>
#include <QPainterPath>


#include "game.h"
#include "qlabel.h"



class GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    GraphicsView(QWidget * parent);
    ~GraphicsView();
    QGraphicsScene * scene;
    QGraphicsItemGroup * grid;


    Game * game;

    QGraphicsPathItem   ** snake;

    QGraphicsEllipseItem * apple;

    int ai_count;

    QLabel * tI;


    bool show_all = false;

    QMutex mux;

    int getAi_count() const;

    void setTI(QLabel *newTI);


    void setCurrentBestSnake(int id);
    int getId_best() const;

private:
    int id_best;


signals:
    void ta();

public slots:
    void snake_moved(QPolygon newPos, int id);
    void apple_pos_changed(QPoint newPos);
    void reconnect();


    // QWidget interface
protected:
    virtual void keyPressEvent(QKeyEvent *event);
};



#endif // GRAPHICSVIEW_H
