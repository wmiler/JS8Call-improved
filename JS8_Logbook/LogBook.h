/**
 * @file LogBook.h
 * @brief High-level logbook operations combining ADIF and CTY data.
 *
 * The LogBook class ties together ADIF parsing, country resolution and the
 * worked status tracker. It provides helpers to query whether a callsign or
 * country has been worked before and to add new worked entries.
 */

#ifndef LOGBOOK_H
#define LOGBOOK_H

#include "ADIF.h"
#include "CountriesWorked.h"
#include "CountryDat.h"
#include "n3fjp.h"

#include <QFont>
#include <QString>

class QDir;

/**
 * @class LogBook
 * @brief Integrates ADIF, CTY and worked-state utilities for logbook queries.
 */
class LogBook {
  public:
    /**
     * @brief Initialize internal helpers and data.
     */
    void init();

    /**
     * @brief Check whether a call on a specific band has been worked before.
     * @param call Callsign to check.
     * @param band Band string to check.
     * @return true if the call/band combination exists in the log.
     */
    bool hasWorkedBefore(const QString &call, const QString &band);

    /**
     * @brief Resolve callsign to a country and determine worked-before flags.
     * @param call Input callsign.
     * @param countryName Output: resolved country/entity name.
     * @param callWorkedBefore Output: whether this call has been worked.
     * @param countryWorkedBefore Output: whether the country has been worked.
     */
    void match(/*in*/ const QString call,
               /*out*/ QString &countryName, bool &callWorkedBefore,
               bool &countryWorkedBefore) const;

    /**
     * @brief Find detailed call information (grid, date, name, comment).
     * @param call Input callsign to search for.
     * @param grid Output: Maidenhead grid if found.
     * @param date Output: QSO date string if found.
     * @param name Output: operator name if present.
     * @param comment Output: comment if present.
     * @return true if call details were found, false otherwise.
     */
    bool findCallDetails(/*in*/
                         const QString call,
                         /*out*/
                         QString &grid, QString &date, QString &name,
                         QString &comment) const;

    /**
     * @brief Mark the given call as worked and add it to the ADIF/log store.
     */
    void addAsWorked(const QString call, const QString band, const QString mode,
                     const QString submode, const QString grid,
                     const QString date, const QString name,
                     const QString comment);

  private:
    CountryDat _countries; /**< CTY prefix resolver */
    CountriesWorked _worked; /**< Worked state tracker */
    ADIF _log; /**< ADIF-backed log store */

    /**
     * @brief Populate the worked-state from entries already present in the log.
     */
    void _setAlreadyWorkedFromLog();
};

#endif // LOGBOOK_H
