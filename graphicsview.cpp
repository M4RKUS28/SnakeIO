#include "graphicsview.h"

#include <QBrush>
#include <QKeyEvent>

GraphicsView::GraphicsView(StartSettings s, QWidget *parent)
    : QGraphicsView(parent), showRays(false), rreconnect(true), connected_to(0)
{
    scene = new QGraphicsScene(this);
    this->setScene(scene);

    ai_count = s.ai_count;

    int groese = 20;
    int anzahl =
#ifdef image_based
        12;
#else
        /*39;//*/21;//12;//39;
#endif
    int abstand = 20;
    int pixel_count = groese * anzahl + abstand * 2;
#ifdef image_based
    ai_count =    500;
#endif

    double speed_game = 10000.0;

    std::cout << "AIS: " << ai_count << std::endl;

    // this->setFixedSize(pixel_count,  pixel_count);
    scene->setSceneRect(0, 0, pixel_count, pixel_count);
    border = scene->addRect(QRect(abstand, abstand, groese * anzahl, groese * anzahl));

    // Karo Muster...
    grid = new QGraphicsItemGroup();
    for(int x = abstand; x <= groese * anzahl + abstand; x += groese) {
        QGraphicsLineItem * a;
        grid->addToGroup( ( a = new QGraphicsLineItem(QLine(x, abstand, x, groese * anzahl + abstand))));
        a->setPen(QPen(QBrush(QColor::fromRgb(160, 160, 160)), 1));
        grid->addToGroup( ( a = new QGraphicsLineItem(QLine(abstand, x, groese * anzahl + abstand, x))));
        a->setPen(QPen(QBrush(QColor::fromRgb(160, 160, 160)), 1));
    }
    scene->addItem(grid);


    //Game...
    game = new Game(anzahl, ai_count, this, speed_game);
    connect(game, SIGNAL(bestSnakeChanged(int,int,int)), this, SLOT(setNewFokusToBest(int,int,int)));

    game->snakes[connected_to]->setFokus(true);




    //Init Game ui's
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

    head = new QGraphicsPathItem();
    scene->addItem(head);
    head->setPen(QPen(Qt::black, 3));
}

void GraphicsView::init(int ai_count)
{



}

GraphicsView::~GraphicsView()
{
    delete game;
}


Snake *GraphicsView::currentSnake()
{
    return game->snakes[connected_to];
}

Net *GraphicsView::currentNet()
{
    return game->population->netAt(connected_to);
}

int GraphicsView::getConnected_to() const
{
    return connected_to;
}

void GraphicsView::setShowRays(bool newShowRays)
{
    showRays = newShowRays;
}

void GraphicsView::setRreconnect(bool newRreconnect)
{
    rreconnect = newRreconnect;
}

int GraphicsView::getAi_count() const
{
    return ai_count;
}

void GraphicsView::connectToSnake(int id)
{
    disconnect(currentSnake(), SIGNAL(posChanged(QPolygon, int)), this, SLOT(snake_moved(QPolygon, int)));
    disconnect(currentSnake(), SIGNAL(foodPosChanged(QPoint,int)), this, SLOT(apple_pos_changed(QPoint,int)));
    disconnect(currentSnake(), SIGNAL(died(int)), this, SLOT(reconnect(int)));


    connect(game->snakes[id], SIGNAL(posChanged(QPolygon,int)), this, SLOT(snake_moved(QPolygon,int)));
    connect(game->snakes[id], SIGNAL(foodPosChanged(QPoint,int)), this, SLOT(apple_pos_changed(QPoint,int)));
    if(rreconnect) {
        connect(game->snakes[id], SIGNAL(died(int)), this, SLOT(reconnect(int)));

    }

    connected_to = id; // speichere neue connection!
    game->snakes[connected_to]->setFokus(true);
    emit fokus_changed(id);

    apple->setPos(currentSnake()->getCurrentFood() * 20);
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

    //update stats, if connected to snake
    if(id == connected_to)
        emit textUpdateNeeded();
}

void GraphicsView::apple_pos_changed(QPoint newPos, int id)
{
    if(id == connected_to)
        apple->setPos(newPos * 20);
    else
        std::cerr << "wrong id" << std::endl;
}

void GraphicsView::reconnect(int id)
{
    size_t best = 0;
    size_t best_score = 0;
    bool one_is_living = false;

    if(this->currentSnake()->getLebt_noch())
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
        connectToSnake(best);
    }
}

void GraphicsView::setNewFokusToBest(int id,int,int)
{
    connectToSnake(id);
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


