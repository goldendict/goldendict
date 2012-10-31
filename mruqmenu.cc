#include "mruqmenu.hh"
#include <QKeyEvent>

MRUQMenu::MRUQMenu(const QString title, QWidget *parent):
        QMenu(title,parent)
{
    installEventFilter(this);
}

bool MRUQMenu::eventFilter(QObject *obj, QEvent *event)
{
    (void) obj;
    if (event->type() == QEvent::KeyRelease){
        QKeyEvent *keyevent = static_cast<QKeyEvent*>(event);
        if (keyevent->key() == Qt::Key_Control){
	    emit ctrlReleased();
            return true;
        }
    }
    return false;
}
