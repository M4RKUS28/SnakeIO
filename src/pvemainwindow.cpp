#include "pvemainwindow.h"
#include "ui_pvemainwindow.h"

#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QRandomGenerator>
#include <QTemporaryFile>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFont>
#include <QSizePolicy>

// ===========================================================================
//  Constants
// ===========================================================================
static constexpr double PLAYER_SPEED     = 600.0;    // human-playable — 25% slower than original 800
static constexpr double AI_SPEED         = 20000.0;  // max AI speed after ramp
static constexpr double RAMP_MULTIPLIER  = 1.5;      // speed multiplier per timer tick after player dies
static constexpr double RAMP_MAX_SPEED   = 100000.0;
static constexpr int    RAMP_TICK_MS     = 300;      // how often to increase speed

// ===========================================================================
//  Helper: copy a Qt resource CSV to a temp file and load it into the net
// ===========================================================================
static bool loadNetFromResource(GraphicsView* gv, const QString& resPath)
{
    QFile res(resPath);
    if (!res.open(QFile::ReadOnly)) return false;

    QTemporaryFile tmp;
    tmp.setAutoRemove(false);
    if (!tmp.open()) return false;
    tmp.write(res.readAll());
    tmp.flush();
    QString tmpPath = tmp.fileName();
    tmp.close();

    bool ok = gv->game->population
                  ->netAt(gv->game->getBest())
                  ->loadFrom(tmpPath.toStdString());
    QFile::remove(tmpPath);

    // Randomize apple seed so AI doesn't play on a fixed known board
    gv->game->gamefield->setSeed(
        static_cast<size_t>(QRandomGenerator::global()->generate64()));

    return ok;
}

// ===========================================================================
//  Constructor
// ===========================================================================
PvEMainWindow::PvEMainWindow(StartSettings s, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::PvEMainWindow), settings(s)
{
    ui->setupUi(this);
    setWindowTitle("Player vs AI");

    buildUi();

    // Load demo model into AI field
    if (!loadNetFromResource(aiView,
            ":/Snakes/Release4_Medi-21-Score-147-zikzak-taktik_snake.csv")) {
        resultLabel->setText("Warning: demo model not found");
    }

    // Give focus to the player field so arrow keys work immediately
    playerView->setFocus();

    timer = this->startTimer(1);
}

PvEMainWindow::~PvEMainWindow()
{
    delete ui;
}

