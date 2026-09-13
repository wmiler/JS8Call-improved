/**
 * @file AudioDevice.h
 * @brief Audio Device declarations for JS8Call-improved.
 *
 * This header defines the exported interfaces, helper types, and shared
 * declarations used by the JS8Call-improved application. It groups related
 * radio, audio, network, transceiver, and UI functionality into one public
 * API surface so the rest of the project can reuse the same contracts.
 *
 */

#ifndef AUDIODEVICE_HPP__
#define AUDIODEVICE_HPP__

#include <QIODevice>

class QDataStream;

/**
 * @class AudioDevice
 * @brief Abstract base class for audio devices exposed as a `QIODevice`.
 *
 * Subclasses implement concrete audio sinks/sources. `AudioDevice` offers
 * channel selection helpers, frame size calculation and convenience routines
 * to store/load interleaved sample frames.
 */
class AudioDevice : public QIODevice {
  public:
    /**
     * @brief Audio channel selection used by the device.
     *
     * These values are mapped to UI combobox indexes elsewhere; do not
     * reorder or renumber them without updating the UI mappings.
     */
    enum Channel {
        Mono,
        Left,
        Right,
        Both
    }; // these are mapped to combobox index so don't change

    static char const *toString(Channel c) {
        switch (c) {
        case Mono:
            return "Mono";
        case Left:
            return "Left";
        case Right:
            return "Right";
        default:
            return "Both";
        }
    }

    /**
     * @brief Convert a channel label into an AudioDevice channel value.
     * @param str Channel name to parse.
     * @return Matching Channel enum value.
     */
    static Channel fromString(QString const &str) {
        QString const s(str.toCaseFolded().trimmed().toLatin1());

        if (s == "both")
            return Both;
        else if (s == "right")
            return Right;
        else if (s == "left")
            return Left;
        else
            return Mono;
    }

    /**
     * @brief Initialize the device for the specified open mode and channel.
     * @param mode QIODevice open mode (ReadOnly/WriteOnly/etc).
     * @param channel Channel selection to use.
     * @return true on successful initialization.
     */
    bool initialize(OpenMode mode, Channel channel);

    /**
     * @brief Whether this device exposes a sequential data stream.
     *
     * Returns true to indicate the device behaves as a sequential
     * QIODevice (no random access). Consumers should treat the device
     * as a stream of sample data and avoid relying on seeking.
     *
     * @return true when the device is sequential.
     */
    bool isSequential() const override { return true; }

    /**
     * @brief Number of bytes per audio frame for the current channel setting.
     *
     * For mono devices this equals the sample size; for stereo it is double.
     */
    size_t bytesPerFrame() const {
        return sizeof(qint16) * (Mono == m_channel ? 1 : 2);
    }

    /**
     * @brief Get the current channel selection used by this device.
     *
     * The channel determines how interleaved frames are interpreted when
     * storing and loading samples (Mono, Left, Right, or Both).
     *
     * @return The active `Channel` value used for store/load operations.
     */
    Channel channel() const { return m_channel; }

  protected:
    explicit AudioDevice(QObject *parent = nullptr) : QIODevice(parent) {}

    /**
     * @brief Copy interleaved source samples into the destination buffer.
     * @param source Source data buffer.
     * @param numFrames Number of frames to copy.
     * @param dest Destination sample buffer.
     */
    void store(char const *source, size_t numFrames, qint16 *dest) {
        qint16 const *begin(reinterpret_cast<qint16 const *>(source));
        for (qint16 const *i = begin;
             i != begin + numFrames * (bytesPerFrame() / sizeof(qint16));
             i += bytesPerFrame() / sizeof(qint16)) {
            switch (m_channel) {
            case Mono:
                *dest++ = *i;
                break;

            case Right:
                *dest++ = *(i + 1);
                break;

            case Both: // should be able to happen but if it
                       // does we'll take left
                Q_ASSERT(Both == m_channel);
                [[fallthrough]];
            case Left:
                *dest++ = *i;
                break;
            }
        }
    }

    /**
     * @brief Expand a sample value into the output buffer for the active channel.
     * @param sample Input sample value.
     * @param dest Output buffer to fill.
     * @return Updated buffer pointer.
     */
    qint16 *load(qint16 const sample, qint16 *dest) {
        switch (m_channel) {
        case Mono:
            *dest++ = sample;
            break;

        case Left:
            *dest++ = sample;
            *dest++ = 0;
            break;

        case Right:
            *dest++ = 0;
            *dest++ = sample;
            break;

        case Both:
            *dest++ = sample;
            *dest++ = sample;
            break;
        }
        return dest;
    }

  private:
    Channel m_channel;
};

Q_DECLARE_METATYPE(AudioDevice::Channel);

#endif
