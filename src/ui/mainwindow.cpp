#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "inputconfig.h"
#include <QFile>
#include <QTemporaryFile>
#include <QDialog>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <algorithm>
#include <cmath>

// Logarithmic mapping: slider 1–1000  →  speed 100–99 999 999 %
// speed = 100 * 10^( (value-1) / 999.0 * 6.0 )
static double sliderToSpeed(int value) {
    return 100.0 * std::pow(10.0, (value - 1) / 999.0 * 6.0);
}
static int speedToSlider(double speed) {
    return qBound(1, static_cast<int>(std::round(std::log10(speed / 100.0) / 6.0 * 999.0 + 1.0)), 1000);
}

MainWindow::MainWindow(StartSettings s, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  gameViewWithGame =
      new GraphicsView(s, this, this->ui->comboBoxMutAlgorithm,
                       sliderToSpeed(ui->sliderSpeed->value()));
  gameViewWithGame->setSizePolicy(QSizePolicy::Policy::Expanding,
                                  QSizePolicy::Policy::Expanding);

  ui->widgetGameScene->layout()->addWidget(gameViewWithGame);

  ui->splitter->setSizes(
      QList<int>{0, this->width() - ui->splitter->widget(2)->width(),
                 ui->splitter->widget(2)->width()});

  // update speed label immediately
  ui->labelSpeedVal->setText(QString("%1 %").arg(sliderToSpeed(ui->sliderSpeed->value()), 0, 'f', 0));

  ui->doubleSpinBoxMutRange->setValue(gameViewWithGame->game->getMut_range());

  ui->doubleSpinBox_learn_rate->setValue(
      gameViewWithGame->game->getMutation_rate());
  gameViewWithGame->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

  connect(gameViewWithGame, SIGNAL(textUpdateNeeded()), this,
          SLOT(textUpdate()));
  connect(gameViewWithGame, SIGNAL(fokus_changed(unsigned)), this,
          SLOT(newFokus(unsigned)));
  connect(gameViewWithGame->game, SIGNAL(finishedEvo()), this, SLOT(evolved()));
  connect(gameViewWithGame->game, SIGNAL(bestSnakeChanged(int, int, int)), this,
          SLOT(bestSnakeChanged(int, int, int)));
  connect(gameViewWithGame->game, SIGNAL(livingCountChanged(int)), this,
          SLOT(snakeCountChanged(int)));

  viewNetScene = new QGraphicsScene(this);
  ui->graphicsView_ViewNet->setScene(viewNetScene);

  const int connCount = InputConfig::totalConnections(s.networkConfig);
  if (connCount <= 2000) {
    viewNet = new ViewNet(
        gameViewWithGame->game->getPopulation()->netAt(0),
        QRect(0, 0, 670, 800), 20, false);
    viewNetScene->addItem(viewNet);

    // Input labels are derived from the NetworkConfig so they always match
    // the actual neuron layout regardless of preset or custom configuration.
    QStringList labels = InputConfig::generateInputLabels(s.networkConfig);
    viewNet->setInputPrefix(labels);
    viewNet->updateInputLabels(true, 30);

    labels.clear();
    labels << " 🡑 Up" << " 🡓 Down" << " 🡒 Right" << " 🡐 Left";
    viewNet->setOutputSuffix(labels);
    viewNet->updateOutputLabels(true, true, 30);
  } else {
    viewNet = nullptr;
    QGraphicsTextItem* msg = viewNetScene->addText(
        QString("Network visualization is disabled.\n\n"
                "The current architecture has %1 connections,\n"
                "which exceeds the limit of 2\u202F000.\n\n"
                "Reduce the number of input neurons or\n"
                "hidden layer sizes to enable visualization.")
            .arg(connCount));
    QFont f = msg->font();
    f.setPointSize(10);
    msg->setFont(f);
    msg->setDefaultTextColor(Qt::gray);
    msg->setPos(20, 40);
  }

  timer = this->startTimer(1);

  diaUber = new DialogUeber(
      QApplication::applicationDirPath() + "/../SnakeIOMaintenanceTool.exe",
      "M$RKUS", "SnakeIO", s.app_version, Qt::red, this, false, true);

  if (diaUber->updater()->getMajorVersion() != 1) {
    std::cerr << "Wrong Updater Lib Version !" << std::endl;
    exit(121);
  }

  diaUber->setPixmap(QPixmap("://docs/1200x600wa.png").scaled(128, 128));

  if (s.appmode == StartSettings::DEMO)
    setupDemoMode(s);
}