// ===========================================================================
//  buildUi()  —  construct both fields + buttons programmatically
// ===========================================================================
void PvEMainWindow::buildUi()
{
    // ---- Make demo NetworkConfig (same as Demo mode: fieldSize=20, 1 snake) ----
    settings.networkConfig = NetworkConfig::makeDemo(20, 1);
    settings.ai_count = 1;

    // ---- AI field ----
    StartSettings aiSettings = settings;
    aiView = new GraphicsView(aiSettings, this, nullptr, PLAYER_SPEED);
    aiView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    aiView->setFocusPolicy(Qt::NoFocus);   // AI doesn't need keyboard

    // ---- Player field ----
    StartSettings playerSettings = settings;
    playerView = new GraphicsView(playerSettings, this, nullptr, PLAYER_SPEED);
    playerView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    playerView->setFocusPolicy(Qt::StrongFocus);

    // ---- Score labels ----
    auto makeTitle = [](const QString& txt) {
        QLabel* l = new QLabel(txt);
        QFont f = l->font(); f.setPointSize(14); f.setBold(true); l->setFont(f);
        l->setAlignment(Qt::AlignCenter);
        return l;
    };

    QLabel* aiTitle     = makeTitle("🤖 AI");
    QLabel* playerTitle = makeTitle("👤 Du");

    aiScoreLabel     = new QLabel("Score: 0");
    aiLengthLabel    = new QLabel("Länge: 0");
    playerScoreLabel = new QLabel("Score: 0");
    playerLengthLabel= new QLabel("Länge: 0");
    for (QLabel* lbl : {aiScoreLabel, aiLengthLabel, playerScoreLabel, playerLengthLabel}) {
        QFont f = lbl->font(); f.setPointSize(13); lbl->setFont(f);
        lbl->setAlignment(Qt::AlignCenter);
    }

    // ---- Buttons ----
    startPauseBtn  = new QPushButton("▶  Start");
    resetBtn       = new QPushButton("↺  Reset");
    homeBtn        = new QPushButton("🏠  Home");
    hiddenAppleChk = new Switch("Apfel verstecken");
    QFont bf = startPauseBtn->font(); bf.setPointSize(12); bf.setBold(true);
    startPauseBtn->setFont(bf);
    resetBtn->setFont(bf);
    homeBtn->setFont(bf);
    QFont cf = hiddenAppleChk->font(); cf.setPointSize(12); hiddenAppleChk->setFont(cf);
    startPauseBtn->setMinimumHeight(40);
    resetBtn->setMinimumHeight(40);
    homeBtn->setMinimumHeight(40);
    hiddenAppleChk->setMinimumHeight(40);

    // ---- Result label ----
    resultLabel = new QLabel("");
    QFont rf = resultLabel->font(); rf.setPointSize(16); rf.setBold(true); resultLabel->setFont(rf);
    resultLabel->setAlignment(Qt::AlignCenter);

    // ---- Column layouts ----
    auto makeCol = [](QLabel* title, QLabel* score, QLabel* length, GraphicsView* gv) {
        QVBoxLayout* col = new QVBoxLayout;
        col->addWidget(title);
        col->addWidget(score);
        col->addWidget(length);
        col->addWidget(gv, 1);
        return col;
    };

    QHBoxLayout* fieldsRow = new QHBoxLayout;
    fieldsRow->addLayout(makeCol(aiTitle,     aiScoreLabel,     aiLengthLabel,     aiView));
    fieldsRow->addLayout(makeCol(playerTitle, playerScoreLabel, playerLengthLabel, playerView));

    QHBoxLayout* btnRow = new QHBoxLayout;
    btnRow->addWidget(homeBtn);
    btnRow->addStretch();
    btnRow->addWidget(startPauseBtn);
    btnRow->addWidget(resetBtn);
    btnRow->addStretch();
    btnRow->addWidget(hiddenAppleChk);

    QVBoxLayout* root = new QVBoxLayout;
    root->addLayout(fieldsRow, 1);
    root->addWidget(resultLabel);
    root->addLayout(btnRow);

    QWidget* central = new QWidget(this);
    central->setLayout(root);
    setCentralWidget(central);

    // ---- Connections ----
    connect(startPauseBtn,  &QPushButton::clicked,   this, &PvEMainWindow::onStartPause);
    connect(resetBtn,       &QPushButton::clicked,   this, &PvEMainWindow::onReset);
    connect(homeBtn,        &QPushButton::clicked,   this, &PvEMainWindow::onHome);
    connect(hiddenAppleChk, &Switch::toggled,      this, &PvEMainWindow::onHiddenAppleToggled);

    // Score updates
    connect(aiView,     &GraphicsView::textUpdateNeeded, this, &PvEMainWindow::updateScores);
    connect(playerView, &GraphicsView::textUpdateNeeded, this, &PvEMainWindow::updateScores);

    // Death signals: initial connection (re-connected in onStartPause before each game)
    connect(aiView->game->snakes[0],     &Snake::died, this, &PvEMainWindow::onAiDied);
    connect(playerView->game->snakes[0], &Snake::died, this, &PvEMainWindow::onPlayerDied);

    setState(State::IDLE);
}

// ===========================================================================
//  State machine
// ===========================================================================
void PvEMainWindow::setState(State s)
{
    state = s;
    switch (s) {
    case State::IDLE:
        startPauseBtn->setText("▶  Start");
        startPauseBtn->setEnabled(true);
        resetBtn->setEnabled(true);
        resultLabel->setText("");
        resultLabel->setStyleSheet("");
        break;
    case State::RUNNING:
        startPauseBtn->setText("⏸  Pause");
        startPauseBtn->setEnabled(true);
        paused = false;
        break;
    case State::PLAYER_DEAD:
        startPauseBtn->setText("⏸  Pause");
        resultLabel->setText("Du bist ausgeschieden — AI läuft weiter...");
        resultLabel->setStyleSheet("color: orange;");
        playerDeadRampSpeed = PLAYER_SPEED;  // ramp starts from player speed baseline
        break;
    case State::GAME_OVER:
        startPauseBtn->setText("▶  Neu starten");
        startPauseBtn->setEnabled(true);
        break;
    }
}

void PvEMainWindow::showResult(const QString& msg)
{
    resultLabel->setText(msg);
    resultLabel->setStyleSheet("color: green; font-weight: bold;");
}

