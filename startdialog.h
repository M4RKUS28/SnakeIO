#ifndef STARTDIALOG_H
#define STARTDIALOG_H

#include <QDialog>


struct StartSettings {
    int ai_count;


};

namespace Ui {
class StartDialog;
}

class StartDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StartDialog(QWidget *parent = nullptr);
    ~StartDialog();
    enum START_MODE{
        SINGLE_PLAER = 0,
        PLAYER_VS_AI = 1,
        AI_TRAINING = 2
    };


    StartSettings getStartSettings() const;

private slots:
    void on_pushButtonStart_clicked();

    void on_radioButton_single_player_clicked();

    void on_radioButton_player_vs_ai_clicked();

    void on_radioButton_training_mode_clicked();

private:
    int changeMode(START_MODE sm);



private:

    Ui::StartDialog *ui;
};



#endif // STARTDIALOG_H
