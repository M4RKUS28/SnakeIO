#include "inputconfig.h"

#include <cmath>
#include <QStringList>

// ===========================================================================
//  FeatureInfo table
//  Edit only this function when adding a new InputFeature.
// ===========================================================================
InputConfig::FeatureInfo InputConfig::getFeatureInfo(InputFeature feature, int /*fieldSize*/)
{
    switch (feature) {
    // --- Food direction ---
    case InputFeature::FOOD_DIR_NW:       return { "Food Direction — NW ↖",        "🍎 ↖",   false };
    case InputFeature::FOOD_DIR_N:        return { "Food Direction — N  ↑",         "🍎 ↑",   false };
    case InputFeature::FOOD_DIR_NE:       return { "Food Direction — NE ↗",         "🍎 ↗",   false };
    case InputFeature::FOOD_DIR_W:        return { "Food Direction — W  ←",         "🍎 ←",   false };
    case InputFeature::FOOD_DIR_E:        return { "Food Direction — E  →",         "🍎 →",   false };
    case InputFeature::FOOD_DIR_SW:       return { "Food Direction — SW ↙",         "🍎 ↙",   false };
    case InputFeature::FOOD_DIR_S:        return { "Food Direction — S  ↓",         "🍎 ↓",   false };
    case InputFeature::FOOD_DIR_SE:       return { "Food Direction — SE ↘",         "🍎 ↘",   false };
    // --- Body proximity ---
    case InputFeature::BODY_PROX_NW:      return { "Body Proximity — NW ↖",         "🐍 ↖",   false };
    case InputFeature::BODY_PROX_N:       return { "Body Proximity — N  ↑",         "🐍 ↑",   false };
    case InputFeature::BODY_PROX_NE:      return { "Body Proximity — NE ↗",         "🐍 ↗",   false };
    case InputFeature::BODY_PROX_W:       return { "Body Proximity — W  ←",         "🐍 ←",   false };
    case InputFeature::BODY_PROX_E:       return { "Body Proximity — E  →",         "🐍 →",   false };
    case InputFeature::BODY_PROX_SW:      return { "Body Proximity — SW ↙",         "🐍 ↙",   false };
    case InputFeature::BODY_PROX_S:       return { "Body Proximity — S  ↓",         "🐍 ↓",   false };
    case InputFeature::BODY_PROX_SE:      return { "Body Proximity — SE ↘",         "🐍 ↘",   false };
    // --- Wall distance ---
    case InputFeature::WALL_DIST_NW:      return { "Wall Distance — NW ↖",          "🚧 ↖",   false };
    case InputFeature::WALL_DIST_N:       return { "Wall Distance — N  ↑",          "🚧 ↑",   false };
    case InputFeature::WALL_DIST_NE:      return { "Wall Distance — NE ↗",          "🚧 ↗",   false };
    case InputFeature::WALL_DIST_W:       return { "Wall Distance — W  ←",          "🚧 ←",   false };
    case InputFeature::WALL_DIST_E:       return { "Wall Distance — E  →",          "🚧 →",   false };
    case InputFeature::WALL_DIST_SW:      return { "Wall Distance — SW ↙",          "🚧 ↙",   false };
    case InputFeature::WALL_DIST_S:       return { "Wall Distance — S  ↓",          "🚧 ↓",   false };
    case InputFeature::WALL_DIST_SE:      return { "Wall Distance — SE ↘",          "🚧 ↘",   false };
    // --- Cell value ---
    case InputFeature::CELL_AT_INDEX:     return { "Cell Value at Index [param]",   "Cell",   true  };
    // --- Food metrics ---
    case InputFeature::FOOD_DISTANCE:     return { "Food Distance (normalised)",    "FoodDist", false };
    case InputFeature::FOOD_ANGLE:        return { "Food Angle (normalised)",       "FoodAngle", false };
    // --- Snake state ---
    case InputFeature::MOVES_LEFT:        return { "Remaining Moves (normalised)",  "MovesLeft", false };
    case InputFeature::SNAKE_LENGTH:      return { "Snake Length (normalised)",     "Length",  false };
    // --- Danger ---
    case InputFeature::DANGER_STRAIGHT:   return { "Danger — Straight ahead",       "⚠ Fwd",  false };
    case InputFeature::DANGER_RIGHT:      return { "Danger — Right",                "⚠ →",    false };
    case InputFeature::DANGER_LEFT:       return { "Danger — Left",                 "⚠ ←",    false };
    // --- Current direction ---
    case InputFeature::DIR_N:             return { "Direction Moving — N  ↑",       "Dir ↑",   false };
    case InputFeature::DIR_W:             return { "Direction Moving — W  ←",       "Dir ←",   false };
    case InputFeature::DIR_E:             return { "Direction Moving — E  →",       "Dir →",   false };
    case InputFeature::DIR_S:             return { "Direction Moving — S  ↓",       "Dir ↓",   false };
    // --- Enemy proximity ---
    case InputFeature::ENEMY_PROX_N:      return { "Enemy Proximity — N  ↑",        "👾 ↑",   false };
    case InputFeature::ENEMY_PROX_W:      return { "Enemy Proximity — W  ←",        "👾 ←",   false };
    case InputFeature::ENEMY_PROX_E:      return { "Enemy Proximity — E  →",        "👾 →",   false };
    case InputFeature::ENEMY_PROX_S:      return { "Enemy Proximity — S  ↓",        "👾 ↓",   false };
    // --- Feedback ---
    case InputFeature::LAST_OUTPUT_UP:    return { "Last Output — Up",              "Out↑-1",  false };
    case InputFeature::LAST_OUTPUT_DOWN:  return { "Last Output — Down",            "Out↓-1",  false };
    case InputFeature::LAST_OUTPUT_RIGHT: return { "Last Output — Right",           "Out→-1",  false };
    case InputFeature::LAST_OUTPUT_LEFT:  return { "Last Output — Left",            "Out←-1",  false };
    // --- Constants ---
    case InputFeature::CONST_ZERO:         return { "Constant — always 0.0",        "□ 0",     false };
    case InputFeature::CONST_ONE:          return { "Constant — always 1.0",        "■ 1",     false };
    // --- Legacy wall distance (pre-rework hyperbolic formula) ---
    case InputFeature::LEGACY_WALL_DIST_N: return { "Wall Dist N — Legacy ↑",       "Wleg↑",   false };
    case InputFeature::LEGACY_WALL_DIST_W: return { "Wall Dist W — Legacy ←",       "Wleg←",   false };
    case InputFeature::LEGACY_WALL_DIST_E: return { "Wall Dist E — Legacy →",       "Wleg→",   false };
    case InputFeature::LEGACY_WALL_DIST_S: return { "Wall Dist S — Legacy ↓",       "Wleg↓",   false };
    case InputFeature::LEGACY_FOOD_ANGLE:  return { "Food Angle — Legacy (QLineF)",  "FAleg",   false };
    case InputFeature::LEGACY_FOOD_DIST:   return { "Food Dist — Legacy 2/(d+1)",    "FDleg",   false };
    }
    return { "Unknown", "?", false };
}

