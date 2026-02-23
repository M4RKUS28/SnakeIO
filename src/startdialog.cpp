#include "startdialog.h"
#include "stylehandler.h"
#include "dialogueber.h"
#include "ui_startdialog.h"

#include <QSettings>
#include <QFont>
#include <QApplication>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QMessageBox>
#include <QScrollArea>
#include <QSplitter>
#include <QFrame>

// ===========================================================================
//  Constructor / Destructor
// ===========================================================================
StartDialog::StartDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::StartDialog)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    startSettings.app_version = version;

    StyleHandler stylehandler("M$RKUS", "SnakeIO");
    QSettings settings("", "");

    setupTrainingPage();

    // --- Add Demo button to the main menu page (page_7) ---
    // Insert it between the AI Training button and the Exit button.
    QWidget* menuPage = ui->stackedWidget->widget(0);
    QVBoxLayout* menuLayout = qobject_cast<QVBoxLayout*>(menuPage->layout());
    if (menuLayout) {
        QPushButton* demoBtn = new QPushButton(" Demo Mode", menuPage);
        QFont f = demoBtn->font();
        f.setPointSize(11);
        demoBtn->setFont(f);
        demoBtn->setMaximumWidth(300);
        QHBoxLayout* row = new QHBoxLayout;
        row->addWidget(demoBtn);
        // Insert before the last spacer (before the Exit row)
        // The last real item is the Exit button row; insert one slot before it.
        menuLayout->insertLayout(menuLayout->count() - 3, row);
        connect(demoBtn, &QPushButton::clicked, this, &StartDialog::onDemoClicked);

        // --- Über button (small, bottom-right footer) ---
        auto* uberBtn = new QPushButton("ℹ  Über SnakeIO", menuPage);
        QFont uf = uberBtn->font();
        uf.setPointSize(9);
        uberBtn->setFont(uf);
        uberBtn->setFlat(true);
        uberBtn->setStyleSheet("QPushButton { color: gray; border: none; } "
                               "QPushButton:hover { color: palette(text); }");
        QHBoxLayout* uberRow = new QHBoxLayout;
        uberRow->addStretch();
        uberRow->addWidget(uberBtn);
        menuLayout->addLayout(uberRow);

        connect(uberBtn, &QPushButton::clicked, this, [this]() {
            DialogUeber dlg(
                QApplication::applicationDirPath() + "/../SnakeIOMaintenanceTool.exe",
                "M$RKUS", "SnakeIO", version, Qt::red, this, false, false);
            dlg.setPixmap(QPixmap("://docs/1200x600wa.png").scaled(128, 128));
            dlg.exec();
        });
    }
}

StartDialog::~StartDialog()
{
    delete ui;
}

StartSettings StartDialog::getStartSettings()
{
    // Sync training config from UI if training mode was selected
    if (startSettings.appmode == StartSettings::TRAINING && inputTable)
        startSettings.networkConfig = readNetworkConfigFromUI();

    // Keep ai_count in sync for backward-compatibility
    startSettings.ai_count = startSettings.networkConfig.snakeCount;

    return startSettings;
}

// ===========================================================================
//  Helper: horizontal separator line
// ===========================================================================
QWidget* StartDialog::createHLine(QWidget* parent)
{
    QFrame* line = new QFrame(parent);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
}

