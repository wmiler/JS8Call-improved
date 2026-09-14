/**
 * @file SpotClient.h
 * @brief Client for sending spots and commands to upstream spot servers.
 *
 * The `SpotClient` encapsulates a network connection to a spotting server and
 * provides methods to enqueue spots or command-style messages. It stores
 * local station information (callsign/grid) to include in outgoing payloads.
 */

#ifndef JS8SPOTCLIENT_H
#define JS8SPOTCLIENT_H

#include "JS8_Include/pimpl_h.h"
#include "JS8_Main/Radio.h"

#include <QObject>
#include <QString>

/**
 * @brief Sends spots and spot-related commands to a remote host.
 */
class SpotClient final : public QObject {
    Q_OBJECT

  public:
    /**
     * @brief Construct a SpotClient.
     * @param host Remote host name or IP.
     * @param port Remote TCP port.
     * @param version Protocol/version string included in handshakes.
     * @param parent Optional QObject parent.
     */
    SpotClient(QString const &host, quint16 port, QString const &version,
               QObject *parent = nullptr);

    /** Start the client and establish the network connection. */
    void start();

    /**
     * @brief Set local station identity included in outgoing messages.
     * @param callsign Local callsign.
     * @param grid Local Maidenhead grid.
     * @param info Optional free-form station info string.
     */
    void setLocalStation(QString const &callsign, QString const &grid,
                         QString const &info);

    /**
     * @brief Enqueue a command-style message for transmission.
     * @param cmd Command verb.
     * @param from Originator callsign.
     * @param to Destination callsign (or group).
     * @param relayPath Relay path string.
     * @param text Message payload.
     * @param grid Maidenhead grid for the sender.
     * @param extra Extra data string.
     * @param submode Numeric submode identifier.
     * @param dial Dial frequency.
     * @param offset Offset from dial in Hz.
     * @param snr Measured SNR value.
     */
    void enqueueCmd(QString const &cmd, QString const &from,
                    QString const &to, QString const &relayPath,
                    QString const &text, QString const &grid,
                    QString const &extra, int const submode,
                    Radio::Frequency const dial, int const offset,
                    int const snr);

    /**
     * @brief Enqueue a simple spot (callsign/grid) for transmission.
     */
    void enqueueSpot(QString const &callsign, QString const &grid,
                     int const submode, Radio::Frequency const dial,
                     int const offset, int const snr);

    /** Emitted when an error occurs while sending or connecting. */
    Q_SIGNAL void error(QString const &) const;

  private:
    class impl;
    pimpl<impl> m_;
};

#endif // JS8SPOTCLIENT_H
