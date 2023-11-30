#include "mainwindow.h"
#include "startdialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    StartDialog s;
    if(s.exec() == QDialog::DialogCode::Rejected)
        return 0;

    MainWindow w(s.getStartSettings());
    w.show();
    return a.exec();
}
