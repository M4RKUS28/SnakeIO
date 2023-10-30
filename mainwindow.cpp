#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->doubleSpinBoxMutRange->setValue(ui->graphicsView->game->getMut_range());
    ui->doubleSpinBox_learn_rate->setValue(ui->graphicsView->game->getMutation_rate());

    ui->graphicsView->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    connect(ui->graphicsView, SIGNAL(textUpdateNeeded()), this, SLOT(textUpdate()));
    connect(ui->graphicsView, SIGNAL(fokus_changed(unsigned)), this, SLOT(newFokus(unsigned)));
    connect(ui->graphicsView->game, SIGNAL(finishedEvo()), this, SLOT(evolved()));
    connect(ui->graphicsView->game, SIGNAL(bestSnakeChanged(int,int,int)), this, SLOT(bestSnakeChanged(int,int,int)));
    connect(ui->graphicsView->game, SIGNAL(livingCountChanged(int)), this, SLOT(snakeCountChanged(int)));

    viewNetScene = new QGraphicsScene(this);
    ui->graphicsView_ViewNet->setScene(viewNetScene);
    viewNet = new ViewNet(ui->graphicsView->game->population->netAt(0), QRect(0, 0, 670, 800), 20, false);
    viewNetScene->addItem(viewNet);

    std::vector<std::string> labels;
    labels.push_back("↖ 🍎 ");
    labels.push_back("🡑 🍎 ");
    labels.push_back("↗ 🍎 ");
    labels.push_back("🡐 🍎 ");
    labels.push_back("🡒 🍎 ");
    labels.push_back("↙ 🍎 ");
    labels.push_back("🡓 🍎 ");
    labels.push_back("↘ 🍎 ");

    labels.push_back("");
    labels.push_back("🡑 🐍 ");
    labels.push_back("");
    labels.push_back("🡐 🐍 ");
    labels.push_back("🡒 🐍 ");
    labels.push_back("");
    labels.push_back("🡓 🐍 ");
    labels.push_back("");

    labels.push_back("");
    labels.push_back("🡑 🚧 ");
    labels.push_back("");
    labels.push_back("🡐 🚧 ");
    labels.push_back("🡒 🚧 ");
    labels.push_back("");
    labels.push_back("🡓 🚧 ");
    labels.push_back("");
    viewNet->setInputPraefix(labels);
    viewNet->updateInputLabels(true, 30);

    labels.clear();
    labels.push_back(" 🡑 Up");
    labels.push_back(" 🡓 Down");
    labels.push_back(" 🡒 Right");
    labels.push_back(" 🡐 Left");
    viewNet->setOutputSuffix(labels);
    viewNet->updateOutputLabels(true, true, 30);

    timer = this->startTimer(1);

    diaUber = new DialogUeber(QApplication::applicationDirPath() + "../SnakeIOMaintenanceTool.exe", "M$RKUS", "SnakeIo",
                              this->version, Qt::red, this, false, true);
    diaUber->setPixmap(QPixmap("://1200x600wa.png").scaled(128, 128));
    ui->horizontalLayout_theme->insertWidget(0, diaUber->styleHandler()->getCombobox());
}

MainWindow::~MainWindow()
{
    if(timer)
        this->killTimer(timer);
    ui->graphicsView->game->stop_and_reset();

    delete viewNet;
    delete ui;
    delete diaUber;
}

void MainWindow::on_pushButtonStart_clicked()
{
    textUpdate();
    ui->graphicsView->connectToSnake(ui->graphicsView->getConnected_to());
    ui->graphicsView->game->startAIs( ui->graphicsView->getConnected_to() );
    ui->label_count->setText(QString::number(ui->graphicsView->getAi_count()));
}


void MainWindow::bestSnakeChanged(int id, int val, int leng)
{
    ui->highscore->setText( " ID: " + QString::number(id) +  " -> Score: " + QString::number(val)
                           + " Länge: " + QString::number(leng));

    if(ui->radioButtonAutoRate->isChecked()) {
        if(ui->graphicsView->game->population->getEvolutionNum() + ui->spinBox_aut_versch->value() > 100)
            ui->graphicsView->game->setMutation_rate(0.08);
        if(ui->graphicsView->game->population->getEvolutionNum() + ui->spinBox_aut_versch->value() > 200)
            ui->graphicsView->game->setMutation_rate(0.01);
        if(ui->graphicsView->game->population->getEvolutionNum() + ui->spinBox_aut_versch->value() > 300)
            ui->graphicsView->game->setMutation_rate(0.001);

    }

    //update text to best
    textUpdate();

    //wait for mutation-> free time -> redraw weights!
    if(ui->radioBUpdateViewNet->isChecked()) {
        viewNet->updateInputLabels(true, 30);
        viewNet->updateOutputLabels(true, true, 30);
        //Redraw and update weights
        viewNet->updateWeightsLabels();
    }
}

