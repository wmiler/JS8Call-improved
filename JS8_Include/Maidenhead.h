/**
 * @file Maidenhead.h
 * @brief Utilities for validating and normalizing Maidenhead grid locators.
 *
 * This header defines functions and classes for validating Maidenhead grid
 * locators, which are used in amateur radio to specify geographic locations.
 * It includes constexpr functions for normalization and validation, as well
 * as a QValidator subclass for use in Qt applications.
 */
#ifndef MAIDENHEAD_HPP__
#define MAIDENHEAD_HPP__

#include <QString>
#include <QStringView>
#include <QValidator>

/**
 * @namespace Maidenhead
 * @brief Namespace containing utilities for managing Maidenhead grid locators.
 *
 * Provides underlying character validation, bounds checking, string validation,
 * and custom Qt input validators.
 */
namespace Maidenhead {

/**
 * @brief Converts a lowercase character code point to uppercase if applicable.
 * 
 * Provides a `constexpr` alternative to `QChar::toUpper()`, allowing for 
 * compile-time evaluation. Non-alphabetic and uppercase characters are returned unchanged.
 *
 * @param[in] u The 16-bit Unicode character value to process.
 * @return The uppercase equivalent if @p u is in the range `[a-z]`; otherwise @p u.
 */
constexpr char16_t normalize(char16_t const u) noexcept {
    return (u >= u'a' && u <= u'z') ? u - (u'a' - u'A') : u;
}

static_assert(normalize(u'0') == u'0');
static_assert(normalize(u'A') == u'A');
static_assert(normalize(u'Z') == u'Z');
static_assert(normalize(u'a') == u'A');
static_assert(normalize(u'z') == u'Z');

/**
 * @brief Locates the first invalid character index in a locator string view.
 *
 * Scans the provided view character-by-character based on standard and 
 * extended Maidenhead format pairs. Validation is case-insensitive.
 *
 * @note 
 * - A view that is incomplete, but correct up to its length, is considered valid.
 * - Odd-length or zero-length views return their respective sizes, signifying they are valid so far.
 *
 * **Maidenhead Structure Definitions:**
 * - **Pair 1 (Field):**      `[0, 1]`  -> Range `[A-R]`
 * - **Pair 2 (Square):**     `[2, 3]`  -> Range `[0-9]`
 * - **Pair 3 (Subsquare):**  `[4, 5]`  -> Range `[A-X]`
 * - **Pair 4 (Extended):**   `[6, 7]`  -> Range `[0-9]`
 * - **Pair 5 (Ultra Ext):**  `[8, 9]`  -> Range `[A-X]`
 * - **Pair 6 (Hyper Ext):**  `[10, 11]` -> Range `[0-9]`
 *
 * @param[in] view The string view tracking the user input grid locator string.
 * @return The index of the first character breaking format rules, or `view.size()` if valid so far.
 */
constexpr auto invalidIndex(QStringView const view) noexcept {
    auto const size = view.size();

    for (qsizetype i = 0; i < size; ++i) {
        auto const u = normalize(view[i].unicode());

        switch (i) {
        case 0:
        case 1:
            if (u >= u'A' && u <= u'R')
                continue;
            break;
        case 2:
        case 3:
        case 6:
        case 7:
        case 10:
        case 11:
            if (u >= u'0' && u <= u'9')
                continue;
            break;
        case 4:
        case 5:
        case 8:
        case 9:
            if (u >= u'A' && u <= u'X')
                continue;
            break;
        }
        return i;
    }

    return size;
}

static_assert(invalidIndex(u"") == 0);
static_assert(invalidIndex(u"S") == 0);
static_assert(invalidIndex(u"AZ") == 1);
static_assert(invalidIndex(u"AAA") == 2);
static_assert(invalidIndex(u"AA00AA00AA00A") == 12);

/**
 * @brief Assesses whether a complete string view matches acceptable grid lengths and constraints.
 *
 * Verifies that the input length is even, fits within the designated bounds, and contains 
 * no character validation failures.
 *
 * @tparam Min The minimum allowed pairs (Defaults to 2, e.g., Field + Square).
 * @tparam Max The maximum allowed pairs (Defaults to 6, up to Hyper-Extended).
 * @param[in] view The complete string view to be evaluated.
 * @return `true` if completely structured and within bounds, `false` otherwise.
 */
template <qsizetype Min = 2, qsizetype Max = 6>
constexpr auto valid(QStringView const view) noexcept {
    static_assert(Min >= 1 && Max >= 1 && Max <= 6 && Min <= Max);

    if (auto const size = view.size();
        !(size & 1) && (size >= 2 * Min) && (size <= 2 * Max)) {
        return invalidIndex(view) == size;
    }

    return false;
}

static_assert(valid(u"AA00"));
static_assert(valid(u"AA00AA"));
static_assert(valid(u"AA00AA00"));
static_assert(valid(u"BP51AD95RF"));
static_assert(valid(u"BP51AD95RF00"));
static_assert(valid(u"aa00"));
static_assert(valid(u"AA00aa"));
static_assert(valid(u"RR00XX"));

static_assert(!valid(u""));
static_assert(!valid(u"A"));
static_assert(!valid(u"0"));
static_assert(!valid(u"AA00 "));
static_assert(!valid(u"AA00 "));
static_assert(!valid(u"AA00 "));
static_assert(!valid(u" AA00"));
static_assert(!valid(u" AA00"));
static_assert(!valid(u"00"));
static_assert(!valid(u"aa00a"));
static_assert(!valid(u"AA00ZZA"));
static_assert(!valid(u"!@#$%^"));
static_assert(!valid(u"123456"));
static_assert(!valid(u"AA00ZZ"));
static_assert(!valid(u"ss00XX"));
static_assert(!valid(u"rr00yy"));
static_assert(!valid(u"AAA1aa"));
static_assert(!valid(u"BP51AD95RF00A"));
static_assert(!valid(u"BP51AD95RF0000"));

/**
 * @class Validator
 * @brief Custom Qt validator for stateful UI evaluation of Maidenhead strings.
 *
 * Re-evaluates target inputs as text is actively modified by the user. Enforces boundaries
 * during keystrokes and responds with appropriate `QValidator::State` steps.
 *
 * @tparam Min Minimum number of acceptable character pairs.
 * @tparam Max Maximum number of acceptable character pairs.
 */
template <qsizetype Min, qsizetype Max>
class Validator final : public QValidator {
    static_assert(Min >= 1 && Max >= 1 && Max <= 6 && Min <= Max);

