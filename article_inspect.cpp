#include "article_inspect.h"
#include <QCloseEvent>
article_inspect::article_inspect( QWidget * parent ) : QDialog( parent, Qt::WindowType::Window )
{
  setAttribute( Qt::WidgetAttribute::WA_DeleteOnClose, false );
  QVBoxLayout * v = new QVBoxLayout( this );
  v->setSpacing( 0 );
  v->setContentsMargins( 0, 0, 0, 0 );
  inspectView = new QWebEngineView();
  v->addWidget( inspectView );
}

void article_inspect::setInspectPage( QWebEnginePage * page )
{
  this->inspectedPage = page;
  page->setDevToolsPage( inspectView->page() );
  page->triggerAction( QWebEnginePage::InspectElement );
  raise();
  show();
}

void article_inspect::closeEvent( QCloseEvent * ev )
{
  inspectedPage->setDevToolsPage( nullptr );
}
