#ifndef SNAKE_H
#define SNAKE_H

#pragma once

#include "gamefield.h"
#include "net.h"
#include "config/inputconfig.h"

#include "qpolygon.h"
#include <QThread>
#include <QMutex>

// ===========================================================================
//  Snake
//
//  Runs on its own QThread. Each tick:
//    1. lookThingsUp()    — fill input buffer from NetworkConfig
//    2. net->feedForward  — inference
//    3. argmax over 4 outputs → move direction
//
//  The input layout (what each neuron receives) is driven entirely by
//  cfg.inputs (a QVector<InputNeuronConfig>). See src/config/inputconfig.h.
// ===========================================================================
class Snake : public QThread
{
    Q_OBJECT
public:
    // cfg must outlive all Snake instances that reference it (owned by Game).
    Snake(GameField* field, Net* net, QObject* parent, int num_id,
          double speed_game, const NetworkConfig& cfg, bool startTop = true);
    ~Snake();

    void startAI(Net* net);
    void startPlayer(Net* netinfo = nullptr);
    void richtungAendern(QPoint richtung);

    QPolygon getEnemyPolygon();

    void setEnemy(Snake* enemy);
    int    getLegth()     const;
    int    getMaxMoves() const;
    size_t getLeftMoves()  const;
    size_t getScore()      const;
    bool   getLebt_noch()  const;

    QPoint getCurrentFood();

    bool getFokus()        const;
    void setFokus(bool newFokus);

    void setSpeed(double speed_game);
    void reset();

    int   getNum_id()  const;
    Snake* getEnemy()  const;
    int   getFoodNum() const;

    QPolygon getPos()  const;

    // Last output neuron values from the previous tick (indices: 0=Up 1=Down 2=Right 3=Left).
    // Default: 0.0. Updated every tick after net->getResults().
    double lastOutput[4] = {0.0, 0.0, 0.0, 0.0};

signals:
    void foodPosChanged(QPoint pos, int id);
    void posChanged(QPolygon snake, int num_id);
    void died(int num_id);

private:
    // Evaluates one InputNeuronConfig against the current game state.
    double evaluateFeature(const InputNeuronConfig& ncfg, const QPoint& foodPos) const;

    // Fills `buffer` with one value per cfg.inputs entry.
    void lookThingsUp(double* buffer, const QPoint& foodPos);

    virtual void run();

    const NetworkConfig& cfg;  // owned by Game, lifetime > Snake

    bool   lebt_noch;
    bool   fokus;
    double speed_game;
    Snake* enemy;

    GameField*          field;
    Net*                net;

    QPolygon pos;
    QPoint   richtung;
    bool     isAI;
    int      foodNum;
    size_t   survive_time;

    QMutex mutex_richtung_aendern;
    int    num_id;
    size_t moves;
    bool   startTop;

    static int snake_init_length;
    static int snake_init_moves;
    static int snake_add_moves_per_apple;
};

#endif // SNAKE_H


