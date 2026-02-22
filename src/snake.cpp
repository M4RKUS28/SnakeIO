#include "snake.h"
#include "qdebug.h"

#include <QPainterPath>
#include <cmath>
#include <algorithm>
#include <vector>

// ===========================================================================
//  Constructor / Destructor
// ===========================================================================
Snake::Snake(GameField* field, Net* net, QObject* parent, int num_id,
             double speed_game, const NetworkConfig& cfg, bool startTop)
    : QThread(parent),
      cfg(cfg),
      lebt_noch(false),
      fokus(false),
      speed_game(speed_game),
      enemy(nullptr),
      field(field),
      net(net),
      isAI(false),
      foodNum(0),
      survive_time(0),
      num_id(num_id),
      moves(0),
      startTop(startTop)
{
    if (!net) {
        std::cerr << "Snake created without a network!" << std::endl;
        std::exit(123);
    }
    reset();
    this->speed_game = speed_game;
}

Snake::~Snake() {}

// ===========================================================================
//  reset()
// ===========================================================================
void Snake::reset()
{
    isAI = false;
    richtung = QPoint(1, 0);
    pos.clear();
    for (int i = 0; i < snake_init_length; ++i) {
        if (startTop)
            pos.append(QPoint(1, 1));
        else
            pos.append(QPoint(1, field->getSize()));
    }
    moves        = snake_init_moves;
    foodNum      = 0;
    lebt_noch    = true;
    survive_time = 0;

    lastOutput[0] = lastOutput[1] = lastOutput[2] = lastOutput[3] = 0.0;
}


// ===========================================================================
//  startAI / startPlayer
// ===========================================================================
void Snake::startAI(Net* net)
{
    if (this->isRunning()) { perror("already running"); return; }
    reset();
    this->isAI = true;
    this->net  = net;
    this->QThread::start();
}

void Snake::startPlayer(Net* netinfo)
{
    if (this->isRunning()) { perror("already running"); return; }
    reset();
    this->setFokus(true);
    this->net = netinfo;
    this->QThread::start();
}

// ===========================================================================
//  run()  —  main game loop
// ===========================================================================
void Snake::run()
{
    foodNum = 0;
    if (fokus) emit foodPosChanged(getCurrentFood(), num_id);

    const int buf_size = cfg.inputCount();
    std::vector<double> buffer(buf_size, 0.0);

    while (!isInterruptionRequested()) {

        if (net) {
            lookThingsUp(buffer.data(), getCurrentFood());

            if (isInterruptionRequested()) break;
            net->feedForward(buffer.data());
            if (isInterruptionRequested()) break;
            net->getResults(buffer.data());
            if (isInterruptionRequested()) break;

            // Cache outputs for LAST_OUTPUT_* features on next tick
            lastOutput[0] = buffer[0];  // Up
            lastOutput[1] = buffer[1];  // Down
            lastOutput[2] = buffer[2];  // Right
            lastOutput[3] = buffer[3];  // Left

            if (isAI) {
                // Argmax over 4 output neurons → pick direction
                int maxIndex = 0;
                double maxVal = buffer[0];
                for (int i = 1; i < 4; ++i) {
                    if (buffer[i] > maxVal) { maxVal = buffer[i]; maxIndex = i; }
                }
                switch (maxIndex) {
                case 0: richtungAendern(QPoint( 0, -1)); break;  // Up
                case 1: richtungAendern(QPoint( 0,  1)); break;  // Down
                case 2: richtungAendern(QPoint( 1,  0)); break;  // Right
                case 3: richtungAendern(QPoint(-1,  0)); break;  // Left
                }
            }
        }

        if (fokus) emit posChanged(pos, num_id);
        if (isInterruptionRequested()) break;
        usleep(static_cast<useconds_t>(1000000.0 * (100.0 / speed_game)));
        if (isInterruptionRequested()) break;

        // --- Validate next move ---
        bool move_is_ok = true;
        QPoint newPos = pos.front() + richtung;

        auto tmp = getPos();
        for (const auto& e : tmp)
            if (e == newPos) { move_is_ok = false; break; }

        if (enemy) {
            auto enmpos = getEnemyPolygon();
            for (const auto& e : enmpos)
                if (e == newPos) { move_is_ok = false; break; }
        }

        if (moves <= 0
            || newPos.x() <= 0 || newPos.x() > field->getSize()
            || newPos.y() <= 0 || newPos.y() > field->getSize())
            move_is_ok = false;

        if (!move_is_ok) {
            fokus     = false;
            lebt_noch = false;
            emit died(num_id);
            return;
        }

        if (isInterruptionRequested()) break;

        // --- Move ---
        pos.prepend(newPos);
        pos.removeLast();
        moves--;

        // --- Eat apple ---
        if (pos.first() == getCurrentFood()) {
            ++foodNum;
            if (fokus) emit foodPosChanged(getCurrentFood(), num_id);
            moves += snake_add_moves_per_apple;
            if (static_cast<ssize_t>(moves) > getMaxMoves())
                moves = getMaxMoves();
            pos.append(pos.last());  // grow
        }
        ++survive_time;
    }

    lebt_noch = false;
    emit died(num_id);
}