// ===========================================================================
//  setupDemoMode()
//  Called only when appmode == DEMO.
//  Hides all training/settings controls, auto-imports the trained CSV, and
//  sets a comfortable playback speed so the snake is easy to follow.
// ===========================================================================
// ===========================================================================
//  importFromPrefix()
//  Shared helper used by both the Import button and Demo auto-load.
//  Loads the apple seed and snake weights from <prefix>apple.seed /
//  <prefix>snake.csv.  Returns true on success; status-bar messages on error.
// ===========================================================================
bool MainWindow::importFromPrefix(const QString& prefix)
{
  // --- Load apple seed ---
    QFile ff(prefix + "apple.seed");
    if (!ff.exists() || !ff.open(QFile::ReadOnly)) {
        ui->statusbar->showMessage("Warnung: " + prefix + "apple.seed nicht gefunden", 2000);
        qDebug() << "Warnung: " + prefix + "apple.seed nicht gefunden";
    } else {
        gameViewWithGame->game->getGamefield()->setSeed(ff.readAll().toULongLong());
        qDebug() << "New Seed: " << gameViewWithGame->game->getGamefield()->getSeed();
    }

    // --- Weights: loadFrom() needs a real filesystem path.
    //     If the prefix points into Qt resources (":/" prefix), copy to a
    //     QTemporaryFile first so we can hand a normal path to loadFrom(). ---
    QString snakePath = prefix + "snake.csv";
    std::string loadPath;

    if (snakePath.startsWith(":/")) {
        QFile res(snakePath);
        if (!res.open(QFile::ReadOnly)) {
            ui->statusbar->showMessage("Laden fehlgeschlagen! (resource open)", 2000);
            return false;
        }
        QTemporaryFile tmp;
        tmp.setAutoRemove(false);   // keep alive until loadFrom() finishes
        if (!tmp.open()) {
            ui->statusbar->showMessage("Laden fehlgeschlagen! (tempfile)", 2000);
            return false;
        }
        tmp.write(res.readAll());
        tmp.flush();
        loadPath = tmp.fileName().toStdString();
        tmp.close();    // file stays on disk because AutoRemove is false
    } else {
        loadPath = snakePath.toStdString();
    }

    if (!gameViewWithGame->game->getPopulation()
            ->netAt(gameViewWithGame->game->getBest())
            ->loadFrom(loadPath)) {
        ui->statusbar->showMessage("Laden fehlgeschlagen!", 2000);
        // clean up temp file if we created one
        if (snakePath.startsWith(":/"))
            QFile::remove(QString::fromStdString(loadPath));
        return false;
    }

    // clean up temp file
    if (snakePath.startsWith(":/"))
        QFile::remove(QString::fromStdString(loadPath));

    gameViewWithGame->connectToSnake(gameViewWithGame->game->getBest());
    ui->highscore->setText("LOADED AI ID: " + QString::number(gameViewWithGame->game->getBest()));
    textUpdate();
    return true;
}

