#ifndef GAME_H
#define GAME_H

#include "qcombobox.h"
#include "snake.h"
#include "gamefield.h"
#include "population.h"
#include "config/inputconfig.h"

// ===========================================================================
//  Game
//
//  Manages the game field, a population of Snakes, and drives the
//  evolutionary training loop on a background QThread.
//
//  The neural-network architecture and input layout are fully determined by
//  the NetworkConfig passed to the constructor.  No compile-time #ifdefs needed.
// ===========================================================================
class Game : public QThread
{
    Q_OBJECT
public:
    // cfg fully describes field size, snake count, input layout, and topology.
    // mutAlgo: combo-box from the UI that selects the evolution algorithm.
    Game(const NetworkConfig& cfg, QObject* parent, double speed_game,
         QComboBox* mutAlgo, bool pve = false);
    ~Game();

    GameField*  gamefield;
    Snake**     snakes;
    Population* population;

    // Stored config — needed by components that query input count / labels.
    const NetworkConfig cfg;

    void startAIs(int fokus);
    void startPlayer();
    void stop_and_reset();
    void do_evolution();

    void setDoResetFieldAfterEvo(bool status);
    void setMutation_rate(double newMutation_rate);
    void setMut_range(double newMut_range);

    unsigned int getBest()           const;
    double       getMutation_rate()  const;
    double       getMut_range()      const;

public slots:
    void auto_restart_ais();
    void snake_died(int id);

signals:
    void finishedEvo();
    void livingCountChanged(int living);
    void bestSnakeChanged(int, int, int);

private:
    enum TO_DO { NONE, STARTING, EVOLUTION_CALCING } toDO;

    void run() override;

    bool doResetFieldAfterEvolution;
    int snakes_count;
    int living_snakes_count;

    unsigned best;
    double   mutation_rate, mut_range;
    int      fokus;
    QMutex   gameCheckFinishedMutex;
    QComboBox* mu_algo;
    bool pve;
};

#endif // GAME_H
