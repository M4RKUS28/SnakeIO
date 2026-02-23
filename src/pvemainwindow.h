#ifndef PVEMAINWINDOW_H
#define PVEMAINWINDOW_H

#include "graphicsview.h"
#include "startdialog.h"
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>

namespace Ui {
class PvEMainWindow;
}

// ===========================================================================
//  PvEMainWindow  —  Player vs AI
//
//  Two independent GraphicsView / Game instances side-by-side.
//  Left:  AI field (demo network, high speed)
//  Right: Player field (arrow keys, human speed)
//
//  State machine:
//    IDLE → RUNNING → PLAYER_DEAD (AI continues, speed ramps) → GAME_OVER
//                   → AI_DEAD_FIRST → GAME_OVER
// ===========================================================================
class PvEMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PvEMainWindow(StartSettings s, QWidget *parent = nullptr);
    ~PvEMainWindow();

    int timer = 0;

private slots:
    void onStartPause();
    void onReset();
    void onPlayerDied(int id);
    void onAiDied(int id);
    void updateScores();

private:
    // ------------------------------------------------------------------ UI
    void buildUi();
    bool loadDemoModel(GraphicsView* gv);   // auto-imports demo CSV

    // ---------------------------------------------------------------- State
    enum class State { IDLE, RUNNING, PLAYER_DEAD, GAME_OVER } state = State::IDLE;
    void setState(State s);
    void showResult(const QString& msg);
    void speedRamp();   // called each timer tick while PLAYER_DEAD

    Ui::PvEMainWindow *ui;
    StartSettings settings;

    GraphicsView* aiView    = nullptr;  // left field  (AI)
    GraphicsView* playerView = nullptr; // right field (player)

    // central-widget children
    QLabel*      aiScoreLabel      = nullptr;
    QLabel*      aiLengthLabel     = nullptr;
    QLabel*      playerScoreLabel  = nullptr;
    QLabel*      playerLengthLabel = nullptr;
    QLabel*      resultLabel       = nullptr;
    QPushButton* startPauseBtn     = nullptr;
    QPushButton* resetBtn          = nullptr;

    bool paused = false;
    double playerDeadRampSpeed = 800.0; // starts at player's normal speed

protected:
    void timerEvent(QTimerEvent*) override;
};

#endif // PVEMAINWINDOW_H