// ===========================================================================
//  setupTrainingPage()
//  Builds the training configuration page entirely in code.
// ===========================================================================
void StartDialog::setupTrainingPage()
{
    QWidget* page = ui->stackedWidget->widget(3); // page_10

    // Outer layout: just holds the scroll area (zero margins so it fills the page)
    QVBoxLayout* outerLayout = qobject_cast<QVBoxLayout*>(page->layout());
    if (!outerLayout) {
        outerLayout = new QVBoxLayout(page);
    }
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // Scroll area wrapping an inner widget
    QScrollArea* scrollArea = new QScrollArea(page);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    outerLayout->addWidget(scrollArea);

    QWidget* inner = new QWidget;
    scrollArea->setWidget(inner);

    // All content goes into inner's layout
    QVBoxLayout* mainLayout = new QVBoxLayout(inner);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(6);

    // alias so rest of function is unchanged
    QWidget* p = inner;  // used below as "page" for parent

    // ---- Header -------------------------------------------------------
    QLabel* header = new QLabel("AI Training Configuration", p);
    header->setFont(QFont("", 10, QFont::Bold));
    mainLayout->addWidget(header);
    mainLayout->addWidget(createHLine(page));

    // ---- Preset + basic settings in a form ----------------------------
    QFormLayout* formLayout = new QFormLayout;
    formLayout->setLabelAlignment(Qt::AlignRight);

    preConfigCombo = new QComboBox(p);
    preConfigCombo->addItem("Classic  (24 inputs — ray cast)",       0);
    preConfigCombo->addItem("Full Field  (1 neuron per cell)",        1);
    preConfigCombo->addItem("Turn Mode  (11 inputs — relative)",      2);    preConfigCombo->addItem("Demo  (pre-rework classic, 24 inputs)",  4);    preConfigCombo->addItem("Custom  (edit manually)",                3);
    formLayout->addRow("Preset:", preConfigCombo);

    fieldSizeSpinBox = new QSpinBox(p);
    fieldSizeSpinBox->setRange(5, 50);
    fieldSizeSpinBox->setValue(20);
    fieldSizeSpinBox->setSuffix(" cells");
    formLayout->addRow("Field Size:", fieldSizeSpinBox);

    snakeCountSpinBox = new QSpinBox(p);
    snakeCountSpinBox->setRange(1, 9999);
    snakeCountSpinBox->setValue(2000);
    formLayout->addRow("Snake Count (AIs):", snakeCountSpinBox);

    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(createHLine(page));

    // ---- Splitter: Input | Hidden | Output ----------------------------
    QSplitter* splitter = new QSplitter(Qt::Vertical, p);
    splitter->setChildrenCollapsible(false);

    // -- Pane 1: Input neurons --
    QWidget* inputPane = new QWidget;
    QVBoxLayout* inputPaneLayout = new QVBoxLayout(inputPane);
    inputPaneLayout->setContentsMargins(0, 4, 0, 4);
    inputPaneLayout->setSpacing(4);

    QHBoxLayout* inputTitleRow = new QHBoxLayout;
    QLabel* inputHeader = new QLabel("Input Neurons");
    inputHeader->setFont(QFont("", 9, QFont::Bold));
    inputCountLabel = new QLabel("Total inputs: 0");
    inputTitleRow->addWidget(inputHeader);
    inputTitleRow->addWidget(inputCountLabel);
    inputTitleRow->addStretch();
    inputPaneLayout->addLayout(inputTitleRow);

    inputTable = new QTableWidget(0, 3);
    inputTable->setHorizontalHeaderLabels({"Feature", "Param", "Label Preview"});
    inputTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    inputTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    inputTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    inputTable->setSelectionMode(QAbstractItemView::SingleSelection);
    inputTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    inputPaneLayout->addWidget(inputTable, 1);

    QHBoxLayout* inputBtnRow = new QHBoxLayout;
    addNeuronBtn    = new QPushButton("+ Add Neuron");
    removeNeuronBtn = new QPushButton("— Remove Selected");
    inputBtnRow->addWidget(addNeuronBtn);
    inputBtnRow->addWidget(removeNeuronBtn);
    inputBtnRow->addStretch();
    inputPaneLayout->addLayout(inputBtnRow);

    splitter->addWidget(inputPane);

    // -- Pane 2: Hidden layers --
    QWidget* hiddenPane = new QWidget;
    QVBoxLayout* hiddenPaneLayout = new QVBoxLayout(hiddenPane);
    hiddenPaneLayout->setContentsMargins(0, 4, 0, 4);
    hiddenPaneLayout->setSpacing(4);

    QLabel* hiddenHeader = new QLabel("Hidden Layers (inner network)");
    hiddenHeader->setFont(QFont("", 9, QFont::Bold));
    hiddenPaneLayout->addWidget(hiddenHeader);

    hiddenLayerTable = new QTableWidget(0, 3);
    hiddenLayerTable->setHorizontalHeaderLabels({"Neurons", "Aggregation", "Activation"});
    hiddenLayerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    hiddenPaneLayout->addWidget(hiddenLayerTable, 1);

    QHBoxLayout* hiddenBtnRow = new QHBoxLayout;
    addLayerBtn    = new QPushButton("+ Add Layer");
    removeLayerBtn = new QPushButton("— Remove Layer");
    hiddenBtnRow->addWidget(addLayerBtn);
    hiddenBtnRow->addWidget(removeLayerBtn);
    hiddenBtnRow->addStretch();
    hiddenPaneLayout->addLayout(hiddenBtnRow);

    splitter->addWidget(hiddenPane);

    // -- Pane 3: Output layer (fixed, read-only) --
    QWidget* outputPane = new QWidget;
    QVBoxLayout* outputPaneLayout = new QVBoxLayout(outputPane);
    outputPaneLayout->setContentsMargins(0, 4, 0, 4);
    outputPaneLayout->setSpacing(4);

    QLabel* outputHeader = new QLabel("Output Layer (fixed)");
    outputHeader->setFont(QFont("", 9, QFont::Bold));
    outputPaneLayout->addWidget(outputHeader);

    QTableWidget* outputLayerTable = new QTableWidget(1, 3);
    outputLayerTable->setHorizontalHeaderLabels({"Neurons", "Aggregation", "Activation"});
    outputLayerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    outputLayerTable->setFixedHeight(
        outputLayerTable->horizontalHeader()->sizeHint().height() + 30);
    outputLayerTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    outputLayerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    outputLayerTable->setSelectionMode(QAbstractItemView::NoSelection);
    outputLayerTable->setFocusPolicy(Qt::NoFocus);

    auto makeOutputItem = [](const QString& text) -> QTableWidgetItem* {
        QTableWidgetItem* item = new QTableWidgetItem(text);
        item->setFlags(Qt::ItemIsEnabled);
        item->setForeground(QColor(150, 150, 150));
        item->setTextAlignment(Qt::AlignCenter);
        return item;
    };
    outputLayerTable->setItem(0, 0, makeOutputItem("4"));
    outputLayerTable->setItem(0, 1, makeOutputItem("SUM"));
    outputLayerTable->setItem(0, 2, makeOutputItem("SMAX  (↑ ↓ → ←)"));
    outputPaneLayout->addWidget(outputLayerTable);
    outputPaneLayout->addStretch();

    splitter->addWidget(outputPane);

    // Initial size hints: input gets most space, hidden medium, output compact
    splitter->setSizes({200, 150, 80});

    mainLayout->addWidget(splitter, 1);

    // ---- Navigation: Back + Start ------------------------------------
    mainLayout->addWidget(createHLine(page));
    QHBoxLayout* navRow = new QHBoxLayout;
    QPushButton* backBtn  = new QPushButton("<",     p);
    QPushButton* startBtn = new QPushButton("Start", p);
    backBtn->setFixedWidth(40);
    navRow->addWidget(backBtn);
    navRow->addWidget(startBtn, 1);
    mainLayout->addLayout(navRow);

    // ---- Connections -------------------------------------------------
    connect(preConfigCombo,    QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &StartDialog::onPreConfigChanged);
    connect(fieldSizeSpinBox,  QOverload<int>::of(&QSpinBox::valueChanged),
            this, &StartDialog::onFieldSizeChanged);
    connect(addNeuronBtn,    &QPushButton::clicked, this, &StartDialog::onAddNeuron);
    connect(removeNeuronBtn, &QPushButton::clicked, this, &StartDialog::onRemoveNeuron);
    connect(addLayerBtn,     &QPushButton::clicked, this, &StartDialog::onAddHiddenLayer);
    connect(removeLayerBtn,  &QPushButton::clicked, this, &StartDialog::onRemoveHiddenLayer);
    connect(backBtn,  &QPushButton::clicked,
            this, [this]{ ui->stackedWidget->setCurrentIndex(0); });
    connect(startBtn, &QPushButton::clicked, this, &StartDialog::onTrainingStart);

    // Load Classic preset as default
    applyPreConfig(0);
}

