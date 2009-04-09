/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "scanpopup.hh"
#include "folding.hh"
#include "mouseover.hh"
#include <QUrl>
#include <QCursor>
#include <QPixmap>
#include <QBitmap>
#include <QMenu>
#include <QMouseEvent>
#include <QDesktopWidget>

using std::wstring;

ScanPopup::ScanPopup( QWidget * parent,
                      Config::Class & cfg_,
                      ArticleNetworkAccessManager & articleNetMgr,                      
                      std::vector< sptr< Dictionary::Class > > const & allDictionaries_,
                      Instances::Groups const & groups_ ):
  QDialog( parent ),
  cfg( cfg_ ),
  isScanningEnabled( false ),
  allDictionaries( allDictionaries_ ),
  groups( groups_ ),
  wordFinder( this ),
  mouseEnteredOnce( false )
{
  ui.setupUi( this );

  ui.queryError->hide();

  definition = new ArticleView( ui.outerFrame, articleNetMgr, groups, true ),
  ui.mainLayout->addWidget( definition );

  ui.diacriticButton->hide();
  ui.prefixButton->hide();

  ui.groupList->fill( groups );
  ui.groupList->setCurrentGroup( cfg.lastPopupGroup );

  setWindowFlags( Qt::Popup );

  if ( cfg.lastPopupSize.isValid() )
    resize( cfg.lastPopupSize );

  #ifdef Q_OS_WIN32
  // On Windows, leaveEvent() doesn't seem to work with popups and we're trying
  // to emulate it.
  setMouseTracking( true );
  #endif

  #if 0 // Experimental code to give window a non-rectangular shape (i.e.
        // balloon) using a colorkey mask.
  QPixmap pixMask( size() );
  render( &pixMask );

  setMask( pixMask.createMaskFromColor( QColor( 255, 0, 0 ) ) );

  // This helps against flickering
  setAttribute( Qt::WA_NoSystemBackground );
  #endif

  connect( ui.groupList, SIGNAL( currentIndexChanged( QString const & ) ),
           this, SLOT( currentGroupChanged( QString const & ) ) );

  connect( &wordFinder, SIGNAL( finished() ),
           this, SLOT( prefixMatchFinished() ) );

  connect( ui.word, SIGNAL( clicked() ),
           this, SLOT( initialWordClicked() ) );
  connect( ui.diacriticButton, SIGNAL( clicked() ),
           this, SLOT( diacriticButtonClicked() ) );
  connect( ui.prefixButton, SIGNAL( clicked() ),
           this, SLOT( prefixButtonClicked() ) );

  connect( ui.pinButton, SIGNAL( clicked( bool ) ),
           this, SLOT( pinButtonClicked( bool ) ) );

  connect( QApplication::clipboard(), SIGNAL( changed( QClipboard::Mode ) ),
           this, SLOT( clipboardChanged( QClipboard::Mode ) ) );

  connect( &MouseOver::instance(), SIGNAL( hovered( QString const & ) ),
           this, SLOT( mouseHovered( QString const & ) ) );
}

ScanPopup::~ScanPopup()
{
  disableScanning();
}

void ScanPopup::enableScanning()
{
  if ( !isScanningEnabled )
  {
    isScanningEnabled = true;
    MouseOver::instance().enableMouseOver();
  }
}

void ScanPopup::disableScanning()
{
  if ( isScanningEnabled )
  {
    MouseOver::instance().disableMouseOver();
    isScanningEnabled = false;
  }
}

void ScanPopup::clipboardChanged( QClipboard::Mode m )
{
  if ( !isScanningEnabled )
    return;
  
  printf( "clipboard changed\n" );

  QString subtype = "plain";

  handleInputWord( QApplication::clipboard()->text( subtype, m ) );
}

void ScanPopup::mouseHovered( QString const & str )
{
  handleInputWord( str );
}

