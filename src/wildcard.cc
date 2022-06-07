#include <QRegularExpression>
#include "wildcard.hh"

/*
  Modified function from Qt
  Translates a wildcard pattern to an equivalent regular expression
  pattern (e.g., *.cpp to .*\.cpp).
*/

QString wildcardsToRegexp( const QString & wc_str )
{
    const int wclen = wc_str.length();
    QString rx;
    int i = 0;
    bool isEscaping = false; // the previous character is '\'
    const QChar *wc = wc_str.unicode();

    while( i < wclen ) {
      const QChar c = wc[ i++ ];
      switch( c.unicode() ) {
        case '\\':
            if( isEscaping ) {
                rx += QLatin1String( "\\\\" );
            } // we insert the \\ later if necessary
            if( i == wclen ) { // the end
                rx += QLatin1String( "\\\\" );
            }
            isEscaping = true;
            break;

        case '*':
            if( isEscaping ) {
                rx += QLatin1String( "\\*" );
                isEscaping = false;
            } else {
                rx += QLatin1String( ".*" );
            }
            break;

        case '?':
            if( isEscaping ) {
                rx += QLatin1String( "\\?" );
                isEscaping = false;
            } else {
                rx += QLatin1Char( '.' );
            }

            break;

        case '$':
        case '(':
        case ')':
        case '+':
        case '.':
        case '^':
        case '{':
        case '|':
        case '}':
            if( isEscaping ) {
                isEscaping = false;
                rx += QLatin1String( "\\\\" );
            }
            rx += QLatin1Char( '\\' );
            rx += c;
            break;
         case '[':
            if(isEscaping) {
                isEscaping = false;
                rx += QLatin1String( "\\[" );
            } else {
                QString tmp;
                tmp += c;
                if( i < wclen && wc[ i ] == QLatin1Char( '!' ) )
                {
                  tmp += QLatin1Char( '^' );
                  ++i;
                }
                while( i < wclen && wc[ i ] != QLatin1Char( ']' ) ) {
                    if( wc[ i ] == QLatin1Char( '\\' ) )
                        tmp += QLatin1Char( '\\' );
                    tmp += wc[ i++ ];
                }
                if( i < wclen )
                    rx += tmp;
                else
                    rx += QRegularExpression::escape( tmp );
            }
            break;

        case ']':
            if( isEscaping ){
                isEscaping = false;
                rx += QLatin1String( "\\" );
            }
            rx += c;
            break;

        default:
            if( isEscaping ){
                isEscaping = false;
                rx += QLatin1String( "\\\\" );
            }
            rx += c;
      }
    }
    return rx;
}
