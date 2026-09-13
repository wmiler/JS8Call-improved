/**
 * @file JS8Submode.h
 * @brief Submode parameter accessors for JS8 modes.
 *
 * Provides functions returning constant data specific to each JS8 submode
 * (names, bandwidth, samples per symbol, durations, thresholds, etc.).
 */

#ifndef JS8_SUBMODE_HPP_
#define JS8_SUBMODE_HPP_

#include "JS8_Mode/JS8.h"

#include <QString>

#include <stdexcept>

namespace JS8::Submode {

/**
 * @brief Exception type thrown for invalid or unsupported submode inputs.
 * @details Construct with a user-readable QString describing the error.
 */
struct error : public std::runtime_error {
    explicit error(QString const &what)
        : std::runtime_error(what.toStdString()) {}
};

// Helper note:
// Be careful when doing arithmetic with unsigned values; expressions like
// `3 - samplesForOneSymbol` may underflow. To inspect concrete submode
// values at runtime, enable logging with `QT_LOGGING_RULES=js8submode.js8=true`.

/**
 * @brief Return the submode name (uppercase).
 * @param submode Numeric submode identifier.
 * @return Uppercase submode name string.
 * @throws JS8::Submode::error if `submode` is invalid.
 */
QString name(int submode);

/**
 * @brief Return the nominal audio bandwidth for the given submode.
 * @param submode Numeric submode identifier.
 * @return Bandwidth in Hz.
 * @throws JS8::Submode::error if `submode` is invalid.
 */
unsigned int bandwidth(int submode);

/**
 * @brief Return the Costas array type used by the submode.
 * @param submode Numeric submode identifier.
 * @return `Costas::Type` describing ORIGINAL or MODIFIED arrays.
 */
Costas::Type costas(int submode);

/**
 * @brief Transmission period (seconds) from start-to-start for the submode.
 * @param submode Numeric submode identifier.
 * @return Period length in seconds (e.g., 30 for SLOW, 6 for JS8 40).
 */
unsigned int period(int submode);

/**
 * @brief Number of audio samples (at 12 kHz) per symbol.
 * @param submode Numeric submode identifier.
 * @return Samples per symbol.
 */
unsigned int samplesForOneSymbol(int submode);

/**
 * @brief Number of audio samples used to transmit all symbols in one period.
 * @param submode Numeric submode identifier.
 * @return Samples for symbols in one period.
 * @note Typically `JS8_NUM_SYMBOLS * samplesForOneSymbol(submode)`.
 */
unsigned int samplesForSymbols(int submode);

/**
 * @brief Samples required to capture the full TX duration including start delay and padding.
 * @param submode Numeric submode identifier.
 * @return Required sample count.
 */
unsigned int samplesNeeded(int submode);

/**
 * @brief Samples per period at 12 kHz (includes guard/padding).
 * @param submode Numeric submode identifier.
 * @return Total samples per period.
 */
unsigned int samplesPerPeriod(int submode);

/**
 * @brief Receiver SNR threshold used for candidate selection.
 * @param submode Numeric submode identifier.
 * @return SNR threshold (dB integer).
 */
int rxSNRThreshold(int submode);

/**
 * @brief Generic receiver threshold (implementation-specific).
 * @param submode Numeric submode identifier.
 * @return Threshold value.
 */
int rxThreshold(int submode);

/**
 * @brief Milliseconds to wait after TX start before sending data.
 * @param submode Numeric submode identifier.
 * @return Start delay in milliseconds.
 */
unsigned int startDelayMS(int submode);

/**
 * @brief Nominal tone spacing (Hz) for the submode.
 * @param submode Numeric submode identifier.
 * @return Tone spacing in Hz.
 */
double toneSpacing(int submode);

/**
 * @brief Duration (seconds) consumed by transmitting the symbol block.
 * @param submode Numeric submode identifier.
 * @return Data duration in seconds (samplesForSymbols / 12000).
 */
double dataDuration(int submode);

/**
 * @brief Full transmission duration in seconds including start delay.
 * @param submode Numeric submode identifier.
 * @return Transmission time in seconds.
 */
double txDuration(int submode);

/**
 * @brief Compute the decode cycle index for a given candidate.
 * @param submode Numeric submode identifier.
 * @param param Additional mode-specific parameter (e.g., cycle offset).
 * @return Cycle index used by the decoder.
 */
int computeCycleForDecode(int submode, int param);

/**
 * @brief Compute an alternate decode cycle index for specialized modes.
 * @param submode Numeric submode identifier.
 * @param a First mode-specific parameter.
 * @param b Second mode-specific parameter.
 * @return Alternate cycle index.
 */
int computeAltCycleForDecode(int submode, int a, int b);

/**
 * @brief Compute a ratio value used by submode-specific calculations.
 * @param submode Numeric submode identifier.
 * @param value Input value used in ratio computation.
 * @return Computed ratio.
 */
double computeRatio(int submode, double value);

} // namespace JS8::Submode

#endif // JS8_SUBMODE_HPP_
