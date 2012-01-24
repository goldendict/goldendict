#ifndef CUSTOMQMENU_HH
#define CUSTOMQMENU_HH

#include <QMenu>
#include <QEvent>

class customQMenu: public QMenu
{
    Q_OBJECT

public:
    customQMenu(const QString title, QWidget *parent = 0);

private:
    bool eventFilter (QObject*, QEvent*);

    signals:
    void control_released();
};



#endif // CUSTOMQMENU_HH