// ===========================================================================
//  applyPreConfig()  —  loads one of the three pre-configurations into the UI
// ===========================================================================
void StartDialog::applyPreConfig(int index)
{
    const int fieldSize  = fieldSizeSpinBox   ? fieldSizeSpinBox->value()  : 20;
    const int snakeCnt   = snakeCountSpinBox  ? snakeCountSpinBox->value() : 21;

    NetworkConfig cfg;
    switch (index) {
    case 0: cfg = NetworkConfig::makeClassic  (fieldSize, snakeCnt); break;
    case 1: cfg = NetworkConfig::makeFullField(fieldSize, snakeCnt); break;
    case 2: cfg = NetworkConfig::makeTurnMode (fieldSize, snakeCnt); break;    case 4: cfg = NetworkConfig::makeDemo     (fieldSize, snakeCnt); break;    default: return; // Custom — leave table as-is
    }

    loadNetworkConfigToUI(cfg);
}

// ===========================================================================
//  loadNetworkConfigToUI()
// ===========================================================================
void StartDialog::loadNetworkConfigToUI(const NetworkConfig& cfg)
{
    // Block signals to avoid spurious cellChanged during bulk fill
    inputTable->blockSignals(true);
    hiddenLayerTable->blockSignals(true);

    // --- Input neurons ---
    inputTable->setRowCount(0);
    int fieldSize = fieldSizeSpinBox ? fieldSizeSpinBox->value() : cfg.fieldSize;
    for (int i = 0; i < cfg.inputs.size(); ++i) {
        inputTable->insertRow(i);
        populateInputRow(i, cfg.inputs[i]);
        // Update label preview column
        QTableWidgetItem* labelItem = new QTableWidgetItem(
            InputConfig::neuronLabel(cfg.inputs[i], fieldSize));
        labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable);
        inputTable->setItem(i, 2, labelItem);
    }

    // --- Hidden layers ---
    hiddenLayerTable->setRowCount(0);
    for (const HiddenLayerConfig& layer : cfg.hiddenLayers)
        addHiddenLayerRow(layer);

    inputTable->blockSignals(false);
    hiddenLayerTable->blockSignals(false);

    updateInputCountLabel();
}

