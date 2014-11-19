/****************************************************************************
** Meta object code from reading C++ file 'dictheadwords.hh'
**
** Created: Mon Apr 28 13:37:42 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../dictheadwords.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dictheadwords.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DictHeadwords[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x05,
      41,   14,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
      55,   14,   14,   14, 0x08,
      65,   14,   14,   14, 0x08,
      89,   14,   14,   14, 0x08,
     105,   14,   14,   14, 0x08,
     127,   14,   14,   14, 0x08,
     151,  145,   14,   14, 0x08,
     182,  176,   14,   14, 0x08,
     209,   14,   14,   14, 0x08,
     231,   14,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_DictHeadwords[] = {
    "DictHeadwords\0\0headwordSelected(QString)\0"
    "closeDialog()\0savePos()\0filterChangedInternal()\0"
    "filterChanged()\0exportButtonClicked()\0"
    "okButtonClicked()\0index\0"
    "itemClicked(QModelIndex)\0state\0"
    "autoApplyStateChanged(int)\0"
    "showHeadwordsNumber()\0reject()\0"
};

void DictHeadwords::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DictHeadwords *_t = static_cast<DictHeadwords *>(_o);
        switch (_id) {
        case 0: _t->headwordSelected((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->closeDialog(); break;
        case 2: _t->savePos(); break;
        case 3: _t->filterChangedInternal(); break;
        case 4: _t->filterChanged(); break;
        case 5: _t->exportButtonClicked(); break;
        case 6: _t->okButtonClicked(); break;
        case 7: _t->itemClicked((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 8: _t->autoApplyStateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->showHeadwordsNumber(); break;
        case 10: _t->reject(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData DictHeadwords::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject DictHeadwords::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_DictHeadwords,
      qt_meta_data_DictHeadwords, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DictHeadwords::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DictHeadwords::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DictHeadwords::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DictHeadwords))
        return static_cast<void*>(const_cast< DictHeadwords*>(this));
    return QDialog::qt_metacast(_clname);
}

int DictHeadwords::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
void DictHeadwords::headwordSelected(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DictHeadwords::closeDialog()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
QT_END_MOC_NAMESPACE
