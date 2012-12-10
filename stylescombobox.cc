/* Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "stylescombobox.hh"
#include "config.hh"
#include <QDir>

StylesComboBox::StylesComboBox( QWidget * parent ):
  QComboBox( parent )
{
  fill();
  setVisible( count() > 1 );
}

void StylesComboBox::fill()
{
  clear();
  addItem( tr( "None" ) );
  QString stylesDir = Config::getStylesDir();
  if( !stylesDir.isEmpty() )
  {
    QDir dir( stylesDir );
    QStringList styles = dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::LocaleAware );
    addItems( styles );
  }
}

void StylesComboBox::setCurrentStyle( QString const & style )
{
  int nom = 0;
  if( !style.isEmpty() )
  {
    for( int i = 1; i < count(); i++ )
      if( style.compare( itemText( i ) ) == 0 )
      {
        nom = i;
        break;
      }
  }
  setCurrentIndex( nom );
}

QString StylesComboBox::getCurrentStyle() const
{
  if( currentIndex() == 0 )
    return QString();
  return itemText( currentIndex() );
}
