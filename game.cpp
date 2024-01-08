#include "game.h"
#include "qdebug.h"

Game::Game(int fieldsize, int snakes_count, QObject *parent, double speed_game, QComboBox * mu_algo)
    : QThread(parent), toDO(NONE), doResetFieldAfterEvolution(false), snakes_count(snakes_count), best(0), mutation_rate(0.0025), mut_range(0.2), fokus(0), mu_algo(mu_algo)
{
    gamefield = new GameField(fieldsize);

#ifdef image_based
    population = new Population("288_SUM_RELU,"
                                "100_SUM_RELU,"
                                "100_SUM_RELU,"
                                "20_SUM_RELU,"
                                "04_SUM_SMAX",
                                snakes_count, 1.0, true);

#else
    population = new Population("24_SUM_RELU,"
                                "25_SUM_RELU,"
                                "18_SUM_RELU,"
                                "04_SUM_SMAX",
                                snakes_count, 0.3, 1);
#endif


//    population = new Population("2_SUM_RELU,"
//                                "5_SUM_TANH,"
//                                "5_SUM_RELU,"
//                                "5_SUM_TANH,"
//                                "2_SUM_SMAX",
//                                2000,
//                                1.0);



    snakes = new Snake*[snakes_count];
    for(int i = 0; i < snakes_count; i++) {
        snakes[i] = new Snake(gamefield, population->netAt(i), this, i, speed_game,
#ifdef image_based
                              Snake::INPUT_MODE::IMAGE_BASED
#else
                              Snake::INPUT_MODE::DETAILED_CLASSIC
#endif
                              );
        connect(snakes[i], SIGNAL(died(int)), this, SLOT(snake_died(int)));
    }

    //connect for auto restart
    connect(this, SIGNAL(finishedEvo()), this, SLOT(auto_restart_ais()));

}

