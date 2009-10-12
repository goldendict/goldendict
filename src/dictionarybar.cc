#include "dictionarybar.hh"
#include <QAction>
#include <QApplication>

using std::vector;

DictionaryBar::DictionaryBar( QWidget * parent,
                              Config::MutedDictionaries & mutedDictionaries_,
                              Config::Events & events ):
  QToolBar( tr( "Dictionary Bar" ), parent ),
  mutedDictionaries( mutedDictionaries_ ),
  configEvents( events )
{
  setObjectName( "dictionaryBar" );

  connect( &events, SIGNAL( mutedDictionariesChanged() ),
           this, SLOT( mutedDictionariesChanged() ) );

  connect( this, SIGNAL(actionTriggered(QAction*)),
           this, SLOT(actionWasTriggered(QAction*)) );
}

static QString elideDictName( QString const & name )
{
  // Some names are way too long -- we insert an ellipsis in the middle of those

  int const maxSize = 33;

  if ( name.size() <= maxSize )
    return name;

  int const pieceSize = maxSize / 2 - 1;

  return name.left( pieceSize ) + QChar( 0x2026 ) + name.right( pieceSize );
}

void DictionaryBar::setDictionaries( vector< sptr< Dictionary::Class > >
                                     const & dictionaries )
{
  setUpdatesEnabled( false );

  clear();
  dictActions.clear();

  bool use14x21 = false;

  for( unsigned x = 0; x < dictionaries.size(); ++x )
  {
    QIcon icon = dictionaries[ x ]->getNativeIcon();

    QString dictName = QString::fromUtf8( dictionaries[ x ]->
                                            getName().c_str() );

    QAction * action = addAction( icon, elideDictName( dictName ) );

    action->setToolTip( dictName ); // Tooltip need not be shortened

    QString id = QString::fromStdString( dictionaries[ x ]->getId() );

    action->setData( id );

    action->setCheckable( true );

    action->setChecked( !mutedDictionaries.contains( id ) );

    QList< QSize > sizes = icon.availableSizes();

    for( QList< QSize >::iterator i = sizes.begin(); i != sizes.end();
         ++i )
      if ( i->width() == 14 && i->height() == 21 )
        use14x21 = true;

    dictActions.append( action );
  }

  setIconSize( use14x21 ? QSize( 14, 21 ) : QSize( 21, 21 ) );

  setUpdatesEnabled( true );
}


void DictionaryBar::mutedDictionariesChanged()
{
//  printf( "Muted dictionaries changed\n" );

  // Update actions

  setUpdatesEnabled( false );

  for( QList< QAction * >::iterator i = dictActions.begin();
       i != dictActions.end(); ++i )
  {
    bool isUnmuted = !mutedDictionaries.contains( (*i)->data().toString() );

    if ( isUnmuted != (*i)->isChecked() )
      (*i)->setChecked( isUnmuted );
  }

  setUpdatesEnabled( true );
}

void DictionaryBar::actionWasTriggered( QAction * action )
{
  QString id = action->data().toString();

  if ( id.isEmpty() )
    return; // Some weird action, not our button

  if ( QApplication::keyboardModifiers() &
         ( Qt::ControlModifier | Qt::ShiftModifier ) )
  {
    // Solo mode -- either use the dictionary exclusively, or toggle
    // back all dictionaries if we do that already.

    // Are we solo already?

    bool isSolo = true;

    // For solo, all dictionaries must be unchecked, since we're handling
    // the result of the dictionary being (un)checked, and in case we were
    // in solo, now we would end up with no dictionaries being checked at all.
    for( QList< QAction * >::iterator i = dictActions.begin();
         i != dictActions.end(); ++i )
    {
      if ( (*i)->isChecked() )
      {
        isSolo = false;
        break;
      }
    }

    if ( isSolo )
    {
      // Toggle back all the dictionaries
      for( QList< QAction * >::iterator i = dictActions.begin();
           i != dictActions.end(); ++i )
        mutedDictionaries.remove( (*i)->data().toString() );
    }
    else
    {
      // Make dictionary solo
      for( QList< QAction * >::iterator i = dictActions.begin();
           i != dictActions.end(); ++i )
      {
        QString dictId = (*i)->data().toString();

        if ( dictId == id )
          mutedDictionaries.remove( dictId );
        else
          mutedDictionaries.insert( dictId );
      }
    }
    configEvents.signalMutedDictionariesChanged();
  }
  else
  {
    // Normal mode

    if ( action->isChecked() )
    {
      // Unmute the dictionary

      if ( mutedDictionaries.contains( id ) )
      {
        mutedDictionaries.remove( id );
        configEvents.signalMutedDictionariesChanged();
      }
    }
    else
    {
      // Mute the dictionary

      if ( !mutedDictionaries.contains( id ) )
      {
        mutedDictionaries.insert( id );
        configEvents.signalMutedDictionariesChanged();
      }
    }
  }
}
