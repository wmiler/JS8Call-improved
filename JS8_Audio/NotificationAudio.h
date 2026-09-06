/**
 * @file NotificationAudio.h
 * @brief High-level helper to play notification sounds with caching.
 */

#ifndef NOTIFICATIONAUDIO_H
#define NOTIFICATIONAUDIO_H

#include "NotificationSoundOutput.h"

#include <QAudioDevice>
#include <QBuffer>
#include <QByteArray>
#include <QHash>
#include <QPair>
#include <QScopedPointer>

class SoundOutput;

/**
 * @class NotificationAudio
 * @brief Plays short notification sounds using an internal output stream.
 *
 * The class maintains an in-memory cache of decoded PCM audio so that
 * frequently used notification sounds play with minimal latency.
 */
class NotificationAudio : public QObject {
    Q_OBJECT

  public:
    explicit NotificationAudio(QObject *parent = nullptr);
    ~NotificationAudio();

  public slots:
    /**
     * @brief Emit a status message.
     */
    void status(QString message);

    /**
     * @brief Emit an error message.
     */
    void error(QString message);

    /**
     * @brief Set output device and optional buffering.
     * @param device Audio output device to use.
     * @param msBuffer Milliseconds of buffer to allocate for playback.
     */
    void setDevice(const QAudioDevice &device, unsigned msBuffer = 0);

    /**
     * @brief Play a sound file referenced by path.
     */
    void play(const QString &filePath);

    /**
     * @brief Stop any currently playing notification sound.
     */
    void stop();

  private:
    using Entry = QPair<QAudioFormat, QByteArray>;
    using Cache = QHash<QString, Entry>;

    void playEntry(Cache::const_iterator);
    static QByteArray pcm24le_to_int32le(const QByteArray &in);
    static bool upmixMonoToStereoInPlace(QAudioFormat &fmt, QByteArray &data);

    QScopedPointer<NotificationSoundOutput> m_stream;
    Cache m_cache;
    QAudioDevice m_device;
    QBuffer m_buffer;
    unsigned m_msBuffer;
};

#endif // NOTIFICATIONAUDIO_H
