#include "snake.h"
#include "qdebug.h"

#include <QPainterPath>

Snake::Snake(GameField *field, Net *net, QObject *parent, int num_id, double speed_game, INPUT_MODE input_mode)
    : QThread(parent), input_mode(input_mode), field(field), net(net), num_id(num_id)
{
    if(!net) {
        std::cout << "Netless snake!" << std::endl;
        exit(123);
    }

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
    for(int i = 0; i < snake_init_length; i++)
        pos.append(QPoint(field->getSize() / 2, field->getSize() / 2));

    moves = snake_init_moves;
    foodNum = 0;
    lebt_noch = true;
    survive_time = 0;
}

void Snake::startAI(Net * net)
{
    if(this->isRunning()) {
        perror("already running");
        return;
    }

    reset();
    this->isAI = true;
    this->net = net;
    this->QThread::start();

}

void Snake::startPlayer(Net *netinfo)
{
    if(this->isRunning()) {
        perror("already running");
        return;
    }
    reset();
    this->setFokus(true);
    this->net = netinfo;
    this->QThread::start();
}

void Snake::run()
{
    foodNum = 0;
    currentFood = field->getApplePos(foodNum);
    if(fokus)
        emit foodPosChanged(currentFood, num_id);

    while (!isInterruptionRequested()) {

        if(net) {
            double buffer[ input_mode == INPUT_MODE::IMAGE_BASED ? (field->getSize() * field->getSize() * 2) : 24];
            lookThingsUp(buffer, currentFood);
            if(isInterruptionRequested())
                break;
            net->feedForward(buffer);
            if(isInterruptionRequested())
                break;
            net->getResults(buffer);
            if(isInterruptionRequested())
                break;

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
        }

        if(fokus)
            emit posChanged(pos, num_id);
        if(isInterruptionRequested())
            break;
        usleep(1000000 * (100.0 / speed_game));
        if(isInterruptionRequested())
            break;

        //check outstanding moves
        bool move_is_ok = true;
        QPoint newPos = pos.front() + richtung;
        //check if run into snake itself
        for(const auto & e : pos)
            if(e == newPos)
                move_is_ok = false;
        // Wenn keine weiteren moves übrig sind oder check run out of map
        if(moves <= 0 || newPos.x() <= 0 || newPos.x() > field->getSize()  || newPos.y() <= 0 || newPos.y() > field->getSize())
            move_is_ok = false;
        //die if not ok
        if(!move_is_ok){
            emit died(num_id);
            fokus = false;
            lebt_noch = false;
            return;
        }

        if(isInterruptionRequested())
            break;

        //Move
        pos.prepend(newPos);
        pos.removeLast();
        moves--;

        //Try eat
        if(pos.first() == currentFood) {

            foodNum++;
            currentFood = field->getApplePos(foodNum);
            if(fokus)
                emit foodPosChanged(currentFood, num_id);
            moves += snake_add_moves_per_apple;
            if((ssize_t)moves > getMaxMoves() )
                moves = getMaxMoves();

            //Wachse!!
            pos.append(pos.last());
        }
        survive_time++;
    }


}