// ===========================================================================
//  lookThingsUp()
//  Fills one buffer slot per cfg.inputs entry by delegating to evaluateFeature.
// ===========================================================================
void Snake::lookThingsUp(double* buffer, const QPoint& foodPos)
{
    const int n = cfg.inputCount();
    for (int i = 0; i < n; ++i)
        buffer[i] = evaluateFeature(cfg.inputs[i], foodPos);
}

// ===========================================================================
//  evaluateFeature()
//
//  Single source of truth for all input values.
//  To add a new input type: add a case here + an entry in inputconfig.cpp.
// ===========================================================================
double Snake::evaluateFeature(const InputNeuronConfig& ncfg, const QPoint& foodPos) const
{
    const QPoint& head = pos.front();
    const int  fsize   = field->getSize();   // field is 1-indexed [1..fsize]

    // --- Helper: is a cell a wall or own body segment? ---
    auto isDangerous = [&](QPoint p) -> bool {
        if (p.x() < 1 || p.x() > fsize || p.y() < 1 || p.y() > fsize) return true;
        auto tmp = getPos();
        for (const auto& e : tmp) if (e == p) return true;
        return false;
    };

    // --- Helper: inverse distance to nearest own body along a ray ---
    auto bodyProxOnRay = [&](int dx, int dy) -> double {
        auto tmp = getPos();
        for (int step = 1; step <= fsize * 2; ++step) {
            QPoint p(head.x() + dx * step, head.y() + dy * step);
            if (p.x() < 1 || p.x() > fsize || p.y() < 1 || p.y() > fsize) break;
            for (const auto& e : tmp)
                if (e == p) return 1.0 / step;
        }
        return 0.0;
    };

    // --- Helper: inverse distance to nearest enemy segment along a ray ---
    auto enemyProxOnRay = [&](int dx, int dy) -> double {
        if (!enemy) return 0.0;
        auto enmpos = enemy->getPos();
        for (int step = 1; step <= fsize * 2; ++step) {
            QPoint p(head.x() + dx * step, head.y() + dy * step);
            if (p.x() < 1 || p.x() > fsize || p.y() < 1 || p.y() > fsize) break;
            for (const auto& e : enmpos)
                if (e == p) return 1.0 / step;
        }
        return 0.0;
    };

    // --- Helper: normalised wall distance along a cardinal direction [0=at wall, 1=far] ---
    auto wallDistCardinal = [&](int dx, int dy) -> double {
        double dist;
        if      (dx == -1) dist = head.x() - 1;
        else if (dx ==  1) dist = fsize - head.x();
        else if (dy == -1) dist = head.y() - 1;
        else               dist = fsize - head.y();
        return fsize > 1 ? dist / static_cast<double>(fsize - 1) : 0.0;
    };

    // --- Helper: normalised wall distance along a diagonal direction ---
    auto wallDistDiag = [&](int dx, int dy) -> double {
        for (int step = 1; step < fsize; ++step) {
            if (head.x() + dx * step < 1 || head.x() + dx * step > fsize ||
                head.y() + dy * step < 1 || head.y() + dy * step > fsize)
                return (step - 1) / static_cast<double>(fsize - 1);
        }
        return 1.0;
    };

    // --- Helper: 1.0 if food lies exactly on the given compass ray ---
    auto foodOnRay = [&](int dx, int dy) -> double {
        float ddx = static_cast<float>(foodPos.x() - head.x());
        float ddy = static_cast<float>(foodPos.y() - head.y());
        if (ddx == 0.0f && ddy == 0.0f) return 0.0;   // food at head (shouldn't happen)
        if (dy == 0) {  // cardinal E/W
            if (ddy != 0.0f) return 0.0;
            return ((dx < 0) == (ddx < 0)) ? 1.0 : 0.0;
        }
        if (dx == 0) {  // cardinal N/S
            if (ddx != 0.0f) return 0.0;
            return ((dy < 0) == (ddy < 0)) ? 1.0 : 0.0;
        }
        // Diagonal: food must be on the exact diagonal (|ddx| == |ddy|) and same quadrant
        if (std::abs(ddx) != std::abs(ddy)) return 0.0;
        return ((dx < 0) == (ddx < 0)) && ((dy < 0) == (ddy < 0)) ? 1.0 : 0.0;
    };

    const QPoint& r = richtung;  // current heading

    switch (ncfg.feature) {

    // -----------------------------------------------------------------------
    // FOOD DIRECTION — one-hot (1.0 if food is on that exact compass ray)
    // -----------------------------------------------------------------------
    case InputFeature::FOOD_DIR_NW: return foodOnRay(-1, -1);
    case InputFeature::FOOD_DIR_N:  return foodOnRay( 0, -1);
    case InputFeature::FOOD_DIR_NE: return foodOnRay( 1, -1);
    case InputFeature::FOOD_DIR_W:  return foodOnRay(-1,  0);
    case InputFeature::FOOD_DIR_E:  return foodOnRay( 1,  0);
    case InputFeature::FOOD_DIR_SW: return foodOnRay(-1,  1);
    case InputFeature::FOOD_DIR_S:  return foodOnRay( 0,  1);
    case InputFeature::FOOD_DIR_SE: return foodOnRay( 1,  1);

    // -----------------------------------------------------------------------
    // BODY PROXIMITY — 1/distance to nearest body segment on that ray
    // -----------------------------------------------------------------------
    case InputFeature::BODY_PROX_NW: return bodyProxOnRay(-1, -1);
    case InputFeature::BODY_PROX_N:  return bodyProxOnRay( 0, -1);
    case InputFeature::BODY_PROX_NE: return bodyProxOnRay( 1, -1);
    case InputFeature::BODY_PROX_W:  return bodyProxOnRay(-1,  0);
    case InputFeature::BODY_PROX_E:  return bodyProxOnRay( 1,  0);
    case InputFeature::BODY_PROX_SW: return bodyProxOnRay(-1,  1);
    case InputFeature::BODY_PROX_S:  return bodyProxOnRay( 0,  1);
    case InputFeature::BODY_PROX_SE: return bodyProxOnRay( 1,  1);

    // -----------------------------------------------------------------------
    // WALL DISTANCE — normalised [0=at wall, 1=farthest from that wall]
    // -----------------------------------------------------------------------
    case InputFeature::WALL_DIST_NW: return wallDistDiag  (-1, -1);
    case InputFeature::WALL_DIST_N:  return wallDistCardinal( 0, -1);
    case InputFeature::WALL_DIST_NE: return wallDistDiag  ( 1, -1);
    case InputFeature::WALL_DIST_W:  return wallDistCardinal(-1,  0);
    case InputFeature::WALL_DIST_E:  return wallDistCardinal( 1,  0);
    case InputFeature::WALL_DIST_SW: return wallDistDiag  (-1,  1);
    case InputFeature::WALL_DIST_S:  return wallDistCardinal( 0,  1);
    case InputFeature::WALL_DIST_SE: return wallDistDiag  ( 1,  1);

    // -----------------------------------------------------------------------
    // CELL VALUE — -1 = own body,  0 = empty,  +1 = food
    // param = flat cell index (col + row * fieldSize, 0-based)
    // -----------------------------------------------------------------------
    case InputFeature::CELL_AT_INDEX: {
        const int col  = ncfg.param % fsize;   // 0-based
        const int row  = ncfg.param / fsize;
        QPoint cell(col + 1, row + 1);          // convert to 1-based field coords
        if (cell == foodPos) return 1.0;
        auto tmp = getPos();
        for (const auto& e : tmp)
            if (e == cell) return -1.0;
        return 0.0;
    }

    // -----------------------------------------------------------------------
    // FOOD METRICS
    // -----------------------------------------------------------------------
    case InputFeature::FOOD_DISTANCE: {
        double dx = foodPos.x() - head.x();
        double dy = foodPos.y() - head.y();
        return std::sqrt(dx*dx + dy*dy) / (fsize * std::sqrt(2.0));
    }
    case InputFeature::FOOD_ANGLE: {
        double dx = foodPos.x() - head.x();
        double dy = foodPos.y() - head.y();
        // Map atan2 from [-pi, pi] to [0, 1]
        return (std::atan2(dy, dx) + M_PI) / (2.0 * M_PI);
    }

    // -----------------------------------------------------------------------
    // SNAKE STATE
    // -----------------------------------------------------------------------
    case InputFeature::MOVES_LEFT: {
        int maxM = getMaxMoves();
        return maxM > 0 ? std::min(1.0, static_cast<double>(moves) / maxM) : 0.0;
    }
    case InputFeature::SNAKE_LENGTH:
        return std::min(1.0, static_cast<double>(pos.size()) / (fsize * fsize));

    // -----------------------------------------------------------------------
    // DANGER (relative to current heading)
    //   turn-right from (dx,dy) = (-dy, dx)
    //   turn-left  from (dx,dy) = ( dy,-dx)
    // -----------------------------------------------------------------------
    case InputFeature::DANGER_STRAIGHT:
        return isDangerous(head + r) ? 1.0 : 0.0;
    case InputFeature::DANGER_RIGHT:
        return isDangerous(head + QPoint(-r.y(),  r.x())) ? 1.0 : 0.0;
    case InputFeature::DANGER_LEFT:
        return isDangerous(head + QPoint( r.y(), -r.x())) ? 1.0 : 0.0;

    // -----------------------------------------------------------------------
    // CURRENT DIRECTION (one-hot, absolute)
    // -----------------------------------------------------------------------
    case InputFeature::DIR_N: return (r == QPoint( 0, -1)) ? 1.0 : 0.0;
    case InputFeature::DIR_W: return (r == QPoint(-1,  0)) ? 1.0 : 0.0;
    case InputFeature::DIR_E: return (r == QPoint( 1,  0)) ? 1.0 : 0.0;
    case InputFeature::DIR_S: return (r == QPoint( 0,  1)) ? 1.0 : 0.0;

    // -----------------------------------------------------------------------
    // ENEMY PROXIMITY (PvE — inverse distance to nearest enemy on ray)
    // -----------------------------------------------------------------------
    case InputFeature::ENEMY_PROX_N: return enemyProxOnRay( 0, -1);
    case InputFeature::ENEMY_PROX_W: return enemyProxOnRay(-1,  0);
    case InputFeature::ENEMY_PROX_E: return enemyProxOnRay( 1,  0);
    case InputFeature::ENEMY_PROX_S: return enemyProxOnRay( 0,  1);

    // -----------------------------------------------------------------------
    // FEEDBACK (last output neuron values from previous tick — default 0.0)
    // -----------------------------------------------------------------------
    case InputFeature::LAST_OUTPUT_UP:    return lastOutput[0];
    case InputFeature::LAST_OUTPUT_DOWN:  return lastOutput[1];
    case InputFeature::LAST_OUTPUT_RIGHT: return lastOutput[2];
    case InputFeature::LAST_OUTPUT_LEFT:  return lastOutput[3];
    }

    return 0.0;
}

