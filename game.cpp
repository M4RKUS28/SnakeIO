#include "game.h"

Game::Game(int fieldsize, int snakes_count, QObject *parent, double speed_game)
    : QThread(parent), toDO(NONE), snakes_count(snakes_count)
{

    gamefield = new GameField(fieldsize);
    snakes = new Snake*[snakes_count];
    for(int i = 0; i < snakes_count; i++)
        snakes[i] = new Snake(gamefield, this, i, speed_game);

    population = new Population("24_SUM_RELU,"
                                "18_SUM_RELU,"
                                "18_SUM_RELU,"
                                "04_SUM_SMAX"
                                , snakes_count);

}

Game::~Game()
{
    for(int i = 0; i < snakes_count; i++) {
        snakes[i]->requestInterruption();
        snakes[i]->quit();
        if(snakes[i]->wait(2000)) {
            snakes[i]->terminate();
            snakes[i]->wait(2000);
        }
        delete snakes[i];
    }
    delete snakes;
    snakes = nullptr;

    delete population;
}

void Game::startAIs(int fokus)
{
    toDO = TO_DO::STARTING;
    this->fokus = fokus;
    this->start();
}

void Game::do_evolution(unsigned int best, double mutation_rate)
{
    toDO = TO_DO::EVOLUTION_CALCING;
    this->best = best;
    this->mutation_rate = mutation_rate;
    this->start();
}

void Game::run()
{
    switch (toDO) {

    case NONE:
        break;
    case STARTING:
        for (int i = 0; i < snakes_count; ++i)
            snakes[i]->reset();

        snakes[fokus]->startAI(population->netAt(fokus));
        this->snakes[fokus]->setFokus(true);

        for (int i = 0; i < snakes_count; ++i) {
            if(i == fokus)
                continue;
            snakes[i]->startAI(population->netAt(i));
            usleep(75);
        }
        break;
    case EVOLUTION_CALCING:
        population->evolve(best, mutation_rate);
        std::cout << " Evolved!.. " << std::endl;

        usleep(500);
        emit finishedEvo();

        break;
    }
    toDO = NONE;
}
