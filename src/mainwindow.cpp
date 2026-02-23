#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "config/inputconfig.h"
#include <QFile>
#include <QTemporaryFile>

MainWindow::MainWindow(StartSettings s, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  gameViewWithGame =
      new GraphicsView(s, this, this->ui->comboBoxMutAlgorithm,
                       ui->doubleSpinBox_speed->value());
  gameViewWithGame->setSizePolicy(QSizePolicy::Policy::Expanding,
                                  QSizePolicy::Policy::Expanding);
  ui->widget->layout()->addWidget(gameViewWithGame);
  ui->splitter->setSizes(
      QList<int>{0, this->width() - ui->splitter->widget(2)->width(),
                 ui->splitter->widget(2)->width()});

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
        gameViewWithGame->game->population->netAt(0),
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
  ui->horizontalLayout_theme->insertWidget(
      0, diaUber->styleHandler()->getCombobox());

  ui->pushButton->setDisabled(true);

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
    // --- Seed ---
    QFile ff(prefix + "apple.seed");
    if (!ff.exists() || !ff.open(QFile::ReadOnly)) {
        ui->statusbar->showMessage("Warnung: " + prefix + "apple.seed nicht gefunden", 2000);
        qDebug() << "Warnung: " + prefix + "apple.seed nicht gefunden";
    } else {
        gameViewWithGame->game->gamefield->setSeed(ff.readAll().toULongLong());
        qDebug() << "New Seed: " << gameViewWithGame->game->gamefield->getSeed();
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

    if (!gameViewWithGame->game->population
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
    // --- Hide the ViewNet visualisation panel ---
    //ui->widget_2->hide();

    // --- Hide score/length/moves strip ---
    //ui->widget->hide();

    // --- Hide everything in the right scroll panel except the 4 keep-items ---
    // Keep: pushButtonStart, pushButton (stop), radioButtonShowGrid,
    //       radioButtonrays, doubleSpinBox_speed  (+ label_8 "Speed").
    const QList<QWidget*> toHide = {
        ui->line,   ui->line_2, ui->line_4, ui->line_5, ui->label_14, ui->label_max_moves, ui->score, ui->highscore, ui->laength, ui->moves_left,  diaUber->styleHandler()->getCombobox(), ui->line_8,
        ui->line_6, ui->line_7,
        ui->label,  ui->label_2, ui->label_3, ui->label_4,
        ui->label_5, ui->label_6, ui->label_7, ui->label_9,
        ui->label_count, ui->label_10_gen, ui->label_ai_num,
        ui->pushButton_2, ui->pushButton_3, ui->pushButton_4,
        ui->pushButton_7, ui->pushButton_8,
        ui->pushButton_export, ui->pushButton_import,
        ui->pushButton_ueber, ui->pushButton_updateWeights,
        ui->comboBoxMutAlgorithm,
        ui->doubleSpinBox_learn_rate, ui->doubleSpinBoxMutRange,
        ui->radioButtonreconnect, ui->radioBUpdateViewNet,
        ui->checkBoxresetapples,
        ui->highscore,
    };
    for (QWidget* w : toHide) w->hide();

    // --- Expand all splitter panes equally (widget(0) was collapsed at startup) ---
    ui->splitter->setSizes(QList<int>{1, 1, 1});

    // --- Set slow demo speed (≈ 8 steps/sec: 100/speed_game seconds per step) ---
    ui->doubleSpinBox_speed->setValue(800.0);
    for (int i = 0; i < gameViewWithGame->getAi_count(); ++i)
        gameViewWithGame->game->snakes[i]->setSpeed(800.0);

    //enale update viewnet
    ui->radioBUpdateViewNet->toggle();//true);

    // --- Auto-import the trained model ---
    if (!QFile(":/Snakes/Release4_Medi-21-Score-147-zikzak-taktik_snake.csv").exists()) {
        ui->statusbar->showMessage("Demo model not found:  snake.csv", 8000);
    } else if (importFromPrefix(":/Snakes/Release4_Medi-21-Score-147-zikzak-taktik_")) {
        ui->statusbar->showMessage("Demo model loaded \u2014 press Start to run.", 5000);
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

void MainWindow::on_pushButtonStart_clicked() {
  this->ui->pushButtonStart->setDisabled(true);
  textUpdate();
  gameViewWithGame->connectToSnake(gameViewWithGame->getConnected_to());
  gameViewWithGame->game->startAIs(gameViewWithGame->getConnected_to());
  ui->label_count->setText(QString::number(gameViewWithGame->getAi_count()));
  ui->pushButton->setDisabled(false);
}

void MainWindow::bestSnakeChanged(int id, int val, int leng) {
  ui->highscore->setText(" ID: " + QString::number(id) +
                         " -> Score: " + QString::number(val) +
                         " Länge: " + QString::number(leng));

  // if(ui->radioButtonAutoRate->isChecked()) {
  //     if(gameViewWithGame->game->population->getEvolutionNum() +
  //     ui->spinBox_aut_versch->value() > 100)
  //         gameViewWithGame->game->setMutation_rate(0.08);
  //     if(gameViewWithGame->game->population->getEvolutionNum() +
  //     ui->spinBox_aut_versch->value() > 200)
  //         gameViewWithGame->game->setMutation_rate(0.01);
  //     if(gameViewWithGame->game->population->getEvolutionNum() +
  //     ui->spinBox_aut_versch->value() > 300)
  //         gameViewWithGame->game->setMutation_rate(0.001);

  // }

  // update text to best
  textUpdate();

  // wait for mutation-> free time -> redraw weights!
  if (viewNet && ui->radioBUpdateViewNet->isChecked()) {
    viewNet->updateInputLabels(true, 30);
    viewNet->updateOutputLabels(true, true, 30);
    // Redraw and update weights
    viewNet->updateWeightsLabels();
  }

  this->ui->pushButtonStart->setDisabled(false);
  ui->pushButton->setDisabled(true);
}

void MainWindow::evolved() {
  this->ui->pushButtonStart->setDisabled(true);
  ui->pushButton->setDisabled(false);

  // Update settings:
  //    if(ui->radioButtonAutoRate->isChecked()) {
  //        ui->doubleSpinBox_learn_rate->setValue( 1.0 / ( 20 *
  //        std::pow(gameViewWithGame->game->population->getEvolutionNum() +
  //        ui->spinBox_aut_versch->value() - 0.9 , 0.8 ) ) );
  //        ui->doubleSpinBoxMutRange->setValue   (0.5 *   (1.0 /
  //        std::pow(gameViewWithGame->game->population->getEvolutionNum() +
  //        ui->spinBox_aut_versch->value()   , 0.2) ));
  //    }

  // update text
  textUpdate();
  ui->label_count->setText(QString::number(gameViewWithGame->getAi_count()));
  ui->label_10_gen->setText(
      QString::number(gameViewWithGame->game->population->getEvolutionCount()));
  if (viewNet)
    viewNet->changeNet(gameViewWithGame->game->population->netAt(
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

  if (viewNet && ui->doubleSpinBox_speed->value() < 10001.0) {
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
  if (viewNet) viewNet->changeNet(gameViewWithGame->game->population->netAt(id));
  ui->statusbar->showMessage("Du verfolgst nun AI-Snake " + QString::number(id),
                             1000);
  ui->label_ai_num->setText(QString::number(id));
}

void MainWindow::on_pushButton_clicked() {
  ui->pushButton->setDisabled(true);
  this->ui->pushButtonStart->setDisabled(false);

  gameViewWithGame->game->stop_and_reset();
  if (this->gameViewWithGame->game->isRunning())
    this->gameViewWithGame->game->quit();
  // ui->pushButton->setDisabled(false);
}

void MainWindow::on_doubleSpinBox_speed_editingFinished() {
  for (int i = 0; i < gameViewWithGame->getAi_count(); ++i) {
    gameViewWithGame->game->snakes[i]->setSpeed(
        ui->doubleSpinBox_speed->value());
  }
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

  //    for(int i = 0; i < 500; i++) {
  //        double in = 0.70;
  //        double out[2];
  //        double corect_v[] = {0.35, 0.65};

  //        std::cout << "\n" <<  i << ": -> IN: " << in << std::endl;
  //        gameViewWithGame->game->population->netAt(0)->feedForward( &in );
  //        gameViewWithGame->game->population->netAt(0)->getResults( out );
  //        std::cout << "  -> OUT: {" << out[0] << ", " << out[1]  << "}" <<
  //        std::endl;

  //        gameViewWithGame->game->population->netAt(0)->backProp( corect_v,
  //        0.85, 0.05 ); std::cout << "  -> CORRECT: {0.35, 0.65}" <<
  //        std::endl;

  //        gameViewWithGame->game->population->netAt(0)->feedForward( &in );
  //        gameViewWithGame->game->population->netAt(0)->getResults( out );
  //        std::cout << i << ": -> AFTER OUT: {" << out[0] << ", " << out[1] <<
  //        "}" << std::endl;

  ////        viewNet->updateInputLabels(true, 30);
  ////        viewNet->updateOutputLabels(true, true, 30);
  ////        viewNet->updateWeightsLabels();

  //        QApplication::processEvents();

  //        usleep(100 * (i % 100 == 0 ? 10000 : 1));

  //        double in2 = 0.20;
  //        double out2[2];
  //        double corect_v2[] = {0.8, 0.2};

  //        std::cout << "\n" <<  i << ": -> IN2: " << in2 << std::endl;
  //        gameViewWithGame->game->population->netAt(0)->feedForward( &in2 );
  //        gameViewWithGame->game->population->netAt(0)->getResults( out2 );
  //        std::cout << "  -> OUT2: {" << out2[0] << ", " << out2[1]  << "}" <<
  //        std::endl;

  //        gameViewWithGame->game->population->netAt(0)->backProp( corect_v2,
  //        0.85, 0.05 ); std::cout << "  -> CORRECT2: {0.8, 0.2}" << std::endl;

  //        gameViewWithGame->game->population->netAt(0)->feedForward( &in2 );
  //        gameViewWithGame->game->population->netAt(0)->getResults( out2 );
  //        std::cout << i << ": -> AFTER OUT2: {" << out2[0] << ", " << out2[1]
  //        << "}" << std::endl;

  ////        viewNet->updateInputLabels(true, 30);
  ////        viewNet->updateOutputLabels(true, true, 30);
  ////        viewNet->updateWeightsLabels();

  //        QApplication::processEvents();

  //        std::cout << " ERROR: " <<
  //        gameViewWithGame->game->population->netAt(0)->recentAverrageError()
  //        << std::endl;

  //        usleep(100 * (i % 100 == 0 ? 10000 : 1));
  //    }

  //    return;

  ui->pushButton->setDisabled(false);
  gameViewWithGame->game->stop_and_reset();   // stop any running AI/evo before player
  gameViewWithGame->game->startPlayer();
  gameViewWithGame->connectToSnake(gameViewWithGame->getConnected_to());
  gameViewWithGame->currentSnake()->startPlayer(gameViewWithGame->currentNet());
}

// void MainWindow::on_radioButtonAutoRate_clicked()
// {
// //
// ui->doubleSpinBox_learn_rate->setDisabled(ui->radioButtonAutoRate->isChecked());
// //
// ui->doubleSpinBoxMutRange->setDisabled(ui->radioButtonAutoRate->isChecked());
// //    if(ui->radioButtonAutoRate->isChecked()) {
// //        ui->doubleSpinBox_learn_rate->setValue(0.5);
// //        ui->doubleSpinBoxMutRange->setValue(0.6);
// //    }
// }

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
  gameViewWithGame->game->gamefield->popBack();
}

void MainWindow::on_pushButton_4_clicked() {
  gameViewWithGame->game->gamefield->addCornerApples();
}

void MainWindow::on_radioButtonShowGrid_clicked(bool checked) {
  if (checked)
    gameViewWithGame->grid->show();
  else
    gameViewWithGame->grid->hide();
}

void MainWindow::on_pushButton_ueber_clicked() { diaUber->show(); }

// #include "setupdialog.h"

void MainWindow::on_pushButton_7_clicked() {
  //    SetUpDialog a;
  //    a.exec();
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

    if (gameViewWithGame->game->population
            ->netAt(gameViewWithGame->game->getBest())
            ->saveTo(d.toStdString() + "_snake.csv") &&
        f.open(QFile::ReadWrite | QFile::Truncate) &&
        f.write(
            QString::number(this->gameViewWithGame->game->gamefield->getSeed())
                .toStdString()
                .c_str()) != -1) {
      ui->statusbar->showMessage(
          archOk ? "Erfoglreich gespeichert!" : "Gespeichert (arch.json fehlgeschlagen)!",
          2000);
      qDebug() << "Seed: "
               << this->gameViewWithGame->game->gamefield->getSeed();
    } else {
      ui->statusbar->showMessage("Speichern fehlgeschlagen!", 2000);
    }
  }
}