void MainWindow::setupDemoMode(const StartSettings& /*s*/)
{
    // --- Hide training-only group boxes ---
    ui->groupBoxStatus->hide();
    ui->groupBoxTraining->hide();
    ui->groupBoxSnake->hide();
    ui->groupBoxApple->hide();

    // --- Expand all splitter panes equally ---
    ui->splitter->setSizes(QList<int>{1, 1, 1});

    // --- Set comfortable demo speed (~800%) ---
    ui->sliderSpeed->setValue(speedToSlider(800.0));
    ui->labelSpeedVal->setText("800 %");
    for (int i = 0; i < gameViewWithGame->getAi_count(); ++i)
        gameViewWithGame->game->snakeAt(i)->setSpeed(800.0);

    // --- Enable network weight visualization ---
    ui->radioBUpdateViewNet->setChecked(true);

    // --- Change Start button label for Demo ---
    ui->pushButtonStartStop->setText("▶  Demo starten");

    // --- Auto-import the trained demo model ---
    if (!QFile(":/Snakes/Release4_Medi-21-Score-147-zikzak-taktik_snake.csv").exists()) {
        ui->statusbar->showMessage("Demo model not found: snake.csv", 8000);
    } else if (importFromPrefix(":/Snakes/Release4_Medi-21-Score-147-zikzak-taktik_")) {
        ui->statusbar->showMessage("Demo model geladen \u2014 Starten zum Abspielen.", 5000);
    }
}

MainWindow::~MainWindow() {
  if (timer)
    this->killTimer(timer);
  gameViewWithGame->game->stop_and_reset();

  delete viewNet;
  delete ui;
  delete diaUber;
}

void MainWindow::on_pushButtonStartStop_clicked() {
  if (!aiRunning) {
    // --- Start ---
    aiRunning = true;
    ui->pushButtonStartStop->setText("\u23F9  Stop");
    textUpdate();
    gameViewWithGame->connectToSnake(gameViewWithGame->getConnected_to());
    gameViewWithGame->game->startAIs(gameViewWithGame->getConnected_to());
    ui->label_count->setText(QString::number(gameViewWithGame->getAi_count()));
  } else {
    // --- Stop ---
    aiRunning = false;
    ui->pushButtonStartStop->setText("\u25B6  Start AIs");
    gameViewWithGame->game->stop_and_reset();
    if (gameViewWithGame->game->isRunning())
      gameViewWithGame->game->quit();
    textUpdate();
  }
}

// on_pushButtonHome_clicked removed — "Home" functionality moved to pushButton_7

void MainWindow::bestSnakeChanged(int id, int val, int leng) {
  ui->highscore->setText(" ID: " + QString::number(id) +
                         " -> Score: " + QString::number(val) +
                         " Länge: " + QString::number(leng));

  // Update text to best
  textUpdate();

  // After each generation, refresh the ViewNet weight labels.
  if (viewNet && ui->radioBUpdateViewNet->isChecked()) {
    viewNet->updateInputLabels(true, 30);
    viewNet->updateOutputLabels(true, true, 30);
    viewNet->updateWeightsLabels();
  }
}
void MainWindow::evolved() {
  // AIs are running again after evolution — keep Start/Stop in the correct state.
  aiRunning = true;
  ui->pushButtonStartStop->setText("\u23F9  Stop");

  textUpdate();
  ui->label_count->setText(QString::number(gameViewWithGame->getAi_count()));
  ui->label_10_gen->setText(
      QString::number(gameViewWithGame->game->getPopulation()->getEvolutionCount()));
  if (viewNet)
    viewNet->changeNet(gameViewWithGame->game->getPopulation()->netAt(
        gameViewWithGame->game->getBest()));
}

void MainWindow::snakeCountChanged(int ic) {
  ui->label_count->setText(QString::number(ic));
}

void MainWindow::textUpdate() {
  ui->laength->setText(
      QString::number(gameViewWithGame->currentSnake()->getLegth()));
  ui->score->setText(
      QString::number(gameViewWithGame->currentSnake()->getScore()));
  ui->moves_left->setText(
      QString::number(gameViewWithGame->currentSnake()->getLeftMoves()));
  ui->label_score->setText(
      QString::number(gameViewWithGame->currentSnake()->getScore()));
  ui->label_length->setText(
      QString::number(gameViewWithGame->currentSnake()->getLegth()));
  if (gameViewWithGame->currentSnake()->getMaxMoves() % 5 == 0)
    if (ui->label_max_moves->text() !=
        QString::number(gameViewWithGame->currentSnake()->getMaxMoves()))
      ui->label_max_moves->setText(
          QString::number(gameViewWithGame->currentSnake()->getMaxMoves()));

  if (viewNet && sliderToSpeed(ui->sliderSpeed->value()) < 10001.0) {
    if (ui->radioBUpdateViewNet->isChecked()) {
      viewNet->updateInputLabels(true, 30);
      viewNet->updateOutputLabels(true, true, 30);
    }
  }
}

