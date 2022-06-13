#include "globalregex.hh"
#include "fulltextsearch.hh"

using namespace RX;

QRegularExpression Ftx::regBrackets(
  "(\\([\\w\\p{M}]+\\)){0,1}([\\w\\p{M}]+)(\\([\\w\\p{M}]+\\)){0,1}([\\w\\p{M}]+){0,1}(\\([\\w\\p{M}]+\\)){0,1}",
  QRegularExpression::UseUnicodePropertiesOption );
QRegularExpression Ftx::regSplit( "[^\\w\\p{M}]+", QRegularExpression::UseUnicodePropertiesOption );

QRegularExpression Ftx::spacesRegExp( "\\W+", QRegularExpression::UseUnicodePropertiesOption );
QRegularExpression Ftx::wordRegExp( QString( "\\w{" ) + QString::number( FTS::MinimumWordSize ) + ",}",
                                    QRegularExpression::UseUnicodePropertiesOption );
QRegularExpression Ftx::setsRegExp( "\\[[^\\]]+\\]", QRegularExpression::CaseInsensitiveOption );
QRegularExpression Ftx::regexRegExp( "\\\\[afnrtvdDwWsSbB]|\\\\x([0-9A-Fa-f]{4})|\\\\0([0-7]{3})",
                                     QRegularExpression::CaseInsensitiveOption );


//mdx

QRegularExpression Mdx::allLinksRe( "(?:<\\s*(a(?:rea)?|img|link|script|source)(?:\\s+[^>]+|\\s*)>)",
                                    QRegularExpression::CaseInsensitiveOption );
QRegularExpression Mdx::wordCrossLink( "([\\s\"']href\\s*=)\\s*([\"'])entry://([^>#]*?)((?:#[^>]*?)?)\\2",
                                       QRegularExpression::CaseInsensitiveOption );
QRegularExpression Mdx::anchorIdRe( "([\\s\"'](?:name|id)\\s*=)\\s*([\"'])\\s*(?=\\S)",
                                    QRegularExpression::CaseInsensitiveOption );
QRegularExpression Mdx::anchorIdReWord( "([\\s\"'](?:name|id)\\s*=)\\s*([\"'])\\s*(?=\\S)([^\"]*)",
                                        QRegularExpression::CaseInsensitiveOption );
QRegularExpression Mdx::anchorIdRe2( "([\\s\"'](?:name|id)\\s*=)\\s*(?=[^\"'])([^\\s\">]+)",
                                     QRegularExpression::CaseInsensitiveOption );
QRegularExpression Mdx::anchorLinkRe( "([\\s\"']href\\s*=\\s*[\"'])entry://#",
                                      QRegularExpression::CaseInsensitiveOption );
QRegularExpression Mdx::audioRe( "([\\s\"']href\\s*=)\\s*([\"'])sound://([^\">]+)\\2",
                                 QRegularExpression::CaseInsensitiveOption
                                   | QRegularExpression::InvertedGreedinessOption );
QRegularExpression Mdx::stylesRe( "([\\s\"']href\\s*=)\\s*([\"'])(?!\\s*\\b(?:(?:bres|https?|ftp)://"
                                  "|(?:data|javascript):))(?:file://)?[\\x00-\\x1f\\x7f]*\\.*/?([^\">]+)\\2",
                                  QRegularExpression::CaseInsensitiveOption );
QRegularExpression Mdx::stylesRe2( "([\\s\"']href\\s*=)\\s*(?![\\s\"']|\\b(?:(?:bres|https?|ftp)://"
                                   "|(?:data|javascript):))(?:file://)?[\\x00-\\x1f\\x7f]*\\.*/?([^\\s\">]+)",
                                   QRegularExpression::CaseInsensitiveOption );
QRegularExpression Mdx::inlineScriptRe( "<\\s*script(?:(?=\\s)(?:(?![\\s\"']src\\s*=)[^>])+|\\s*)>",
                                        QRegularExpression::CaseInsensitiveOption );
QRegularExpression Mdx::closeScriptTagRe( "<\\s*/script\\s*>", QRegularExpression::CaseInsensitiveOption );
QRegularExpression Mdx::srcRe( "([\\s\"']src\\s*=)\\s*([\"'])(?!\\s*\\b(?:(?:bres|https?|ftp)://"
                               "|(?:data|javascript):))(?:file://)?[\\x00-\\x1f\\x7f]*\\.*/?([^\">]+)\\2",
                               QRegularExpression::CaseInsensitiveOption );
QRegularExpression Mdx::srcRe2( "([\\s\"']src\\s*=)\\s*(?![\\s\"']|\\b(?:(?:bres|https?|ftp)://"
                                "|(?:data|javascript):))(?:file://)?[\\x00-\\x1f\\x7f]*\\.*/?([^\\s\">]+)",
                                QRegularExpression::CaseInsensitiveOption );
