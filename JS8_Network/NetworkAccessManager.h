/**
 * @file NetworkAccessManager.h
 * @brief Subclass of `QNetworkAccessManager` that manages SSL-error exceptions.
 *
 * This helper captures SSL errors presented by remote servers and caches
 * user-approved exceptions so subsequent requests to the same host do not
 * repeatedly prompt. The class exposes the same API surface as
 * `QNetworkAccessManager` but overrides request creation to inject the cache
 * of allowed SSL errors.
 */

#ifndef NETWORK_ACCESS_MANAGER_HPP__
#define NETWORK_ACCESS_MANAGER_HPP__

#include "JS8_Main/JS8MessageBox.h"

#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslError>
#include <QString>

class QNetworkRequest;
class QIODevice;
class QWidget;

// Subclass QNAM to keep a list of accepted SSL errors and allow
// them in future replies.
/**
 * @brief QNetworkAccessManager subclass that remembers allowed SSL errors.
 *
 * When the underlying network stack emits `sslErrors`, this manager will
 * prompt the user (via `JS8MessageBox`) to ignore specific errors and cache
 * the user's decision. The cached exceptions are then applied to subsequent
 * replies produced by `createRequest()` so the user experience is smoother
 * when connecting to servers with known-but-acceptable certificate issues.
 */
class NetworkAccessManager : public QNetworkAccessManager {
  public:
        /**
         * @brief Construct a NetworkAccessManager with a parent widget used for prompts.
         * @param parent Parent widget shown when prompting the user about SSL errors.
         */
        NetworkAccessManager(QWidget *parent) : QNetworkAccessManager(parent) {
        // handle SSL errors that have not been cached as allowed
        // exceptions and offer them to the user to add to the ignored
        // exception cache
        connect(
            this, &QNetworkAccessManager::sslErrors,
            [this, &parent](QNetworkReply *reply,
                            QList<QSslError> const &errors) {
                QString message;
                QList<QSslError> new_errors;
                for (auto const &error : errors) {
                    if (!allowed_ssl_errors_.contains(error)) {
                        new_errors << error;
                        message += '\n' +
                                   reply->request().url().toDisplayString() +
                                   ": " + error.errorString();
                    }
                }
                if (new_errors.size()) {
                    QString certs;
                    for (auto const &cert :
                         reply->sslConfiguration().peerCertificateChain()) {
                        certs += cert.toText() + '\n';
                    }
                    if (JS8MessageBox::Ignore ==
                        JS8MessageBox::query_message(
                            parent, tr("Network SSL Errors"), message, certs,
                            JS8MessageBox::Abort | JS8MessageBox::Ignore)) {
                        // accumulate new SSL error exceptions that have been
                        // allowed
                        allowed_ssl_errors_.append(new_errors);
                        reply->ignoreSslErrors(allowed_ssl_errors_);
                    }
                } else {
                    // no new exceptions so silently ignore the ones already
                    // allowed
                    reply->ignoreSslErrors(allowed_ssl_errors_);
                }
            });
    }

  protected:
        /**
         * @brief Intercept request creation and apply cached allowed SSL errors.
         *
         * Overrides `QNetworkAccessManager::createRequest` to attach the
         * previously accepted SSL error exceptions to the produced `QNetworkReply`.
         */
        QNetworkReply *createRequest(Operation operation,
                                                                 QNetworkRequest const &request,
                                                                 QIODevice *outgoing_data = nullptr) override {
        auto reply = QNetworkAccessManager::createRequest(operation, request,
                                                          outgoing_data);
        // errors are usually certificate specific so passing all cached
        // exceptions here is ok
        reply->ignoreSslErrors(allowed_ssl_errors_);
        return reply;
    }

  private:
    QList<QSslError> allowed_ssl_errors_;
};

#endif
