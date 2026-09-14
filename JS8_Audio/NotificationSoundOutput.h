/**
 * @file NotificationSoundOutput.h
 * @brief Self-contained audio output for notification sounds.
 */

#pragma once

#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSink>
#include <QBuffer>
#include <QObject>
#include <qmath.h>

#include <memory>

/**
 * @brief Audio output helper for playing short notification sounds.
 *
 * This QObject-derived helper owns a QAudioSink and an in-memory buffer
 * to play short notification audio clips with optional attenuation and
 * a configurable buffer size in milliseconds.
 */
class NotificationSoundOutput : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct a NotificationSoundOutput instance.
     * @param parent Optional parent QObject.
     */
    explicit NotificationSoundOutput(QObject *parent = nullptr);

    /**
     * @brief Destroy the NotificationSoundOutput and release audio resources.
     */
    ~NotificationSoundOutput() override;

    /**
     * @brief Select the audio output device and set the internal buffer size.
     * @param device The chosen output device to use for playback.
     * @param msBuffer Buffer size in milliseconds used for the QBuffer sink.
     *
     * The device is stored and used for subsequent calls to play(). The
     * @p msBuffer controls the internal buffering size to trade off latency
     * versus smoothness for short notification sounds.
     */
    void setDevice(QAudioDevice const &device, unsigned msBuffer);

    /**
     * @brief Set attenuation factor applied to played audio.
     * @param a Attenuation multiplier (1.0 = no attenuation).
     */
    void setAttenuation(qreal a);

    /**
     * @brief Play the provided audio data using the given format.
     * @param data Raw audio bytes to play.
     * @param format Audio format describing the samples in @p data.
     *
     * The method will open an internal QBuffer and QAudioSink as needed and
     * start playback. If another sound is currently playing it will be
     * stopped and the new data will be played instead.
     */
    void play(QByteArray const &data, QAudioFormat const &format);

    /**
     * @brief Stop playback immediately and release the playback buffer.
     */
    void stop();

signals:
    /**
     * @brief Emitted to provide informational status messages.
     * @param message Human-readable status text.
     */
    void status(QString message);

    /**
     * @brief Emitted when an error occurs during setup or playback.
     * @param message Error description suitable for logging or display.
     */
    void error(QString message);

private slots:
    /**
     * @brief Internal handler for QAudioSink state changes.
     * @param newState The new state of the QAudio::State enum.
     */
    void handleStateChanged(QAudio::State newState);

private:
    /**
     * @brief Release and reset internal audio resources.
     *
     * Called when stopping playback or destroying the object to ensure the
     * QAudioSink and QBuffer are properly released.
     */
    void release();

    QAudioDevice                  m_device;                 ///< Currently selected audio device.
    std::unique_ptr<QAudioSink>   m_sink;                   ///< Owned QAudioSink for playback.
    std::unique_ptr<QBuffer>      m_buffer;                 ///< In-memory buffer holding audio bytes.
    QAudioFormat                  m_currentFormat;          ///< Format of the currently loaded audio.
    qreal                         m_volume  = 1.0;         ///< Playback volume/attenuation multiplier.
    unsigned                      m_msBuffer = 0;          ///< Buffer size in milliseconds.
};
