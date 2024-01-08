#include "gamefield.h"
#include <QDateTime>



#include <QDebug>

GameField::GameField(int size)
    : randomGenerator( (this->seed = QDateTime::currentMSecsSinceEpoch()) ), size(size)
{

}

QPoint GameField::getApplePos(int num)
{
    QMutexLocker m_lock(&mutex);
    while(applePos.length() <= num) {
        applePos += QPoint(randomGenerator.bounded(1, size), randomGenerator.bounded(1, size));
        // qDebug() << "New Apple at: [ " << applePos.length()-1 << "]: " << applePos.back();
    }
    return applePos.at(num);
}

void GameField::setSeed(size_t seed)
{
    this->reset(seed);
}

size_t GameField::getSeed()
{
    return seed;
}

void GameField::reset(size_t seed)
{
    applePos.clear();
    randomGenerator.seed( (seed != 0) ? (this->seed = seed) : (this->seed = QDateTime::currentMSecsSinceEpoch()) );
}

void GameField::popBack()
{
    if(applePos.length())
        applePos.pop_back();
}

void GameField::addCornerApples()
{
    QPoint m = QPoint(size / 2, size / 2);
    applePos += QPoint(1, 1);
    applePos += m;
    applePos += QPoint(1, size);
    applePos += m;
    applePos += QPoint(size, size);
    applePos += m;
    applePos += QPoint(size, 1);
    applePos += m;
}

int GameField::getSize() const
{
    return size;
}







