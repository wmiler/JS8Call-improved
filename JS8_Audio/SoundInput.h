/**
 * @file SoundInput.h
 * @brief Capture audio from an input device and forward to an `AudioDevice` sink.
 */

#ifndef SOUNDIN_H__
#define SOUNDIN_H__

#include "JS8_Audio/AudioDevice.h"

#include <QAudioDevice>
#include <QAudioSource>
#include <QDateTime>
#include <QObject>
#include <QPointer>
#include <QScopedPointer>
#include <QString>


/**
 * @class SoundInput
 * @brief Gets audio data from a `QAudioSource` and passes frames to a sink.
 *
 * The sink must outlive the active capture session. Start/stop/suspend
 * operations control the underlying `QAudioSource` stream.
 */
class SoundInput : public QObject {
    Q_OBJECT;

  public:
    /**
     * @brief Construct a SoundInput helper.
     * @param parent Optional parent QObject.
     */
    SoundInput(QObject *parent = nullptr) : QObject{parent}, m_sink{nullptr} {}

    /**
     * @brief Destroy the SoundInput and release resources.
     */
    ~SoundInput();

    /**
     * @brief Start capturing audio and forward it to @p sink.
     * @param device Input audio device to use.
     * @param framesPerBuffer Number of frames to request per buffer.
     * @param sink Destination `AudioDevice` to receive frames. Must outlive capture.
     * @param channel Channel selection for the sink (default: Mono).
     */
    Q_SLOT void start(QAudioDevice const &device, int framesPerBuffer,
                      AudioDevice *sink,
                      AudioDevice::Channel channel = AudioDevice::Mono);

    /**
     * @brief Temporarily suspend audio capture without dropping the stream.
     */
    Q_SLOT void suspend();

    /**
     * @brief Resume a previously suspended capture session.
     */
    Q_SLOT void resume();

    /**
     * @brief Stop audio capture and release the configured sink.
     */
    Q_SLOT void stop();

    /**
     * @brief Emitted when a non-fatal error occurs during capture.
     * @param message Human-readable error description.
     */
    Q_SIGNAL void error(QString message) const;

    /**
     * @brief Emitted to report status messages from the capture helper.
     * @param message Informational text.
     */
    Q_SIGNAL void status(QString message) const;

  private:
    /**
     * @brief Internal slot handling QAudioSource state transitions.
     * @param The new QAudio::State value.
     */
    Q_SLOT void handleStateChanged(QAudio::State) const;

    /**
     * @brief Return true if the internal audio stream has an error.
     */
    bool audioError() const;

    QScopedPointer<QAudioSource> m_stream; ///< Owned audio source for capture.
    QPointer<AudioDevice> m_sink;         ///< Non-owning pointer to the sink receiving frames.
};

#endif
