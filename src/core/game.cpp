#include "game.h"
#include "inputconfig.h"

#include <QDebug>

// ===========================================================================
//  Constructor
// ===========================================================================
Game::Game(const NetworkConfig& cfg, QObject* parent, double speed_game,
           QComboBox* mu_algo, bool pve)
    : QThread(parent),
      cfg(cfg),
      snakes_count(cfg.snakeCount),
      mu_algo(mu_algo),
      pve(pve)
{
    gamefield = new GameField(cfg.fieldSize);

    // Build topology string from the NetworkConfig and construct the population.
    const std::string topology = InputConfig::buildTopologyString(cfg);
    population = new Population(topology, snakes_count, 0.3, 1);

    // Create one Snake per population member.
    // The first half starts at the top of the field, the rest at the bottom.
    snakes = new Snake*[snakes_count];
    for (int i = 0; i < snakes_count; ++i) {
        snakes[i] = new Snake(gamefield, population->netAt(i), this, i,
                               speed_game, this->cfg,
                               /* startTop */ i < snakes_count / 2);
        connect(snakes[i], SIGNAL(died(int)), this, SLOT(snake_died(int)));
    }

    // Wire PvE enemy references: each snake's enemy is its mirror partner.
    if (pve) {
        for (int i = 0; i < snakes_count; ++i)
            snakes[i]->setEnemy(snakes[snakes_count - 1 - i]);
    }

    connect(this, SIGNAL(finishedEvo()), this, SLOT(auto_restart_ais()));
}

Game::~Game()
{
    for (int i = 0; i < snakes_count; ++i) {
        snakes[i]->requestInterruption();
        snakes[i]->quit();
        if (!snakes[i]->wait(3000)) {
            qWarning("Game::~Game: snake %d did not stop in time — terminating", i);
            snakes[i]->terminate();
            snakes[i]->wait(2000);
        }
        delete snakes[i];
    }
    delete[] snakes;   // array new → array delete
    snakes = nullptr;

    delete population;
    delete gamefield;
}

void Game::startAIs(int focusId)
{
    // Wait for any previous Game thread iteration to finish before starting new one.
    for (int i = 0; i < 1000 && this->isRunning(); ++i) {
        QThread::msleep(10);
        qWarning("Game::startAIs: thread still running — waiting (%d/1000)", i);
    }
    if (this->isRunning()) {
        qWarning("Game::startAIs: FAILED — game thread still running after timeout");
        return;
    }

    toDO      = ToDo::STARTING;
    this->fokus = focusId;
    this->start();
}

void Game::startPlayer()
{
    // Player mode: snake threads are started directly from the UI —
    // the Game thread itself is not used for coordination.
    living_snakes_count = 0;
}

void Game::stop_and_reset()
{
    // Stop the Game coordinator thread first.
    this->requestInterruption();
    if (!this->wait(3000)) {
        qWarning("Game::stop_and_reset: Game thread did not stop in time — terminating");
        this->terminate();
    }

    // Signal all snake threads to stop.
    for (int i = 0; i < snakes_count; ++i) {
        if (snakes[i]->isRunning())
            snakes[i]->requestInterruption();
    }

    // Wait for each snake thread and reset its state.
    for (int i = 0; i < snakes_count; ++i) {
        if (snakes[i]->isRunning()) {
            if (!snakes[i]->wait(1000)) {
                qWarning("Game::stop_and_reset: snake %d did not stop in time — terminating", i);
                snakes[i]->terminate();
            }
        }
        snakes[i]->reset();
    }
}

void Game::do_evolution()
{
    // Wait for the Game thread to be free (it may still be in the STARTING phase).
    for (int i = 0; i < 1000 && this->isRunning(); ++i) {
        QThread::msleep(10);
        qWarning("Game::do_evolution: thread still running — waiting (%d/1000)", i);
    }
    if (this->isRunning()) {
        qWarning("Game::do_evolution: FAILED — game thread still running after timeout");
        return;
    }
    toDO = ToDo::EVOLUTION_CALCING;
    this->start();
}