// ===========================================================================
//  allFeatures() — canonical order for the combo box
// ===========================================================================
QVector<InputFeature> InputConfig::allFeatures()
{
    return {
        // Food direction
        InputFeature::FOOD_DIR_NW, InputFeature::FOOD_DIR_N,  InputFeature::FOOD_DIR_NE,
        InputFeature::FOOD_DIR_W,  InputFeature::FOOD_DIR_E,
        InputFeature::FOOD_DIR_SW, InputFeature::FOOD_DIR_S,  InputFeature::FOOD_DIR_SE,
        // Body proximity
        InputFeature::BODY_PROX_NW, InputFeature::BODY_PROX_N,  InputFeature::BODY_PROX_NE,
        InputFeature::BODY_PROX_W,  InputFeature::BODY_PROX_E,
        InputFeature::BODY_PROX_SW, InputFeature::BODY_PROX_S,  InputFeature::BODY_PROX_SE,
        // Wall distance
        InputFeature::WALL_DIST_NW, InputFeature::WALL_DIST_N,  InputFeature::WALL_DIST_NE,
        InputFeature::WALL_DIST_W,  InputFeature::WALL_DIST_E,
        InputFeature::WALL_DIST_SW, InputFeature::WALL_DIST_S,  InputFeature::WALL_DIST_SE,
        // Cell value
        InputFeature::CELL_AT_INDEX,
        // Food metrics
        InputFeature::FOOD_DISTANCE, InputFeature::FOOD_ANGLE,
        // Snake state
        InputFeature::MOVES_LEFT, InputFeature::SNAKE_LENGTH,
        // Danger
        InputFeature::DANGER_STRAIGHT, InputFeature::DANGER_RIGHT, InputFeature::DANGER_LEFT,
        // Direction
        InputFeature::DIR_N, InputFeature::DIR_W, InputFeature::DIR_E, InputFeature::DIR_S,
        // Enemy proximity
        InputFeature::ENEMY_PROX_N, InputFeature::ENEMY_PROX_W,
        InputFeature::ENEMY_PROX_E, InputFeature::ENEMY_PROX_S,
        // Feedback
        InputFeature::LAST_OUTPUT_UP,   InputFeature::LAST_OUTPUT_DOWN,
        InputFeature::LAST_OUTPUT_RIGHT, InputFeature::LAST_OUTPUT_LEFT,
        // Constants
        InputFeature::CONST_ZERO, InputFeature::CONST_ONE,
        // Legacy wall distance
        InputFeature::LEGACY_WALL_DIST_N, InputFeature::LEGACY_WALL_DIST_W,
        InputFeature::LEGACY_WALL_DIST_E, InputFeature::LEGACY_WALL_DIST_S,
        // Legacy food metrics
        InputFeature::LEGACY_FOOD_ANGLE, InputFeature::LEGACY_FOOD_DIST,
    };
}

