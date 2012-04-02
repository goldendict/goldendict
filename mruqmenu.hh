#ifndef MRUQMENU_HH
#define MRUQMENU_HH

#include <QMenu>
#include <QEvent>

//The only difference between this class and QMenu is that this class emits
//a signal when Ctrl button is released
class MRUQMenu: public QMenu
{
    Q_OBJECT

public:
    MRUQMenu(const QString title, QWidget *parent = 0);

private:
    bool eventFilter (QObject*, QEvent*);

    signals:
    void ctrlReleased();
};



#endif // MRUQMENU_HH
