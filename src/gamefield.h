#ifndef GAMEFIELD_H
#define GAMEFIELD_H

#include <QPoint>
#include <QVector>
#include <QRandomGenerator>
#include <QMutex>


//#define image_based


class GameField
{
public:
    GameField(int size);
    QPoint getApplePos(int num);

    void setSeed(size_t seed);
    size_t getSeed();

    void reset(size_t seed = 0);
    void popBack();
    void addCornerApples();

    int getSize() const;

private:
    QVector<QPoint> applePos;
    QMutex mutex;
    QRandomGenerator randomGenerator;
    size_t seed;

    int size;
};

#endif // GAMEFIELD_H
