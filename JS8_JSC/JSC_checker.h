#ifndef JSC_CHECKER_H
#define JSC_CHECKER_H

/**
 * @file JSC_checker.h
 * @brief Spelling and suggestion helper for the JS8Call compressed word list.
 *
 * This small utility provides routines to check text ranges in a
 * `QTextEdit` for words that appear in the JS8 compression dictionary and
 * to generate suggestion lists for misspelled or unknown words.
 * 
 * @author Jordan Sherer <jordan@sherer.org>
 * @copyright (c) 2018 Jordan Sherer
 */

#include <QObject>

class QTextEdit;

/**
 * @class JSCChecker
 * @brief Helper for checking and suggesting compressed-codewords.
 *
 * The class exposes two static convenience methods: one to examine a
 * range inside a `QTextEdit` and mark or process words, and another to
 * produce suggestion candidates for a single word.
 */
class JSCChecker : public QObject {
    Q_OBJECT
  public:
    /**
     * @brief Construct a checker instance.
     * @param parent Optional parent QObject.
     */
    explicit JSCChecker(QObject *parent = nullptr);

    /**
     * @brief Check the text in @p edit between @p start and @p end.
     * @param edit The QTextEdit containing the text to check.
     * @param start Byte offset (or cursor position) of the range start.
     * @param end Byte offset (or cursor position) of the range end.
     *
     * This routine inspects words in the given range and can be used to
     * highlight unknown words or trigger UI updates. Implementation may
     * interact with the UI; call from the GUI thread.
     */
    static void checkRange(QTextEdit *edit, int start, int end);

    /**
     * @brief Generate up to @p n suggestion words for @p word.
     * @param word The input word to generate suggestions for.
     * @param n Maximum number of suggestions to return.
     * @param pFound Optional output boolean set to true when an exact match is found.
     * @return A list of suggestion strings ordered by relevance.
     */
    static QStringList suggestions(QString word, int n, bool *pFound);

  signals:

  public slots:

  private:
};

#endif // JSC_CHECKER_H
