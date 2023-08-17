#include "gamefield.h"
#include <QDateTime>





GameField::GameField(int size)
    : randomGenerator(QDateTime::currentMSecsSinceEpoch()), size(size)
{

}

QPoint GameField::getApplePos(int num)
{
    QMutexLocker m_lock(&mutex);
    while(applePos.length() <= num)
        applePos += QPoint(randomGenerator.bounded(1, size), randomGenerator.bounded(1, size));
    return applePos.at(num);
}

void GameField::reset()
{
    applePos.clear();
}

int GameField::getSize() const
{
    return size;
}







