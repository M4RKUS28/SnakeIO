#ifndef GAME_H
#define GAME_H



#include "qcombobox.h"
#include "snake.h"
#include "gamefield.h"
#include "population.h"


class Game : public QThread
{
    Q_OBJECT
public:
    Game(int fieldsize, int snakes_count, QObject * parent, double speed_game, QComboBox *mu_algo);
    ~Game();

    GameField * gamefield;
    Snake ** snakes;
    Population * population;

    void startAIs(int fokus);
    void startPlayer();
    void stop_and_reset();

    void do_evolution();

    void setDoResetFieldAfterEvo(bool status);
    void setMutation_rate(double newMutation_rate);
    void setMut_range(double newMut_range);

    unsigned int getBest() const;

    double getMutation_rate() const;

    double getMut_range() const;

public slots:
    void auto_restart_ais();
    void snake_died(int id);

signals:
    void finishedEvo();
    void livingCountChanged(int living);
    void bestSnakeChanged(int, int, int);

private:
    enum TO_DO{
        NONE,
        STARTING,
        EVOLUTION_CALCING
    } toDO;


    void run() override;

    bool doResetFieldAfterEvolution;

    int snakes_count;
    int living_snakes_count;

    unsigned best;
    double mutation_rate, mut_range;
    int fokus;
    QMutex gameCheckFinishedMutex;
    QComboBox *mu_algo;
};

#endif // GAME_H
