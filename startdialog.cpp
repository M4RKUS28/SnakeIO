#include "startdialog.h"
#include "ui_startdialog.h"

#include <QSettings>
#include <iostream>
#include <ostream>

StartDialog::StartDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::StartDialog)
{
    ui->setupUi(this);


    QSettings settings("", "");


}

StartDialog::~StartDialog()
{
    delete ui;
}

StartSettings StartDialog::getStartSettings() const
{
    StartSettings startSettings;
    startSettings.ai_count = ui->spinBox->value();
    std::cout << "HIER: " << startSettings.ai_count << std::endl;
    return startSettings;
}

int StartDialog::changeMode(START_MODE sm)
{
    // std::cout << ui->stackedWidget_left->count() << " sm: " << sm << std::endl;
    // ui->stackedWidget_left->setCurrentIndex(0);
    // ui->stackedWidget_right->setCurrentIndex(1);

     // ui->radioButton_single_player->setChecked(false);
     // ui->radioButton_player_vs_ai->setChecked(false);
     // ui->radioButton_training_mode->setChecked(false);



    switch (sm) {
    case SINGLE_PLAER:
        // ui->radioButton_single_player->setChecked(true);

        break;
    case PLAYER_VS_AI:
        // ui->radioButton_player_vs_ai->setChecked(true);

        break;
    case AI_TRAINING:
        // ui->radioButton_training_mode->setChecked(true);

        break;
    }




    return 0;
}

void StartDialog::on_pushButtonStart_clicked()
{
    this->accept();
}


void StartDialog::on_radioButton_single_player_clicked()
{
    changeMode(START_MODE::SINGLE_PLAER);
}


void StartDialog::on_radioButton_player_vs_ai_clicked()
{
    changeMode(START_MODE::PLAYER_VS_AI);
}


void StartDialog::on_radioButton_training_mode_clicked()
{
    changeMode(START_MODE::AI_TRAINING);
}

