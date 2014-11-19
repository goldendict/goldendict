/****************************************************************************
** Meta object code from reading C++ file 'dictinfo.hh'
**
** Created: Wed Nov 19 16:06:57 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../dictinfo.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dictinfo.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DictInfo[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x08,
      23,    9,    9,    9, 0x08,
      51,    9,    9,    9, 0x08,
      75,    9,    9,    9, 0x08,
      97,    9,    9,    9, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_DictInfo[] = {
    "DictInfo\0\0savePos(int)\0"
    "on_editDictionary_clicked()\0"
    "on_openFolder_clicked()\0on_OKButton_clicked()\0"
    "on_headwordsButton_clicked()\0"
};

void DictInfo::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DictInfo *_t = static_cast<DictInfo *>(_o);
        switch (_id) {
        case 0: _t->savePos((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->on_editDictionary_clicked(); break;
        case 2: _t->on_openFolder_clicked(); break;
        case 3: _t->on_OKButton_clicked(); break;
        case 4: _t->on_headwordsButton_clicked(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData DictInfo::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject DictInfo::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_DictInfo,
      qt_meta_data_DictInfo, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DictInfo::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DictInfo::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DictInfo::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DictInfo))
        return static_cast<void*>(const_cast< DictInfo*>(this));
    return QDialog::qt_metacast(_clname);
}

int DictInfo::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