Game::~Game()
{
    for(int i = 0; i < snakes_count; i++) {
        snakes[i]->requestInterruption();
        snakes[i]->quit();
        if(!snakes[i]->wait(3000)) {
            std::cout << "snakes[i]->terminate(); " << i << std::endl;
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
    for(int i = 0; i < 1000 && this->isRunning(); i++) {
        usleep(10000);
        perror("startAIs DELAYED  -> waiting...");
    }
    if(this->isRunning()) {
        perror("startAIs FAILED - Starter Thread still running:");
        return;
    }

    toDO = TO_DO::STARTING;
    this->fokus = fokus;
    this->start();
}

void Game::startPlayer()
{
    living_snakes_count = 0;
}

void Game::stop_and_reset()
{

    this->requestInterruption();
    if(!this->wait(3000)) {
        std::cout << "TERMINATING... this" << __FUNCTION__ << std::endl;
        this->terminate();
    }

    for (int i = 0; i < snakes_count; ++i) {
        if(snakes[i]->isRunning()) {
            snakes[i]->requestInterruption();
        }
    }

    for (int i = 0; i < snakes_count; ++i) {
        if(snakes[i]->isRunning()) {
            if(!snakes[i]->wait(1000)) {
                std::cout << "TERMINATING..." << __FUNCTION__ << std::endl;
                snakes[i]->terminate();
            }
        }
        snakes[i]->reset();
    }
    // this->requestInterruption();
    // if(!this->wait(3000)) {
    //     std::cout << "TERMINATING... this" << __FUNCTION__ << std::endl;
    //     this->terminate();
    // }
}

void Game::do_evolution()
{
    for(int i = 0; i < 1000 && this->isRunning(); i++) {
        usleep(10000);
        perror("START EVO DELAYED  -> waiting...");
    }
    if(this->isRunning()) {
        perror("START EVO FAILED - Starter Thread still running:");
        return;
    }
    toDO = TO_DO::EVOLUTION_CALCING;
    this->start();
}

void Game::setDoResetFieldAfterEvo(bool status)
{
    doResetFieldAfterEvolution = status;
}

void Game::auto_restart_ais()
{
    //Update settings:
//    if(ui->radioButtonAutoRate->isChecked()) {
//        ui->doubleSpinBox_learn_rate->setValue( 1.0 / ( 20 * std::pow(ui->graphicsView->game->population->getEvolutionNum() + ui->spinBox_aut_versch->value() - 0.9 , 0.8 ) ) );
//        ui->doubleSpinBoxMutRange->setValue   (0.5 *   (1.0 / std::pow(ui->graphicsView->game->population->getEvolutionNum() + ui->spinBox_aut_versch->value()   , 0.2) ));
//    }

    if(!this->wait(1000)) {
        std::cout << "auto_restart_ais failed: QThread still evolving!" << std::endl;
        return;
    }

    //reset field
    if(doResetFieldAfterEvolution)
        gamefield->reset();

    //start ais
    // std::cout << "RESTART AIs..." << std::endl;
    startAIs(best);
}


//FastRandom/*std::mt19937*//*minstd_rand*/ generator3(std::random_device{}());


void Game::snake_died(int)
{
    QMutexLocker mutLock(&gameCheckFinishedMutex);
    living_snakes_count--;

    // bool finished = true;
    // int ic = 0;
    // for (int i = 0; i < snakes_count; ++i) {
    //     if(snakes[i]->getLebt_noch()) {
    //         finished = false;
    //         ic++;
    //     }
    // }
    emit livingCountChanged(living_snakes_count);

    if(living_snakes_count == 0 /*finished*/) {
        size_t best_score = 0;

        for (int i = 0; i < snakes_count; ++i) {
            if(snakes[i]->getScore() > best_score) {
                best_score = snakes[i]->getScore();
                best = i;
            }
        }

        //ui->highscore->setText( " ID: " + QString::number(hig_score_id) +  " -> Score: " + QString::number(best_score)+ " Länge: " + QString::number(ui->graphicsView->game->snakes[hig_score_id]->getLegth()));
        //ui->graphicsView->setCurrentBestSnake ( hig_score_id );

        // update ui...
        emit bestSnakeChanged(best, best_score, snakes[best]->getLegth()); // werte mitsenden..werden in evolute resetet!

        //evolute...
        do_evolution(); // --> Thread->  take some time ->
    }
}

void Game::run()
{
    switch (toDO) {

    case NONE:
        std::cout << " Game Thread with no task!" << std::endl;
        break;
    case STARTING:
        for (int i = 0; i < snakes_count; ++i) {
            if(snakes[i]->isRunning()) {
                snakes[i]->requestInterruption();
            }
        }

        for (int i = 0; i < snakes_count; ++i) {
            if(snakes[i]->isRunning()) {
                if(!snakes[i]->wait(1000)) {
                    std::cout << "TERMINATING..." << __FUNCTION__ << std::endl;
                    snakes[i]->terminate();
                }
            }
            // snakes[i]->reset(); in startAI
        }

        if(this->isInterruptionRequested()) {
            std::cout << "Break Game Thread" << std::endl;
            break;
        }

        living_snakes_count = snakes_count;
        snakes[fokus]->startAI(population->netAt(fokus));
        this->snakes[fokus]->setFokus(true);

        for (int i = 0; i < snakes_count; ++i) {
            if(i == fokus)
                continue;
            if(this->isInterruptionRequested()) {
                std::cout << "Break Game Thread" << std::endl;
                break;
            }
            snakes[i]->startAI(population->netAt(i));
            usleep(50);
        }
        break;
    case EVOLUTION_CALCING:
        std::cout << " Evolving... " << std::endl;

        if(mu_algo->currentIndex() == 0) {
            population->evolve(best, mutation_rate, mut_range);
        } else {
            for(int i = 0; i < snakes_count; i++)
                population->scoreMap()[i] = snakes[i]->getScore();
            population->evolveWithSimulatedAnnealing(mutation_rate, mut_range, 0.99);
        }

        std::cout << " Evolved!.. " << std::endl;
        if(this->isInterruptionRequested()) {
            std::cout << "Break Game Thread" << std::endl;
            break;
        }
        // save backup
        population->netAt(best)->save_to("current_best_ai-bak.csv");

        usleep(100);
        emit finishedEvo();

        break;
    }
    toDO = NONE;
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
