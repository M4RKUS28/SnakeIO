#ifndef SNAKE_H
#define SNAKE_H

#include "gamefield.h"
#include "net.h"
#include "qpolygon.h"
#include <QThread>
#include <QMutex>



class Snake : public QThread
{
    Q_OBJECT
public:
    Snake(GameField * field, Net *net, QObject * parent, int num_id, double speed_game);
    ~Snake();

    void startAI(Net *net);
    void startPlayer(Net *netinfo = nullptr);
    void richtungAendern(QPoint richtung);


    int getLegth();
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
    void foodPosChanged(QPoint pos);
    void posChanged(QPolygon snake, int num_id);
    void textUpdate();
    void died();


private:

    void lookThingsUp(double *buffer, const QPoint &foodPos);

    virtual void run();

    bool lebt_noch;
    bool fokus;
    double speed_game;

    GameField * field;
    QPoint currentFood;
    Net * net;
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
