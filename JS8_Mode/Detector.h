/**
 * @file Detector.h
 * @brief Audio sink that collects, down-samples and provides buffered samples.
 *
 * `Detector` is an `AudioDevice` sink that accumulates incoming PCM samples
 * into an internal, de-interleaved buffer sized for one JS8 period. The
 * class exposes thread-safe access to the buffer via a mutex and emits
 * notifications when frames are written so downstream consumers (the
 * decoder) can process fixed-size chunks.
 */

#ifndef DETECTOR_HPP__
#define DETECTOR_HPP__

#include "JS8_Audio/AudioDevice.h"

#include <QMutex>
#include <vendor/Eigen/Dense>

#include <array>

// Output device that distributes data in predefined chunks via a signal;
// underlying device for this abstraction is just the buffer that stores
// samples throughout a receiving period.

class Detector : public AudioDevice {
    Q_OBJECT;

    /**
     * @brief Lowpass FIR filter used to down-sample 48kHz input to 12kHz.
     *
     * The nested `Filter` class provides an efficient small FIR down-sampler
     * using Eigen vectors. It is configured with a fixed number of taps
     * (`NTAPS`) and performs a decimation by `NDOWN` producing one output
     * sample per `NDOWN` inputs.
     */
    class Filter final {
      public:
        /** Down-sample factor (48k -> 12k = 4). */
        static constexpr std::size_t NDOWN = 48 / 12;
        /** Number of FIR taps. */
        static constexpr std::size_t NTAPS = 49;
        /** Amount to shift the internal tap buffer on each new input. */
        static constexpr std::size_t SHIFT = NTAPS - NDOWN;

        using Vector = Eigen::Vector<float, NTAPS>;
        using Sample = Eigen::Map<Eigen::Vector<short, NDOWN> const>;

        /**
         * @brief Construct a filter with the provided low-pass coefficients.
         * @param lowpass Array of FIR coefficients sized `NTAPS`.
         */
        explicit Filter(std::array<Vector::value_type, NTAPS> const &lowpass)
            : m_w(lowpass.data()), m_t(Vector::Zero()) {}

        /**
         * @brief Consume NDOWN input samples and return a single down-sampled value.
         * @param data Pointer to `NDOWN` input samples.
         * @return Down-sampled output sample.
         */
        auto downSample(Sample::value_type const *const data) {
            m_t.head(SHIFT) = m_t.segment(NDOWN, SHIFT);
            m_t.tail(NDOWN) = Sample(data).cast<Vector::value_type>();

            return static_cast<Sample::value_type>(std::round(m_w.dot(m_t)));
        }

      private:
        Eigen::Map<Vector const> m_w; ///< FIR coefficient view
        Vector m_t;                   ///< Tap buffer
    };

    /** Maximum buffer size (samples per input signals-worth). */
    static constexpr std::size_t MaxBufferSize = 7 * 512;

    /** De-interleaved sample buffer covering one period at input rate. */
    using Buffer = std::array<short, MaxBufferSize * Filter::NDOWN>;

  public:
    /**
     * @brief Construct a Detector.
     * @param frameRate Input sampling rate (Hz).
     * @param periodLengthInSeconds Period length (seconds) used to size buffers.
     * @param parent Optional parent QObject.
     */
    Detector(unsigned frameRate, unsigned periodLengthInSeconds,
             QObject *parent = nullptr);

    /** Return configured period length in seconds. */
    unsigned period() const { return m_period; }

    /** Return pointer to the internal mutex protecting buffer operations. */
    QMutex *getMutex() { return &m_lock; }

    /** Set the transmit/receive period used for timing adjustments. */
    void setTRPeriod(unsigned p) { m_period = p; }

    /**
     * @brief Return how many seconds into the current period we are.
     */
    unsigned secondInPeriod() const;

    /** Clear buffered samples and reset buffer position. */
    void clear();

    /** Reset device state and buffers. */
    bool reset() override;

    /** Reset buffer contents to their initial state. */
    void resetBufferContent();

    /** Reset buffer write position to the beginning. */
    void resetBufferPosition();

    /** Emitted when `writeData` has appended frames into the internal buffer. */
    Q_SIGNAL void framesWritten(qint64) const;

    /** Set the internal block size used when aggregating frames. */
    Q_SLOT void setBlockSize(unsigned);

  protected:
    /** We are a sink; `readData` is unused. */
    qint64 readData(char *, qint64) override { return -1; }

    /** Write input PCM bytes into the internal buffer. */
    qint64 writeData(char const *, qint64) override;

  private:
    unsigned m_frameRate;               ///< Input frame rate in Hz
    unsigned m_period;                  ///< Period length in seconds
    QMutex m_lock;                      ///< Protects buffer and position
    Filter m_filter;                    ///< Down-sampling filter instance
    Buffer m_buffer;                    ///< De-interleaved sample buffer
    Buffer::size_type m_bufferPos = 0;  ///< Current write position in buffer
    std::size_t m_samplesPerFFT = MaxBufferSize; ///< FFT window size in samples
    qint32 m_ns = 999;                  ///< Small-number sentinel / debug
};

#endif
