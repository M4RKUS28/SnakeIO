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







