/**
 * @file sleep.h
 * @brief Sleep utility class for thread management.
 *
 * This header defines the Sleep class, which provides static methods for
 * sleeping the current thread and retrieving the ideal thread count.
 * It is a simple wrapper around QThread's static methods.
 */
#ifndef SLEEP_H
#define SLEEP_H
#include <qthread.h>

/**
 * @class Sleep
 * @brief Utility class for thread sleeping and ideal thread count retrieval.
 *
 * The Sleep class provides static methods to pause the current thread for a
 * specified duration in milliseconds and to retrieve the ideal number of
 * threads for the system. It is a lightweight wrapper around QThread's
 * static methods.
 */
class Sleep : public QThread {
  public:
    static void msleep(int ms) { QThread::msleep(ms); }
    static int idealThreadCount() { return QThread::idealThreadCount(); }
};

#endif // SLEEP_H
