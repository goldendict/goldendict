/****************************************************************************
** Meta object code from reading C++ file 'extlineedit.hh'
**
** Created: Mon Apr 28 13:37:34 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../extlineedit.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'extlineedit.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_IconButton[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       2,   14, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
      19,   11, 0x41095103,
      32,   26, 0x87095103,

       0        // eod
};

static const char qt_meta_stringdata_IconButton[] = {
    "IconButton\0QPixmap\0pixmap\0float\0opacity\0"
};

void IconButton::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData IconButton::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject IconButton::staticMetaObject = {
    { &QAbstractButton::staticMetaObject, qt_meta_stringdata_IconButton,
      qt_meta_data_IconButton, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &IconButton::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *IconButton::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *IconButton::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_IconButton))
        return static_cast<void*>(const_cast< IconButton*>(this));
    return QAbstractButton::qt_metacast(_clname);
}

int IconButton::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractButton::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QPixmap*>(_v) = pixmap(); break;
        case 1: *reinterpret_cast< float*>(_v) = opacity(); break;
        }
        _id -= 2;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: setPixmap(*reinterpret_cast< QPixmap*>(_v)); break;
        case 1: setOpacity(*reinterpret_cast< float*>(_v)); break;
        }
        _id -= 2;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 2;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}
static const uint qt_meta_data_ExtLineEdit[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       1,   34, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x05,
      33,   12,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
      54,   12,   12,   12, 0x08,
      73,   68,   12,   12, 0x08,

 // enums: name, flags, count, data
      96, 0x0,    2,   38,

 // enum data: key, value
     101, uint(ExtLineEdit::Left),
     106, uint(ExtLineEdit::Right),

       0        // eod
};

static const char qt_meta_stringdata_ExtLineEdit[] = {
    "ExtLineEdit\0\0leftButtonClicked()\0"
    "rightButtonClicked()\0iconClicked()\0"
    "text\0updateButtons(QString)\0Side\0Left\0"
    "Right\0"
};

void ExtLineEdit::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ExtLineEdit *_t = static_cast<ExtLineEdit *>(_o);
        switch (_id) {
        case 0: _t->leftButtonClicked(); break;
        case 1: _t->rightButtonClicked(); break;
        case 2: _t->iconClicked(); break;
        case 3: _t->updateButtons((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ExtLineEdit::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ExtLineEdit::staticMetaObject = {
    { &QLineEdit::staticMetaObject, qt_meta_stringdata_ExtLineEdit,
      qt_meta_data_ExtLineEdit, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ExtLineEdit::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ExtLineEdit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ExtLineEdit::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ExtLineEdit))
        return static_cast<void*>(const_cast< ExtLineEdit*>(this));
    return QLineEdit::qt_metacast(_clname);
}

int ExtLineEdit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QLineEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void ExtLineEdit::leftButtonClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void ExtLineEdit::rightButtonClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
QT_END_MOC_NAMESPACE
