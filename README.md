# SnakeIO — Snake-KI mit neuroevolutionärem Training

SnakeIO ist eine Qt-Applikation, die eine Population von Snake-KIs mittels **Neuroevolution** trainiert und dabei den gesamten Lernprozess in Echtzeit visualisiert. Das Projekt verbindet echtes maschinelles Lernen mit einem spielbaren Snake-Spiel und erlaubt es, alle Parameter des neuronalen Netzes sowie den Evolutionsprozess direkt über die Oberfläche zu steuern.

---

## Kernidee: Neuroevolution

Die KI lernt **nicht** durch Backpropagation oder Reinforcement Learning mit Rewards, sondern durch einen evolutionären Prozess:

1. Eine **Population** von N Schlangen (typisch: 2000) startet gleichzeitig auf dem Spielfeld.
2. Jede Schlange wird von einem **eigenen neuronalen Netz** gesteuert — Gewichte starten zufällig.
3. Alle Schlangen spielen parallel, jede auf ihrem eigenen Thread.
4. Sobald alle Schlangen tot sind, wird die **Fitnesskennzahl** berechnet:
   `Score = Anzahl fressene Äpfel × 300 + Überlebenszeit (Ticks)`
5. Das Netz der besten Schlange wird als **Eltern-Netz** verwendet — alle anderen N-1 Netze werden als **mutierte Kopien** davon neu erzeugt.
6. Die nächste Generation startet sofort automatisch.

Dieser Zyklus (Spielen → Selektion → Mutation → Spielen) wiederholt sich vollautomatisch und konvergiert über Generationen zu einem strategisch spielenden Agenten.

---

## Das neuronale Netz

Jedes Netz ist ein **Feedforward-Netz** mit konfigurierbarer Topologie, implementiert in der mitgelieferten Bibliothek **GenNet** (reines C++17, keine Qt-Abhängigkeit).

### Eingabeschicht — vollständig konfigurierbar

Das Besondere an SnakeIO ist, dass die **Eingabeneuronen pro Training frei zusammengestellt** werden können. Jedes Neuron liest einen bestimmten Aspekt des Spielzustands aus:

| Kategorie | Was das Neuron misst |
|---|---|
| **Nahrungsrichtung** | Liegt der Apfel auf einem der 8 Kompassstrahlen vom Kopf aus? (1.0 / 0.0) |
| **Körpernähe** | Inverse Distanz zum nächsten eigenen Körpersegment in 8 Richtungen |
| **Wanddistanz** | Normierter Abstand zur Wand in 8 Richtungen |
| **Zellwert** | Direkte Zellabfrage: −1 = Körper, 0 = leer, +1 = Apfel (ein Neuron pro Zelle) |
| **Nahrungsmetriken** | Euklidische Distanz zum Apfel / Winkel zum Apfel |
| **Schlangenzustand** | Verbleibende Züge (normiert), aktuelle Länge (normiert) |
| **Gefahrensensoren** | Direkte Kollisionsgefahr geradeaus / rechts / links (relativ zur Fahrtrichtung) |
| **Fahrtrichtung** | Aktuelle Richtung als One-Hot (N/S/E/W) |
| **Gegnernähe** | Inverse Distanz zu Gegnersegmenten (PvE-Modus) |
| **Rückkopplung** | Die vier Ausgabewerte des letzten Ticks (Netz sieht seine eigene letzte Entscheidung) |
| **Konstanten** | Immer 0.0 oder 1.0 (zum Auffüllen/Testen von Architekturen) |
| **Legacy-Inputs** | Historische Formeln für Wanddistanz und Nahrungswinkel (Kompatibilität mit alten Gewichten) |

### Versteckte Schichten

Beliebig viele versteckte Schichten, jeweils mit einstellbarer:
- **Neuronenzahl**
- **Aggregationsfunktion**: SUM, AVG, MAX, MIN
- **Aktivierungsfunktion**: RELU, TANH, SIGMOID, LEAKYRELU, SOFTPLUS, SMAX, IDENTITY

### Ausgabeschicht (fest)

Immer 4 Neuronen mit Softmax-Aktivierung:

```
↑ Up    ↓ Down    → Right    ← Left
```

Das Netz berechnet pro Tick einen Argmax über diese 4 Werte — die Richtung mit dem höchsten Wert gewinnt.

---

## Voreinstellungen (Preset-Konfigurationen)

Im Startdialog stehen vier fertige Architekturen zur Auswahl:

| Preset | Eingaben | Beschreibung |
|---|---|---|
| **Classic** | 24 | 8× Nahrungsrichtung + 8× Körpernähe + 8× Wanddistanz. Versteckt: 25-RELU → 18-RELU |
| **Full Field** | fieldSize² | Ein CELL_AT_INDEX-Neuron pro Spielfeldzelle (−1/0/+1). Lernt durch vollständige Weltsicht |
| **Turn Mode** | 11 | 3× Gefahrensensoren + 4× Fahrtrichtung + 2× Nahrungswinkel/Distanz + 2× Legacy-Nahrung. Kompaktes relatives Modell |
| **Demo** | 24 | Legacy-Format (pre-rework), kompatibel mit den mitgelieferten trainierten Gewichten |
| **Custom** | beliebig | Volle manuelle Konfiguration im Netzwerk-Editor |

