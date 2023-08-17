#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , fins(true), hig_score_id(0), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->graphicsView->setTI(ui->label_ai_num);

    ui->graphicsView->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    connect(ui->graphicsView, SIGNAL(ta()), this, SLOT(textUpdate()));
    connect(ui->graphicsView->game, SIGNAL(finishedEvo()), this, SLOT(restartAIs()));



    for (int i = 0; i < ui->graphicsView->getAi_count(); ++i) {
        connect(ui->graphicsView->game->snakes[i], SIGNAL(died()), this, SLOT(updateCount()));
    }


}

MainWindow::~MainWindow()
{
    ui->graphicsView->setTI(nullptr);
    delete ui;
}

#include <unistd.h>


void MainWindow::on_pushButtonStart_clicked()
{
//    fins = false;
    textUpdate();
    ui->graphicsView->game->startAIs(hig_score_id);

    ui->label_count->setText(QString::number(ui->graphicsView->getAi_count()));
//    ui->graphicsView->setFocus();
//    ui->graphicsView->game->snakes[hig_score_id]->setFokus(true);
//    ui->graphicsView->apple->setPos(ui->graphicsView->game->snakes[hig_score_id]->getCurrentFood() * 20);


}


void MainWindow::on_pushButton_gridHider_clicked()
{
    if(ui->graphicsView->grid->isVisible())
        ui->graphicsView->grid->hide();
    else
        ui->graphicsView->grid->show();
}

void MainWindow::restartAIs()
{
    textUpdate();
    ui->graphicsView->game->startAIs(hig_score_id);
    ui->label_count->setText(QString::number(ui->graphicsView->getAi_count()));
}

void MainWindow::textUpdate()
{
    ui->laength->setText(QString::number(ui->graphicsView->game->snakes[ui->graphicsView->getId_best()]->getLegth()));
    ui->score->setText(QString::number(ui->graphicsView->game->snakes[ui->graphicsView->getId_best()]->getScore()));
    ui->moves_left->setText(QString::number(ui->graphicsView->game->snakes[ui->graphicsView->getId_best()]->getLeftMoves()));

}

void MainWindow::updateCount()
{
//    if(fins)
//        return;

    bool finished = true;
    int ic = 0;
    for (int i = 0; i < ui->graphicsView->getAi_count(); ++i) {
        if(ui->graphicsView->game->snakes[i]->getLebt_noch()) {
            finished = false;
            ic++;
        }
    }
    ui->label_count->setText(QString::number(ic));

    if(finished) {

        std::cout << " 1 mal restart..." << std::endl;

        size_t best_score = 0;

        for (int i = 0; i < ui->graphicsView->getAi_count(); ++i) {
            if(ui->graphicsView->game->snakes[i]->getScore() > best_score) {
                best_score = ui->graphicsView->game->snakes[i]->getScore();
                hig_score_id = i;
            }
        }
        ui->highscore->setText(QString::number(best_score) + " ID: " + QString::number(hig_score_id));
        ui->graphicsView->setCurrentBestSnake ( hig_score_id );
        textUpdate();

        for (int i = 0; i < ui->graphicsView->getAi_count(); ++i) {
            ui->graphicsView->game->snakes[i]->requestInterruption();
            if(ui->graphicsView->game->snakes[i]->isRunning())
                if(!ui->graphicsView->game->snakes[i]->wait(3000))
                    ui->graphicsView->game->snakes[i]->terminate();
        }

        ui->graphicsView->game->do_evolution(hig_score_id, ui->doubleSpinBox_learn_rate->value());
    }

}



void MainWindow::on_pushButton_clicked()
{
    ui->graphicsView->game->gamefield->reset();

}


void MainWindow::on_pushButton_store_clicked()
{
    if(ui->graphicsView->game->population->netAt(hig_score_id)->save_to("net_save.csv"))
        ui->statusbar->showMessage("Erfoglreich gespeichert!", 2000);
    else
        ui->statusbar->showMessage("Speichern fehlgeschlagen!", 2000);

}


void MainWindow::on_pushButton_load_clicked()
{
    for (int i = 0; i < ui->graphicsView->getAi_count(); ++i) {
        ui->graphicsView->game->snakes[i]->requestInterruption();
        if(ui->graphicsView->game->snakes[i]->isRunning())
            if(!ui->graphicsView->game->snakes[i]->wait(3000))
                ui->graphicsView->game->snakes[i]->terminate();
    }

    if(ui->graphicsView->game->population->netAt(hig_score_id)->load_from("net_save.csv"))
    ui->statusbar->showMessage("Erfoglreich geladen!", 2000);
    else
        ui->statusbar->showMessage("Laden fehlgeschlagen!", 2000);

    ui->graphicsView->game->do_evolution(hig_score_id, ui->doubleSpinBox_learn_rate->value());
    ui->graphicsView->setCurrentBestSnake ( hig_score_id );


    ui->highscore->setText(QString::number(-1) + " ID: " + QString::number(hig_score_id));
    ui->graphicsView->setCurrentBestSnake ( hig_score_id );
    textUpdate();
    ui->label_count->setText(QString::number(0));

}







void MainWindow::on_doubleSpinBox_speed_editingFinished()
{
    for (int i = 0; i < ui->graphicsView->getAi_count(); ++i) {
        ui->graphicsView->game->snakes[i]->setSpeed(ui->doubleSpinBox_speed->value());
    }
}

