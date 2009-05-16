/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "editdictionaries.hh"
#include "loaddictionaries.hh"
#include <QMessageBox>

using std::vector;

EditDictionaries::EditDictionaries( QWidget * parent, Config::Class & cfg_,
                                    vector< sptr< Dictionary::Class > > & dictionaries_,
                                    Instances::Groups & groupInstances_,
                                    QNetworkAccessManager & dictNetMgr_ ):
  QDialog( parent ), cfg( cfg_ ), dictionaries( dictionaries_ ),
  groupInstances( groupInstances_ ),
  dictNetMgr( dictNetMgr_ ),
  origCfg( cfg ),
  sources( this, cfg.paths, cfg.soundDirs, cfg.hunspell, cfg.transliteration,
           cfg.mediawikis, cfg.webSites ),
  groups( new Groups( this, dictionaries, cfg.groups ) ),
  dictionariesChanged( false ),
  groupsChanged( false ),
  lastCurrentTab( 0 )
{
  ui.setupUi( this );

  setWindowIcon( QIcon(":/icons/book.png") );

  ui.tabs->clear();

  ui.tabs->addTab( &sources, QIcon(":/icons/book.png"), tr( "&Sources" ) );
  ui.tabs->addTab( groups.get(), QIcon(":/icons/bookcase.png"), tr( "&Groups" ) );

  connect( &sources, SIGNAL( rescan() ), this, SLOT( rescanSources() ) );
}


void EditDictionaries::accept()
{
  Config::Groups newGroups = groups->getGroups();

  if ( isSourcesChanged() )
    acceptChangedSources( false );

  if ( origCfg.groups != newGroups )
  {
    groupsChanged = true;
    cfg.groups = newGroups;
  }

  QDialog::accept();
}

void EditDictionaries::on_tabs_currentChanged( int index )
{
  if ( index == -1 || !isVisible() )
    return; // Sent upon the construction/destruction

  if ( !lastCurrentTab && index )
  {
    // We're switching away from the Sources tab -- if its contents were
    // changed, we need to either apply or reject now.

    if ( isSourcesChanged() )
    {
      ui.tabs->setCurrentIndex( 0 );

      QMessageBox question( QMessageBox::Question, tr( "Sources changed" ),
                            tr( "Some sources were changed. Would you like to accept the changes?" ),
                            QMessageBox::NoButton, this );

      QPushButton * accept = question.addButton( tr( "Accept" ), QMessageBox::AcceptRole );

      question.addButton( tr( "Cancel" ), QMessageBox::RejectRole );

      question.exec();

      if ( question.clickedButton() == accept )
      {
        acceptChangedSources( true );
        
        lastCurrentTab = index;
        ui.tabs->setCurrentIndex( index );
      }
      else
      {
        // Prevent tab from switching
        lastCurrentTab = 0;
        return;
      }
    }
  }

  lastCurrentTab = index;
}

void EditDictionaries::rescanSources()
{
  acceptChangedSources( true );
}

bool EditDictionaries::isSourcesChanged() const
{
  return sources.getPaths() != cfg.paths ||
         sources.getSoundDirs() != cfg.soundDirs ||
         sources.getHunspell() != cfg.hunspell ||
         sources.getTransliteration() != cfg.transliteration ||
         sources.getMediaWikis() != cfg.mediawikis ||
         sources.getWebSites() != cfg.webSites;
}

void EditDictionaries::acceptChangedSources( bool rebuildGroups )
{
  dictionariesChanged = true;

  Config::Groups savedGroups = groups->getGroups();

  cfg.paths = sources.getPaths();
  cfg.soundDirs = sources.getSoundDirs();
  cfg.hunspell = sources.getHunspell();
  cfg.transliteration = sources.getTransliteration();
  cfg.mediawikis = sources.getMediaWikis();
  cfg.webSites = sources.getWebSites();

  groupInstances.clear(); // Those hold pointers to dictionaries, we need to
                          // free them.

  ui.tabs->setUpdatesEnabled( false );
  ui.tabs->removeTab( 1 );
  groups.reset();
  
  loadDictionaries( this, true, cfg, dictionaries, dictNetMgr );

  if ( rebuildGroups )
  {
    groups = new Groups( this, dictionaries, savedGroups );
    ui.tabs->insertTab( 1, groups.get(), QIcon(":/icons/bookcase.png"), tr( "&Groups" ) );
    ui.tabs->setUpdatesEnabled( true );
  }
}