---

## Evolutionsalgorithmen

Zwei Strategien sind direkt im Training-Lab umschaltbar:

### 1. Fixed Split Mutation (Standard)
- Das beste Netz der Generation wird **1:1 kopiert** für alle N-1 Nachkommen.
- Jedes Gewicht jedes Nachkommers wird mit der eingestellten **Mutationsrate** (Wahrscheinlichkeit) und dem **Mutationsbereich** (max. Änderungsgröße) unabhängig zufällig verschoben.
- Elitismus: Das Original-Eltern-Netz überlebt unverändert in die nächste Generation.

### 2. Simulated Annealing
- Ähnlich wie Fixed Split, aber die Akzeptanzwahrscheinlichkeit für schlechtere Mutationen nimmt mit einem **Temperaturfaktor** (Decay: 0.99/Generation) ab.
- Hilft, lokale Optima in frühen Generationen zu überwinden.
- Nutzt die Score-Map (`population->scoreMap()`) aller Schlangen als Fitnessbewertung.

### Automatisches Backup
Nach jeder Evolution wird das beste Netz automatisch als `current_best_ai-bak.csv` gespeichert.

---

## Spielmodi

### Training Lab
Das Hauptfenster während des Trainings. Zeigt:
- Das **Spielfeld** der gerade beobachteten Schlange mit optionalem Raster und Eingabe-Strahlen
- Die **Netzwerkvisualisierung** (ViewNet) des aktuell fokussierten Netzes mit Live-Gewichten und beschrifteten Ein-/Ausgabeneuronen
- **Statusinfos**: aktueller Score, Länge, verbleibende Züge, Highscore, Generation, lebende Schlangen
- **Trainingssteuerung**: Start/Stop, Mutationsrate und -bereich, Evolutionsalgorithmus

Wenn das Auto-Switch aktiviert ist, folgt die Kamera automatisch der aktuell besten lebenden Schlange.

### Demo-Modus
Spielt ein **vortrainiertes Modell** ab, das mit dem Demo-Preset trainiert wurde (Score: 147) und einen charakteristischen Zick-Zack-Stil entwickelt hat. Netzwerkvisualisierung und Live-Gewichte sind standardmäßig aktiv. Kein Trainingsinterface — nur Beobachten.

### Selber spielen (im Training Lab)
Stoppt die KI-Simulation und übernimmt Snake 0 manuell mit den **Pfeiltasten**. Der Start/Stop-Button wechselt auf "⏹ Stop" — Drücken beendet den Spieler-Modus. Nützlich um das Spielfeld zu erkunden oder das trainierte Schlangenverhalten mit menschlichem Spiel zu vergleichen.

### Player vs. AI (PvE-Modus)
Zwei synchronisierte Spielfelder nebeneinander:
- **Links**: Der menschliche Spieler (Pfeiltasten, Fokus liegt sofort hier)
- **Rechts**: Eine KI, die automatisch mit dem trainierten Demo-Modell geladen wird

Beide Schlangen spielen auf identischen Apfelpositionen (gleicher Seed). Die KI beschleunigt nach dem Tod des Spielers stufenweise von 600 % auf bis zu 100.000 %, sodass die restliche Strecke immer noch beobachtbar ist. Ergebnis- und Score-Labels zeigen den Vergleich.

---

## Fitnessfunktion

```
Score = fressene_Äpfel × 300 + Überlebenszeit_in_Ticks
```

Die maximale Zugzahl pro Leben begrenzt Endlosschleifen:

```
MaxMoves = fieldSize × 10 + 2 × aktuelle_Körperlänge
```

Pro gefressenen Apfel werden zusätzliche Züge gutgeschrieben (bis zum Maximum), was das Netz dazu zwingt, aktiv nach Nahrung zu suchen statt Zeit zu schinden.

---

## Netzwerk-Editor (Startdialog)

Beim Start öffnet sich ein Konfigurationsdialog mit drei Bereichen:

- **Eingabeneuronen**: Tabelleneditor zum Hinzufügen, Entfernen und Konfigurieren aller Input-Neuronen. Jede Zeile zeigt Feature-Typ, optionalen Parameter (für CELL_AT_INDEX: Zellenindex) und eine Live-Vorschau des Neuronenlabels.
- **Versteckte Schichten**: Tabelleneditor für beliebig viele Hidden Layers mit Neuronenzahl, Aggregations- und Aktivierungsfunktion.
- **Ausgabeschicht**: Nur-Lesen-Anzeige (immer 4 × SUM × SMAX).

