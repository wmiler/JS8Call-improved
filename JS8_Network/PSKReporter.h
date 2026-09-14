/**
 * @file PSKReporter.h
 * @brief Helper to aggregate and send PSK Reporter spots.
 *
 * `PSKReporter` collects local and remote station information and periodically
 * posts spot reports to the PSK Reporter network. The implementation uses a
 * pimpl to keep the header lightweight.
 */

#ifndef PSK_REPORTER_HPP_
#define PSK_REPORTER_HPP_

#include "JS8_Include/pimpl_h.h"
#include "JS8_Main/Radio.h"

#include <QObject>

class QString;
class Configuration;
class Bands;

/**
 * @brief Aggregates station spots and posts them to PSK Reporter.
 *
 * Constructed with a `Configuration` pointer and program info string used in
 * HTTP headers. The object emits `errorOccurred` when network/serialization
 * failures happen.
 */
class PSKReporter final : public QObject {
    Q_OBJECT

  public:
    /**
     * @brief Create a PSKReporter.
     * @param config Pointer to global configuration (not owned).
     * @param program_info Short program identifier embedded in reports.
     */
    explicit PSKReporter(Configuration const *, QString const &program_info);

    ~PSKReporter();

    /** Start the reporter's background processing. */
    void start();

    /** Attempt to re-establish any network connections used by the reporter. */
    void reconnect();

    /**
     * @brief Set the local station information used in outgoing reports.
     * @param call Callsign string.
     * @param grid Maidenhead grid locator.
     * @param antenna Antenna description string.
     */
    void setLocalStation(QString const &call, QString const &grid,
                         QString const &antenna);

    /**
     * @brief Add a remote station reception to the aggregated report.
     * @param call Remote callsign.
     * @param grid Remote grid locator.
     * @param freq Dial frequency where the signal was observed.
     * @param mode Mode string for the observation.
     * @param snr Measured SNR value.
     * @param utcTimestamp UTC timestamp for the reception.
     */
    void addRemoteStation(QString const &call, QString const &grid,
                          Radio::Frequency freq, QString const &mode, int snr,
                          QDateTime const &utcTimestamp);

    /**
     * @brief Flush any pending spots to PSK Reporter.
     * @param last When true indicates this is the final report (shutdown).
     */
    void sendReport(bool last = false);

    /** Emitted when an error occurs. */
    Q_SIGNAL void errorOccurred(QString const &reason);

  private:
    class impl;
    pimpl<impl> m_;
};

#endif