// ===========================================================================
//  Accessors
// ===========================================================================
QPolygon Snake::getPos()         const { return pos; }
int      Snake::getFoodNum()     const { return foodNum; }
Snake*   Snake::getEnemy()       const { return enemy; }
int      Snake::getNum_id()      const { return num_id; }
bool     Snake::getLebt_noch()   const { return lebt_noch; }
bool     Snake::getFokus()       const { return fokus; }
size_t   Snake::getLeftMoves()   const { return moves; }
size_t   Snake::getScore()       const { return foodNum * 300 + survive_time; }

int Snake::getLegth()     const { return pos.length(); }
int Snake::getMaxMoves() const { return field->getSize() * 10 + 2 * getLegth(); }

void Snake::setFokus(bool v)      { fokus = v; }
void Snake::setSpeed(double s)    { speed_game = s; }
void Snake::setEnemy(Snake* e)    { enemy = e; }

void Snake::richtungAendern(QPoint r)
{
    QMutexLocker m(&mutex_richtung_aendern);
    richtung = r;
}

QPoint Snake::getCurrentFood()
{
    if (enemy && foodNum != enemy->getFoodNum()) {
        foodNum = std::max(foodNum, enemy->getFoodNum());
        emit foodPosChanged(field->getApplePos(foodNum), num_id);
    }
    return field->getApplePos(foodNum);
}

QPolygon Snake::getEnemyPolygon()
{
    if (!enemy) qDebug() << "getEnemyPolygon() called with no enemy!";
    return enemy ? enemy->getPos() : QPolygon();
}

