#include "graphicsview.h"

#include <QKeyEvent>

GraphicsView::GraphicsView(QWidget *parent)
    : QGraphicsView(parent), tI(nullptr)
{
    scene = new QGraphicsScene(this);
    this->setScene(scene);


    int groese = 20;
    int anzahl = 39;
    int abstand = 20;
    int pixel_count = groese * anzahl + abstand * 2;

    ai_count = 2000;
    double speed_game = 5000.0;

    id_best = 0;


    this->setFixedSize(pixel_count,  pixel_count);

    scene->setSceneRect(0, 0, pixel_count, pixel_count);

    scene->addRect(QRect(abstand, abstand, groese * anzahl, groese * anzahl));

    grid = new QGraphicsItemGroup();
    for(int x = abstand; x <= groese * anzahl + abstand; x += groese) {
        grid->addToGroup(new QGraphicsLineItem(QLine(x, abstand, x, groese * anzahl + abstand)));
        grid->addToGroup(new QGraphicsLineItem(QLine(abstand, x, groese * anzahl + abstand, x)));
    }

    scene->addItem(grid);
    game = new Game(anzahl, ai_count, this, speed_game);

    for (int i = 0; i < (show_all ? ai_count : 1); ++i) {
        connect(game->snakes[i], SIGNAL(posChanged(QPolygon,int)), this, SLOT(snake_moved(QPolygon,int)));
        connect(game->snakes[i], SIGNAL(foodPosChanged(QPoint)), this, SLOT(apple_pos_changed(QPoint)));
        connect(game->snakes[i], SIGNAL(died()), this, SLOT(reconnect()));
        game->snakes[i]->setFokus(true);
    }



//    void scoreChanged(int pos);

    snake = new QGraphicsPathItem* [ai_count];
    for (int i = 0; i < ai_count; ++i) {
        snake[i] = new QGraphicsPathItem();
        scene->addItem(snake[i]);
        snake[i]->setPen(QPen(QBrush(QColor::fromRgb(( 1.0 - (double)i / (double)ai_count) * 200.0 + 55, 0, 0)), 8 ));
        if(!show_all && i > 0) {
            snake[i]->hide();
        }
    }

    apple = new QGraphicsEllipseItem();
    scene->addItem(apple);
    apple->setRect(-3 + 10, -3 + 10, 8, 8);
    apple->setPen(QPen(QBrush(Qt::darkGreen), 8));



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
    if(!show_all) {
        disconnect(game->snakes[id_best], SIGNAL(posChanged(QPolygon, int)), this, SLOT(snake_moved(QPolygon, int)));
        disconnect(game->snakes[id_best], SIGNAL(foodPosChanged(QPoint)), this, SLOT(apple_pos_changed(QPoint)));
        disconnect(game->snakes[id_best], SIGNAL(died()), this, SLOT(reconnect()));

        game->snakes[id]->setFokus(true);

        connect(game->snakes[id], SIGNAL(posChanged(QPolygon,int)), this, SLOT(snake_moved(QPolygon,int)));
        connect(game->snakes[id], SIGNAL(foodPosChanged(QPoint)), this, SLOT(apple_pos_changed(QPoint)));
        connect(game->snakes[id], SIGNAL(died()), this, SLOT(reconnect()));

    }

    id_best = id;

    std::cout << " Du beobachtest nun: " << id <<std::endl;

    if(tI)
        tI->setText(QString::number( id_best ));


    apple->setPos(this->game->snakes[id_best]->getCurrentFood() * 20);
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
    snake[0]->setPen(QPen(QBrush(QColor::fromRgb(( 1.0 - (double)id / (double)ai_count) * 100.0 + 155, 0, 0)), 8 ));


    if(show_all)
        snake[id]->setPath(p);
    else
        snake[0]->setPath(p);

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
