/****************************************************************************
** Meta object code from reading C++ file 'preferences.hh'
**
** Created: Wed Nov 19 16:48:40 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../preferences.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'preferences.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Preferences[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x08,
      42,   12,   12,   12, 0x08,
      80,   12,   12,   12, 0x08,
     102,   12,   12,   12, 0x08,
     125,   12,   12,   12, 0x08,
     149,   12,   12,   12, 0x08,
     170,   12,   12,   12, 0x08,
     192,   12,   12,   12, 0x08,
     223,  215,   12,   12, 0x08,
     263,  215,   12,   12, 0x08,
     302,   12,   12,   12, 0x08,
     334,  326,   12,   12, 0x08,
     369,   12,   12,   12, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Preferences[] = {
    "Preferences\0\0enableScanPopupToggled(bool)\0"
    "enableScanPopupModifiersToggled(bool)\0"
    "wholeAltClicked(bool)\0wholeCtrlClicked(bool)\0"
    "wholeShiftClicked(bool)\0sideAltClicked(bool)\0"
    "sideCtrlClicked(bool)\0sideShiftClicked(bool)\0"
    "checked\0on_enableMainWindowHotkey_toggled(bool)\0"
    "on_enableClipboardHotkey_toggled(bool)\0"
    "on_buttonBox_accepted()\0enabled\0"
    "on_useExternalPlayer_toggled(bool)\0"
    "customProxyToggled(bool)\0"
};

void Preferences::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Preferences *_t = static_cast<Preferences *>(_o);
        switch (_id) {
        case 0: _t->enableScanPopupToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->enableScanPopupModifiersToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->wholeAltClicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->wholeCtrlClicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->wholeShiftClicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->sideAltClicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->sideCtrlClicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: _t->sideShiftClicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->on_enableMainWindowHotkey_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->on_enableClipboardHotkey_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 10: _t->on_buttonBox_accepted(); break;
        case 11: _t->on_useExternalPlayer_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 12: _t->customProxyToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Preferences::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Preferences::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_Preferences,
      qt_meta_data_Preferences, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Preferences::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Preferences::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Preferences::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Preferences))
        return static_cast<void*>(const_cast< Preferences*>(this));
    return QDialog::qt_metacast(_clname);
}

int Preferences::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
