#include "snake.h"
#include "qdebug.h"

#include <QPainterPath>

Snake::Snake(GameField *field, Net *net, QObject *parent, int num_id, double speed_game, MODE input_mode, bool startTop)
    : QThread(parent), input_mode(input_mode), enemy(nullptr), field(field), net(net), num_id(num_id), startTop(startTop)
{
    // agent = new Agent(net, 0.9, 0.0, 100);

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
    for(int i = 0; i < snake_init_length; i++) {
        //pos.append(QPoint(field->getSize() / 2, field->getSize() / 2));
        if(startTop) {
            pos.append(QPoint(1, 1));
        } else {
            pos.append(QPoint(1, field->getSize() ));
        }

    }

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


// int argmax2(/*const Action& vec*/) {
//     if (vec.empty()) {
//         // Handle the case where the vector is empty
//         return -1; // or any other appropriate value
//     }

//     // Use std::max_element to find the iterator pointing to the maximum element
//     auto maxElementIterator = std::max_element(vec.begin(), vec.end());

//     // Check if maxElementIterator is valid before getting the index
//     if (maxElementIterator != vec.end()) {
//         // Calculate the index by subtracting the beginning iterator
//         int index = std::distance(vec.begin(), maxElementIterator);
//         return index;
//     } else {
//         // Handle the case where the vector is empty or other specific conditions
//         return -1; // or any other appropriate value
//     }
// }


void Snake::run()
{
    foodNum = 0;

    if(fokus)
        emit foodPosChanged(getCurrentFood(), num_id);

    int buf_size = 1;
    switch (input_mode) {
    case Snake::CLASSIC:
        buf_size = 24;
        break;
    case Snake::DETAILED_CLASSIC:
        buf_size = 24;
        break;
    case Snake::IMAGE_BASED:
        buf_size = (field->getSize() * field->getSize() * 2);
        break;
    case Snake::TURN_MODE:
        buf_size = 11;
        break;
    }

    double buffer[buf_size];
    // State state, stateAfter;
    // Action action = {0, 0, 0, 0};
    // double reward = 1.0;
    // bool isOver = false;

    while (!isInterruptionRequested()) {

        if(net) {

            lookThingsUp(buffer, getCurrentFood());


            // for(int i = 0; i < buf_size; i++)
            //     state.push_back(buffer[i]);

            if(isInterruptionRequested())
                break;
            net->feedForward(buffer);
            if(isInterruptionRequested())
                break;
            net->getResults(buffer);
            if(isInterruptionRequested())
                break;

            if(isAI) {

                if(false /*&& input_mode == Snake::TURN_MODE*/) {

                } else {
                    int maxIndex = 0;
                    double maxProbability = buffer[0];

                    for (int i = 1; i < 4; ++i) {
                        if (buffer[i] > maxProbability) {
                            maxProbability = buffer[i];
                            maxIndex = i;
                        }
                    }

                    switch (maxIndex /*argmax2(agent->getAction(state))*/ ) {
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

                    // action.at(maxIndex) = 1;
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
        auto tmp = getPos();
        for(const auto & e : tmp)
            if(e == newPos)
                move_is_ok = false;

        //check if run into enemy

        if(enemy) {
            auto enmpos = getEnemyPolygon();

            for(const auto & e : enmpos)
                if(e == newPos) {
                    move_is_ok = false;
                    // qDebug() << "run into enemy!";
                }

        }


        // Wenn keine weiteren moves übrig sind oder check run out of map
        if(moves <= 0 || newPos.x() <= 0 || newPos.x() > field->getSize()  || newPos.y() <= 0 || newPos.y() > field->getSize())
            move_is_ok = false;
        //die if not ok
        if(!move_is_ok){
            fokus = false;
            lebt_noch = false;

            emit died(num_id);


            // lookThingsUp(buffer, currentFood);
            // for(int i = 0; i < buf_size; i++)
            //     stateAfter.push_back(buffer[i]);
            // agent->addStep(state, stateAfter, action, -10, true);

            return;
        }

        if(isInterruptionRequested())
            break;

        //Move
        pos.prepend(newPos);
        pos.removeLast();
        moves--;

        //Try eat
        if(pos.first() == getCurrentFood()) {

            foodNum++;
            if(fokus)
                emit foodPosChanged(getCurrentFood(), num_id);
            moves += snake_add_moves_per_apple;
            if((ssize_t)moves > getMaxMoves() )
                moves = getMaxMoves();

            //Wachse!!
            pos.append(pos.last());

            // reward = 10;
        }
        survive_time++;


        // lookThingsUp(buffer, currentFood);
        // for(int i = 0; i < buf_size; i++)
        //     stateAfter.push_back(buffer[i]);
        // agent->addStep(state, stateAfter, action, reward, false);
    }

    lebt_noch = false;
    emit died(num_id);
}

QPolygon Snake::getPos() const
{
    return pos;
}

int Snake::getFoodNum() const
{
    return foodNum;
}

Snake *Snake::getEnemy() const
{
    return enemy;
}

int Snake::getNum_id() const
{
    return num_id;
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



bool Snake::getLebt_noch() const
{
    return lebt_noch;
}

QPoint Snake::getCurrentFood()
{
    if(enemy && foodNum != enemy->getFoodNum()) {
        foodNum = std::max(foodNum, enemy->getFoodNum());
        emit foodPosChanged(field->getApplePos(foodNum), num_id);
    }
    return field->getApplePos(foodNum);
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
    case Snake::TURN_MODE:
        /*
        buffer[0]  = # Danger straight
        buffer[1]  = # Danger right
        buffer[2]  = # Danger left

        buffer[3]  = # Move direction l
        buffer[4]  = # Move direction r
        buffer[5]  = # Move direction u
        buffer[6]  = # Move direction d

        buffer[7]  = # Food location l
        buffer[8]  = # Food location r
        buffer[9]  = # Food location u
        buffer[10] = # Food location d
        */


        break;
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

        auto tmp = getPos();

        for(int x = 1; x < head.x(); x++ ) {
            for(const auto & e : tmp)
                if(QPoint(x, head.y()) == e)
                    buffer[11] = std::max(buffer[11],  1.0 / std::abs( head.x() - e.x() ));
        }
        for(unsigned x = head.x() + 1; x < size; x++ ) {
            for(const auto & e : tmp)
                if(QPoint(x, head.y()) == e)
                    buffer[12] = std::max(buffer[12],  1.0 / std::abs( head.x() - e.x() ));

        }
        for(int y = 1; y < head.y(); y++ ) {
            for(const auto & e : tmp)
                if(QPoint(head.x(), y) == e)
                    buffer[9] = std::max(buffer[9],  1.0 / std::abs( head.y() - e.y() ));
        }
        for(unsigned y = head.y() + 1; y < size; y++ ) {
            for(const auto & e : tmp)
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
            } else if (dx < 0.0 && dy > 0.0) {
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
        auto enem = getEnemyPolygon();

        buffer[8]  = 0.0; //1.0 / (moves + 1.0); // übrige Leben
        buffer[10] = enem.contains(head + QPoint(1, 0));
        buffer[13] = lineFood.angle() / 360.0; // Winkel food
        buffer[15] = enem.contains(head + QPoint(-1, 0));
        buffer[16] = 0.0; // 1.0 - 1.0 / (getScore() + 1.0); // score
        buffer[18] = enem.contains(head + QPoint(0, 1));
        buffer[21] = 2.0 / (lineFood.length() + 1.0);  // entfernung food
        buffer[23] = enem.contains(head + QPoint(0, -1));
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

            auto tmp = getPos();

            for(int x = 1; x < head.x(); x++ ) {
                for(const auto & e : tmp)
                    if(QPoint(x, head.y()) == e)
                        buffer[11] = std::max(buffer[11],  1.0 / std::abs( head.x() - e.x() ));
            }
            for(unsigned x = head.x() + 1; x < size; x++ ) {
                for(const auto & e : tmp)
                    if(QPoint(x, head.y()) == e)
                        buffer[12] = std::max(buffer[12],  1.0 / std::abs( head.x() - e.x() ));

            }
            for(int y = 1; y < head.y(); y++ ) {
                for(const auto & e : tmp)
                    if(QPoint(head.x(), y) == e)
                        buffer[9] = std::max(buffer[9],  1.0 / std::abs( head.y() - e.y() ));
            }
            for(unsigned y = head.y() + 1; y < size; y++ ) {
                for(const auto & e : tmp)
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

QPolygon Snake::getEnemyPolygon()
{
    if(!enemy)
        qDebug() << "getEnemyPolygon() return empty!!!";

    return enemy ? enemy->getPos() : QPolygon();
}

void Snake::setEnemy(Snake *enemy)
{
    this->enemy = enemy;
}


