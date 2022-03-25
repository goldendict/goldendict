#include "article_inspect.h"
#include <QCloseEvent>
#include <QWebEngineContextMenuRequest>
ArticleInspector::ArticleInspector( QWidget * parent ) : QDialog( parent, Qt::WindowType::Window )
{
  setAttribute( Qt::WidgetAttribute::WA_DeleteOnClose, false );
  QVBoxLayout * v = new QVBoxLayout( this );
  v->setSpacing( 0 );
  v->setContentsMargins( 0, 0, 0, 0 );
  inspectView = new QWebEngineView();
  v->addWidget( inspectView );
}

void ArticleInspector::setInspectPage( QWebEngineView * view )
{
  auto page=view->page();
  this->inspectedPage = page;
  page->setDevToolsPage( inspectView->page() );
  // without this line, application will crash on qt6.2 ,see https://bugreports.qt.io/browse/QTBUG-101724
  if( view->lastContextMenuRequest() )
  {
    page->triggerAction( QWebEnginePage::InspectElement );
  }
  raise();
  show();
}

void ArticleInspector::closeEvent( QCloseEvent * )
{
  inspectedPage->setDevToolsPage( nullptr );
}
