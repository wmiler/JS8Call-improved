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
    /**
     * @brief Construct a NotificationAudio helper.
     * @param parent Optional parent QObject.
     */
    explicit NotificationAudio(QObject *parent = nullptr);

    /**
     * @brief Destroy the NotificationAudio and release cached resources.
     */
    ~NotificationAudio();

  public slots:
    /**
     * @brief Relay a status message from internal components.
     * @param message Informational text.
     */
    void status(QString message);

    /**
     * @brief Relay an error message from internal components.
     * @param message Error description.
     */
    void error(QString message);

    /**
     * @brief Set output device and optional buffering for playback.
     * @param device Audio output device to use.
     * @param msBuffer Milliseconds of buffer to allocate for playback.
     */
    void setDevice(const QAudioDevice &device, unsigned msBuffer = 0);

    /**
     * @brief Play a sound file referenced by @p filePath.
     * @param filePath Path to an audio resource on disk.
     */
    void play(const QString &filePath);

    /**
     * @brief Stop any currently playing notification sound.
     */
    void stop();

  private:
    using Entry = QPair<QAudioFormat, QByteArray>;
    using Cache = QHash<QString, Entry>;

    /**
     * @brief Play a cached entry referenced by iterator.
     */
    void playEntry(Cache::const_iterator it);

    /**
     * @brief Convert 24-bit little-endian PCM to 32-bit little-endian.
     * @param in Input buffer containing 24-bit PCM.
     * @return Converted 32-bit PCM buffer.
     */
    static QByteArray pcm24le_to_int32le(const QByteArray &in);

    /**
     * @brief Upmix mono audio to stereo in-place when required by format.
     * @param fmt Audio format; may be modified to reflect channel layout.
     * @param data PCM data to upmix in-place.
     * @return true on success.
     */
    static bool upmixMonoToStereoInPlace(QAudioFormat &fmt, QByteArray &data);

    QScopedPointer<NotificationSoundOutput> m_stream; ///< Owned playback stream.
    Cache m_cache;                                    ///< In-memory cache of decoded sounds.
    QAudioDevice m_device;                            ///< Current output device.
    QBuffer m_buffer;                                 ///< Temporary buffer used for playback.
    unsigned m_msBuffer;                              ///< Buffering size in milliseconds.
};

#endif // NOTIFICATIONAUDIO_H