// ===========================================================================
//  neuronLabel()
// ===========================================================================
QString InputConfig::neuronLabel(const InputNeuronConfig& cfg, int fieldSize)
{
    if (cfg.feature == InputFeature::CELL_AT_INDEX) {
        int col = cfg.param % fieldSize;
        int row = cfg.param / fieldSize;
        return QString("Cell [%1,%2]").arg(col).arg(row);
    }
    return getFeatureInfo(cfg.feature, fieldSize).labelShort;
}

// ===========================================================================
//  generateInputLabels()
// ===========================================================================
QStringList InputConfig::generateInputLabels(const NetworkConfig& cfg)
{
    QStringList labels;
    labels.reserve(cfg.inputs.size());
    for (const InputNeuronConfig& n : cfg.inputs)
        labels << neuronLabel(n, cfg.fieldSize);
    return labels;
}

// ===========================================================================
//  buildTopologyString()
//  Format: "N_AGG_ACT,M_AGG_ACT,...,04_SUM_SMAX"
// ===========================================================================
std::string InputConfig::buildTopologyString(const NetworkConfig& cfg)
{
    std::string result;

    // Input layer (implicit in GenNet — just the count determines first layer size)
    result += std::to_string(cfg.inputCount()) + "_SUM_RELU";

    // Hidden layers
    for (const HiddenLayerConfig& layer : cfg.hiddenLayers) {
        result += ",";
        result += std::to_string(layer.neurons);
        result += "_";
        result += layer.aggregation.toStdString();
        result += "_";
        result += layer.activation.toStdString();
    }

    // Fixed output layer: 4 neurons, SUM, Softmax
    result += ",04_SUM_SMAX";

    return result;
}

// ===========================================================================
//  Pre-configurations (factory methods)
// ===========================================================================

