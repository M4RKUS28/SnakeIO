#ifndef STARTDIALOG_H
#define STARTDIALOG_H

#include <QDialog>
#include "config/inputconfig.h"

// ===========================================================================
//  StartSettings
//  Carries all user choices from the start dialog to the main window.
// ===========================================================================
struct StartSettings {
    enum APPMODE {
        TRAINING,
        SINGLE_PLAYER,
        PLAYER_VS_AI,
        EXIT
    } appmode = TRAINING;

    QString app_version;

    // Full neural-network / training configuration (only relevant for TRAINING).
    // ai_count is stored inside networkConfig.snakeCount for training mode.
    NetworkConfig networkConfig;

    // Convenience accessor — delegates to networkConfig for training, kept for
    // backward compatibility with code that still reads s.ai_count directly.
    int ai_count = 21;
};

class QComboBox;
class QSpinBox;
class QLabel;
class QTableWidget;
class QPushButton;

namespace Ui {
class StartDialog;
}

class StartDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StartDialog(QWidget *parent = nullptr);
    ~StartDialog();

    StartSettings getStartSettings();

private slots:
    // --- Mode selection buttons (wired via .ui) ---
    void on_pushButton_3_clicked();   // Player vs AI
    void on_pushButton_2_clicked();   // Single Player
    void on_pushButton_4_clicked();   // AI Training
    void on_pushButton_8_clicked();   // Exit

    // --- Training page slots (wired in code) ---
    void onPreConfigChanged(int index);
    void onFieldSizeChanged(int value);
    void onAddNeuron();
    void onRemoveNeuron();
    void onAddHiddenLayer();
    void onRemoveHiddenLayer();
    void onTrainingStart();
    void updateInputCountLabel();

private:
    // --- Training page setup ---
    void setupTrainingPage();
    void applyPreConfig(int presetIndex);
    void loadNetworkConfigToUI(const NetworkConfig& cfg);
    NetworkConfig readNetworkConfigFromUI() const;
    void populateInputRow(int row, const InputNeuronConfig& cfg);
    void addHiddenLayerRow(const HiddenLayerConfig& cfg);
    static QWidget* createHLine(QWidget* parent);

    // --- Training page widgets (built in code, owned by Qt parent chain) ---
    QComboBox*    preConfigCombo     = nullptr;
    QSpinBox*     fieldSizeSpinBox   = nullptr;
    QSpinBox*     snakeCountSpinBox  = nullptr;
    QLabel*       inputCountLabel    = nullptr;
    QTableWidget* inputTable         = nullptr;
    QTableWidget* hiddenLayerTable   = nullptr;
    QPushButton*  addNeuronBtn       = nullptr;
    QPushButton*  removeNeuronBtn    = nullptr;
    QPushButton*  addLayerBtn        = nullptr;
    QPushButton*  removeLayerBtn     = nullptr;

    StartSettings  startSettings;
    static QString version;
    Ui::StartDialog *ui;
};



#endif // STARTDIALOG_H
