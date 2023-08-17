#ifndef GAME_H
#define GAME_H



#include "snake.h"
#include "gamefield.h"
#include "population.h"


class Game : public QThread
{
    Q_OBJECT
public:
    Game(int fieldsize, int snakes_count, QObject * parent, double speed_game);
    ~Game();

    GameField * gamefield;
    Snake ** snakes;
    Population * population;

    void startAIs(int fokus);
    void do_evolution(unsigned best, double mutation_rate);

signals:
    void finishedEvo();


private:
    enum TO_DO{
        NONE,
        STARTING,
        EVOLUTION_CALCING
    } toDO;


    void run() override;


    int snakes_count;
    unsigned best;
    double mutation_rate;
    int fokus;

};

#endif // GAME_H