// ---------------------------------------------------------------------------
//  makeClassic  —  24 inputs matching the classic "ray-cast" layout
// ---------------------------------------------------------------------------
NetworkConfig NetworkConfig::makeClassic(int fieldSize, int snakeCount)
{
    NetworkConfig cfg;
    cfg.fieldSize  = fieldSize;
    cfg.snakeCount = snakeCount;

    // 8 × food direction (one-hot)
    cfg.inputs << InputNeuronConfig{ InputFeature::FOOD_DIR_NW }
               << InputNeuronConfig{ InputFeature::FOOD_DIR_N  }
               << InputNeuronConfig{ InputFeature::FOOD_DIR_NE }
               << InputNeuronConfig{ InputFeature::FOOD_DIR_W  }
               << InputNeuronConfig{ InputFeature::FOOD_DIR_E  }
               << InputNeuronConfig{ InputFeature::FOOD_DIR_SW }
               << InputNeuronConfig{ InputFeature::FOOD_DIR_S  }
               << InputNeuronConfig{ InputFeature::FOOD_DIR_SE };

    // 8 × body proximity (inverse distance)
    cfg.inputs << InputNeuronConfig{ InputFeature::BODY_PROX_NW }
               << InputNeuronConfig{ InputFeature::BODY_PROX_N  }
               << InputNeuronConfig{ InputFeature::BODY_PROX_NE }
               << InputNeuronConfig{ InputFeature::BODY_PROX_W  }
               << InputNeuronConfig{ InputFeature::BODY_PROX_E  }
               << InputNeuronConfig{ InputFeature::BODY_PROX_SW }
               << InputNeuronConfig{ InputFeature::BODY_PROX_S  }
               << InputNeuronConfig{ InputFeature::BODY_PROX_SE };

    // 8 × wall distance (normalised)
    cfg.inputs << InputNeuronConfig{ InputFeature::WALL_DIST_NW }
               << InputNeuronConfig{ InputFeature::WALL_DIST_N  }
               << InputNeuronConfig{ InputFeature::WALL_DIST_NE }
               << InputNeuronConfig{ InputFeature::WALL_DIST_W  }
               << InputNeuronConfig{ InputFeature::WALL_DIST_E  }
               << InputNeuronConfig{ InputFeature::WALL_DIST_SW }
               << InputNeuronConfig{ InputFeature::WALL_DIST_S  }
               << InputNeuronConfig{ InputFeature::WALL_DIST_SE };

    // Default hidden layers (matching the previously hard-coded topology)
    cfg.hiddenLayers << HiddenLayerConfig{ 25, "SUM", "RELU" }
                     << HiddenLayerConfig{ 18, "SUM", "RELU" };

    return cfg;
}

// ---------------------------------------------------------------------------
//  makeFullField  —  fieldSize² inputs, one per cell
// ---------------------------------------------------------------------------
NetworkConfig NetworkConfig::makeFullField(int fieldSize, int snakeCount)
{
    NetworkConfig cfg;
    cfg.fieldSize  = fieldSize;
    cfg.snakeCount = snakeCount;

    const int cellCount = fieldSize * fieldSize;
    cfg.inputs.reserve(cellCount);
    for (int i = 0; i < cellCount; ++i)
        cfg.inputs << InputNeuronConfig{ InputFeature::CELL_AT_INDEX, i };

    // Default hidden layers matching the previously hard-coded image-based topology
    cfg.hiddenLayers << HiddenLayerConfig{ 100, "SUM", "RELU" }
                     << HiddenLayerConfig{ 100, "SUM", "RELU" }
                     << HiddenLayerConfig{  20, "SUM", "RELU" };

    return cfg;
}

// ---------------------------------------------------------------------------
//  makeTurnMode  —  11 inputs (relative to current heading)
// ---------------------------------------------------------------------------
NetworkConfig NetworkConfig::makeTurnMode(int fieldSize, int snakeCount)
{
    NetworkConfig cfg;
    cfg.fieldSize  = fieldSize;
    cfg.snakeCount = snakeCount;

    // 3 × danger (relative)
    cfg.inputs << InputNeuronConfig{ InputFeature::DANGER_STRAIGHT }
               << InputNeuronConfig{ InputFeature::DANGER_RIGHT    }
               << InputNeuronConfig{ InputFeature::DANGER_LEFT     };

    // 4 × current direction (one-hot, absolute)
    cfg.inputs << InputNeuronConfig{ InputFeature::DIR_N }
               << InputNeuronConfig{ InputFeature::DIR_W }
               << InputNeuronConfig{ InputFeature::DIR_E }
               << InputNeuronConfig{ InputFeature::DIR_S };

    // 4 × food direction (one-hot, cardinal only)
    cfg.inputs << InputNeuronConfig{ InputFeature::FOOD_DIR_N }
               << InputNeuronConfig{ InputFeature::FOOD_DIR_W }
               << InputNeuronConfig{ InputFeature::FOOD_DIR_E }
               << InputNeuronConfig{ InputFeature::FOOD_DIR_S };

    cfg.hiddenLayers << HiddenLayerConfig{ 12, "SUM", "RELU" };

    return cfg;
}

