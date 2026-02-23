#pragma once

#include <QVector>
#include <QString>
#include <QStringList>
#include <string>

// ===========================================================================
//  InputFeature
//
//  Describes what a single input neuron reads from the game state.
//
//  HOW TO ADD A NEW INPUT TYPE:
//    1. Add an enum value below, inside the appropriate category comment.
//    2. Add a case in Snake::evaluateFeature() in snake.cpp.
//    3. Add a FeatureInfo entry in InputConfig::getFeatureInfo() in inputconfig.cpp.
//
//  That's all — the rest (label generation, topology strings, UI combo box)
//  is derived automatically.
// ===========================================================================
enum class InputFeature {

    // -----------------------------------------------------------------------
    // FOOD DIRECTION  (one-hot, 8 compass directions)
    // Value: 1.0 if food lies in that exact compass direction, else 0.0.
    // "Exact" means the nearest axis/diagonal ray from the head passes through
    // the food cell.
    // -----------------------------------------------------------------------
    FOOD_DIR_NW,  // ↖
    FOOD_DIR_N,   // ↑
    FOOD_DIR_NE,  // ↗
    FOOD_DIR_W,   // ←
    FOOD_DIR_E,   // →
    FOOD_DIR_SW,  // ↙
    FOOD_DIR_S,   // ↓
    FOOD_DIR_SE,  // ↘

    // -----------------------------------------------------------------------
    // BODY PROXIMITY  (8 compass directions, inverse distance)
    // Value: 1.0 / distance_to_nearest_own_body_segment_on_that_ray, or 0.0.
    // -----------------------------------------------------------------------
    BODY_PROX_NW,
    BODY_PROX_N,
    BODY_PROX_NE,
    BODY_PROX_W,
    BODY_PROX_E,
    BODY_PROX_SW,
    BODY_PROX_S,
    BODY_PROX_SE,

    // -----------------------------------------------------------------------
    // WALL DISTANCE  (4 cardinal + 4 diagonal, normalised to [0, 1])
    // Value: manhattan / diagonal dist to wall / (fieldSize-1), so 0 = at wall.
    // -----------------------------------------------------------------------
    WALL_DIST_NW,
    WALL_DIST_N,
    WALL_DIST_NE,
    WALL_DIST_W,
    WALL_DIST_E,
    WALL_DIST_SW,
    WALL_DIST_S,
    WALL_DIST_SE,

    // -----------------------------------------------------------------------
    // CELL VALUE  (one neuron per field cell)
    // param = flat cell index (row-major, 0-based): col + row * fieldSize
    // Value: -1.0 = snake body  |  0.0 = empty  |  +1.0 = food
    // -----------------------------------------------------------------------
    CELL_AT_INDEX,

    // -----------------------------------------------------------------------
    // FOOD METRICS
    // -----------------------------------------------------------------------
    FOOD_DISTANCE,  // Euclidean dist to food / (fieldSize * sqrt(2)),  [0, 1]
    FOOD_ANGLE,     // atan2(dy,dx) mapped to [0, 1]  (0/1 = same direction)

    // -----------------------------------------------------------------------
    // SNAKE STATE
    // -----------------------------------------------------------------------
    MOVES_LEFT,    // remaining moves / maxMoves,                       [0, 1]
    SNAKE_LENGTH,  // current body length / (fieldSize * fieldSize),    [0, 1]

    // -----------------------------------------------------------------------
    // DANGER  (turn-mode style, relative to current heading)
    // Value: 1.0 if the adjacent cell in that direction is a wall or own body.
    // -----------------------------------------------------------------------
    DANGER_STRAIGHT,
    DANGER_RIGHT,
    DANGER_LEFT,

    // -----------------------------------------------------------------------
    // CURRENT DIRECTION  (one-hot, absolute)
    // -----------------------------------------------------------------------
    DIR_N,
    DIR_W,
    DIR_E,
    DIR_S,

    // -----------------------------------------------------------------------
    // ENEMY PROXIMITY  (PvE mode, 4 cardinal directions, inverse distance)
    // Value: 1.0 / dist_to_nearest_enemy_segment, or 0.0.
    // -----------------------------------------------------------------------
    ENEMY_PROX_N,
    ENEMY_PROX_W,
    ENEMY_PROX_E,
    ENEMY_PROX_S,

    // -----------------------------------------------------------------------
    // FEEDBACK  (previous tick's output neuron values, stored per snake)
    // Allows the network to condition on its own last decision.
    // Default value on tick 0: 0.0.
    // -----------------------------------------------------------------------
    LAST_OUTPUT_UP,
    LAST_OUTPUT_DOWN,
    LAST_OUTPUT_RIGHT,
    LAST_OUTPUT_LEFT,

    // -----------------------------------------------------------------------
    // CONSTANTS  (trivial neurons — always emit a fixed value)
    // Useful to pad an existing architecture to match a trained model that
    // had dead/unused input neurons.
    // -----------------------------------------------------------------------
    CONST_ZERO,   // always 0.0
    CONST_ONE,    // always 1.0

    // -----------------------------------------------------------------------
    // LEGACY WALL DISTANCE  (hyperbolic formula from pre-rework DETAILED_CLASSIC)
    // Formula: 1.0 / (1.0 - dist / (fieldSize * sqrt(2))) - 1
    // where dist is the 1-indexed coordinate distance toward that wall.
    // Included so Demo mode can faithfully replay networks trained with the
    // old formula without modifying the cardinal WALL_DIST_* features.
    // -----------------------------------------------------------------------
    LEGACY_WALL_DIST_N,
    LEGACY_WALL_DIST_W,
    LEGACY_WALL_DIST_E,
    LEGACY_WALL_DIST_S,

