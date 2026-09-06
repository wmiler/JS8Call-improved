// -*- Mode: C++ -*-
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
    SoundInput(QObject *parent = nullptr) : QObject{parent}, m_sink{nullptr} {}

    ~SoundInput();

    /**
     * @brief Start capturing audio and forward it to `sink`.
     * @param device Input audio device to use.
     * @param framesPerBuffer Number of frames to request per buffer.
     * @param sink Destination `AudioDevice` to receive frames.
     * @param channel Channel selection for the sink.
     */
    Q_SLOT void start(QAudioDevice const &, int framesPerBuffer,
                      AudioDevice *sink,
                      AudioDevice::Channel = AudioDevice::Mono);

    /**
     * @brief Temporarily suspend audio capture.
     */
    Q_SLOT void suspend();

    /**
     * @brief Resume a suspended capture session.
     */
    Q_SLOT void resume();

    /**
     * @brief Stop audio capture and release the sink.
     */
    Q_SLOT void stop();

    Q_SIGNAL void error(QString message) const;
    Q_SIGNAL void status(QString message) const;

  private:
    // used internally
    Q_SLOT void handleStateChanged(QAudio::State) const;

    bool audioError() const;

    QScopedPointer<QAudioSource> m_stream;
    QPointer<AudioDevice> m_sink;
};

#endif
