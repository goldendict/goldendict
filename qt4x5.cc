#include "qt4x5.hh"

#if IS_QT_5
#include <QRegularExpression>
#else
#include <QRegExp>
#endif

QString & Qt4x5::Regex::replace( QString & text, const QString & pattern, const QString & after,
                                 Qt::CaseSensitivity cs, bool useUnicodeProperties )
{
// QRegularExpression usually outperforms QRegExp, but is not available in Qt4.
#if IS_QT_5
  QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
  if( cs == Qt::CaseInsensitive )
    options |= QRegularExpression::CaseInsensitiveOption;
  if( useUnicodeProperties )
    options |= QRegularExpression::UseUnicodePropertiesOption;
  return text.replace( QRegularExpression( pattern, options ), after );
#else
  // When using QRegExp, character classes such as \w, \d, etc. always
  // match characters with the corresponding Unicode property.
  Q_UNUSED( useUnicodeProperties )
  return text.replace( QRegExp( pattern, cs ), after );
#endif
}
