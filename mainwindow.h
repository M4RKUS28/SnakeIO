#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGraphicsScene>
#include <QMainWindow>
#include <QMutex>
#include "viewnet.h"

#include <QRect>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool fins;
    int hig_score_id;
    int timer;



private slots:
    void on_pushButtonStart_clicked();

    void on_pushButton_gridHider_clicked();


    void restartAIs();

    void textUpdate();

    void updateCount();

    void on_pushButton_clicked();


    void on_pushButton_store_clicked();

    void on_pushButton_load_clicked();

    void on_doubleSpinBox_speed_editingFinished();

    void on_radioButtonrays_clicked(bool checked);

    void on_radioButtonreconnect_clicked(bool checked);

    void on_pushButton_2_clicked();

    void on_pushButton_updateWeights_clicked();

private:
    Ui::MainWindow *ui;
    QGraphicsScene * viewNetScene;
    ViewNet * viewNet;



    // QObject interface
protected:
    virtual void timerEvent(QTimerEvent *event);
};


#endif // MAINWINDOW_H
