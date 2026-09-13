/**
 * @file soft_combiner.h
 * @brief Soft combiner for accumulating repeated LLR frames.
 */

#pragma once

#include <QDebug>
#include <QLoggingCategory>
#include <QtGlobal>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

Q_DECLARE_LOGGING_CATEGORY(decoder_js8);

namespace js8 {
/**
 * @brief Cache and combine repeated LLR frames for the same decode candidate.
 *
 * Uses a coarse freq/dt bin and a small LLR signature as the key; repeated
 * receptions accumulate LLRs to improve decode probability without changing
 * over-the-air behavior. The combiner is templated on the LLR length `N`
 * so the caller binds it to the decoder's bit count. Typical usage:
 * - Call `makeKey()` to construct a lookup key for a candidate.
 * - Call `combine()` with the candidate's LLRs to accumulate and retrieve
 *   combined LLRs and repeat count.
 */
template <std::size_t N> class SoftCombiner {
    using Clock = std::chrono::steady_clock;

  public:
    /**
     * @brief Lookup key describing a decode candidate.
     *
     * `mode`, `freqBin` and `dtBin` are coarse-binned coordinates; `signature`
     * is a compact fingerprint computed from a subset of LLR values.
     */
    struct Key {
        int mode;       ///< Submode/mode index
        int freqBin;    ///< Coarse frequency bin
        int dtBin;      ///< Coarse timing bin
        uint32_t signature; ///< Compact LLR signature

        bool operator==(Key const &other) const noexcept {
            return mode == other.mode && freqBin == other.freqBin &&
                   dtBin == other.dtBin && signature == other.signature;
        }
    };

    /**
     * @brief Result returned by `combine()` describing combined LLRs.
     *
     * `repeats` indicates how many repeated receptions were merged.
     * `combined` is true when the returned LLRs are the accumulated values
     * from previous repeats; false when this is the first seen instance.
     */
    struct Combined {
        Key key;                      ///< Candidate key
        std::array<float, N> llr0;    ///< Combined LLR0 values
        std::array<float, N> llr1;    ///< Combined LLR1 values
        int repeats;                  ///< Repeat count
        bool combined;                ///< True when combined from prior entries
    };

    /**
     * @brief Construct a SoftCombiner with defaults.
     *
     * The default constructor queries the environment to decide whether
     * soft-combining is enabled.
     */
    SoftCombiner() : SoftCombiner(defaultEnabled(), true) {}

    /**
     * @brief Construct a SoftCombiner with explicit enable flag.
     * @param enabled Whether combining is active.
     * @param runSelfTest If true, run a self-test when constructing.
     */
    explicit SoftCombiner(bool enabled, bool runSelfTest = true)
        : m_enabled(enabled) {
        if (!m_enabled) {
            qCDebug(decoder_js8)
                << "soft-combining disabled (JS8_SOFT_COMBINING=0)";
        }
        if (runSelfTest)
            maybeRunSelfTest();
    }

        /**
         * @brief Create a `Key` for a candidate from its parameters and LLRs.
         * @param mode Submode/mode index.
         * @param f1 Frequency estimate (Hz) used to compute a coarse bin.
         * @param dt Timing offset (s) used to compute a coarse bin.
         * @param llr0 LLR0 array for the candidate.
         * @param llr1 LLR1 array for the candidate.
         * @return Constructed `Key` with computed signature.
         */
        Key makeKey(int mode, float f1, float dt, std::array<float, N> const &llr0,
            std::array<float, N> const &llr1) const {
        return Key{mode, static_cast<int>(std::lround(f1)),
               static_cast<int>(std::lround(dt * 10.0f)), // 100 ms bins
               signature(llr0, llr1)};
        }

    /**
     * @brief Attempt to combine incoming LLRs with cached repeats.
     * @param key Candidate key constructed by `makeKey()`.
     * @param llr0 LLR0 values for this reception.
     * @param llr1 LLR1 values for this reception.
     * @param ttl Time-to-live for cached repeats; older entries are flushed.
     * @return `Combined` describing the output LLRs and repeat count.
     */
    Combined combine(Key const &key, std::array<float, N> const &llr0,
                     std::array<float, N> const &llr1,
                     std::chrono::seconds ttl) {
        flush(ttl);

        if (!m_enabled) {
            return Combined{key, llr0, llr1, 1, false};
        }

        auto &bucket = m_entries[keyForLookup(key)];
        auto it = findEntry(bucket, key.signature);

        if (it == bucket.end()) {
            bucket.push_back(makeEntry(key.signature, llr0, llr1));
            return Combined{key, llr0, llr1, 1, false};
        }

        for (std::size_t i = 0; i < llr0.size(); ++i) {
            it->llr0[i] += llr0[i];
            it->llr1[i] += llr1[i];
        }

        ++it->repeats;
        it->lastSeen = Clock::now();

        qCDebug(decoder_js8)
            << "soft-combining repeats" << it->repeats << "mode" << key.mode
            << "freq" << key.freqBin << "dtbin" << key.dtBin;

        return Combined{key, it->llr0, it->llr1, it->repeats, true};
    }

    /**
     * @brief Remove cached repeats for a candidate that has been decoded.
     * @param key Candidate key whose cached entries should be removed.
     */
    void markDecoded(Key const &key) {
        if (!m_enabled)
            return;

        auto lookup = keyForLookup(key);
        auto it = m_entries.find(lookup);

        if (it == m_entries.end())
            return;

        auto &bucket = it->second;
        bucket.erase(std::remove_if(bucket.begin(), bucket.end(),
                                    [&key](Entry const &entry) {
                                        return entry.signature == key.signature;
                                    }),
                     bucket.end());

        if (bucket.empty())
            m_entries.erase(it);
    }

    /**
     * @brief Remove cached entries older than `ttl`.
     * @param ttl Time-to-live; entries not seen within `ttl` are purged.
     */
    void flush(std::chrono::seconds ttl) {
        if (!m_enabled)
            return;

        auto const now = Clock::now();

        for (auto it = m_entries.begin(); it != m_entries.end();) {
            auto &bucket = it->second;

            bucket.erase(std::remove_if(bucket.begin(), bucket.end(),
                                        [now, ttl](Entry const &entry) {
                                            return now - entry.lastSeen > ttl;
                                        }),
                         bucket.end());

            if (bucket.empty())
                it = m_entries.erase(it);
            else
                ++it;
        }
    }

  private:
    /** Coarse lookup key grouping similar candidates. */
    struct CoarseKey {
        int mode;
        int freqBin;
        int dtBin;

        bool operator==(CoarseKey const &other) const noexcept {
            return mode == other.mode && freqBin == other.freqBin &&
                   dtBin == other.dtBin;
        }
    };

    struct CoarseHash {
        std::size_t operator()(CoarseKey const &key) const noexcept {
            std::size_t const h1 = std::hash<int>{}(key.mode);
            std::size_t const h2 = std::hash<int>{}(key.freqBin);
            std::size_t const h3 = std::hash<int>{}(key.dtBin);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };

    /**
     * @brief Stored entry representing accumulated LLRs for a single signature.
     */
    struct Entry {
        uint32_t signature;           ///< Signature fingerprint
        std::array<float, N> llr0;    ///< Accumulated LLR0
        std::array<float, N> llr1;    ///< Accumulated LLR1
        int repeats;                  ///< Number of repeats combined
        Clock::time_point lastSeen;   ///< Last-seen timestamp
    };

    using Bucket = std::vector<Entry>;

    /**
     * @brief Indices selected from the LLR arrays to compute the compact signature.
     *
     * The signature uses a fixed set of indices derived from `N` so that the
     * signature maps noisy repeats to similar bit patterns.
     */
    static constexpr auto signatureIndices() {
        std::array<int, 32> indices{};
        int value = 0;
        for (std::size_t i = 0; i < indices.size(); ++i) {
            value = (value + 37) % static_cast<int>(N);
            indices[i] = value;
        }
        return indices;
    }

    /**
     * @brief Find an entry in a bucket whose signature has small Hamming distance.
     * @param bucket Vector of entries for a coarse key.
     * @param signature Compact signature to match.
     * @return Iterator to matching entry or `bucket.end()` if none.
     */
    static typename Bucket::iterator findEntry(Bucket &bucket, uint32_t signature) {
        constexpr int MAX_HAMMING =
            4; // allow small differences between noisy repeats

        return std::find_if(
            bucket.begin(), bucket.end(), [signature](Entry const &entry) {
                return hamming(signature, entry.signature) <= MAX_HAMMING;
            });
    }

    /**
     * @brief Query environment to determine default enabled state.
     * @return True if soft combining is enabled by default.
     */
    static bool defaultEnabled() {
        bool ok = false;
        int value = qEnvironmentVariableIntValue("JS8_SOFT_COMBINING", &ok);
        return ok ? value != 0 : true;
    }

    /**
     * @brief Compute compact 32-bit signature from two LLR arrays.
     */
    static uint32_t signature(std::array<float, N> const &llr0,
                              std::array<float, N> const &llr1) {
        static constexpr auto INDICES = signatureIndices();

        uint32_t sig = 0;
        for (std::size_t i = 0; i < INDICES.size(); ++i) {
            auto const idx = INDICES[i];
            float const v = 0.5f * (llr0[idx] + llr1[idx]);
            if (v >= 0.0f)
                sig |= (1u << i);
        }
        return sig;
    }

    /** Compute Hamming distance between two 32-bit signatures. */
    static int hamming(uint32_t a, uint32_t b) {
        uint32_t v = a ^ b;
        int c = 0;
        while (v) {
            v &= (v - 1);
            ++c;
        }
        return c;
    }

    /**
     * @brief Optionally run a deterministic self-test when requested via env.
     */
    static void maybeRunSelfTest() {
        static std::once_flag once;
        std::call_once(once, []() {
            if (!qEnvironmentVariableIsSet("JS8_SOFT_COMBINING_TEST"))
                return;

            SoftCombiner combiner(true, false);

            std::array<float, N> baseline{};
            for (std::size_t i = 0; i < baseline.size(); ++i) {
                baseline[i] = (i % 2 == 0) ? 2.0f : -2.0f;
            }

            auto noisy = [](std::array<float, N> base, int flipStride) {
                for (std::size_t i = 0; i < base.size(); ++i) {
                    if (i % flipStride == 0) {
                        base[i] *= -0.4f; // flip sign and reduce magnitude
                    } else {
                        base[i] *= 0.8f; // weaken but keep sign
                    }
                }
                return base;
            };

            auto llrA = noisy(baseline, 7);
            auto llrB = noisy(baseline, 11);

            auto key = combiner.makeKey(0, 1500.0f, 1.0f, llrA, llrB);
            auto first =
                combiner.combine(key, llrA, llrA, std::chrono::seconds{30});
            auto second =
                combiner.combine(key, llrB, llrB, std::chrono::seconds{30});

            auto countMatches = [](std::array<float, N> const &llr,
                                   std::array<float, N> const &reference) {
                int matches = 0;
                for (std::size_t i = 0; i < llr.size(); ++i) {
                    if (llr[i] * reference[i] > 0)
                        ++matches;
                }
                return matches;
            };

            int const matchesA = countMatches(first.llr0, baseline);
            int const matchesB = countMatches(llrB, baseline);
            int const matchesCombined = countMatches(second.llr0, baseline);

            qCDebug(decoder_js8)
                << "soft-combining self-test: A matches" << matchesA
                << "B matches" << matchesB << "combined matches"
                << matchesCombined << "repeats" << second.repeats;
        });
    }

    /** Create an Entry from raw signature and LLR arrays. */
    static Entry makeEntry(uint32_t signature, std::array<float, N> const &llr0,
                           std::array<float, N> const &llr1) {
        return Entry{signature, llr0, llr1, 1, Clock::now()};
    }

    /** Map a full `Key` to its coarse `CoarseKey` used for bucket lookup. */
    CoarseKey keyForLookup(Key const &key) const {
        return CoarseKey{key.mode, key.freqBin, key.dtBin};
    }
    std::unordered_map<CoarseKey, Bucket, CoarseHash> m_entries; ///< Bucketed entries
    bool m_enabled; ///< Whether soft combining is active
};
} // namespace js8
