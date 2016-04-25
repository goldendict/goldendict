/****************************************************************************
** Meta object code from reading C++ file 'translatebox.hh'
**
** Created: Wed Nov 19 16:07:01 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../translatebox.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'translatebox.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_CompletionList[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      21,   15,   16,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_CompletionList[] = {
    "CompletionList\0\0bool\0acceptCurrentEntry()\0"
};

void CompletionList::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CompletionList *_t = static_cast<CompletionList *>(_o);
        switch (_id) {
        case 0: { bool _r = _t->acceptCurrentEntry();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        default: ;
        }
    }
}

const QMetaObjectExtraData CompletionList::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject CompletionList::staticMetaObject = {
    { &WordList::staticMetaObject, qt_meta_stringdata_CompletionList,
      qt_meta_data_CompletionList, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CompletionList::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CompletionList::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CompletionList::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CompletionList))
        return static_cast<void*>(const_cast< CompletionList*>(this));
    return WordList::qt_metacast(_clname);
}

int CompletionList::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = WordList::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_TranslateBox[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      21,   14,   13,   13, 0x0a,
      43,   13,   13,   13, 0x08,
      55,   13,   13,   13, 0x08,
      76,   13,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_TranslateBox[] = {
    "TranslateBox\0\0enable\0setPopupEnabled(bool)\0"
    "showPopup()\0rightButtonClicked()\0"
    "onTextEdit()\0"
};

void TranslateBox::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        TranslateBox *_t = static_cast<TranslateBox *>(_o);
        switch (_id) {
        case 0: _t->setPopupEnabled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->showPopup(); break;
        case 2: _t->rightButtonClicked(); break;
        case 3: _t->onTextEdit(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData TranslateBox::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject TranslateBox::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_TranslateBox,
      qt_meta_data_TranslateBox, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &TranslateBox::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *TranslateBox::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *TranslateBox::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TranslateBox))
        return static_cast<void*>(const_cast< TranslateBox*>(this));
    return QWidget::qt_metacast(_clname);
}

int TranslateBox::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