// ===========================================================================
//  populateInputRow()  —  fills one row of the input neuron table
// ===========================================================================
void StartDialog::populateInputRow(int row, const InputNeuronConfig& cfg)
{
    const int fieldSize = fieldSizeSpinBox ? fieldSizeSpinBox->value() : 20;
    const QVector<InputFeature> features = InputConfig::allFeatures();

    // Column 0: feature combo
    QComboBox* combo = new QComboBox;
    int selectedIdx = 0;
    for (int i = 0; i < features.size(); ++i) {
        const InputConfig::FeatureInfo info = InputConfig::getFeatureInfo(features[i], fieldSize);
        combo->addItem(info.displayName, static_cast<int>(features[i]));
        if (features[i] == cfg.feature) selectedIdx = i;
    }
    combo->setCurrentIndex(selectedIdx);
    inputTable->setCellWidget(row, 0, combo);

    // Update label preview when feature changes
    connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this, row]() {
        const int fieldSz = fieldSizeSpinBox ? fieldSizeSpinBox->value() : 20;
        QComboBox* cb   = qobject_cast<QComboBox*>(inputTable->cellWidget(row, 0));
        QSpinBox*  sb   = qobject_cast<QSpinBox*> (inputTable->cellWidget(row, 1));
        if (!cb) return;

        const InputFeature f = static_cast<InputFeature>(cb->currentData().toInt());
        const InputConfig::FeatureInfo info = InputConfig::getFeatureInfo(f, fieldSz);

        // Show/hide param spinbox
        if (sb) sb->setEnabled(info.hasParam);

        // Refresh label preview
        InputNeuronConfig nc;
        nc.feature = f;
        nc.param   = sb ? sb->value() : 0;
        auto* label = inputTable->item(row, 2);
        if (label) label->setText(InputConfig::neuronLabel(nc, fieldSz));
    });

    // Column 1: param spinbox (only relevant for CELL_AT_INDEX)
    QSpinBox* paramSpin = new QSpinBox;
    paramSpin->setMinimum(0);
    paramSpin->setMaximum(fieldSize * fieldSize - 1);
    paramSpin->setValue(cfg.param);
    const InputConfig::FeatureInfo info = InputConfig::getFeatureInfo(cfg.feature, fieldSize);
    paramSpin->setEnabled(info.hasParam);
    inputTable->setCellWidget(row, 1, paramSpin);

    // Column 2: label preview (read-only)
    QString lbl = InputConfig::neuronLabel(cfg, fieldSize);
    QTableWidgetItem* labelItem = new QTableWidgetItem(lbl);
    labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable);
    inputTable->setItem(row, 2, labelItem);
}