Jede Änderung schaltet die Preset-Auswahl automatisch auf „Custom". Feldgröße und Schlangenanzahl sind als Basiseinstellungen ebenfalls editierbar.

---

## Export / Import von trainierten Modellen

Trainierte Netze können als **Datei-Paar** gespeichert werden:

| Datei | Inhalt |
|---|---|
| `<name>_snake.csv` | Serializiertes Netz (Gewichte als CSV, GenNet-Format) |
| `<name>_apple.seed` | Zufalls-Seed des Spielfeldes zur Reproduzierbarkeit |
| `<name>_arch.json` | Netzwerk-Architektur (InputConfig/NetworkConfig als JSON) |

Beim Import wird automatisch erkannt, ob der Dateiname `_snake.csv` oder `_apple.seed` enthält — beide führen zum gleichen Ladevorgang. Importierte Modelle laufen sofort weiter und können mit neuen Hyper-parametern weiter trainiert werden.

Mitgelieferte Snakes im `Snakes/`-Ordner decken verschiedene Trainingsstände ab (Scores von ~87 bis 147).

---

## Apfel-Debug-Werkzeuge

Für gezielte Experimente gibt es drei Werkzeuge im Training-Lab:

- **← Letzten**: Entfernt den nächsten (letzten vorgenerierten) Apfel aus der Warteschlange. Erlaubt es, der Schlange Apfelpositionen "vorzuenthalten".
- **⊞ Ecken**: Fügt Äpfel an allen vier Ecken und in der Mitte hinzu, sodass die KI das vollständige Spielfeld erkunden muss.
- **📋 Liste**: Öffnet einen Dialog, der alle vorgenerierten Apfelpositionen (in Reihenfolge) auflistet und selektives Löschen ermöglicht.

---

## Netzwerkvisualisierung (ViewNet)

Das linke Panel zeigt das neuronale Netz der aktuell beobachteten Schlange als interaktives Diagramm:

- **Neuronen** werden als Knoten dargestellt, **Gewichte** als Kanten
- Beschriftung der Eingabeneuronen mit den konfigurierten Feature-Labels (z. B. „🍎 ↑", „🐍 ←", „🚧 →")
- Ausgabeneuronen zeigen die Richtungen (↑ Up, ↓ Down, → Right, ← Left)
- Live-Aktualisierung der Aktivierungswerte pro Tick (bei niedrigen Geschwindigkeiten)
- Nach jeder Evolution werden die Gewichte des neuen Eltern-Netzes sofort neu gezeichnet
- Automatisch deaktiviert wenn das Netz > 2000 Verbindungen hat (z. B. Full-Field mit 20×20 = 400 Eingaben)

---

## Score-System in Zahlen

Ein typischer Trainingsverlauf:

| Stand | Verhalten |
|---|---|
| Generation 1–5 | Schlangen sterben meist nach wenigen Zügen, oft direkt gegen die Wand |
| Generation 10–30 | Erste Nahrungsaufnahme, beginnen Wände zu meiden |
| Generation 50–150 | Verlässliche Nahrungssuche, Score ~40–80 |
| Generation 500+ | Strategisches Spielen, Zick-Zack-Muster, Score >100 |
| Trainierte Demos | Score 117–147 (enthaltene Modelle) |

---

## Technische Besonderheiten

- **Vollständig multithreaded**: Jede Schlange läuft auf einem eigenen `QThread`. Der Game-Thread koordiniert Start, Stop und Evolution. Signale/Slots verbinden Threads sicher über Queued Connections.
- **Konfigurationsgetriebenes Input-System**: Die gesamte Eingabelogik (`evaluateFeature`) ist eine einzige switch/case-Funktion. Eine neue Inputart erfordert nur: Enum-Eintrag + case + FeatureInfo-Eintrag — der Rest (UI, Labels, Topology-String) folgt automatisch.
- **Deterministische Apfelsequenz**: Der Zufallsgenerator für Apfelpositionen wird per Seed initialisiert, sodass alle Schlangen einer Generation auf identischen Apfelpositionen spielen (fairer Vergleich). Der Seed kann exportiert und importiert werden.
- **Logarithmischer Geschwindigkeitsregler**: Der Geschwindigkeits-Slider mappt auf 100 % – 100.000.000 % (6 Zehnerpotenzen logarithmisch), was sowohl langsames Beobachten als auch maximale Trainingsgeschwindigkeit ohne Neustart ermöglicht.
- **Visuelle Strahlen-Anzeige**: Die 8 Eingabe-Strahlen können live eingeblendet werden, um zu sehen, welche Sichtrichtungen das Netz gerade bewertet.
- **Apfel-Verstecken**: Schalter „Apfel verstecken (off-ray)" blendet den Apfel aus, wenn er nicht auf einem der 8 Strahlen liegt — zeigt genau, was das Netz sehen kann.
