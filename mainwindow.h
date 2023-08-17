#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMutex>


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

private:
    Ui::MainWindow *ui;


};
#endif // MAINWINDOW_H
