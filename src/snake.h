#ifndef SNAKE_H
#define SNAKE_H

#pragma once


#include "gamefield.h"
#include "net.h"

#include "qpolygon.h"
#include <QThread>
#include <QMutex>




class Snake : public QThread
{
    Q_OBJECT
public:
    enum MODE {
        CLASSIC,
        DETAILED_CLASSIC,
        IMAGE_BASED,
        TURN_MODE

    } input_mode;

    Snake(GameField * field, Net *net, QObject * parent, int num_id, double speed_game, MODE input_mode = CLASSIC, bool startTop = true);
    ~Snake();

    void startAI(Net *net);
    void startPlayer(Net *netinfo = nullptr);
    void richtungAendern(QPoint richtung);

    QPolygon getEnemyPolygon();

    void setEnemy(Snake * enemy);
    int getLegth();
    int getMaxMoves();
    size_t getLeftMoves() const;
    size_t getScore() const;

    bool getLebt_noch() const;

    QPoint getCurrentFood();

    bool getFokus() const;
    void setFokus(bool newFokus);

    void setSpeed(double speed_game);
    void reset();

    int getNum_id() const;

    Snake *getEnemy() const;

    int getFoodNum() const;

    QPolygon getPos() const;

signals:
    void foodPosChanged(QPoint pos, int id);
    void posChanged(QPolygon snake, int num_id);
    void died(int num_id);


private:

    void lookThingsUp(double *buffer, const QPoint &foodPos);

    virtual void run();

    bool lebt_noch;
    bool fokus;
    double speed_game;
    Snake * enemy;

    GameField * field;
    Net * net;
    // Agent * agent;
    QPolygon pos;
    QPoint richtung;
    bool isAI;
    int foodNum;
    size_t survive_time;

    QMutex mutex_richtung_aendern;
    int num_id;
    size_t moves;
    bool startTop;

    static int snake_init_length;
    static int snake_init_moves;
    static int snake_add_moves_per_apple;
};





#endif // SNAKE_H
