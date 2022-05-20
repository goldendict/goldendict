#include "article_inspect.h"
#include <QCloseEvent>
#if (QT_VERSION > QT_VERSION_CHECK(6,0,0))
#include <QWebEngineContextMenuRequest>
#endif
ArticleInspector::ArticleInspector( QWidget * parent ) : QWidget( parent, Qt::WindowType::Window )
{
  setWindowTitle(tr("Inspect"));
  setAttribute( Qt::WidgetAttribute::WA_DeleteOnClose, false );
  QVBoxLayout * v = new QVBoxLayout( this );
  v->setSpacing( 0 );
  v->setContentsMargins( 0, 0, 0, 0 );
  inspectView = new QWebEngineView( this );
  v->addWidget( inspectView );

  resize(800,600);
}

void ArticleInspector::setInspectPage( QWebEngineView * view )
{
  auto page=view->page();
  this->inspectedPage = page;
  page->setDevToolsPage( inspectView->page() );
#if( QT_VERSION > QT_VERSION_CHECK( 6, 0, 0 ) )
  // without this line, application will crash on qt6.2 ,see https://bugreports.qt.io/browse/QTBUG-101724
  if( view->lastContextMenuRequest() )
  {
    page->triggerAction( QWebEnginePage::InspectElement );
  }
#else
  page->triggerAction( QWebEnginePage::InspectElement );
#endif
  raise();
  show();
}

void ArticleInspector::closeEvent( QCloseEvent * )
{
  inspectedPage->setDevToolsPage( nullptr );
}