void MainWindow::evolved()
{
    //Update settings:
//    if(ui->radioButtonAutoRate->isChecked()) {
//        ui->doubleSpinBox_learn_rate->setValue( 1.0 / ( 20 * std::pow(ui->graphicsView->game->population->getEvolutionNum() + ui->spinBox_aut_versch->value() - 0.9 , 0.8 ) ) );
//        ui->doubleSpinBoxMutRange->setValue   (0.5 *   (1.0 / std::pow(ui->graphicsView->game->population->getEvolutionNum() + ui->spinBox_aut_versch->value()   , 0.2) ));
//    }

    //update text
    textUpdate();
    ui->label_count->setText(QString::number(ui->graphicsView->getAi_count()));
    ui->label_10_gen->setText(QString::number(ui->graphicsView->game->population->getEvolutionNum()));
    viewNet->changeNet(ui->graphicsView->game->population->netAt(ui->graphicsView->game->getBest()));
}

void MainWindow::snakeCountChanged(int ic)
{
    ui->label_count->setText(QString::number(ic));
}

void MainWindow::textUpdate()
{
    ui->laength->setText(QString::number(ui->graphicsView->currentSnake()->getLegth()));
    ui->score->setText(QString::number(ui->graphicsView->currentSnake()->getScore()));
    ui->moves_left->setText(QString::number(ui->graphicsView->currentSnake()->getLeftMoves()));

    if(ui->doubleSpinBox_speed->value() < 10001.0) {
        if(ui->radioBUpdateViewNet->isChecked()) {
            viewNet->updateInputLabels(true, 30);
            viewNet->updateOutputLabels(true, true, 30);
        }
    }
}

void MainWindow::on_pushButton_updateWeights_clicked()
{
    viewNet->updateWeightsLabels();
}

void MainWindow::newFokus(unsigned int id)
{
    viewNet->changeNet(ui->graphicsView->game->population->netAt(id));
    ui->statusbar->showMessage("Du verfolgst nun AI-Snake " + QString::number(id), 1000);
    ui->label_ai_num->setText(QString::number(id));
}

void MainWindow::on_pushButton_clicked()
{
    ui->graphicsView->game->stop_and_reset();
}


void MainWindow::on_pushButton_store_clicked()
{
    if(ui->graphicsView->game->population->netAt(ui->graphicsView->game->getBest())->save_to("net_save.csv"))
        ui->statusbar->showMessage("Erfoglreich gespeichert!", 2000);
    else
        ui->statusbar->showMessage("Speichern fehlgeschlagen!", 2000);

}

void MainWindow::on_pushButton_load_clicked()
{
    ui->graphicsView->game->stop_and_reset();

    if(ui->graphicsView->game->population->netAt(ui->graphicsView->game->getBest())->load_from("net_save.csv"))
        ui->statusbar->showMessage("Erfoglreich geladen!", 2000);
    else {
        ui->statusbar->showMessage("Laden fehlgeschlagen!", 2000);
        return;
    }

    ui->graphicsView->connectToSnake(ui->graphicsView->getConnected_to());
    ui->graphicsView->game->do_evolution();
    ui->highscore->setText("LOADED AI ID: " + QString::number(ui->graphicsView->game->getBest()));
    textUpdate();
}


void MainWindow::on_doubleSpinBox_speed_editingFinished()
{
    for (int i = 0; i < ui->graphicsView->getAi_count(); ++i) {
        ui->graphicsView->game->snakes[i]->setSpeed(ui->doubleSpinBox_speed->value());
    }
}


void MainWindow::timerEvent(QTimerEvent *)
{
    QApplication::processEvents();
}

void MainWindow::on_radioButtonrays_clicked(bool checked)
{
    ui->graphicsView->setShowRays(checked);
}


void MainWindow::on_radioButtonreconnect_clicked(bool checked)
{
    ui->graphicsView->setRreconnect(checked);
}

#include "net.h"
#include "unistd.h"
#include <QApplication>

