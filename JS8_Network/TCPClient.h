/**
 * @file TCPClient.h
 * @brief Lightweight TCP client utilities used throughout the network code.
 *
 * `TCPClient` provides convenience methods to ensure a TCP connection and to
 * send a short message to a host:port pair. The implementation is hidden by
 * a pimpl to keep this header stable.
 */

#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include "JS8_Include/pimpl_h.h"

#include <QObject>
#include <QTcpSocket>

/**
 * @brief Simple TCP client wrapper.
 *
 * The API is intentionally small: `ensureConnected()` will try to establish
 * a connection within a timeout and `sendNetworkMessage()` sends a message
 * optionally terminated with CR/LF.
 */
class TCPClient : public QObject {
    Q_OBJECT
  public:
    using port_type = quint16;

    explicit TCPClient(QObject *parent = nullptr);

  signals:

  public slots:
    /**
     * @brief Ensure a TCP connection to `host:port` within `msecs` milliseconds.
     * @return `true` when connected, `false` on timeout or error.
     */
    Q_SLOT bool ensureConnected(QString host, port_type port, int msecs = 5000);

    /**
     * @brief Send a short network message to `host:port`.
     * @param host Destination hostname or IP.
     * @param port Destination port.
     * @param message Byte payload to send.
     * @param crlf When true append CRLF to the message.
     * @param msecs Connection/send timeout in milliseconds.
     * @return `true` on success, `false` on failure.
     */
    Q_SLOT bool sendNetworkMessage(QString host, port_type port,
                                   QByteArray const &message, bool crlf = true,
                                   int msecs = 5000);

  private:
    class impl;
    pimpl<impl> m_;
};

#endif // TCPCLIENT_H
