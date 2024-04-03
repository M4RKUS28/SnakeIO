#include "startdialog.h"
#include "stylehandler.h"
#include "ui_startdialog.h"

#include <QSettings>

StartDialog::StartDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::StartDialog)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    startSettings.app_version = version;


    StyleHandler stylehandler("M$RKUS", "SnakeIO");
    QSettings settings("", "");
}

StartDialog::~StartDialog()
{
    delete ui;
}

StartSettings StartDialog::getStartSettings()
{
    startSettings.ai_count = ui->spinBox->value();
    return startSettings;
}


void StartDialog::on_pushButton_3_clicked()
{
    startSettings.appmode = StartSettings::APPMODE::PLAYER_VS_AI;
    this->accept();

    ui->stackedWidget->setCurrentIndex(1);
}


void StartDialog::on_pushButton_2_clicked()
{
    startSettings.appmode = StartSettings::APPMODE::SINGLE_PLAYER;
    ui->stackedWidget->setCurrentIndex(2);
    this->accept();
}


void StartDialog::on_pushButton_4_clicked()
{
    startSettings.appmode = StartSettings::APPMODE::TRAINING;
    ui->stackedWidget->setCurrentIndex(3);
}


void StartDialog::on_pushButton_7_clicked()
{
    this->accept();
}


void StartDialog::on_pushButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}


void StartDialog::on_pushButton_8_clicked()
{
    startSettings.appmode = StartSettings::APPMODE::EXIT;
    this->accept();
}

