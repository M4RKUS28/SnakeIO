#include "gamefield.h"
#include <QDateTime>
#include <QDebug>

GameField::GameField(int size)
    : seed(static_cast<size_t>(QDateTime::currentMSecsSinceEpoch())),
      randomGenerator(seed),
      size(size)
{
}

QPoint GameField::getApplePos(int num)
{
    QMutexLocker lock(&mutex);
    // Lazily generate apple positions on demand.
    while (applePos.size() <= num) {
        applePos += QPoint(randomGenerator.bounded(1, size),
                          randomGenerator.bounded(1, size));
    }
    return applePos.at(num);
}


void GameField::setSeed(size_t seed)
{
    this->reset(seed);
}

void GameField::reset(size_t seed)
{
    applePos.clear();
    if (seed != 0) {
        this->seed = seed;
    } else {
        this->seed = static_cast<size_t>(QDateTime::currentMSecsSinceEpoch());
    }
    randomGenerator.seed(this->seed);
}

void GameField::popBack()
{
    if (!applePos.isEmpty())
        applePos.pop_back();
}

void GameField::removeAppleAt(int index)
{
    if (index >= 0 && index < applePos.size())
        applePos.removeAt(index);
}

void GameField::addCornerApples()
{
    const QPoint centre(size / 2, size / 2);
    // Four corners, each followed by the centre so the snake always has
    // a reachable next apple.
    applePos += QPoint(1,    1);    applePos += centre;
    applePos += QPoint(1,    size); applePos += centre;
    applePos += QPoint(size, size); applePos += centre;
    applePos += QPoint(size, 1);    applePos += centre;
}






