#include "dictionarybar.hh"
#include <QAction>

using std::vector;

DictionaryBar::DictionaryBar( QWidget * parent,
                              Config::MutedDictionaries & mutedDictionaries_,
                              Config::Events & events ):
  QToolBar( tr( "Dictionary Bar" ), parent ),
  mutedDictionaries( mutedDictionaries_ ),
  configEvents( events )
{
  setObjectName( "dictionaryBar" );

  connect( &events, SIGNAL( dictionaryMuted( QString const & ) ),
           this, SLOT( dictionaryMuted( QString const & ) ) );

  connect( &events, SIGNAL( dictionaryUnmuted( QString const & ) ),
           this, SLOT( dictionaryUnmuted( QString const & ) ) );

  connect( this, SIGNAL(actionTriggered(QAction*)),
           this, SLOT(actionWasTriggered(QAction*)) );
}

void DictionaryBar::setDictionaries( vector< sptr< Dictionary::Class > >
                                     const & dictionaries )
{
  setUpdatesEnabled( false );

  clear();

  bool use14x21 = false;

  for( unsigned x = 0; x < dictionaries.size(); ++x )
  {
    QIcon icon = dictionaries[ x ]->getNativeIcon();

    QAction * action = addAction( icon,
                                  QString::fromUtf8(
                                      dictionaries[ x ]->getName().c_str() ) );

    QString id = QString::fromStdString( dictionaries[ x ]->getId() );

    action->setData( id );

    action->setCheckable( true );

    action->setChecked( !mutedDictionaries.contains( id ) );

    QList< QSize > sizes = icon.availableSizes();

    for( QList< QSize >::iterator i = sizes.begin(); i != sizes.end();
         ++i )
      if ( i->width() == 14 && i->height() == 21 )
        use14x21 = true;
  }

  setIconSize( use14x21 ? QSize( 14, 21 ) : QSize( 21, 21 ) );

  setUpdatesEnabled( true );
}


void DictionaryBar::dictionaryMuted( QString const & )
{
  printf( "Dictionary muted\n" );
}

void DictionaryBar::dictionaryUnmuted( QString const & )
{
  printf( "Dictionary unmuted\n" );
}

void DictionaryBar::actionWasTriggered( QAction * action )
{
  QString id = action->data().toString();

  if ( id.isEmpty() )
    return; // Some weird action, not our button

  if ( action->isChecked() )
    configEvents.unmuteDictionary( id );
  else
    configEvents.muteDictionary( id );
}
