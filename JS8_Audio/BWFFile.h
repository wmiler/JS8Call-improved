/**
 * @file BWFFile.h
 * @brief Broadcast Wave Format (BWF) file helper and QIODevice wrapper.
 *
 * BWF is a WAV-compatible format with EBU 'bext' metadata. This class
 * exposes the audio sample data as a `QIODevice` while providing access
 * to the bext and LIST-INFO metadata chunks.
 *
 * The InfoDictionary used by some constructors should contain valid WAV
 * LIST-INFO identifiers as keys; a list of common identifiers is
 * available at:
 * http://bwfmetaedit.sourceforge.net/listinfo.html
 *
 * For files opened ReadOnly the dictionary is not written back. For files
 * opened ReadWrite, any existing LIST-INFO tags are merged into the
 * dictionary when the file is opened and the merged dictionary will be
 * written back to the file if the file is modified.
 *
 * The sample data may not be in the native endian. Callers are
 * responsible for any required endian conversions; internally the
 * class presents data in native endian and performs conversions
 * automatically. Use the `format()` accessor and
 * `QAudioFormat::byteOrder()` to determine byte ordering.
 *
 * @see https://tech.ebu.ch/docs/tech/tech3285.pdf
 * @see https://tech.ebu.ch/docs/r/r098.pdf
 */

#ifndef BWF_FILE_HPP__
#define BWF_FILE_HPP__

#include "JS8_Include/pimpl_h.h"

#include <QByteArray>
#include <QFile>
#include <QMap>

#include <array>

class QObject;
class QString;
class QAudioFormat;

/**
 * @class BWFFile
 * @brief QIODevice-style access to BWF/WAV sample data and metadata.
 *
 * The class hides header/trailer chunks and exposes only the sample data
 * as a contiguous device. Metadata may be read and modified via the
 * provided bext_* and list_info operations.
 */
class BWFFile : public QIODevice {
    Q_OBJECT
  public:
    using FileHandleFlags = QFile::FileHandleFlags;
    using Permissions = QFile::Permissions;
    using FileError = QFile::FileError;
    using MemoryMapFlags = QFile::MemoryMapFlags;
    using InfoDictionary = QMap<std::array<char, 4>, QByteArray>;
    using UMID = std::array<quint8, 64>;

    explicit BWFFile(QAudioFormat const &, QObject *parent = nullptr);
    explicit BWFFile(QAudioFormat const &, QString const &name,
                     QObject *parent = nullptr);

    
    explicit BWFFile(QAudioFormat const &, QString const &name,
                     InfoDictionary const &, QObject *parent = nullptr);

    ~BWFFile();
    QAudioFormat const &format() const;
    InfoDictionary &list_info();

    //
    // Broadcast Audio Extension fields
    //
    // If any of these modifiers are  called then a "bext" chunk will be
    // written to the file if the  file is writeable and the sample data
    // is modified.
    //
    enum class BextVersion : quint16 { v_0, v_1, v_2 };
    BextVersion bext_version() const;
    void bext_version(BextVersion = BextVersion::v_2);

    QByteArray bext_description() const;
    void bext_description(QByteArray const &); // max 256 bytes

    QByteArray bext_originator() const;
    void bext_originator(QByteArray const &); // max 32 bytes

    QByteArray bext_originator_reference() const;
    void bext_originator_reference(QByteArray const &); // max 32 bytes

    QDateTime bext_origination_date_time() const;
    void bext_origination_date_time(QDateTime const &); // 1s resolution

    quint64 bext_time_reference() const;
    void bext_time_reference(quint64); // samples since midnight at start

    UMID bext_umid() const; // bext version >= 1 only
    void bext_umid(UMID const &);

    quint16 bext_loudness_value() const;
    void bext_loudness_value(quint16); // bext version >= 2 only

    quint16 bext_loudness_range() const;
    void bext_loudness_range(quint16); // bext version >= 2 only

    quint16 bext_max_true_peak_level() const;
    void bext_max_true_peak_level(quint16); // bext version >= 2 only

    quint16 bext_max_momentary_loudness() const;
    void bext_max_momentary_loudness(quint16); // bext version >= 2 only

    quint16 bext_max_short_term_loudness() const;
    void bext_max_short_term_loudness(quint16); // bext version >= 2 only

    QByteArray bext_coding_history() const;
    void bext_coding_history(QByteArray const &); // See EBU R 98

    // Emulate QFile interface
    bool open(OpenMode) override;
    bool open(FILE *, OpenMode, FileHandleFlags = QFile::DontCloseHandle);
    bool open(int fd, OpenMode, FileHandleFlags = QFile::DontCloseHandle);
    bool copy(QString const &new_name);
    bool exists() const;
    bool link(QString const &link_name);
    bool remove();
    bool rename(QString const &new_name);
    void setFileName(QString const &name);
    QString symLinkTarget() const;
    QString fileName() const;
    Permissions permissions() const;

    // Resize is of the sample data portion, header and trailer chunks
    // are excess to the given size
    bool resize(qint64 new_size);

    bool setPermissions(Permissions permissions);
    FileError error() const;
    bool flush();
    int handle() const;

    // The mapping offset is relative to the start of the sample data
    uchar *map(qint64 offset, qint64 size, MemoryMapFlags = QFile::NoOptions);
    bool unmap(uchar *address);

    void unsetError();

    //
    // QIODevice implementation
    //

    // The size returned is of the sample data only, header and trailer
    // chunks are hidden and handled internally
    qint64 size() const override;

    bool isSequential() const override;

    quint16 bitsPerSample() const;
    quint16 blockAlign() const;

    // The reset  operation clears the  'bext' and LIST-INFO as  if they
    // were  never supplied.  If the  file  is writable  the 'bext'  and
    // LIST-INFO chunks will not be  written making the resulting file a
    // lowest common denominator WAV file.
    bool reset() override;

    // Seek offsets are relative to the start of the sample data
    bool seek(qint64) override;

    // this can fail due to updating header issues, errors are ignored
    void close() override;

  protected:
    qint64 readData(char *data, qint64 max_size) override;
    qint64 writeData(char const *data, qint64 max_size) override;

  private:
    class impl;
    pimpl<impl> m_; ///< Private implementation pointer.
};

#endif
