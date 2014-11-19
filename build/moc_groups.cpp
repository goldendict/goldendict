/****************************************************************************
** Meta object code from reading C++ file 'groups.hh'
**
** Created: Mon Apr 28 13:36:44 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../groups.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'groups.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Groups[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      11,    8,    7,    7, 0x05,

 // slots: signature, parameters, type, tag, flags
      39,    7,    7,    7, 0x08,
      48,    7,    7,    7, 0x08,
      64,    7,    7,    7, 0x08,
      80,    7,    7,    7, 0x08,
      92,    7,    7,    7, 0x08,
     105,    7,    7,    7, 0x08,
     123,    7,    7,    7, 0x08,
     143,  139,    7,    7, 0x08,
     164,    7,    7,    7, 0x08,
     185,  181,    7,    7, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Groups[] = {
    "Groups\0\0id\0showDictionaryInfo(QString)\0"
    "addNew()\0renameCurrent()\0removeCurrent()\0"
    "removeAll()\0addToGroup()\0removeFromGroup()\0"
    "addAutoGroups()\0pos\0showDictInfo(QPoint)\0"
    "fillGroupsMenu()\0act\0switchToGroup(QAction*)\0"
};

void Groups::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Groups *_t = static_cast<Groups *>(_o);
        switch (_id) {
        case 0: _t->showDictionaryInfo((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->addNew(); break;
        case 2: _t->renameCurrent(); break;
        case 3: _t->removeCurrent(); break;
        case 4: _t->removeAll(); break;
        case 5: _t->addToGroup(); break;
        case 6: _t->removeFromGroup(); break;
        case 7: _t->addAutoGroups(); break;
        case 8: _t->showDictInfo((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 9: _t->fillGroupsMenu(); break;
        case 10: _t->switchToGroup((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Groups::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Groups::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Groups,
      qt_meta_data_Groups, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Groups::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Groups::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Groups::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Groups))
        return static_cast<void*>(const_cast< Groups*>(this));
    return QWidget::qt_metacast(_clname);
}

int Groups::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void Groups::showDictionaryInfo(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
