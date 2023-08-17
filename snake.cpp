#include "snake.h"

#include <QPainterPath>

Snake::Snake(GameField *field, Net *net, QObject *parent, int num_id, double speed_game)
    : QThread(parent), field(field), net(net), num_id(num_id)
{
    if(!net)
        exit(123);

    isAI = false;
    fokus = false;
    reset();
    this->speed_game = speed_game;
}

Snake::~Snake()
{

}


void Snake::reset()
{
    isAI = false;
    richtung = QPoint(1, 0);
    pos.clear();
    for(int i = 0; i < 10; i++)
        pos.append(QPoint(field->getSize() / 2, field->getSize() / 2));

    moves = 250;
    foodNum = 0;
    lebt_noch = true;
    survive_time = 0;

//    emit textUpdate();
}

void Snake::run()
{
    foodNum = 0;
    currentFood = field->getApplePos(foodNum);
    if(fokus)
        emit foodPosChanged(currentFood);

    while (!isInterruptionRequested()) {

        double buffer[24];
        lookThingsUp(buffer, currentFood);
        net->feedForward(buffer);
        net->getResults(buffer);

        if(isAI) {
            int maxIndex = 0;
            double maxProbability = buffer[0];

            for (int i = 1; i < 4; ++i) {
                if (buffer[i] > maxProbability) {
                    maxProbability = buffer[i];
                    maxIndex = i;
                }
            }

            switch (maxIndex) {
            case 0:
                richtungAendern(QPoint(0, -1));
                break;
            case 1:
                richtungAendern(QPoint(0, 1));
                break;
            case 2:
                richtungAendern(QPoint(1, 0));
                break;
            case 3:
                richtungAendern(QPoint(-1, 0));
                break;
            }


        }
        if(fokus) {
            emit posChanged(pos, num_id);
            emit textUpdate();
        }
        usleep(1000000 * (100.0 / speed_game));

        //check outstanding moves
        if(moves <= 0) {
            emit died();
            fokus = false;
            lebt_noch = false;
            return;
        }

        QPoint newPos = pos.front() + richtung;

        //check if run into
        for(const auto & e : pos) {
            if(e == newPos) {
                emit died();
                lebt_noch = false;
                fokus = false;
                return;
            }
        }

        if(newPos.x() <= 0 || newPos.x() > field->getSize()
        || newPos.y() <= 0 || newPos.y() > field->getSize()) {
            emit died();
            lebt_noch = false;
            fokus = false;
            return;
        }

        //Move
        pos.prepend(newPos);
        pos.removeLast();
        moves--;


        //Try eat
        if(pos.first() == currentFood) {

//            qDebug() << "!!!!/*!*/GEGESSEN!!!";


            foodNum++;
            currentFood = field->getApplePos(foodNum);
            if(fokus)
                emit foodPosChanged(currentFood);


            moves += 100;

            //Wachse!!
            pos.append(pos.last());
        }
        survive_time++;

//        if(fokus) {
//            emit posChanged(pos, num_id);
//            emit textUpdate();
//        }
//        usleep(1000000 * (100.0 / speed_game));

    }


}

bool Snake::getFokus() const
{
    return fokus;
}

void Snake::setFokus(bool newFokus)
{
    fokus = newFokus;
}

void Snake::setSpeed(double speed_game)
{
    this->speed_game = speed_game;
}

QPoint Snake::getCurrentFood() const
{
    return currentFood;
}

void Snake::setCurrentFood(QPoint newCurrentFood)
{
    currentFood = newCurrentFood;
}



bool Snake::getLebt_noch() const
{
    return lebt_noch;
}

size_t Snake::getLeftMoves() const
{
    return moves;
}

size_t Snake::getScore() const
{
    return foodNum * 300 + survive_time;
}

