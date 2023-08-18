#include "graphicsview.h"

#include <QBrush>
#include <QKeyEvent>

GraphicsView::GraphicsView(QWidget *parent)
    : QGraphicsView(parent), tI(nullptr), showRays(false), rreconnect(true)
{
    scene = new QGraphicsScene(this);
    this->setScene(scene);


    int groese = 20;
    int anzahl = 39;
    int abstand = 20;
    int pixel_count = groese * anzahl + abstand * 2;

    ai_count = 3000;
    double speed_game = 10000.0;

    id_best = 0;


    this->setFixedSize(pixel_count,  pixel_count);

    scene->setSceneRect(0, 0, pixel_count, pixel_count);

    border = scene->addRect(QRect(abstand, abstand, groese * anzahl, groese * anzahl));

    grid = new QGraphicsItemGroup();
    for(int x = abstand; x <= groese * anzahl + abstand; x += groese) {
        QGraphicsLineItem * a;
        grid->addToGroup( ( a = new QGraphicsLineItem(QLine(x, abstand, x, groese * anzahl + abstand))));
        a->setPen(QPen(QBrush(QColor::fromRgb(160, 160, 160)), 1));
        grid->addToGroup( ( a = new QGraphicsLineItem(QLine(abstand, x, groese * anzahl + abstand, x))));
        a->setPen(QPen(QBrush(QColor::fromRgb(160, 160, 160)), 1));
    }


    scene->addItem(grid);
    game = new Game(anzahl, ai_count, this, speed_game);

    connect(game->snakes[0], SIGNAL(posChanged(QPolygon,int)), this, SLOT(snake_moved(QPolygon,int)));
    connect(game->snakes[0], SIGNAL(foodPosChanged(QPoint)), this, SLOT(apple_pos_changed(QPoint)));
    connect(game->snakes[0], SIGNAL(died()), this, SLOT(reconnect()));
    game->snakes[0]->setFokus(true);


    snake = new QGraphicsPathItem();
    scene->addItem(snake);
    snake->setPen(QPen(QBrush(QColor::fromRgb(255, 0, 0)), 8 ));

    apple = new QGraphicsEllipseItem();
    scene->addItem(apple);
    apple->setRect(-3 + 10, -3 + 10, 8, 8);
    apple->setPen(QPen(QBrush(Qt::darkGreen), 8));

    rays = new QGraphicsPathItem();
    scene->addItem(rays);
    rays->setPen(QPen(Qt::black, 1));
    rays->show();

    head = new QGraphicsPathItem();
    scene->addItem(head);
    head->setPen(QPen(Qt::black, 3));
//    head->show();
}

GraphicsView::~GraphicsView()
{
    delete game;
}

void GraphicsView::setTI(QLabel *newTI)
{
    tI = newTI;
}

void GraphicsView::setCurrentBestSnake(int id)
{
    disconnect(game->snakes[id_best], SIGNAL(posChanged(QPolygon, int)), this, SLOT(snake_moved(QPolygon, int)));
    disconnect(game->snakes[id_best], SIGNAL(foodPosChanged(QPoint)), this, SLOT(apple_pos_changed(QPoint)));
    disconnect(game->snakes[id_best], SIGNAL(died()), this, SLOT(reconnect()));


    connect(game->snakes[id], SIGNAL(posChanged(QPolygon,int)), this, SLOT(snake_moved(QPolygon,int)));
    connect(game->snakes[id], SIGNAL(foodPosChanged(QPoint)), this, SLOT(apple_pos_changed(QPoint)));
    if(rreconnect) {
        connect(game->snakes[id], SIGNAL(died()), this, SLOT(reconnect()));
    }
    game->snakes[id]->setFokus(true);
    emit fokus_changed(id);
    apple->setPos(this->game->snakes[id_best]->getCurrentFood() * 20);
    id_best = id;

    if(tI)  tI->setText(QString::number( id_best ));

}

int GraphicsView::getId_best() const
{
    return id_best;
}

int GraphicsView::getAi_count() const
{
    return ai_count;
}

void GraphicsView::snake_moved(QPolygon newPos, int id)
{
    for(auto & e : newPos) {
        e *= 20;
        e += QPoint(10, 10);
    }

    QPainterPath p;
    p.addPolygon(newPos);
    snake->setPen(QPen(QBrush(QColor::fromRgb(( 1.0 - (double)id / (double)ai_count) * 100.0 + 155, 0, 0)), 8 ));
    snake->setPath(p);

    QPainterPath p2;
    p2.addEllipse(newPos.front(), 3, 3);
    head->setPath(p2);

    if(showRays) {
        QPainterPath path;
        QPointF center = newPos.front(); // Der Punkt, in dem sich die Strahlen schneiden
        int numLines = 8; // Anzahl der Linien
        for (int i = 0; i < numLines; ++i) {
            double angle = 2 * M_PI * i / numLines;

            double endX = center.x() + 0.5 * 1800 * qCos(angle);
            double endY = center.y() + 0.5 * 1800 * qSin(angle);

            path.moveTo(center);
            path.lineTo(endX, endY);
        }
        rays->setPath(path);
        if(!rays->isVisible())
            rays->show();
    } else if(rays->isVisible())
        rays->hide();

    emit ta();
}

void GraphicsView::apple_pos_changed(QPoint newPos)
{
    apple->setPos(newPos * 20);
}

void GraphicsView::reconnect()
{
    size_t best = 0;
    size_t best_score = 0;
    bool one_is_living = false;
    QMutexLocker m(&mux);

    if(this->game->snakes[getId_best()]->getLebt_noch())
        return;

    for (int i = 0; i < ai_count; ++i) {
        if(this->game->snakes[i]->getLebt_noch()) {
            if(this->game->snakes[i]->getScore() > best_score) {
                best_score = this->game->snakes[i]->getScore();
                best = i;
                one_is_living = true;
            }
        }
    }

    if(one_is_living) {
        setCurrentBestSnake(best);
    }




}

void GraphicsView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
        game->snakes[0]->richtungAendern(QPoint(0, -1));
        break;
    case Qt::Key_Down:
        game->snakes[0]->richtungAendern(QPoint(0, 1));
        break;
    case Qt::Key_Right:
        game->snakes[0]->richtungAendern(QPoint(1, 0));
        break;
    case Qt::Key_Left:
        game->snakes[0]->richtungAendern(QPoint(-1, 0));
        break;
    }
}