void MainWindow::on_pushButton_updateWeights_clicked() {
  if (viewNet) viewNet->updateWeightsLabels();
}

void MainWindow::newFokus(unsigned int id) {
  if (viewNet) viewNet->changeNet(gameViewWithGame->game->getPopulation()->netAt(id));
  ui->statusbar->showMessage("Du verfolgst nun AI-Snake " + QString::number(id), 1000);
  ui->label_ai_num->setText(QString::number(id));
}

// (on_pushButton_clicked removed — Stop is now part of pushButtonStartStop toggle)

void MainWindow::on_sliderSpeed_valueChanged(int value) {
  const double speed = sliderToSpeed(value);
  ui->labelSpeedVal->setText(QString("%1 %").arg(speed, 0, 'f', 0));
  for (int i = 0; i < gameViewWithGame->getAi_count(); ++i)
    gameViewWithGame->game->snakeAt(i)->setSpeed(speed);
}

void MainWindow::timerEvent(QTimerEvent *) { QApplication::processEvents(); }

void MainWindow::on_radioButtonrays_clicked(bool checked) {
  gameViewWithGame->setShowRays(checked);
}

void MainWindow::on_radioButtonHiddenApple_clicked(bool checked) {
  gameViewWithGame->setHiddenApple(checked);
}

void MainWindow::on_radioButtonreconnect_clicked(bool checked) {
  gameViewWithGame->setRreconnect(checked);
}

#include "net.h"
#include <QApplication>

void MainWindow::on_pushButton_2_clicked() {
  // Stop any running AI/evo before entering player mode.
  gameViewWithGame->game->stop_and_reset();
  gameViewWithGame->game->startPlayer();
  gameViewWithGame->connectToSnake(gameViewWithGame->getConnected_to());
  gameViewWithGame->currentSnake()->startPlayer(gameViewWithGame->currentNet());

  // Mark as "running" so the Start/Stop button acts as Stop.
  aiRunning = true;
  ui->pushButtonStartStop->setText("\u23F9  Stop");

  // Give keyboard focus to the game view so arrow keys are received.
  gameViewWithGame->setFocus();
}

// void MainWindow::on_radioButtonAutoRate_clicked()
// Removed: automatic rate scheduling was never used in the current build.

void MainWindow::on_doubleSpinBox_learn_rate_valueChanged(double arg1) {
  gameViewWithGame->game->setMutation_rate(arg1);
}

void MainWindow::on_doubleSpinBoxMutRange_valueChanged(double arg1) {
  gameViewWithGame->game->setMut_range(arg1);
}

void MainWindow::on_checkBoxresetapples_stateChanged(int arg1) {
  gameViewWithGame->game->setDoResetFieldAfterEvo(arg1);
}

void MainWindow::on_pushButton_3_clicked() {
  gameViewWithGame->game->getGamefield()->popBack();
}

void MainWindow::on_pushButton_4_clicked() {
  gameViewWithGame->game->getGamefield()->addCornerApples();
}