    using QValidator::QValidator;

    /**
     * @brief Validates input text continuously during text input mutations.
     * 
     * Converts inputs to upper case and checks cursor placement constraints to determine 
     * if strings are valid, intermediate (needing more input), or definitively invalid.
     *
     * @param[in,out] input The active working text from the UI widget.
     * @param[in,out] pos The tracking position index of the text cursor.
     * @return QValidator::State `Acceptable` if complete, `Intermediate` if partial, `Invalid` if broken.
     */
    State validate(QString &input, int &pos) const override {
        // Ensure the input is upper case and get the size.

        input = input.toUpper();
        auto const size = input.size();

        // If nothing's been entered, we need more from them; if over
        // the maximum, less.

        if (size == 0)
            return Intermediate;
        if (size > Max * 2)
            return Invalid;

        // If anything up to the cursor is invalid, then we're invalid.
        // Anything after the cursor, we're willing to be hopeful about.

        if (auto const index = invalidIndex(input); index != size) {
            return index < pos ? Invalid : Intermediate;
        }

        // Entire input was valid. If the count is odd, or we haven't yet
        // hit the minimum, we need more from them, otherwise, we're good.

        return ((size & 1) || (size < Min * 2)) ? Intermediate : Acceptable;
    }
};

/**
 * @typedef StandardValidator
 * @brief Convenience validator requiring Field and Square, allowing Subsquare.
 * 
 * Perfect for basic contact (QSO) logging setups. Expects between 2 and 3 character pairs.
 */
using StandardValidator = Validator<2, 3>;

/**
 * @typedef ExtendedValidator
 * @brief Convenience validator permitting high fidelity sub-allocations up to Hyper Extended format.
 * 
 * Standard fitment for exact base station grid alignments. Expects between 2 and 6 character pairs.
 */
using ExtendedValidator = Validator<2, 6>;

} // namespace Maidenhead

#endif
