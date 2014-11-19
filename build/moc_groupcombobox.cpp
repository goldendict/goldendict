/****************************************************************************
** Meta object code from reading C++ file 'groupcombobox.hh'
**
** Created: Mon Apr 28 13:36:59 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../groupcombobox.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'groupcombobox.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_GroupComboBox[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x08,
      29,   14,   14,   14, 0x08,
      47,   14,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_GroupComboBox[] = {
    "GroupComboBox\0\0popupGroups()\0"
    "selectNextGroup()\0selectPreviousGroup()\0"
};

void GroupComboBox::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        GroupComboBox *_t = static_cast<GroupComboBox *>(_o);
        switch (_id) {
        case 0: _t->popupGroups(); break;
        case 1: _t->selectNextGroup(); break;
        case 2: _t->selectPreviousGroup(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData GroupComboBox::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject GroupComboBox::staticMetaObject = {
    { &QComboBox::staticMetaObject, qt_meta_stringdata_GroupComboBox,
      qt_meta_data_GroupComboBox, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GroupComboBox::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GroupComboBox::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GroupComboBox::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GroupComboBox))
        return static_cast<void*>(const_cast< GroupComboBox*>(this));
    return QComboBox::qt_metacast(_clname);
}

int GroupComboBox::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QComboBox::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