int Snake::getMaxMoves()
{
    return field->getSize() * 10+ 2* getLegth();
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
    switch (input_mode) {
    case DETAILED_CLASSIC:
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
            } else if (dx > 0.0 && dy < 0.0) {
                buffer[0] = 1.0; // Links oben
            } else if (dx < 0.0 && dy > 0.0f )  {
                buffer[5] = 1.0; // Links unten
            } else if (dx < 0.0 && dy < 0.0) {
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
        double wallRight = field->getSize() - head.x() + 1; // Abstand zur rechten Wand
        double wallUp = head.y(); // Abstand zur oberen Wand
        double wallDown = field->getSize() - head.y() + 1; // Abstand zur unteren Wand

        buffer[16] = 0.0; // 1.0 / (1.0- wallUp / max); // Links Oben
        buffer[17] = 1.0 / (1.0 -wallUp/ max) - 1; // Oben
        buffer[18] = 0.0; //1.0 /(1.0 - wallUp/ max); // Rechts Oben
        buffer[19] = 1.0 /(1.0 - wallLeft/ max) - 1; // Links
        buffer[20] = 1.0 /(1.0 - wallRight/ max) - 1; // Rechts
        buffer[21] = 0.0; //; 1.0 / (1.0 -wallDown/ max); // Links Unten
        buffer[22] = 1.0 / (1.0 -wallDown/ max) - 1; // Unten
        buffer[23] = 0.0; //1.0 /(1.0 - wallDown/ max); // Rechts Unten

        /*buffer[8]  = Links Oben    [ Körper ] = (0|1)
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

        auto lineFood = QLineF(head, foodPos);

        buffer[8]  = 0.0; //1.0 / (moves + 1.0); // übrige Leben
        buffer[10] = 0.0;
        buffer[13] = lineFood.angle() / 360.0; // Winkel food
        buffer[15] = 0.0;
        buffer[16] = 0.0; // 1.0 - 1.0 / (getScore() + 1.0); // score
        buffer[18] = 0.0;
        buffer[21] = 2.0 / (lineFood.length() + 1.0);  // entfernung food
        buffer[23] = 0.0;
        break;
    }
    case CLASSIC: {
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
                } else if (dx > 0.0 && dy < 0.0) {
                    buffer[0] = 1.0; // Links oben
                } else if (dx < 0.0 && dy > 0.0f )  {
                    buffer[5] = 1.0; // Links unten
                } else if (dx < 0.0 && dy < 0.0) {
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
            double wallRight = field->getSize() - head.x() + 1; // Abstand zur rechten Wand
            double wallUp = head.y(); // Abstand zur oberen Wand
            double wallDown = field->getSize() - head.y() + 1; // Abstand zur unteren Wand

            buffer[16] = 0.0; // 1.0 / (1.0- wallUp / max); // Links Oben
            buffer[17] = 1.0 / (1.0 -wallUp/ max) - 1; // Oben
            buffer[18] = 0.0; //1.0 /(1.0 - wallUp/ max); // Rechts Oben
            buffer[19] = 1.0 /(1.0 - wallLeft/ max) - 1; // Links
            buffer[20] = 1.0 /(1.0 - wallRight/ max) - 1; // Rechts
            buffer[21] = 0.0; //; 1.0 / (1.0 -wallDown/ max); // Links Unten
            buffer[22] = 1.0 / (1.0 -wallDown/ max) - 1; // Unten
            buffer[23] = 0.0; //1.0 /(1.0 - wallDown/ max); // Rechts Unten

            /*buffer[8]  = Links Oben    [ Körper ] = (0|1)
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
        break;
    }
    case IMAGE_BASED: {
        int field_size = field->getSize();
        for (int i = 0; i < field_size * field_size * 2 ; ++i)
                buffer[i] = 0.0;

        for(int i = 0; i < this->pos.size(); i++) {
                if( (this->pos.at(i).x()-1) + (this->pos.at(i).y()-1) * field_size >= field_size*field_size || (this->pos.at(i).x()-1) + (this->pos.at(i).y()-1) * field_size < 0)
                    qDebug() << " 1. Access: " << (this->pos.at(i).x()-1) + (this->pos.at(i).y()-1) << " Buffersize: " <<field_size * field_size << "snake: " << this->pos ;
                buffer[ (this->pos.at(i).x()-1) + (this->pos.at(i).y()-1) * field_size] = 1.0;
        }

        if(foodPos.x() +foodPos.y() * field_size >= field_size*field_size)
        qDebug() << " 2. Access: " << foodPos.x() +foodPos.y() * field_size;
        buffer[field->getSize()*field->getSize() + foodPos.x() +foodPos.y() * field_size] = 1.0;

        break;
    }
    }


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