void MainWindow::on_pushButton_2_clicked()
{

//    for(int i = 0; i < 500; i++) {
//        double in = 0.70;
//        double out[2];
//        double corect_v[] = {0.35, 0.65};

//        std::cout << "\n" <<  i << ": -> IN: " << in << std::endl;
//        ui->graphicsView->game->population->netAt(0)->feedForward( &in );
//        ui->graphicsView->game->population->netAt(0)->getResults( out );
//        std::cout << "  -> OUT: {" << out[0] << ", " << out[1]  << "}" << std::endl;

//        ui->graphicsView->game->population->netAt(0)->backProp( corect_v, 0.85, 0.05 );
//        std::cout << "  -> CORRECT: {0.35, 0.65}" << std::endl;

//        ui->graphicsView->game->population->netAt(0)->feedForward( &in );
//        ui->graphicsView->game->population->netAt(0)->getResults( out );
//        std::cout << i << ": -> AFTER OUT: {" << out[0] << ", " << out[1]  << "}" << std::endl;


////        viewNet->updateInputLabels(true, 30);
////        viewNet->updateOutputLabels(true, true, 30);
////        viewNet->updateWeightsLabels();

//        QApplication::processEvents();

//        usleep(100 * (i % 100 == 0 ? 10000 : 1));

//        double in2 = 0.20;
//        double out2[2];
//        double corect_v2[] = {0.8, 0.2};

//        std::cout << "\n" <<  i << ": -> IN2: " << in2 << std::endl;
//        ui->graphicsView->game->population->netAt(0)->feedForward( &in2 );
//        ui->graphicsView->game->population->netAt(0)->getResults( out2 );
//        std::cout << "  -> OUT2: {" << out2[0] << ", " << out2[1]  << "}" << std::endl;

//        ui->graphicsView->game->population->netAt(0)->backProp( corect_v2, 0.85, 0.05 );
//        std::cout << "  -> CORRECT2: {0.8, 0.2}" << std::endl;

//        ui->graphicsView->game->population->netAt(0)->feedForward( &in2 );
//        ui->graphicsView->game->population->netAt(0)->getResults( out2 );
//        std::cout << i << ": -> AFTER OUT2: {" << out2[0] << ", " << out2[1]  << "}" << std::endl;

////        viewNet->updateInputLabels(true, 30);
////        viewNet->updateOutputLabels(true, true, 30);
////        viewNet->updateWeightsLabels();

//        QApplication::processEvents();


//        std::cout << " ERROR: " << ui->graphicsView->game->population->netAt(0)->recentAverrageError() << std::endl;

//        usleep(100 * (i % 100 == 0 ? 10000 : 1));
//    }


//    return;



    ui->graphicsView->connectToSnake(ui->graphicsView->getConnected_to());
    ui->graphicsView->currentSnake()->startPlayer(ui->graphicsView->currentNet());
}


void MainWindow::on_radioButtonAutoRate_clicked()
{
//    ui->doubleSpinBox_learn_rate->setDisabled(ui->radioButtonAutoRate->isChecked());
//    ui->doubleSpinBoxMutRange->setDisabled(ui->radioButtonAutoRate->isChecked());
//    if(ui->radioButtonAutoRate->isChecked()) {
//        ui->doubleSpinBox_learn_rate->setValue(0.5);
//        ui->doubleSpinBoxMutRange->setValue(0.6);
//    }
}


void MainWindow::on_doubleSpinBox_learn_rate_valueChanged(double arg1)
{
    ui->graphicsView->game->setMutation_rate(arg1);
}


void MainWindow::on_doubleSpinBoxMutRange_valueChanged(double arg1)
{
    ui->graphicsView->game->setMut_range(arg1);

}


void MainWindow::on_checkBoxresetapples_stateChanged(int arg1)
{
    ui->graphicsView->game->setDoResetFieldAfterEvo(arg1);
}


void MainWindow::on_pushButton_3_clicked()
{
    ui->graphicsView->game->gamefield->popBack();
}


void MainWindow::on_pushButton_4_clicked()
{
    ui->graphicsView->game->gamefield->addCornerApples();
}


void MainWindow::on_radioButtonShowGrid_clicked(bool checked)
{
    if(checked)
        ui->graphicsView->grid->show();
    else
        ui->graphicsView->grid->hide();
}



void MainWindow::on_pushButton_ueber_clicked()
{
    diaUber->show();
}

