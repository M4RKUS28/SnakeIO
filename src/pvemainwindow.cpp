#include "pvemainwindow.h"
#include "ui_pvemainwindow.h"

PvEMainWindow::PvEMainWindow(StartSettings startset, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PvEMainWindow)
{
    ui->setupUi(this);

    startset.ai_count = 1 + 600;
    int SPEED = 40000;
    timer = this->startTimer(1);

    gameViewWithGame = new GraphicsView(startset, this, nullptr, 20, 40, SPEED, true);
    gameViewWithGame->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    gameViewWithGame->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    ui->verticalLayoutGameField->addWidget(gameViewWithGame);


    connect(gameViewWithGame, SIGNAL(textUpdateNeeded()), this, SLOT(textUpdate()));
    connect(gameViewWithGame, SIGNAL(fokus_changed(unsigned)), this, SLOT(newFokus(unsigned)));
    connect(gameViewWithGame->game, SIGNAL(finishedEvo()), this, SLOT(evolved()));
    connect(gameViewWithGame->game, SIGNAL(bestSnakeChanged(int,int,int)), this, SLOT(bestSnakeChanged(int,int,int)));
    connect(gameViewWithGame->game, SIGNAL(livingCountChanged(int)), this, SLOT(snakeCountChanged(int)));
}

PvEMainWindow::~PvEMainWindow()
{
    delete ui;
}

void PvEMainWindow::on_pushButton_clicked()
{
    gameViewWithGame->game->startPlayer();

    ui->pushButton_2->setDisabled(false);
    ui->pushButton->setDisabled(true);


    for(int i = 0; i < gameViewWithGame->getAi_count(); i++) {
        if(i == gameViewWithGame->getConnected_to())
            continue;
        gameViewWithGame->game->population->netAt(i)->load_from("C:/Users/Markus/Nextcloud/CPP-Projekte/MachineLearning/SnakeIO/Snakes/Release4_Medi-21-Score-147-zikzak-taktik_snake.csv");
    }


    gameViewWithGame->connectToSnake(gameViewWithGame->getConnected_to());
    //start AI
    gameViewWithGame->game->startAIs( gameViewWithGame->getConnected_to() );


}

void PvEMainWindow::timerEvent(QTimerEvent *)
{
    QApplication::processEvents();
}

void PvEMainWindow::on_pushButton_2_clicked()
{
    ui->pushButton->setDisabled(false);
    ui->pushButton_2->setDisabled(true);
}

void textUpdate()
{
    // ui->laength->setText(QString::number(gameViewWithGame->currentSnake()->getLegth()));
    // ui->score->setText(QString::number(gameViewWithGame->currentSnake()->getScore()));
    // ui->moves_left->setText(QString::number(gameViewWithGame->currentSnake()->getLeftMoves()));
    // ui->label_score->setText(QString::number(gameViewWithGame->currentSnake()->getScore()));
    // ui->label_length->setText(QString::number(gameViewWithGame->currentSnake()->getLegth()));
    // if(gameViewWithGame->currentSnake()->getMaxMoves() % 5 == 0)
    //     if(ui->label_max_moves->text() != QString::number(gameViewWithGame->currentSnake()->getMaxMoves()))
    //         ui->label_max_moves->setText(QString::number(gameViewWithGame->currentSnake()->getMaxMoves()));

    // if(ui->doubleSpinBox_speed->value() < 10001.0) {
    //     if(ui->radioBUpdateViewNet->isChecked()) {
    //         viewNet->updateInputLabels(true, 30);
    //         viewNet->updateOutputLabels(true, true, 30);
    //     }
    // }
}