void ScanPopup::handleInputWord( QString const & str )
{
  if ( !cfg.preferences.enableScanPopup )
    return;

  // Check key modifiers

  if ( cfg.preferences.enableScanPopupModifiers &&
       !checkModifiersPressed( cfg.preferences.scanPopupModifiers ) )
    return;

  inputWord = QString::fromStdWString( Folding::trimWhitespaceOrPunct( str.toStdWString() ) );

  if ( !inputWord.size() )
    return;

  /// Too large strings make window expand which is probably not what user
  /// wants
  ui.word->setText( elideInputWord() );

  if ( !isVisible() )
  {
    // Decide where should the window land
    
    QPoint currentPos = QCursor::pos();

    QRect desktop = QApplication::desktop()->screenGeometry();

    QSize windowSize = geometry().size();

    int x, y;

    /// Try the to-the-right placement
    if ( currentPos.x() + 4 + windowSize.width() <= desktop.topRight().x() )
      x = currentPos.x() + 4;
    else
    /// Try the to-the-left placement
    if ( currentPos.x() - 4 - windowSize.width() >= desktop.x() )
      x = currentPos.x() - 4 - windowSize.width();
    else
    // Center it
      x = desktop.x() + ( desktop.width() - windowSize.width() ) / 2;
    
    /// Try the to-the-buttom placement
    if ( currentPos.y() + 15 + windowSize.height() <= desktop.bottomLeft().y() )
      y = currentPos.y() + 15;
    else
    /// Try the to-the-top placement
    if ( currentPos.y() - 15 - windowSize.height() >= desktop.y() )
      y = currentPos.y() - 15 - windowSize.height();
    else
    // Center it
      y = desktop.y() + ( desktop.height() - windowSize.height() ) / 2;
      
    move( x, y );

    show();

    mouseEnteredOnce = false; // Windows-only

    // This produced some funky mouse grip-related bugs so we commented it out
    //QApplication::processEvents(); // Make window appear immediately no matter what
  }

  initiateTranslation();
}

QString ScanPopup::elideInputWord()
{
  return inputWord.size() > 32 ? inputWord.mid( 0, 32 ) + "..." : inputWord;
}


void ScanPopup::currentGroupChanged( QString const & gr )
{
  if ( isVisible() )
    initiateTranslation();

  cfg.lastPopupGroup = gr;
}

void ScanPopup::initiateTranslation()
{
  definition->showDefinition( inputWord, ui.groupList->currentText() );
  wordFinder.prefixMatch( inputWord, getActiveDicts() );
}

vector< sptr< Dictionary::Class > > const & ScanPopup::getActiveDicts()
{
  int currentGroup = ui.groupList->currentIndex();

  return
    currentGroup < 0 || currentGroup >= (int)groups.size() ? allDictionaries : 
    groups[ currentGroup ].dictionaries;
}

void ScanPopup::mousePressEvent( QMouseEvent * ev )
{
  startPos = ev->globalPos();
  setCursor( Qt::ClosedHandCursor );

  QDialog::mousePressEvent( ev );
}

void ScanPopup::mouseMoveEvent( QMouseEvent * event )
{
  if ( event->buttons() )
  {
    QPoint newPos = event->globalPos();

    QPoint delta = newPos - startPos;

    startPos = newPos;

    // Find a top-level window

    move( pos() + delta );
  }
  #ifdef Q_OS_WIN32
  else
  if ( !ui.pinButton->isChecked() )
  {
    if ( !mouseEnteredOnce )
    {
      // We're waiting for mouse to enter window
      if ( geometry().contains( event->globalPos() ) )
        mouseEnteredOnce = true;
    }
    else
    {
      // We're waiting for mouse to leave window
      if ( !geometry().contains( event->globalPos() ) )
      {
        mouseEnteredOnce = false;
        hide();
      }
    }
  }
  #endif

  QDialog::mouseMoveEvent( event );
}

void ScanPopup::mouseReleaseEvent( QMouseEvent * ev )
{
  unsetCursor();
  QDialog::mouseReleaseEvent( ev );
}

