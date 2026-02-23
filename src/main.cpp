#include "mainwindow.h"
#include "pvemainwindow.h"
#include "startdialog.h"
#include "stylehandler.h"
#include <QApplication>

QString StartDialog::version = "2.0.0";

int Snake::snake_init_length = 10;
int Snake::snake_init_moves = 250;
int Snake::snake_add_moves_per_apple = 100;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    StyleHandler globalStyle("M$RKUS", "SnakeIO");
    globalStyle.setStyle("windows11");

    StartDialog s;

    while(s.exec() != QDialog::DialogCode::Rejected && s.getStartSettings().appmode != StartSettings::APPMODE::EXIT) {
        switch(s.getStartSettings().appmode) {
        case StartSettings::TRAINING: {
            MainWindow w(s.getStartSettings());
            w.show();
            a.exec();
            break;
        } case StartSettings::DEMO: {
            MainWindow w(s.getStartSettings());
            w.show();
            a.exec();
            break;
        } case StartSettings::SINGLE_PLAYER: {
            break;
        } case StartSettings::PLAYER_VS_AI: {
            PvEMainWindow w(s.getStartSettings());
            w.show();
            a.exec();
            break;
        } case StartSettings::EXIT: {
            break;
        }}
    }
    return 0;
}