// ===========================================================================
//  NetworkConfig::makeDemo()
//  Replicates the pre-rework DETAILED_CLASSIC 24-input layout exactly.
//  Positions that were always 0 in the old code use CONST_ZERO.
//  Diagonal food-direction indices 0 and 2 are swapped (matching the original
//  bug where NW/NE labels were accidentally transposed in the old code).
//  Wall distances use LEGACY_WALL_DIST_* to match the original hyperbolic
//  formula: 1/(1 - dist/(fieldSize*sqrt(2))) - 1.
// ===========================================================================
NetworkConfig NetworkConfig::makeDemo(int fieldSize, int snakeCount)
{
    NetworkConfig cfg;
    cfg.fieldSize  = fieldSize;
    cfg.snakeCount = snakeCount;

    using F = InputFeature;
    // --- Food direction [0..7] ---
    // Note: indices 0 and 2 are intentionally swapped vs. the compass layout
    // to reproduce the NW/NE transposition bug present in the trained model.
    cfg.inputs << InputNeuronConfig{ F::FOOD_DIR_NE };   // [0] labelled NW in old code, actually NE
    cfg.inputs << InputNeuronConfig{ F::FOOD_DIR_N  };   // [1] North
    cfg.inputs << InputNeuronConfig{ F::FOOD_DIR_NW };   // [2] labelled NE in old code, actually NW
    cfg.inputs << InputNeuronConfig{ F::FOOD_DIR_W  };   // [3] West
    cfg.inputs << InputNeuronConfig{ F::FOOD_DIR_E  };   // [4] East
    cfg.inputs << InputNeuronConfig{ F::FOOD_DIR_SW };   // [5] SW
    cfg.inputs << InputNeuronConfig{ F::FOOD_DIR_S  };   // [6] South
    cfg.inputs << InputNeuronConfig{ F::FOOD_DIR_SE };   // [7] SE

    // --- Body proximity [8..15] --- (diagonals always 0 in old code)
    cfg.inputs << InputNeuronConfig{ F::CONST_ZERO   };  // [8]  NW body (always 0)
    cfg.inputs << InputNeuronConfig{ F::BODY_PROX_N  };  // [9]  North
    cfg.inputs << InputNeuronConfig{ F::CONST_ZERO   };  // [10] NE body (always 0)
    cfg.inputs << InputNeuronConfig{ F::BODY_PROX_W  };  // [11] West
    cfg.inputs << InputNeuronConfig{ F::BODY_PROX_E  };  // [12] East
    cfg.inputs << InputNeuronConfig{ F::LEGACY_FOOD_ANGLE };  // [13] food angle (QLineF formula, overwritten in old code)
    cfg.inputs << InputNeuronConfig{ F::BODY_PROX_S        };  // [14] South
    cfg.inputs << InputNeuronConfig{ F::CONST_ZERO         };  // [15] SE body (enemy west check, 0 in single-player)

    // --- Wall distance [16..23] --- (diagonals always 0 in old code)
    cfg.inputs << InputNeuronConfig{ F::CONST_ZERO        };  // [16] NW wall (always 0)
    cfg.inputs << InputNeuronConfig{ F::LEGACY_WALL_DIST_N };  // [17] North
    cfg.inputs << InputNeuronConfig{ F::CONST_ZERO        };  // [18] NE wall (always 0)
    cfg.inputs << InputNeuronConfig{ F::LEGACY_WALL_DIST_W };  // [19] West
    cfg.inputs << InputNeuronConfig{ F::LEGACY_WALL_DIST_E };  // [20] East
    cfg.inputs << InputNeuronConfig{ F::LEGACY_FOOD_DIST   };  // [21] inverse food distance (2/(d+1), overwritten in old code)
    cfg.inputs << InputNeuronConfig{ F::LEGACY_WALL_DIST_S };  // [22] South
    cfg.inputs << InputNeuronConfig{ F::CONST_ZERO        };  // [23] SE wall (always 0)

    cfg.hiddenLayers << HiddenLayerConfig{ 25, "SUM", "RELU" };
    cfg.hiddenLayers << HiddenLayerConfig{ 18, "SUM", "RELU" };

    return cfg;
}

