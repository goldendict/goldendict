#ifndef __DICTIONARYBAR_HH_INCLUDED__
#define __DICTIONARYBAR_HH_INCLUDED__

#include <QToolBar>
#include <QSize>
#include <QList>
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
                 Config::MutedDictionaries & mutedDictionaries,
                 Config::Events & );

  /// Sets dictionaries to be displayed in the bar. Their statuses (enabled/
  /// disabled) are taken from the configuration data.
  void setDictionaries( std::vector< sptr< Dictionary::Class > > const & );

signals:

  /// Signalled when the user decided to edit group the bar currently
  /// shows.
  void editGroupRequested();

private:

  Config::MutedDictionaries & mutedDictionaries;
  Config::Events & configEvents;

  /// All the actions we have added to the toolbar
  QList< QAction * > dictActions;

protected:

  void contextMenuEvent( QContextMenuEvent * event );

private slots:

  void mutedDictionariesChanged();

  void actionWasTriggered( QAction * );

  void dictsPaneClicked( QString const & );
};

#endif