void Snake::lookThingsUp( double *buffer, const QPoint &foodPos)
{
    /*

buffer[0]  = Links Oben    [ Essen ] = (0|1)
buffer[1]  = Oben          [ Essen ] = (0|1)
buffer[2]  = Rechts Oben   [ Essen ] = (0|1)
buffer[3]  = Links         [ Essen ] = (0|1)
buffer[4]  = Rechts        [ Essen ] = (0|1)
buffer[5]  = Links Unten    [ Essen ] = (0|1)
buffer[6]  = Unten         [ Essen ] = (0|1)
buffer[7]  = Rechts Unten  [ Essen ] = (0|1)

buffer[8]  = Links Oben    [ Körper ] = (0|1)
buffer[9]  = Oben          [ Körper ] = (0|1)
buffer[10] = Rechts Oben   [ Körper ] = (0|1)
buffer[11] = Links         [ Körper ] = (0|1)
buffer[12] = Rechts        [ Körper ] = (0|1)
buffer[13] = Links Unten    [ Körper ] = (0|1)
buffer[14] = Unten         [ Körper ] = (0|1)
buffer[15] = Rechts Unten  [ Körper ] = (0|1)

buffer[16] = Links Oben    [ Entf. ] = (0.0 - 1.0)
buffer[17] = Oben          [ Entf. ] = (0.0 - 1.0)
buffer[18] = Rechts Oben   [ Entf. ] = (0.0 - 1.0)
buffer[19] = Links         [ Entf. ] = (0.0 - 1.0)
buffer[20] = Rechts        [ Entf. ] = (0.0 - 1.0)
buffer[21] = Links Unten    [ Entf. ] = (0.0 - 1.0)
buffer[22] = Unten         [ Entf. ] = (0.0 - 1.0)
buffer[23] = Rechts Unten  [ Entf. ] = (0.0 - 1.0)

*/

    for (int i = 0; i < 24; ++i) {
        buffer[i] = 0.0;
    }

    const QPoint &head = pos.front();
    unsigned size = field->getSize();

    for(int x = 1; x < head.x(); x++ ) {
        for(const auto & e : pos)
            if(QPoint(x, head.y()) == e)
                buffer[11] = std::max(buffer[11],  1.0 / std::abs( head.x() - e.x() ));
    }
    for(unsigned x = head.x() + 1; x < size; x++ ) {
        for(const auto & e : pos)
            if(QPoint(x, head.y()) == e)
                buffer[12] = std::max(buffer[12],  1.0 / std::abs( head.x() - e.x() ));

    }
    for(int y = 1; y < head.y(); y++ ) {
        for(const auto & e : pos)
            if(QPoint(head.x(), y) == e)
                buffer[9] = std::max(buffer[9],  1.0 / std::abs( head.y() - e.y() ));
    }
    for(unsigned y = head.y() + 1; y < size; y++ ) {
        for(const auto & e : pos)
            if(QPoint(head.x(), y) == e)
                buffer[14] = std::max(buffer[14],  1.0 / std::abs( head.y() - e.y() ));
    }


    float dx = foodPos.x() - head.x();
    float dy = foodPos.y() - head.y();

    if (abs(dx) == abs(dy)) {
        if (dx > 0 && dy > 0) {
            buffer[7] = 1.0; // Rechts unten
        } else if (dx > 0 && dy < 0) {
            buffer[0] = 1.0; // Links oben
        } else if (dx < 0 && dy > 0) {
            buffer[5] = 1.0; // Links unten
        } else if (dx < 0 && dy < 0) {
            buffer[2] = 1.0; // Rechts oben
        }
    } else if (dx == 0 && dy < 0) {
        buffer[1] = 1.0; // Geradeaus oben
    } else if (dx == 0 && dy > 0) {
        buffer[6] = 1.0; // Geradeaus unten
    } else if (dy == 0 && dx < 0) {
        buffer[3] = 1.0; // Geradeaus links
    } else if (dy == 0 && dx > 0) {
        buffer[4] = 1.0; // Geradeaus rechts
    }



    double max = size * sqrt(2);

    double wallLeft = head.x(); // Abstand zur linken Wand
    double wallRight = field->getSize() - head.x() - 1; // Abstand zur rechten Wand
    double wallUp = head.y(); // Abstand zur oberen Wand
    double wallDown = field->getSize() - head.y() - 1; // Abstand zur unteren Wand

    buffer[8] = 1.0 / (1.0- wallUp / max); // Links Oben
    buffer[9] = 1.0 / (1.0 -wallUp/ max); // Oben
    buffer[10] = 1.0 /(1.0 - wallUp/ max); // Rechts Oben
    buffer[11] = 1.0 /(1.0 - wallLeft/ max); // Links
    buffer[12] = 1.0 /(1.0 - wallRight/ max); // Rechts
    buffer[13] = 1.0 / (1.0 -wallDown/ max); // Links Unten
    buffer[14] = 1.0 / (1.0 -wallDown/ max); // Unten
    buffer[15] = 1.0 /(1.0 - wallDown/ max); // Rechts Unten

//    for(int i = 0; i < 24; i++)
//        qDebug() << i << buffer[i];
}



void Snake::startAI()
{
    reset();
    isAI = true;
    this->QThread::start();

}

void Snake::start()
{
    reset();
    this->setFokus(true);
    this->QThread::start();

}

int Snake::getLegth()
{
    return pos.length();
}

void Snake::richtungAendern(QPoint richtung)
{
    QMutexLocker m(&mutex_richtung_aendern);
    this->richtung = richtung;
}