    // -----------------------------------------------------------------------
    // LEGACY FOOD METRICS  (formulas from pre-rework DETAILED_CLASSIC)
    // Used by makeDemo() to fill buffer[13] and buffer[21] exactly as the
    // trained model saw them.
    // -----------------------------------------------------------------------
    LEGACY_FOOD_ANGLE,  // QLineF(head,food).angle() / 360.0  — atan2(-dy,dx), mapped [0,1)
    LEGACY_FOOD_DIST,   // 2.0 / (euclidean_distance + 1.0)
};

// ===========================================================================
//  InputNeuronConfig
//  One entry per input neuron. A NetworkConfig holds a flat list of these.
// ===========================================================================
struct InputNeuronConfig {
    InputFeature feature = InputFeature::FOOD_DIR_N;

    // Used only by CELL_AT_INDEX: flat cell index  col + row * fieldSize.
    // Ignored by all other features.
    int param = 0;
};

// ===========================================================================
//  HiddenLayerConfig
//  Describes one hidden (inner) layer.
//
//  aggregation: "SUM" | "AVG" | "MAX" | "MIN"
//  activation:  "RELU" | "TANH" | "SIGMOID" | "LEAKYRELU" | "SOFTPLUS"
//               | "SMAX" | "IDENTITY"
// ===========================================================================
struct HiddenLayerConfig {
    int     neurons     = 25;
    QString aggregation = "SUM";
    QString activation  = "RELU";
};

// ===========================================================================
//  NetworkConfig
//  Complete description of the neural architecture for one training run.
//
//  The output layer is always fixed: 4 neurons, SUM aggregation, SMAX activation
//  (Up / Down / Right / Left). It is appended automatically by buildTopologyString().
// ===========================================================================
struct NetworkConfig {
    QVector<InputNeuronConfig> inputs;      // input layer (one entry = one neuron)
    QVector<HiddenLayerConfig> hiddenLayers; // inner layers (arbitrary count)

    int fieldSize  = 20;   // game field side-length (cells)
    int snakeCount = 21;   // number of snakes / AIs in training

    // Convenience accessor
    int inputCount() const { return inputs.size(); }

    // -----------------------------------------------------------------------
    // Pre-configurations (factory methods)
    // -----------------------------------------------------------------------

    // 24 inputs matching the classic ray-cast layout:
    //   8 × food direction (one-hot)  +  8 × body proximity  +  8 × wall distance
    // Default hidden layers: 25-RELU → 18-RELU
    static NetworkConfig makeClassic(int fieldSize = 20, int snakeCount = 21);

    // fieldSize² inputs, one CELL_AT_INDEX neuron per cell:
    //   value -1 = own body,  0 = empty,  +1 = food
    // Default hidden layers: 100-RELU → 100-RELU → 20-RELU
    static NetworkConfig makeFullField(int fieldSize = 20, int snakeCount = 21);

    // 11 inputs (turn-mode / relative sensing):
    //   3 × danger (straight/right/left)  +  4 × current direction (one-hot)
    //   + 4 × food direction (one-hot, cardinal only)
    // Default hidden layers: 12-RELU
    static NetworkConfig makeTurnMode(int fieldSize = 20, int snakeCount = 21);

    // 24 inputs replicating the pre-rework DETAILED_CLASSIC layout exactly.
    // Uses CONST_ZERO for the 12 always-unused diagonal neurons and
    // LEGACY_WALL_DIST_* for the original hyperbolic wall-distance formula.
    // Topology: 24 → 25 → 18 → 4  (SUM/RELU throughout, SMAX output).
    // Intended for loading and replaying networks trained before the rework.
    static NetworkConfig makeDemo(int fieldSize = 20, int snakeCount = 21);
};

// ===========================================================================
//  InputConfig  (namespace of stateless helpers)
// ===========================================================================
namespace InputConfig {

    // -----------------------------------------------------------------------
    //  UI metadata for one feature (used to populate the feature combo box).
    // -----------------------------------------------------------------------
    struct FeatureInfo {
        QString displayName;    // shown in combo box, e.g. "Food Direction — North"
        QString labelShort;     // compact ViewNet label,  e.g. "🍎 ↑"
        bool    hasParam;       // true only for CELL_AT_INDEX
    };

    // Returns metadata for a single feature.
    // fieldSize is used to compute the valid param range for CELL_AT_INDEX.
    FeatureInfo getFeatureInfo(InputFeature feature, int fieldSize = 20);

    // Returns all features in display order (for populating the combo box).
    QVector<InputFeature> allFeatures();

    // -----------------------------------------------------------------------
    //  Label / topology helpers
    // -----------------------------------------------------------------------

    // Human-readable English label for one neuron (used in ViewNet).
    // For CELL_AT_INDEX the label includes the derived row / column.
    QString neuronLabel(const InputNeuronConfig& cfg, int fieldSize);

    // Returns one label per input neuron  →  pass to ViewNet::setInputPrefix().
    QStringList generateInputLabels(const NetworkConfig& cfg);

    // Builds the topology string for the Population constructor,
    // e.g. "24_SUM_RELU,25_SUM_RELU,18_SUM_RELU,04_SUM_SMAX".
    // The fixed output layer "04_SUM_SMAX" is always appended automatically.
    std::string buildTopologyString(const NetworkConfig& cfg);

    // Returns the total number of weights (connections) in a fully-connected
    // feed-forward network described by cfg.
    // Formula: sum over consecutive layer-size pairs (inputs → h0 → … → 4).
    int totalConnections(const NetworkConfig& cfg);

} // namespace InputConfig