// ===========================================================================
//  addHiddenLayerRow()  —  fills one row of the hidden layer table
// ===========================================================================
void StartDialog::addHiddenLayerRow(const HiddenLayerConfig& cfg)
{
    const int row = hiddenLayerTable->rowCount();
    hiddenLayerTable->insertRow(row);

    // Column 0: neuron count spinbox
    QSpinBox* countSpin = new QSpinBox;
    countSpin->setRange(1, 9999);
    countSpin->setValue(cfg.neurons);
    hiddenLayerTable->setCellWidget(row, 0, countSpin);

    // Column 1: aggregation combo
    QComboBox* aggCombo = new QComboBox;
    for (const QString& opt : QStringList{"SUM","AVG","MAX","MIN"})
        aggCombo->addItem(opt);
    aggCombo->setCurrentText(cfg.aggregation);
    hiddenLayerTable->setCellWidget(row, 1, aggCombo);

    // Column 2: activation combo
    QComboBox* actCombo = new QComboBox;
    for (const QString& opt : QStringList{"RELU","TANH","SIGMOID","LEAKYRELU","SOFTPLUS","IDENTITY"})
        actCombo->addItem(opt);
    actCombo->setCurrentText(cfg.activation);
    hiddenLayerTable->setCellWidget(row, 2, actCombo);
}

// ===========================================================================
//  readNetworkConfigFromUI()  —  serialises the current table state to a Config
// ===========================================================================
NetworkConfig StartDialog::readNetworkConfigFromUI() const
{
    NetworkConfig cfg;
    cfg.fieldSize  = fieldSizeSpinBox  ? fieldSizeSpinBox->value()  : 20;
    cfg.snakeCount = snakeCountSpinBox ? snakeCountSpinBox->value() : 21;

    // --- Inputs ---
    for (int i = 0; i < inputTable->rowCount(); ++i) {
        QComboBox* cb = qobject_cast<QComboBox*>(inputTable->cellWidget(i, 0));
        QSpinBox*  sb = qobject_cast<QSpinBox*> (inputTable->cellWidget(i, 1));
        if (!cb) continue;
        InputNeuronConfig nc;
        nc.feature = static_cast<InputFeature>(cb->currentData().toInt());
        nc.param   = sb ? sb->value() : 0;
        cfg.inputs << nc;
    }

    // --- Hidden layers ---
    for (int i = 0; i < hiddenLayerTable->rowCount(); ++i) {
        QSpinBox*  nb = qobject_cast<QSpinBox*> (hiddenLayerTable->cellWidget(i, 0));
        QComboBox* ac = qobject_cast<QComboBox*>(hiddenLayerTable->cellWidget(i, 1));
        QComboBox* av = qobject_cast<QComboBox*>(hiddenLayerTable->cellWidget(i, 2));
        HiddenLayerConfig lc;
        lc.neurons     = nb ? nb->value()          : 25;
        lc.aggregation = ac ? ac->currentText()    : "SUM";
        lc.activation  = av ? av->currentText()    : "RELU";
        cfg.hiddenLayers << lc;
    }

    return cfg;
}

// ===========================================================================
//  Slots
// ===========================================================================
void StartDialog::onPreConfigChanged(int /*index*/)
{
    const int dataId = preConfigCombo ? preConfigCombo->currentData().toInt() : 0;
    if (dataId != 3) // 3 = Custom → no auto-apply
        applyPreConfig(dataId);
}