void ScanPopup::leaveEvent( QEvent * event )
{
  QDialog::leaveEvent( event );

  // We hide the popup when the mouse leaves it. So in order to close it
  // without any clicking the cursor has to get inside and then to leave.
  
  // Combo-boxes seem to generate leave events for their parents when
  // unfolded, so we check coordinates as well.
  // If the dialog is pinned, we don't hide the popup.
  // If some mouse buttons are pressed, we don't hide the popup either,
  // since it indicates the move operation is underway.
  if ( !ui.pinButton->isChecked() && !geometry().contains( QCursor::pos() ) &&
       QApplication::mouseButtons() == Qt::NoButton )
  {
    unsetCursor(); // Just in case
    hide();
  }
}

void ScanPopup::resizeEvent( QResizeEvent * event )
{
  cfg.lastPopupSize = event->size();

  QDialog::resizeEvent( event );
}

void ScanPopup::showEvent( QShowEvent * ev )
{
  QDialog::showEvent( ev );
  
  if ( groups.empty() )
    ui.groupList->hide();
}

void ScanPopup::prefixMatchFinished()
{
  // Check that there's a window there at all
  if ( isVisible() )
  {
    if ( wordFinder.getErrorString().size() )
    {
      ui.queryError->setToolTip( wordFinder.getErrorString() );
      ui.queryError->show();
    }
    else
      ui.queryError->hide();

    // Find the matches that aren't prefix. If there're more than one,
    // show the diacritic toolbutton. If there are prefix matches, show
    // the prefix toolbutton.

    diacriticMatches.clear();
    prefixMatches.clear();

    wstring foldedInputWord = Folding::apply( inputWord.toStdWString() );

    WordFinder::SearchResults const & results = wordFinder.getPrefixMatchResults();
    
    for( unsigned x = 0; x < results.size(); ++x )
    {
      if ( Folding::apply( results[ x ].first.toStdWString() ) == foldedInputWord )
        diacriticMatches.push_back( results[ x ].first );
      else
        prefixMatches.push_back( results[ x ].first );
    }

    if ( diacriticMatches.size() > 1 )
    {
      ui.diacriticButton->setToolTip( tr( "%1 results differing in diacritic marks" ).arg( diacriticMatches.size() ) );
      ui.diacriticButton->show();
    }
    else
      ui.diacriticButton->hide();

    if ( prefixMatches.size() )
    {
      ui.prefixButton->setToolTip( tr( "%1 result(s) beginning with the search word" ).arg( prefixMatches.size() ) );
      ui.prefixButton->show();
    }
    else
      ui.prefixButton->hide();
  }
}

void ScanPopup::diacriticButtonClicked()
{
  popupWordlist( diacriticMatches, ui.diacriticButton );
}

void ScanPopup::prefixButtonClicked()
{
  popupWordlist( prefixMatches, ui.prefixButton );
}

void ScanPopup::popupWordlist( vector< QString > const & words, QToolButton * button )
{
  if ( !isVisible() )
    return;

  if ( words.empty() )
    return;

  QMenu menu( this );

  for( unsigned x = 0; x < words.size(); ++x )
    menu.addAction( words[ x ] );

  QAction * result = menu.exec( mapToGlobal( button->pos() ) +
                                  QPoint( 0, button->height() ) );

  if ( result )
    definition->showDefinition( result->text(), ui.groupList->currentText() );
}

void ScanPopup::initialWordClicked()
{
  if ( isVisible() && diacriticMatches.size() )
    definition->showDefinition( diacriticMatches[ 0 ], ui.groupList->currentText() );
}

void ScanPopup::pinButtonClicked( bool checked )
{
  if ( checked )
  {
    setWindowFlags( Qt::Dialog );
    setWindowTitle( elideInputWord() );
  }
  else
    setWindowFlags( Qt::Popup );

  show();
}