// ===========================================================================
//  totalConnections()
// ===========================================================================
int InputConfig::totalConnections(const NetworkConfig& cfg)
{
    int prev  = cfg.inputs.size();
    int total = 0;
    for (const HiddenLayerConfig& h : cfg.hiddenLayers) {
        total += prev * h.neurons;
        prev   = h.neurons;
    }
    total += prev * 4;   // last hidden → output (always 4 neurons)
    return total;
}

// ===========================================================================
//  featureEnumName()  —  returns the C++ enum identifier as a string
// ===========================================================================
QString InputConfig::featureEnumName(InputFeature feature)
{
    switch (feature) {
    case InputFeature::FOOD_DIR_NW:          return "FOOD_DIR_NW";
    case InputFeature::FOOD_DIR_N:           return "FOOD_DIR_N";
    case InputFeature::FOOD_DIR_NE:          return "FOOD_DIR_NE";
    case InputFeature::FOOD_DIR_W:           return "FOOD_DIR_W";
    case InputFeature::FOOD_DIR_E:           return "FOOD_DIR_E";
    case InputFeature::FOOD_DIR_SW:          return "FOOD_DIR_SW";
    case InputFeature::FOOD_DIR_S:           return "FOOD_DIR_S";
    case InputFeature::FOOD_DIR_SE:          return "FOOD_DIR_SE";
    case InputFeature::BODY_PROX_NW:         return "BODY_PROX_NW";
    case InputFeature::BODY_PROX_N:          return "BODY_PROX_N";
    case InputFeature::BODY_PROX_NE:         return "BODY_PROX_NE";
    case InputFeature::BODY_PROX_W:          return "BODY_PROX_W";
    case InputFeature::BODY_PROX_E:          return "BODY_PROX_E";
    case InputFeature::BODY_PROX_SW:         return "BODY_PROX_SW";
    case InputFeature::BODY_PROX_S:          return "BODY_PROX_S";
    case InputFeature::BODY_PROX_SE:         return "BODY_PROX_SE";
    case InputFeature::WALL_DIST_NW:         return "WALL_DIST_NW";
    case InputFeature::WALL_DIST_N:          return "WALL_DIST_N";
    case InputFeature::WALL_DIST_NE:         return "WALL_DIST_NE";
    case InputFeature::WALL_DIST_W:          return "WALL_DIST_W";
    case InputFeature::WALL_DIST_E:          return "WALL_DIST_E";
    case InputFeature::WALL_DIST_SW:         return "WALL_DIST_SW";
    case InputFeature::WALL_DIST_S:          return "WALL_DIST_S";
    case InputFeature::WALL_DIST_SE:         return "WALL_DIST_SE";
    case InputFeature::CELL_AT_INDEX:        return "CELL_AT_INDEX";
    case InputFeature::FOOD_DISTANCE:        return "FOOD_DISTANCE";
    case InputFeature::FOOD_ANGLE:           return "FOOD_ANGLE";
    case InputFeature::MOVES_LEFT:           return "MOVES_LEFT";
    case InputFeature::SNAKE_LENGTH:         return "SNAKE_LENGTH";
    case InputFeature::DANGER_STRAIGHT:      return "DANGER_STRAIGHT";
    case InputFeature::DANGER_RIGHT:         return "DANGER_RIGHT";
    case InputFeature::DANGER_LEFT:          return "DANGER_LEFT";
    case InputFeature::DIR_N:                return "DIR_N";
    case InputFeature::DIR_W:                return "DIR_W";
    case InputFeature::DIR_E:                return "DIR_E";
    case InputFeature::DIR_S:                return "DIR_S";
    case InputFeature::ENEMY_PROX_N:         return "ENEMY_PROX_N";
    case InputFeature::ENEMY_PROX_W:         return "ENEMY_PROX_W";
    case InputFeature::ENEMY_PROX_E:         return "ENEMY_PROX_E";
    case InputFeature::ENEMY_PROX_S:         return "ENEMY_PROX_S";
    case InputFeature::LAST_OUTPUT_UP:       return "LAST_OUTPUT_UP";
    case InputFeature::LAST_OUTPUT_DOWN:     return "LAST_OUTPUT_DOWN";
    case InputFeature::LAST_OUTPUT_RIGHT:    return "LAST_OUTPUT_RIGHT";
    case InputFeature::LAST_OUTPUT_LEFT:     return "LAST_OUTPUT_LEFT";
    case InputFeature::CONST_ZERO:           return "CONST_ZERO";
    case InputFeature::CONST_ONE:            return "CONST_ONE";
    case InputFeature::LEGACY_WALL_DIST_N:   return "LEGACY_WALL_DIST_N";
    case InputFeature::LEGACY_WALL_DIST_W:   return "LEGACY_WALL_DIST_W";
    case InputFeature::LEGACY_WALL_DIST_E:   return "LEGACY_WALL_DIST_E";
    case InputFeature::LEGACY_WALL_DIST_S:   return "LEGACY_WALL_DIST_S";
    case InputFeature::LEGACY_FOOD_ANGLE:    return "LEGACY_FOOD_ANGLE";
    case InputFeature::LEGACY_FOOD_DIST:     return "LEGACY_FOOD_DIST";
    }
    return "UNKNOWN";
}

