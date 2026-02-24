#include "graphicsview.h"

#include <QBrush>
#include <QKeyEvent>

GraphicsView::GraphicsView(StartSettings s, QWidget* parent, QComboBox* mutAlgo,
                           double speed_game, bool isPvE)
    : QGraphicsView(parent), showRays(false), rreconnect(true), connected_to(0), isPvE(isPvE)
{
    scene = new QGraphicsScene(this);
    this->setScene(scene);

    constexpr int cellPx = 20;   // pixels per grid cell
    constexpr int margin = 20;   // border offset in pixels
    const int fieldCells = s.networkConfig.fieldSize;
    ai_count             = s.networkConfig.snakeCount;

    const int totalPx = cellPx * fieldCells + margin * 2;

    qDebug("GraphicsView: field=%d cells, %d AIs", fieldCells, ai_count);

    scene->setSceneRect(0, 0, totalPx, totalPx);
    border = scene->addRect(QRect(margin, margin, cellPx * fieldCells, cellPx * fieldCells));

    // Grid lines
    grid = new QGraphicsItemGroup();
    for (int x = margin; x <= cellPx * fieldCells + margin; x += cellPx) {
        QGraphicsLineItem* line;
        grid->addToGroup((line = new QGraphicsLineItem(
            QLine(x, margin, x, cellPx * fieldCells + margin))));
        line->setPen(QPen(QColor(160, 160, 160), 1));
        grid->addToGroup((line = new QGraphicsLineItem(
            QLine(margin, x, cellPx * fieldCells + margin, x))));
        line->setPen(QPen(QColor(160, 160, 160), 1));
    }
    scene->addItem(grid);

    // Create the Game with the full NetworkConfig.
    game = new Game(s.networkConfig, this, speed_game, mutAlgo, isPvE);
    connect(game, SIGNAL(bestSnakeChanged(int,int,int)), this, SLOT(setNewFokusToBest(int,int,int)));

    game->snakeAt(connected_to)->setFokus(true);

    // ---- Snake rendering items ----
    snakeItem = new QGraphicsPathItem();
    scene->addItem(snakeItem);
    snakeItem->setPen(QPen(QColor(255, 0, 0), 8));

    appleItem = new QGraphicsEllipseItem();
    scene->addItem(appleItem);
    appleItem->setRect(-3 + 10, -3 + 10, 8, 8);
    appleItem->setPen(QPen(Qt::darkGreen, 8));

    rays = new QGraphicsPathItem();
    scene->addItem(rays);
    rays->setPen(QPen(Qt::black, 1));

    headItem = new QGraphicsPathItem();
    scene->addItem(headItem);
    headItem->setPen(QPen(Qt::black, 3));

    snakeEnemy = new QGraphicsPathItem();
    scene->addItem(snakeEnemy);
    snakeEnemy->setPen(QPen(QColor(0, 0, 255), 8));
    if (!isPvE)
        snakeEnemy->hide();

    headEnemy = new QGraphicsPathItem();
    headEnemy->setPen(QPen(Qt::black, 3));
    scene->addItem(headEnemy);
    if (!isPvE)
        headEnemy->hide();
}


GraphicsView::~GraphicsView()
{
    delete game;
}


Snake* GraphicsView::currentSnake() const
{
    return game->snakeAt(connected_to);
}

Net* GraphicsView::currentNet() const
{
    return game->getPopulation()->netAt(connected_to);
}

int GraphicsView::getConnected_to() const
{
    return connected_to;
}

void GraphicsView::setShowRays(bool show)
{
    showRays = show;
}

void GraphicsView::setHiddenApple(bool enabled)
{
    hiddenApple = enabled;
    updateAppleVisibility();
}

void GraphicsView::updateAppleVisibility()
{
    if (!hiddenApple) {
        appleItem->show();
        return;
    }
    // The apple lies on one of the 8 compass / diagonal rays from the head when:
    //   same column (N/S), same row (E/W), or equal-distance diagonal (NE/NW/SE/SW).
    const int dx = lastFoodGrid.x() - lastHeadGrid.x();
    const int dy = lastFoodGrid.y() - lastHeadGrid.y();
    const bool onRay = (dx == 0) || (dy == 0) || (qAbs(dx) == qAbs(dy));
    if (onRay) appleItem->show(); else appleItem->hide();
}

void GraphicsView::setRreconnect(bool enabled)
{
    rreconnect = enabled;
}

void GraphicsView::setGridVisible(bool visible)
{
    if (visible) grid->show(); else grid->hide();
}

int GraphicsView::getAi_count() const
{
    return ai_count;
}

