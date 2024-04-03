#ifndef PVEMAINWINDOW_H
#define PVEMAINWINDOW_H

#include "src/graphicsview.h"
#include <QMainWindow>

namespace Ui {
class PvEMainWindow;
}

class PvEMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PvEMainWindow(StartSettings startset, QWidget *parent = nullptr);
    ~PvEMainWindow();
    int timer;

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();
private:
    Ui::PvEMainWindow *ui;
    GraphicsView * gameViewWithGame;
protected:
    virtual void timerEvent(QTimerEvent *event);
};

#endif // PVEMAINWINDOW_H
