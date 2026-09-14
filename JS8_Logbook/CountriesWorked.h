/**
 * @file CountriesWorked.h
 * @brief Track which DXCC entities (countries) have been worked.
 */

#ifndef __COUNTRIESWORKDED_H
#define __COUNTRIESWORKDED_H

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>

/**
 * @class CountriesWorked
 * @brief Simple boolean set indicating which country names have been worked.
 *
 * The class stores a mapping from country name to a boolean flag which is
 * toggled when a country is marked as worked. It supports initialization
 * from a list of country names and exposes simple accessors for querying and
 * counting worked entities.
 */
class CountriesWorked {
  public:
    /**
     * @brief Initialize the set with a list of known country names.
     * @param countryNames List of country/entity names to initialize.
     */
    void init(const QStringList countryNames);

    /**
     * @brief Mark a country as worked.
     * @param countryName Name of the country to mark worked.
     */
    void setAsWorked(const QString countryName);

    /**
     * @brief Query whether a country has been worked.
     * @param countryName Name to check.
     * @return true if worked, false otherwise.
     */
    bool getHasWorked(const QString countryName) const;

    /**
     * @brief Count how many countries have been marked as worked.
     */
    qsizetype getWorkedCount() const;

    /**
     * @brief Get the total number of tracked country names.
     */
    qsizetype getSize() const;

  private:
    QHash<QString, bool> _data; /**< country name -> worked flag */
};

#endif
