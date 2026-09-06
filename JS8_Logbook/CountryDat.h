/**
 * @file CountryDat.h
 * @brief Parse cty.dat and map callsign prefixes to country names.
 *
 * This utility reads a CTY-style file (cty.dat) and produces a mapping from
 * prefix strings to human-readable country/entity names. The mapping is used
 * by the logbook to resolve DXCC entity names from callsigns.
 */

#ifndef __COUNTRYDAT_H
#define __COUNTRYDAT_H

#include <QHash>
#include <QString>
#include <QStringList>

/**
 * @class CountryDat
 * @brief Reader and lookup helper for CTY prefix files.
 *
 * Typical usage: call `init(filename)` then `load()` to populate the
 * internal prefix->country mapping. Use `find()` to resolve a prefix to the
 * country name and `getCountryNames()` to obtain the set of known entities.
 */
class CountryDat {
  public:
    /**
     * @brief Set the filename of the CTY/cty.dat file to parse.
     * @param filename Path to the CTY data file.
     */
    void init(const QString filename);

    /**
     * @brief Parse the configured file and populate the internal mapping.
     */
    void load();

    /**
     * @brief Find the country/entity name for a callsign prefix.
     * @param prefix Callsign prefix to resolve.
     * @return Country/entity name or an empty string when unknown.
     */
    QString find(QString prefix) const; // return country name or ""

    /**
     * @brief Get a list of country/entity names discovered in the file.
     * @return List of country names.
     */
    QStringList getCountryNames() const { return _countryNames; };

  private:
    /**
     * @brief Extract the display name from a CTY line.
     */
    QString _extractName(const QString line) const;

    /**
     * @brief Remove a pair of enclosing brackets from a string, in-place.
     */
    void _removeBrackets(QString &line, const QString a, const QString b) const;

    /**
     * @brief Extract one or more prefixes from a CTY line.
     * @param line Input line, modified during extraction.
     * @param more Set to true if further prefixes remain to be parsed.
     */
    QStringList _extractPrefix(QString &line, bool &more) const;

    /**
     * @brief Perform country name fixups based on callsign context.
     */
    QString fixup(QString country, QString const &call) const;

    QString _filename; /**< Path to cty.dat */
    QStringList _countryNames; /**< Discovered country/entity names */
    QHash<QString, QString> _data; /**< prefix -> country name map */
};

#endif
/*
 * Reads cty.dat file
 * Establishes a map between prefixes and their country names
 * VK3ACF July 2013
 */

#ifndef __COUNTRYDAT_H
#define __COUNTRYDAT_H

#include <QHash>
#include <QString>
#include <QStringList>

class CountryDat {
  public:
    void init(const QString filename);
    void load();
    QString find(QString prefix) const; // return country name or ""
    QStringList getCountryNames() const { return _countryNames; };

  private:
    QString _extractName(const QString line) const;
    void _removeBrackets(QString &line, const QString a, const QString b) const;
    QStringList _extractPrefix(QString &line, bool &more) const;
    QString fixup(QString country, QString const &call) const;

    QString _filename;
    QStringList _countryNames;
    QHash<QString, QString> _data;
};

#endif