void GraphicsView::connectToSnake(int id)
{
    disconnect(currentSnake(), SIGNAL(posChanged(QPolygon, int)),    this, SLOT(snake_moved(QPolygon, int)));
    disconnect(currentSnake(), SIGNAL(foodPosChanged(QPoint,int)),   this, SLOT(apple_pos_changed(QPoint,int)));
    disconnect(currentSnake(), SIGNAL(died(int)),                    this, SLOT(reconnect(int)));
    game->snakeAt(connected_to)->setFokus(false);

    connect(game->snakeAt(id), SIGNAL(posChanged(QPolygon,int)),    this, SLOT(snake_moved(QPolygon,int)));
    connect(game->snakeAt(id), SIGNAL(foodPosChanged(QPoint,int)),  this, SLOT(apple_pos_changed(QPoint,int)));

    if (rreconnect)
        connect(game->snakeAt(id), SIGNAL(died(int)), this, SLOT(reconnect(int)));

    connected_to = id;
    game->snakeAt(connected_to)->setFokus(true);
    emit fokus_changed(id);

    appleItem->setPos(currentSnake()->getCurrentFood() * 20);
    lastFoodGrid = currentSnake()->getCurrentFood();

    if (rreconnect && !game->snakeAt(id)->getLebt_noch())
        reconnect(-1);
}

void GraphicsView::snake_moved(QPolygon newPos, int id, bool isFirstCall)
{
    // Record head grid position before transforming to pixel coords.
    if (isFirstCall)
        lastHeadGrid = newPos.at(0);

    // Transform grid coords to pixel coords (cell centre).
    for (auto& p : newPos) {
        p *= 20;
        p += QPoint(10, 10);
    }

    // Choose the right path items depending on whether this is the
    // focused snake or its enemy.
    QGraphicsPathItem* bodyItem = isFirstCall ? snakeItem  : snakeEnemy;
    QGraphicsPathItem* headPt   = isFirstCall ? headItem   : headEnemy;

    QPainterPath p;
    p.addPolygon(newPos);
    bodyItem->setPath(p);

    QPainterPath p2;
    p2.addEllipse(newPos.front(), 3, 3);
    headPt->setPath(p2);

    // Draw visibility rays from head.
    if (showRays && isFirstCall) {
        QPainterPath rayPath;
        const QPointF centre = newPos.front();
        for (int i = 0; i < 8; ++i) {
            const double angle = 2.0 * M_PI * i / 8.0;
            rayPath.moveTo(centre);
            rayPath.lineTo(centre.x() + 900.0 * qCos(angle),
                           centre.y() + 900.0 * qSin(angle));
        }
        rays->setPath(rayPath);
        if (!rays->isVisible()) rays->show();
    } else if (rays->isVisible()) {
        rays->hide();
    }

    // Refresh status text and apple visibility for the connected snake.
    if (id == connected_to) {
        updateAppleVisibility();
        emit textUpdateNeeded();
    }

    // In PvE mode, also draw the enemy snake on the same field.
    if (isPvE && isFirstCall && game->snakeAt(id)->getEnemy()) {
        const bool enemyAlive = game->snakeAt(id)->getEnemy()->getLebt_noch();
        snakeEnemy->setVisible(enemyAlive);
        headEnemy->setVisible(enemyAlive);
        if (enemyAlive)
            snake_moved(game->snakeAt(id)->getEnemyPolygon(),
                        game->snakeAt(id)->getEnemy()->getNum_id(), false);
    }
}

void GraphicsView::apple_pos_changed(QPoint newPos, int id)
{
    if (id == connected_to) {
        lastFoodGrid = newPos;  // grid coords
        appleItem->setPos(newPos * 20);
        updateAppleVisibility();
    } else {
        qWarning("GraphicsView::apple_pos_changed: id %d does not match connected_to %d", id, connected_to);
    }
}

void GraphicsView::reconnect(int)
{
    if (currentSnake()->getLebt_noch())
        return;  // still alive — spurious signal

    if (isPvE) {
        Snake* enemy = currentSnake()->getEnemy();
        if (enemy && enemy->getLebt_noch())
            enemy->reset();
    }

    // Find the highest-scoring snake that is still alive.
    int    best       = -1;
    size_t best_score = 0;
    for (int i = 0; i < ai_count; ++i) {
        if (game->snakeAt(i)->getLebt_noch() &&
            game->snakeAt(i)->getScore() > best_score) {
            best_score = game->snakeAt(i)->getScore();
            best       = i;
        }
    }

    if (best >= 0)
        connectToSnake(best);
}

void GraphicsView::setNewFokusToBest(int id, int, int)
{
    connectToSnake(id);
}

void GraphicsView::keyPressEvent(QKeyEvent* event)
{
    // Route arrow keys to the snake currently connected (player mode).
    Snake* s = game->snakeAt(connected_to);
    switch (event->key()) {
    case Qt::Key_Up:    s->richtungAendern(QPoint( 0, -1)); break;
    case Qt::Key_Down:  s->richtungAendern(QPoint( 0,  1)); break;
    case Qt::Key_Right: s->richtungAendern(QPoint( 1,  0)); break;
    case Qt::Key_Left:  s->richtungAendern(QPoint(-1,  0)); break;
    default: QGraphicsView::keyPressEvent(event); break;
    }
}