void Game::setDoResetFieldAfterEvo(bool status)
{
    doResetFieldAfterEvolution = status;
}

void Game::auto_restart_ais()
{
    // Ensure the evolution thread has finished before restarting.
    if (!this->wait(1000)) {
        qWarning("Game::auto_restart_ais: evolution thread still running — skipping restart");
        return;
    }

    if (doResetFieldAfterEvolution)
        gamefield->reset();

    startAIs(best);
}


void Game::snake_died(int)
{
    QMutexLocker mutLock(&gameCheckFinishedMutex);
    --living_snakes_count;
    emit livingCountChanged(living_snakes_count);

    if (living_snakes_count == 0) {
        // Find the best-scoring snake of this generation.
        size_t best_score = 0;
        for (int i = 0; i < snakes_count; ++i) {
            if (snakes[i]->getScore() > best_score) {
                best_score = snakes[i]->getScore();
                best = i;
            }
        }

        // Notify UI, then kick off evolution (skipped in PvE — no neuroevolution there).
        emit bestSnakeChanged(best, static_cast<int>(best_score), snakes[best]->getLegth());
        if (!pve)
            do_evolution();
    }
}

void Game::run()
{
    switch (toDO) {

    case ToDo::NONE:
        qWarning("Game::run: started with no task assigned");
        break;

    case ToDo::STARTING: {
        // Stop any snake threads still running from a previous generation.
        for (int i = 0; i < snakes_count; ++i) {
            if (snakes[i]->isRunning())
                snakes[i]->requestInterruption();
        }
        for (int i = 0; i < snakes_count; ++i) {
            if (snakes[i]->isRunning()) {
                if (!snakes[i]->wait(1000)) {
                    qWarning("Game::run STARTING: snake %d did not stop — terminating", i);
                    snakes[i]->terminate();
                }
            }
        }

        if (this->isInterruptionRequested())
            break;

        // Start the focused snake first (player or AI).
        if (!pve)
            snakes[fokus]->startAI(population->netAt(fokus));
        else
            snakes[fokus]->startPlayer(population->netAt(fokus));

        living_snakes_count = snakes_count;
        snakes[fokus]->setFokus(true);

        // Start all other AI snakes with a tiny stagger to spread CPU load.
        for (int i = 0; i < snakes_count; ++i) {
            if (i == fokus) continue;
            if (this->isInterruptionRequested()) break;
            snakes[i]->startAI(population->netAt(i));
            QThread::usleep(50);
        }
        break;
    }

    case ToDo::EVOLUTION_CALCING: {
        qDebug("Game::run: evolving, best=%u length=%d", best, snakes[best]->getLegth());

        if (!mu_algo || mu_algo->currentIndex() == 0) {
            // Standard elitist evolution: copy best, mutate rest.
            population->evolve(best, mutation_rate, mut_range);
        } else {
            // Simulated-annealing evolution: uses per-snake scores.
            for (int i = 0; i < snakes_count; ++i)
                population->scoreMap()[i] = snakes[i]->getScore();
            population->evolveWithSimulatedAnnealing(mutation_rate, mut_range, 0.99);
        }

        if (this->isInterruptionRequested())
            break;

        // Persist the best network so the user can resume after a crash.
        population->netAt(best)->saveTo("current_best_ai-bak.csv");

        emit finishedEvo();
        break;
    }
    }
    toDO = ToDo::NONE;
}

double Game::getMut_range() const
{
    return mut_range;
}

double Game::getMutation_rate() const
{
    return mutation_rate;
}

unsigned int Game::getBest() const
{
    return best;
}

void Game::setMut_range(double newMut_range)
{
    mut_range = newMut_range;
}

void Game::setMutation_rate(double newMutation_rate)
{
    mutation_rate = newMutation_rate;
}