// ===========================================================================
//  networkConfigToJson()
//  Produces a human-readable JSON string describing the full architecture.
//  Written as  <baseName>_arch.json  alongside snake.csv / apple.seed.
// ===========================================================================
QString InputConfig::networkConfigToJson(const NetworkConfig& cfg)
{
    QString s;
    s += "{\n";

    // --- meta ---
    s += QString("  \"fieldSize\"  : %1,\n").arg(cfg.fieldSize);
    s += QString("  \"snakeCount\" : %1,\n").arg(cfg.snakeCount);
    s += QString("  \"topology\"   : \"%1\",\n\n")
            .arg(QString::fromStdString(buildTopologyString(cfg)));

    // --- input layer ---
    s += QString("  \"inputLayer\" : {\n");
    s += QString("    \"neuronCount\" : %1,\n").arg(cfg.inputs.size());
    s += QString("    \"neurons\" : [\n");
    for (int i = 0; i < cfg.inputs.size(); ++i) {
        const InputNeuronConfig& n = cfg.inputs[i];
        QString enumName = featureEnumName(n.feature);
        QString desc     = getFeatureInfo(n.feature, cfg.fieldSize).displayName;
        // escape any special chars in desc (rare, but safe)
        desc.replace("\\", "\\\\").replace("\"", "\\\"");
        s += "      { ";
        s += QString("\"index\": %1, ").arg(i);
        s += QString("\"feature\": \"%1\", ").arg(enumName);
        if (n.feature == InputFeature::CELL_AT_INDEX)
            s += QString("\"param\": %1, ").arg(n.param);
        s += QString("\"description\": \"%1\"").arg(desc);
        s += " }";
        if (i < cfg.inputs.size() - 1) s += ",";
        s += "\n";
    }
    s += "    ]\n";
    s += "  },\n\n";

    // --- hidden layers ---
    s += "  \"hiddenLayers\" : [\n";
    for (int i = 0; i < cfg.hiddenLayers.size(); ++i) {
        const HiddenLayerConfig& h = cfg.hiddenLayers[i];
        s += "    { ";
        s += QString("\"index\": %1, ").arg(i);
        s += QString("\"neurons\": %1, ").arg(h.neurons);
        s += QString("\"aggregation\": \"%1\", ").arg(h.aggregation);
        s += QString("\"activation\": \"%1\"").arg(h.activation);
        s += " }";
        if (i < cfg.hiddenLayers.size() - 1) s += ",";
        s += "\n";
    }
    s += "  ],\n\n";

    // --- output layer (always fixed) ---
    s += "  \"outputLayer\" : {\n";
    s += "    \"neurons\": 4,\n";
    s += "    \"aggregation\": \"SUM\",\n";
    s += "    \"activation\": \"SMAX\",\n";
    s += "    \"outputs\": [\"Up\", \"Down\", \"Right\", \"Left\"]\n";
    s += "  }\n";

    s += "}\n";
    return s;
}
