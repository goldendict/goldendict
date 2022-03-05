#include "article_inspect.h"
#include <QCloseEvent>
ArticleInspector::ArticleInspector( QWidget * parent ) : QDialog( parent, Qt::WindowType::Window )
{
  setAttribute( Qt::WidgetAttribute::WA_DeleteOnClose, false );
  QVBoxLayout * v = new QVBoxLayout( this );
  v->setSpacing( 0 );
  v->setContentsMargins( 0, 0, 0, 0 );
  inspectView = new QWebEngineView();
  v->addWidget( inspectView );
}

void ArticleInspector::setInspectPage( QWebEnginePage * page )
{
  this->inspectedPage = page;
  page->setDevToolsPage( inspectView->page() );
  page->triggerAction( QWebEnginePage::InspectElement );
  raise();
  show();
}

void ArticleInspector::closeEvent( QCloseEvent * ev )
{
  inspectedPage->setDevToolsPage( nullptr );
}
