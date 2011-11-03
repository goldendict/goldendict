#include "customqmenu.hh"
#include <QKeyEvent>

customQMenu::customQMenu(const QString title, QWidget *parent):
        QMenu(title,parent)
{
    installEventFilter(this);
}

bool customQMenu::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease){
        QKeyEvent *keyevent = static_cast<QKeyEvent*>(event);
        if (keyevent->key() == Qt::Key_Control){
            emit control_released();
            return true;
        }
    }
    return false;
}
