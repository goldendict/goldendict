/****************************************************************************
** Meta object code from reading C++ file 'mruqmenu.hh'
**
** Created: Mon Apr 28 13:37:31 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../mruqmenu.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mruqmenu.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MRUQMenu[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_MRUQMenu[] = {
    "MRUQMenu\0\0ctrlReleased()\0"
};

void MRUQMenu::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MRUQMenu *_t = static_cast<MRUQMenu *>(_o);
        switch (_id) {
        case 0: _t->ctrlReleased(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData MRUQMenu::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MRUQMenu::staticMetaObject = {
    { &QMenu::staticMetaObject, qt_meta_stringdata_MRUQMenu,
      qt_meta_data_MRUQMenu, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MRUQMenu::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MRUQMenu::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MRUQMenu::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MRUQMenu))
        return static_cast<void*>(const_cast< MRUQMenu*>(this));
    return QMenu::qt_metacast(_clname);
}

int MRUQMenu::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMenu::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void MRUQMenu::ctrlReleased()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