// ===========================================================================
//  Slots
// ===========================================================================
void PvEMainWindow::onStartPause()
{
    if (state == State::IDLE || state == State::GAME_OVER) {
        // stop & re-connect death signals before starting
        aiView->game->stop_and_reset();
        playerView->game->stop_and_reset();

        disconnect(aiView->game->snakes[0],     &Snake::died, this, nullptr);
        disconnect(playerView->game->snakes[0], &Snake::died, this, nullptr);
        connect(aiView->game->snakes[0],     &Snake::died, this, &PvEMainWindow::onAiDied,     Qt::QueuedConnection);
        connect(playerView->game->snakes[0], &Snake::died, this, &PvEMainWindow::onPlayerDied, Qt::QueuedConnection);

        // start AI — same speed as player; ramps up only after player dies
        loadNetFromResource(aiView,
            ":/Snakes/Release4_Medi-21-Score-147-zikzak-taktik_snake.csv");
        aiView->game->snakes[0]->setSpeed(PLAYER_SPEED);
        aiView->connectToSnake(0);
        aiView->game->startAIs(0);

        // start player
        playerView->game->startPlayer();
        playerView->connectToSnake(0);
        playerView->currentSnake()->startPlayer(playerView->currentNet());
        playerView->currentSnake()->setSpeed(PLAYER_SPEED);

        playerView->setFocus();
        setState(State::RUNNING);
        return;
    }

    if (state == State::RUNNING || state == State::PLAYER_DEAD) {
        paused = !paused;
        if (paused) {
            startPauseBtn->setText("▶  Weiter");
            // Note: snakes run on their own threads; we just stop ticking UI
            // Real pause would need Game-level support — mark visually for now
        } else {
            startPauseBtn->setText("⏸  Pause");
        }
    }
}

void PvEMainWindow::onReset()
{
    // stop everything
    aiView->game->stop_and_reset();
    playerView->game->stop_and_reset();

    updateScores();
    paused = false;
    setState(State::IDLE);
}

void PvEMainWindow::onHome()
{
    aiView->game->stop_and_reset();
    playerView->game->stop_and_reset();
    close();  // main.cpp loop will re-show StartDialog
}

void PvEMainWindow::onHiddenAppleToggled(bool checked)
{
    aiView->setHiddenApple(checked);
    playerView->setHiddenApple(checked);
}

void PvEMainWindow::onPlayerDied(int /*id*/)
{
    if (state != State::RUNNING) return;
    setState(State::PLAYER_DEAD);
}

void PvEMainWindow::onAiDied(int /*id*/)
{
    if (state == State::IDLE || state == State::GAME_OVER) return;

    if (state == State::RUNNING) {
        // Player still alive → restart AI at normal speed, game continues
        aiView->game->stop_and_reset();
        loadNetFromResource(aiView,
            ":/Snakes/Release4_Medi-21-Score-147-zikzak-taktik_snake.csv");
        disconnect(aiView->game->snakes[0], &Snake::died, this, nullptr);
        connect(aiView->game->snakes[0], &Snake::died, this, &PvEMainWindow::onAiDied,
                Qt::QueuedConnection);
        aiView->game->snakes[0]->setSpeed(PLAYER_SPEED);
        aiView->connectToSnake(0);
        aiView->game->startAIs(0);
        return;
    }

    // state == PLAYER_DEAD → AI finally dies too → GAME_OVER
    const size_t aiScore     = aiView->game->snakes[0]->getScore();
    const size_t playerScore = playerView->game->snakes[0]->getScore();

    // Stop both games cleanly
    aiView->game->stop_and_reset();
    playerView->game->stop_and_reset();

    setState(State::GAME_OVER);

    if (playerScore > aiScore)
        showResult(QString("🎉 Du gewinnst!  Du: %1  |  AI: %2").arg(playerScore).arg(aiScore));
    else if (aiScore > playerScore)
        showResult(QString("🤖 AI gewinnt!  AI: %1  |  Du: %2").arg(aiScore).arg(playerScore));
    else
        showResult(QString("🤝 Unentschieden!  Score: %1").arg(aiScore));
}

void PvEMainWindow::updateScores()
{
    aiScoreLabel->setText(
        QString("Score: %1").arg(aiView->game->snakes[0]->getScore()));
    aiLengthLabel->setText(
        QString("Länge: %1").arg(aiView->game->snakes[0]->getLegth()));
    playerScoreLabel->setText(
        QString("Score: %1").arg(playerView->game->snakes[0]->getScore()));
    playerLengthLabel->setText(
        QString("Länge: %1").arg(playerView->game->snakes[0]->getLegth()));
}

// ===========================================================================
//  Speed ramp after player dies
// ===========================================================================
void PvEMainWindow::speedRamp()
{
    if (playerDeadRampSpeed < RAMP_MAX_SPEED) {
        playerDeadRampSpeed = qMin(playerDeadRampSpeed * RAMP_MULTIPLIER, RAMP_MAX_SPEED);
        aiView->game->snakes[0]->setSpeed(playerDeadRampSpeed);
    }
}

// ===========================================================================
//  Timer
// ===========================================================================
void PvEMainWindow::timerEvent(QTimerEvent*)
{
    QApplication::processEvents();

    // Speed ramp logic: called every RAMP_TICK_MS after player died
    static qint64 lastRamp = 0;
    if (state == State::PLAYER_DEAD && !paused) {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (now - lastRamp >= RAMP_TICK_MS) {
            lastRamp = now;
            speedRamp();
        }
    }
}