void StartDialog::onFieldSizeChanged(int /*value*/)
{
    // Re-apply current preset so FULL_FIELD re-expands to the new cell count.
    // For Custom, just refresh the param spin box ranges.
    const int dataId = preConfigCombo ? preConfigCombo->currentData().toInt() : 0;
    if (dataId != 3) {
        applyPreConfig(dataId);
    } else {
        // In custom mode: only update CELL_AT_INDEX param ranges + label previews
        const int fieldSize = fieldSizeSpinBox->value();
        for (int i = 0; i < inputTable->rowCount(); ++i) {
            QSpinBox* sb = qobject_cast<QSpinBox*>(inputTable->cellWidget(i, 1));
            if (sb) sb->setMaximum(fieldSize * fieldSize - 1);
        }
        updateInputCountLabel();
    }
}

void StartDialog::onAddNeuron()
{
    const int row = inputTable->rowCount();
    inputTable->insertRow(row);
    // Default: first feature in the list (FOOD_DIR_NW)
    populateInputRow(row, InputNeuronConfig{ InputFeature::FOOD_DIR_NW, 0 });
    // Switch preset combo to "Custom" (data value 3, regardless of list position)
    if (preConfigCombo) preConfigCombo->setCurrentIndex(preConfigCombo->findData(3));
    updateInputCountLabel();
}

void StartDialog::onRemoveNeuron()
{
    if (inputTable->rowCount() <= 1) {
        QMessageBox::warning(this, "Eingabe-Fehler",
            "Der Input-Layer muss mindestens 1 Neuron enthalten.");
        return;
    }
    const int row = inputTable->currentRow();
    if (row >= 0)
        inputTable->removeRow(row);
    if (preConfigCombo) preConfigCombo->setCurrentIndex(preConfigCombo->findData(3));
    updateInputCountLabel();
}

void StartDialog::onAddHiddenLayer()
{
    addHiddenLayerRow(HiddenLayerConfig{ 25, "SUM", "RELU" });
    if (preConfigCombo) preConfigCombo->setCurrentIndex(preConfigCombo->findData(3));
}

void StartDialog::onRemoveHiddenLayer()
{
    const int row = hiddenLayerTable->currentRow();
    if (row >= 0)
        hiddenLayerTable->removeRow(row);
    if (preConfigCombo) preConfigCombo->setCurrentIndex(preConfigCombo->findData(3));
}

void StartDialog::updateInputCountLabel()
{
    if (inputCountLabel)
        inputCountLabel->setText(
            QString("Total inputs: %1").arg(inputTable->rowCount()));
}

void StartDialog::onTrainingStart()
{
    if (inputTable && inputTable->rowCount() < 1) {
        QMessageBox::warning(this, "Eingabe-Fehler",
            "Der Input-Layer muss mindestens 1 Neuron enthalten.");
        return;
    }
    startSettings.networkConfig = readNetworkConfigFromUI();
    startSettings.ai_count      = startSettings.networkConfig.snakeCount;
    this->accept();
}

// ===========================================================================
//  Mode selection buttons (wired via Qt Designer auto-connect)
// ===========================================================================
void StartDialog::on_pushButton_3_clicked()   // Player vs AI
{
    startSettings.appmode = StartSettings::APPMODE::PLAYER_VS_AI;
    this->accept();
}

void StartDialog::on_pushButton_4_clicked()   // AI Training
{
    startSettings.appmode = StartSettings::APPMODE::TRAINING;
    ui->stackedWidget->setCurrentIndex(3);
    resize(1200, 800);
}

void StartDialog::on_pushButton_8_clicked()   // Exit
{
    startSettings.appmode = StartSettings::APPMODE::EXIT;
    this->accept();
}

void StartDialog::onDemoClicked()   // Demo Mode
{
    startSettings.appmode       = StartSettings::APPMODE::DEMO;
    startSettings.networkConfig = NetworkConfig::makeDemo(21, 1);  // 1 snake for demo
    startSettings.ai_count      = 1;
    this->accept();
}
