#ifndef GAMEFIELD_H
#define GAMEFIELD_H

#include <QPoint>
#include <QVector>
#include <QRandomGenerator>
#include <QMutex>


class GameField
{
public:
    GameField(int size);
    QPoint getApplePos(int num);

    void reset();

    int getSize() const;

private:
    QVector<QPoint> applePos;
    QMutex mutex;
    QRandomGenerator randomGenerator;

    int size;
};

#endif // GAMEFIELD_H
