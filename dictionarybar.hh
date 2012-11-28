#ifndef __DICTIONARYBAR_HH_INCLUDED__
#define __DICTIONARYBAR_HH_INCLUDED__

#include <QToolBar>
#include <QSize>
#include <QList>
#include <QString>
#include "dictionary.hh"
#include "config.hh"

/// A bar containing dictionary icons of the currently chosen group.
/// Individual dictionaries can be toggled on and off.
class DictionaryBar: public QToolBar
{
  Q_OBJECT

public:

  /// Constructs an empty dictionary bar
  DictionaryBar( QWidget * parent,
                 Config::Events &, QString const & _editDictionaryCommand );

  /// Sets dictionaries to be displayed in the bar. Their statuses (enabled/
  /// disabled) are taken from the configuration data.
  void setDictionaries( std::vector< sptr< Dictionary::Class > > const & );
  void setMutedDictionaries( Config::MutedDictionaries * mutedDictionaries_ )
  { mutedDictionaries = mutedDictionaries_; }
  Config::MutedDictionaries const * getMutedDictionaries() const
  { return mutedDictionaries; }
  void setDictionaryIconSize( int extent );

signals:

  /// Signalled when the user decided to edit group the bar currently
  /// shows.
  void editGroupRequested();

  /// Signal for show dictionary info command from context menu
  void showDictionaryInfo( QString const & id );

  /// Signal to close context menu
  void closePopupMenu();

private:

  Config::MutedDictionaries * mutedDictionaries;
  Config::Events & configEvents;
  Config::MutedDictionaries storedMutedSet;
  QString editDictionaryCommand;
  std::vector< sptr< Dictionary::Class > > allDictionaries;
  /// All the actions we have added to the toolbar
  QList< QAction * > dictActions;

  bool use14x21;

protected:

  void contextMenuEvent( QContextMenuEvent * event );

private slots:

  void mutedDictionariesChanged();

  void actionWasTriggered( QAction * );

  void dictsPaneClicked( QString const & );
};

#endif
