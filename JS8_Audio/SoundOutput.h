// -*- Mode: C++ -*-
/**
 * @file SoundOutput.h
 * @brief Send audio data to a configured soundcard/output device.
 */

#ifndef SOUNDOUT_H__
#define SOUNDOUT_H__

#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSink>
#include <QObject>
#include <QString>

/**
 * @class SoundOutput
 * @brief Manages audio output format, buffering and the underlying sink.
 */
class SoundOutput : public QObject {
    Q_OBJECT;

  public:
    SoundOutput() = default;

    /**
     * @brief Current attenuation (volume multiplier).
     */
    qreal attenuation() const;

    /**
     * @brief Active audio format used by the output.
     */
    QAudioFormat format() const;

  public Q_SLOTS:
    /**
     * @brief Configure format based on device and channel count.
     * @param device Output device to use.
     * @param channels Number of channels (1 or 2).
     * @param msBuffered Milliseconds of buffering to set up.
     */
    void setFormat(QAudioDevice const &device, unsigned channels,
                   unsigned msBuffered = 0u);

    /**
     * @brief Set device and explicit format to use for playback.
     */
    void setDeviceFormat(QAudioDevice const &device, QAudioFormat const &format,
                         unsigned msBuffered = 0u);

    void restart(QIODevice *);
    void suspend();
    void resume();
    void reset();
    void stop();
    void setAttenuation(qreal); /* unsigned */
    void resetAttenuation();    /* to zero */

  Q_SIGNALS:
    void error(QString message) const;
    void status(QString message) const;

  private:
    bool checkStream() const;

  private Q_SLOTS:
    void handleStateChanged(QAudio::State) const;

  private:
    QAudioDevice m_device;
    QScopedPointer<QAudioSink> m_stream;
    QAudioFormat m_format;
    unsigned m_msBuffered = 0u;
    qreal m_volume = 1.0;
    bool m_error = false;
};

#endif
