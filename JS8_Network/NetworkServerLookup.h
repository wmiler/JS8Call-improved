/**
 * @file NetworkServerLookup.h
 * @brief Blocking DNS / address lookup utility used by network setup code.
 *
 * The `network_server_lookup()` function parses a user-style `query` (which
 * may include an explicit port and/or IPv4/IPv6 address) and returns a
 * tuple of the resolved `QHostAddress` and the service port to use. If no
 * matching address is found the returned `QHostAddress` is `Null`.
 */

#ifndef NETWORK_SERVER_LOOKUP_HPP__
#define NETWORK_SERVER_LOOKUP_HPP__

#include <QAbstractSocket>
#include <QHostAddress>

#include <tuple>

class QString;

/**
 * @brief Parse `query` and perform a blocking lookup for host and port.
 *
 * `query` supports several shorthand forms:
 * - "" (empty) — use the supplied defaults.
 * - ":nnnnn" — override the default service port.
 * - "<hostname>" — resolve hostname via DNS.
 * - "nnn.nnn.nnn.nnn" — use explicit IPv4 address.
 * - "[<ipv6>]" — use explicit IPv6 address.
 * - Each address form may append `:port` to override the port.
 *
 * @param query User-specified host[:port] or special shorthand.
 * @param default_service_port Port number used when none is specified.
 * @param default_host_address Host address used when `query` is empty.
 * @param protocol Network protocol preference (IPv4/IPv6/Any).
 * @return Tuple of (`QHostAddress`, `quint16` port). If resolution fails the
 *         returned `QHostAddress` will be `QHostAddress::Null`.
 */
std::tuple<QHostAddress, quint16> network_server_lookup(
    QString query, quint16 default_service_port,
    QHostAddress default_host_address = QHostAddress::LocalHost,
    QAbstractSocket::NetworkLayerProtocol protocol =
        QAbstractSocket::AnyIPProtocol);

#endif

