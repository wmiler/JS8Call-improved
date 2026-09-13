/**
 * @file Decoder.h
 * @brief Worker and `Decoder` QObject wrapper for an external decoder process.
 *
 * `Worker` manages the external process used for decoding (life-cycle and
 * I/O), while `Decoder` exposes a Qt-friendly interface that runs the
 * worker in a separate thread and forwards ready/error/finished signals.
 * 
 * @author Jordan Sherer <kn4crd@gmail.com>
 * @copyright (c) 2019 Jordan Sherer
 */

#ifndef DECODER_H
#define DECODER_H

#include "JS8_Main/ProcessThread.h"

#include <QByteArray>
#include <QLoggingCategory>
#include <QPointer>
#include <QProcess>

/**
 * @class Worker
 * @brief Manage an external decoder process and forward its I/O.
 *
 * `Worker` encapsulates the lifetime and I/O of an external decoder
 * process. It exposes slots to start and stop the process and emits
 * signals when data is ready on stdout, when an error occurs, and when
 * the process finishes.
 */
class Worker : public QObject {
    Q_OBJECT
  public:
    /** Destroy the worker and ensure the subprocess is cleaned up. */
    ~Worker();

  public slots:
    /**
     * @brief Launch the external decoder process.
     * @param path Path to the decoder executable.
     * @param args Arguments passed to the process.
     */
    void start(QString path, QStringList args);

    /** @brief Request the running process to quit. */
    void quit();

    /**
     * @brief Return the underlying QProcess instance, if any.
     * @return Pointer to the managed QProcess or nullptr.
     */
    QProcess *process() const { return m_proc.data(); }

  private:
    /**
     * @brief Associate a QProcess with this worker and configure timeouts.
     * @param proc Process instance to manage.
     * @param msecs Milliseconds used for short waits during setup.
     */
    void setProcess(QProcess *proc, int msecs = 1000);

  signals:
    /** Emitted when the process has produced stdout data. */
    void ready(QByteArray t);

    /** Emitted when the process reports an error. */
    void error(int errorCode, QString errorString);

    /** Emitted when the process exits. */
    void finished(int exitCode, int statusCode, QString errorString);

  private:
    QScopedPointer<QProcess> m_proc; ///< Owned process instance
};

/**
 * @class Decoder
 * @brief Qt-friendly controller that runs the decoder `Worker` in a thread.
 *
 * `Decoder` creates and runs a `Worker` in a dedicated `QThread`, exposing
 * slots to start/stop the worker and signals forwarding worker events. The
 * class also provides convenience accessors for the currently configured
 * program and arguments.
 */
class Decoder : public QObject {
    Q_OBJECT
  public:
    /** Construct a Decoder controller. */
    Decoder(QObject *parent = nullptr);

    /** Destroy and tear down the worker/thread. */
    ~Decoder();

    /** Lock internal synchronization primitive used by the decoder. */
    void lock();

    /** Unlock the internal synchronization primitive. */
    void unlock();

    /**
     * @brief Return the program path of the managed decoder process.
     * @return Program path or empty string if not available.
     */
    QString program() const {
        if (!m_worker.isNull() && m_worker->process() != nullptr) {
            return m_worker->process()->program();
        }
        return {};
    }

    /**
     * @brief Return the argument list configured for the decoder process.
     * @return Argument list or empty if not available.
     */
    QStringList arguments() const {
        if (!m_worker.isNull() && m_worker->process() != nullptr) {
            return m_worker->process()->arguments();
        }
        return {};
    }

  private:
    /** Create and configure a new `Worker` instance. */
    Worker *createWorker();

  public slots:
    /** Start the decoder worker with given thread priority. */
    void start(QThread::Priority priority);

    /** Request worker shutdown. */
    void quit();

    /**
     * @brief Wait for the worker to finish shutdown.
     * @return true if the worker finished successfully.
     */
    bool wait();

    // Slots forwarded from the UI or internal code to control the process.
    void processStart(QString path, QStringList args);
    void processReady(QByteArray t);
    void processQuit();

    void processError(int errorCode, QString errorString);
    void processFinished(int exitCode, int statusCode, QString errorString);

  signals:
    /** Request the worker to start with given path and args. */
    void startWorker(QString path, QStringList args);
    /** Request the worker to quit. */
    void quitWorker();

    /** Forwarded worker signals: stdout data ready. */
    void ready(QByteArray t);
    /** Forwarded worker signals: error. */
    void error(int errorCode, QString errorString);
    /** Forwarded worker signals: finished. */
    void finished(int exitCode, int statusCode, QString errorString);

  private:
    QPointer<Worker> m_worker; ///< Non-owning pointer to the worker living in m_thread
    QThread m_thread;         ///< Thread running the worker
};

Q_DECLARE_LOGGING_CATEGORY(decoder_js8)

#endif // DECODER_H
