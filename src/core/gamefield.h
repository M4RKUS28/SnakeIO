#ifndef GAMEFIELD_H
#define GAMEFIELD_H

#include <QPoint>
#include <QVector>
#include <QRandomGenerator>
#include <QMutex>

// ===========================================================================
//  GameField
//
//  Maintains the deterministic apple-position queue for a single game.
//  Apples are generated lazily on demand from a seeded QRandomGenerator.
//  All methods that modify or read applePos are protected by a QMutex so
//  that Snake threads can call getApplePos() safely.
//
//  The field is logically a square grid with 1-based coordinates [1..size].
// ===========================================================================
class GameField
{
public:
    explicit GameField(int size);

    /// Returns (or generates) the apple at queue position @p num.
    QPoint getApplePos(int num);

    /// Replaces the current seed and fully resets the apple queue (calls reset()).
    void   setSeed(size_t seed);
    size_t getSeed() const { return seed; }

    /// Clears the apple queue and re-seeds the generator.
    /// If @p seed == 0, a new timestamp-based seed is chosen.
    void reset(size_t seed = 0);

    /// Removes the last apple in the queue (shrinks the pre-seeded list).
    void popBack();

    /// Adds 8 apples in a corner / centre pattern (used for debug layouts).
    void addCornerApples();

    /// Removes the apple at @p index from the queue.
    void removeAppleAt(int index);

    /// Read-only access to the full apple list.
    /// @warning Thread-unsafe — call only from the UI thread.
    const QVector<QPoint>& getApples() const { return applePos; }

    /// Side-length of the square field in cells.
    int getSize() const { return size; }

private:
    QVector<QPoint>  applePos;
    QMutex           mutex;
    size_t           seed = 0;          ///< Must be declared before randomGenerator (init order).
    QRandomGenerator randomGenerator;   ///< Seeded from @p seed — depends on seed being initialized first.

    const int size;  ///< Fixed at construction — never changes.
};

#endif // GAMEFIELD_H