void MainWindow::on_pushButton_8_clicked() {
  // Build a modal for viewing and editing the apple list
  QDialog dlg(this);
  dlg.setWindowTitle("Apfel-Liste bearbeiten");
  dlg.setMinimumSize(320, 400);

  auto* layout = new QVBoxLayout(&dlg);

  QLabel* info = new QLabel(
      "Äpfel werden in dieser Reihenfolge nacheinander gesetzt.\n"
      "Wähle Einträge aus und klicke 'Löschen' um sie zu entfernen.", &dlg);
  info->setWordWrap(true);
  layout->addWidget(info);

  auto* list = new QListWidget(&dlg);
  list->setSelectionMode(QAbstractItemView::ExtendedSelection);
  const QVector<QPoint>& apples = gameViewWithGame->game->getGamefield()->getApples();
  for (int i = 0; i < apples.size(); ++i) {
    list->addItem(QString("Apfel #%1:  (%2, %3)").arg(i).arg(apples[i].x()).arg(apples[i].y()));
  }
  layout->addWidget(list, 1);

  auto* btnRow = new QHBoxLayout;
  auto* delBtn   = new QPushButton("🗑  Ausgewählte löschen", &dlg);
  auto* closeBtn = new QPushButton("Schließen", &dlg);
  btnRow->addWidget(delBtn);
  btnRow->addStretch();
  btnRow->addWidget(closeBtn);
  layout->addLayout(btnRow);

  connect(delBtn, &QPushButton::clicked, &dlg, [&]() {
    // Collect selected rows in descending order so removal doesn't shift indices
    QList<int> rows;
    for (auto* item : list->selectedItems())
      rows.prepend(list->row(item));
    std::sort(rows.begin(), rows.end(), std::greater<int>());
    for (int row : rows) {
      gameViewWithGame->game->getGamefield()->removeAppleAt(row);
      delete list->takeItem(row);
    }
    // Renumber labels
    for (int i = 0; i < list->count(); ++i) {
      const QVector<QPoint>& a = gameViewWithGame->game->getGamefield()->getApples();
      if (i < a.size())
        list->item(i)->setText(QString("Apfel #%1:  (%2, %3)").arg(i).arg(a[i].x()).arg(a[i].y()));
    }
  });
  connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

  dlg.exec();
}

void MainWindow::on_radioButtonShowGrid_clicked(bool checked) {
  gameViewWithGame->setGridVisible(checked);
}

// on_pushButton_ueber_clicked removed — Über button moved to StartDialog

// #include "setupdialog.h"

void MainWindow::on_pushButton_7_clicked() {
  gameViewWithGame->game->stop_and_reset();
  this->close();
}

#include <QFileDialog>

void MainWindow::on_pushButton_import_clicked() {
  // Stopp running snakes
  gameViewWithGame->game->stop_and_reset();

  auto d = QFileDialog::getOpenFileName(this, "Import Snake");
  if (d.isEmpty()) {
    ui->statusbar->showMessage("Laden fehlgeschlagen!", 2000);
    return;
  }
  d.replace("_snake.csv", "_").replace("_apple.seed", "_");
  if (!QFile(d + "snake.csv").exists()) {
    ui->statusbar->showMessage(
        "Laden fehlgeschlagen! " + d + "snake.csv" + " nicht gefunden!", 2000);
    return;
  }

  if (importFromPrefix(d))
    ui->statusbar->showMessage("Erfoglreich geladen!", 2000);
}
void MainWindow::on_pushButton_export_clicked() {
  auto d = QFileDialog::getSaveFileName(this, "Export Pfad");
  QFile f(d + "_apple.seed");
  if (!d.isEmpty()) {
    // Write arch JSON
    QFile fArch(d + "_arch.json");
    bool archOk = fArch.open(QFile::ReadWrite | QFile::Truncate) &&
                  fArch.write(InputConfig::networkConfigToJson(
                                  gameViewWithGame->game->cfg)
                                  .toUtf8()) != -1;
    fArch.close();

    if (gameViewWithGame->game->getPopulation()
            ->netAt(gameViewWithGame->game->getBest())
            ->saveTo(d.toStdString() + "_snake.csv") &&
        f.open(QFile::ReadWrite | QFile::Truncate) &&
        f.write(
            QString::number(this->gameViewWithGame->game->getGamefield()->getSeed())
                .toStdString()
                .c_str()) != -1) {
      ui->statusbar->showMessage(
          archOk ? "Erfoglreich gespeichert!" : "Gespeichert (arch.json fehlgeschlagen)!",
          2000);
      qDebug() << "Seed: " << this->gameViewWithGame->game->getGamefield()->getSeed();
    } else {
      ui->statusbar->showMessage("Speichern fehlgeschlagen!", 2000);
    }
  }
}
