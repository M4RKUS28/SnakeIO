#ifndef STARTDIALOG_H
#define STARTDIALOG_H

#include <QDialog>



struct StartSettings {
    int ai_count;
    enum APPMODE {
        TRAINING,
        SINGLE_PLAYER,
        PLAYER_VS_AI,
        EXIT
    } appmode;
    QString app_version;
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

    StartSettings getStartSettings();

private slots:

    void on_pushButton_3_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_clicked();

    void on_pushButton_8_clicked();

private:
    StartSettings startSettings;
    static QString version ;
    Ui::StartDialog *ui;
};



#endif // STARTDIALOG_H
