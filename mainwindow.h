#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGraphicsScene>
#include <QMainWindow>
#include <QMutex>
#include "viewnet.h"
#include "dialogueber.h"
#include <QRect>
#include "startdialog.h"
#include "graphicsview.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    const QString version = "1.1.2";

public:
    MainWindow( StartSettings s, QWidget *parent = nullptr);
    ~MainWindow();

    int timer;



private slots:
    void on_pushButtonStart_clicked();

    void bestSnakeChanged(int, int, int);
    void evolved();
    void snakeCountChanged(int ic);
    void textUpdate();
    void newFokus(unsigned id);

    void on_pushButton_clicked();

    void on_doubleSpinBox_speed_editingFinished();

    void on_radioButtonrays_clicked(bool checked);

    void on_radioButtonreconnect_clicked(bool checked);

    void on_pushButton_2_clicked();

    void on_pushButton_updateWeights_clicked();

    void on_doubleSpinBox_learn_rate_valueChanged(double arg1);

    void on_doubleSpinBoxMutRange_valueChanged(double arg1);

    void on_checkBoxresetapples_stateChanged(int arg1);

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_radioButtonShowGrid_clicked(bool checked);

    void on_pushButton_ueber_clicked();

    void on_pushButton_7_clicked();


    void on_pushButton_import_clicked();

    void on_pushButton_export_clicked();

private:
    Ui::MainWindow *ui;
    QGraphicsScene * viewNetScene;
    ViewNet * viewNet;
    DialogUeber * diaUber;
    GraphicsView * gameViewWithGame;


    // QObject interface
protected:
    virtual void timerEvent(QTimerEvent *event);
};


#endif // MAINWINDOW_H
