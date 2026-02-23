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
    void removeAppleAt(int index);

    /** @brief Read-only access to the full apple list (thread-unsafe, call only from UI thread). */
    const QVector<QPoint>& getApples() const { return applePos; }

    int getSize() const;

private:
    QVector<QPoint> applePos;
    QMutex mutex;
    QRandomGenerator randomGenerator;
    size_t seed;

    int size;
};

#endif // GAMEFIELD_H
