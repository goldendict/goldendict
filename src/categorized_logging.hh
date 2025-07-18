/* This file is (c) 2022 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef CATEGORIZED_LOGGING_HH_INCLUDED
#define CATEGORIZED_LOGGING_HH_INCLUDED

// Lots of changes have been made to Qt's implementation of categorized logging in versions 5.3 and 5.4.
// __VA_ARGS__ was introduced in C++11.
#if QT_VERSION >= QT_VERSION_CHECK( 5, 4, 0 ) && __cplusplus >= 201103L
#include <QLoggingCategory>
#define GD_CATEGORIZED_LOGGING
#endif

#ifdef GD_CATEGORIZED_LOGGING
Q_DECLARE_LOGGING_CATEGORY( dictionaryResourceLc )

#if !defined(QT_NO_WARNING_OUTPUT)
/// Print a categorized warning message.
#  define gdCWarning(category, ...) \
    for (bool qt_category_enabled = category().isWarningEnabled(); qt_category_enabled; qt_category_enabled = false) \
        gdCWarningImpl( category(), __VA_ARGS__ )
#else
#  define qCWarning(category, ...) QT_NO_QDEBUG_MACRO()
#endif

#if !defined(QT_NO_DEBUG_OUTPUT)
/// Print a categorized debug message.
#  define gdCDebug(category, ...) \
    for (bool qt_category_enabled = category().isDebugEnabled(); qt_category_enabled; qt_category_enabled = false) \
        gdCDebugImpl( category(), __VA_ARGS__ )
#else
#  define qCDebug(category, ...) QT_NO_QDEBUG_MACRO()
#endif

void gdCWarningImpl( QLoggingCategory const & category, char const * message, ... ) Q_ATTRIBUTE_FORMAT_PRINTF( 2, 3 );
void gdCDebugImpl( QLoggingCategory const & category, char const * message, ... ) Q_ATTRIBUTE_FORMAT_PRINTF( 2, 3 );

#else // GD_CATEGORIZED_LOGGING
// Compatibility shims.

enum GdLoggingCategory
{
  dictionaryResourceLc,
};

/// Equivalent to gdWarning( @p message, ... )
void gdCWarning( GdLoggingCategory, char const * message, ... )
#if defined(Q_CC_GNU) && !defined(__INSURE__)
__attribute__ ((format (printf, 2, 3)))
#endif
;
/// Equivalent to gdDebug( @p message, ... )
void gdCDebug( GdLoggingCategory, char const * message, ... )
#if defined(Q_CC_GNU) && !defined(__INSURE__)
__attribute__ ((format (printf, 2, 3)))
#endif
;
#endif // GD_CATEGORIZED_LOGGING

#endif // CATEGORIZED_LOGGING_HH_INCLUDED
