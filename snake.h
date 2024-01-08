#ifndef SNAKE_H
#define SNAKE_H

#include "gamefield.h"
#include "net.h"
#include "agent.h"

#include "qpolygon.h"
#include <QThread>
#include <QMutex>


static int snake_init_length = 10;
static int snake_init_moves = 250;
static int snake_add_moves_per_apple = 100;


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

    Snake(GameField * field, Net *net, QObject * parent, int num_id, double speed_game, MODE input_mode = CLASSIC);
    ~Snake();

    void startAI(Net *net);
    void startPlayer(Net *netinfo = nullptr);
    void richtungAendern(QPoint richtung);



    int getLegth();
    int getMaxMoves();
    size_t getLeftMoves() const;
    size_t getScore() const;

    bool getLebt_noch() const;

    QPoint getCurrentFood() const;
    void setCurrentFood(QPoint newCurrentFood);

    bool getFokus() const;
    void setFokus(bool newFokus);

    void setSpeed(double speed_game);
    void reset();

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

    GameField * field;
    QPoint currentFood;
    Net * net;
    Agent * agent;
    QPolygon pos;
    QPoint richtung;
    bool isAI;
    int foodNum;
    size_t survive_time;

    QMutex mutex_richtung_aendern;
    int num_id;
    size_t moves;
};



#endif // SNAKE_H
