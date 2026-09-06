/**
 * @file ADIF.h
 * @brief ADIF (Amateur Data Interchange Format) log file reader and writer.
 *
 * Reads an ADIF log file into memory and provides search and append helpers.
 * This header declares the ADIF utility class used by the logbook
 * implementation to parse, query and append QSO records.
 *
 * Compatible with Doxygen 1.16.
 *
 * @author VK3ACF
 * @date July 2013
 */

#ifndef __ADIF_H
#define __ADIF_H

#include "JS8_Main/fileutils.h"

#include <QtGui>

class QDateTime;

/**
 * @brief Known ADIF field names used when parsing ADIF records.
 *
 * This list contains the ADIF field names (upper-case) that are
 * extracted from ADIF records by the implementation. Users should treat
 * this symbol as read-only.
 */
extern const QStringList ADIF_FIELDS;

/**
 * @class ADIF
 * @brief Parser and in-memory store for ADIF QSO records.
 *
 * The ADIF class provides basic functionality to read ADIF files into an
 * in-memory container, search for QSOs by callsign, and append new QSO
 * records back to the ADIF file.
 */
class ADIF {
  public:
    /**
     * @brief Minimal representation of a QSO (contact) record.
     *
     * Only a small subset of ADIF fields are stored here for quick lookup
     * and display in the logbook UI.
     */
    struct QSO;

    /**
     * @brief Initialize the ADIF handler with a filename.
     * @param filename Path to the ADIF file to read or append to.
     */
    void init(QString const &filename);

    /**
     * @brief Load and parse the configured ADIF file into memory.
     *
     * After calling this method, the in-memory store is populated and
     * searchable via `find` and `getCallList`.
     */
    void load();

    /**
     * @brief Add a QSO to the in-memory store.
     * @param call Remote station callsign.
     * @param band Band string (e.g. "20m").
     * @param mode Mode string (e.g. "FT8").
     * @param submode Submode string, if any.
     * @param grid Maidenhead grid square of the remote station.
     * @param date Date string for the QSO.
     * @param name Name associated with the remote station.
     * @param comment Optional comment.
     */
    void add(QString const &call, QString const &band, QString const &mode,
             const QString &submode, QString const &grid, QString const &date,
             const QString &name, const QString &comment);

    /**
     * @brief Test whether a call/band combination exists in memory.
     * @param call Callsign to match.
     * @param band Band to match.
     * @return true if a matching QSO exists, false otherwise.
     */
    bool match(QString const &call, QString const &band) const;

    /**
     * @brief Find all QSOs matching a callsign.
     * @param call Callsign to search for.
     * @return List of `QSO` records matching the callsign.
     */
    QList<ADIF::QSO> find(QString const &call) const;

    /**
     * @brief Get a list of unique calls found in the in-memory store.
     * @return List of callsigns as strings.
     */
    QList<QString> getCallList() const;

    /**
     * @brief Number of QSO records currently stored in memory.
     * @return Count of stored QSOs.
     */
    qsizetype getCount() const;

    /**
     * @brief Append a raw ADIF record to the configured ADIF file.
     * @param ADIF_record Byte array containing a properly formatted ADIF record.
     * @return true on success, false on failure (e.g. file I/O error).
     */
    bool addQSOToFile(QByteArray const &ADIF_record);

    /**
     * @brief Convert QSO parameters into a ADIF formatted record.
     *
     * This helper builds a byte array containing a single ADIF record using
     * the provided fields. Additional fields may be appended via
     * `additionalFields`.
     *
     * @return ADIF record as a `QByteArray` suitable for appending to file.
     */
    QByteArray QSOToADIF(QString const &hisCall, QString const &hisGrid,
                         QString const &mode, QString const &submode,
                         QString const &rptSent, QString const &rptRcvd,
                         QDateTime const &dateTimeOn,
                         QDateTime const &dateTimeOff, QString const &band,
                         QString const &comments, QString const &name,
                         QString const &strDialFreq, QString const &m_myCall,
                         QString const &m_myGrid, QString const &operator_call,
                         const QMap<QString, QVariant> &additionalFields);

    /**
     * @brief Minimal representation of a QSO (contact) record.
     *
     * Members are stored as plain strings matching common ADIF fields.
     */
    struct QSO {
      QString call;     ///< Remote station callsign (e.g. "K1ABC").
      QString band;     ///< Band string used for the contact (e.g. "20m").
      QString mode;     ///< Mode used for the contact (e.g. "FT8").
      QString submode;  ///< Submode, if any.
      QString grid;     ///< Maidenhead grid square of the remote station.
      QString date;     ///< Date string for the QSO.
      QString name;     ///< Operator name of the remote station.
      QString comment;  ///< Optional comment associated with the QSO.
    };

  private:
    QMultiHash<QString, QSO> _data; /**< In-memory index: call -> QSO records */
    QString _filename; /**< Path to the ADIF file backing this store */

    /**
     * @brief Extract a named ADIF field value from a line of text.
     * @param line Input line containing ADIF field tokens.
     * @param fieldName Name of the field to extract (case-insensitive).
     * @return Extracted field value or empty string if not found.
     */
    QString extractField(QString const &line, QString const &fieldName) const;
};

#endif
