/****************************************************************************
** Meta object code from reading C++ file 'fulltextsearch.hh'
**
** Created: Mon Apr 28 13:37:44 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../fulltextsearch.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'fulltextsearch.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_FTS__Indexing[] = {

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
      15,   14,   14,   14, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_FTS__Indexing[] = {
    "FTS::Indexing\0\0sendNowIndexingName(QString)\0"
};

void FTS::Indexing::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Indexing *_t = static_cast<Indexing *>(_o);
        switch (_id) {
        case 0: _t->sendNowIndexingName((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData FTS::Indexing::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject FTS::Indexing::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_FTS__Indexing,
      qt_meta_data_FTS__Indexing, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &FTS::Indexing::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *FTS::Indexing::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *FTS::Indexing::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FTS__Indexing))
        return static_cast<void*>(const_cast< Indexing*>(this));
    if (!strcmp(_clname, "QRunnable"))
        return static_cast< QRunnable*>(const_cast< Indexing*>(this));
    return QObject::qt_metacast(_clname);
}

int FTS::Indexing::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
void FTS::Indexing::sendNowIndexingName(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_FTS__FtsIndexing[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      23,   18,   17,   17, 0x05,

 // slots: signature, parameters, type, tag, flags
      48,   18,   17,   17, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_FTS__FtsIndexing[] = {
    "FTS::FtsIndexing\0\0name\0newIndexingName(QString)\0"
    "setNowIndexedName(QString)\0"
};

void FTS::FtsIndexing::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        FtsIndexing *_t = static_cast<FtsIndexing *>(_o);
        switch (_id) {
        case 0: _t->newIndexingName((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->setNowIndexedName((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData FTS::FtsIndexing::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject FTS::FtsIndexing::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_FTS__FtsIndexing,
      qt_meta_data_FTS__FtsIndexing, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &FTS::FtsIndexing::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *FTS::FtsIndexing::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *FTS::FtsIndexing::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FTS__FtsIndexing))
        return static_cast<void*>(const_cast< FtsIndexing*>(this));
    return QObject::qt_metacast(_clname);
}

int FTS::FtsIndexing::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void FTS::FtsIndexing::newIndexingName(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_FTS__HeadwordsListModel[] = {

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
      25,   24,   24,   24, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_FTS__HeadwordsListModel[] = {
    "FTS::HeadwordsListModel\0\0contentChanged()\0"
};

void FTS::HeadwordsListModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        HeadwordsListModel *_t = static_cast<HeadwordsListModel *>(_o);
        switch (_id) {
        case 0: _t->contentChanged(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData FTS::HeadwordsListModel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject FTS::HeadwordsListModel::staticMetaObject = {
    { &QAbstractListModel::staticMetaObject, qt_meta_stringdata_FTS__HeadwordsListModel,
      qt_meta_data_FTS__HeadwordsListModel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &FTS::HeadwordsListModel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *FTS::HeadwordsListModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *FTS::HeadwordsListModel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FTS__HeadwordsListModel))
        return static_cast<void*>(const_cast< HeadwordsListModel*>(this));
    return QAbstractListModel::qt_metacast(_clname);
}

int FTS::HeadwordsListModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
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
void FTS::HeadwordsListModel::contentChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_FTS__FullTextSearchDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      49,   27,   26,   26, 0x05,
      97,   26,   26,   26, 0x05,

 // slots: signature, parameters, type, tag, flags
     111,   26,   26,   26, 0x08,
     139,   26,   26,   26, 0x08,
     150,   26,   26,   26, 0x08,
     159,   26,   26,   26, 0x08,
     176,   26,   26,   26, 0x08,
     196,   26,   26,   26, 0x08,
     209,  205,   26,   26, 0x08,
     234,   26,   26,   26, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_FTS__FullTextSearchDialog[] = {
    "FTS::FullTextSearchDialog\0\0"
    ",dictIDs,searchRegExp\0"
    "showTranslationFor(QString,QStringList,QRegExp)\0"
    "closeDialog()\0setNewIndexingName(QString)\0"
    "saveData()\0accept()\0setLimitsUsing()\0"
    "searchReqFinished()\0reject()\0idx\0"
    "itemClicked(QModelIndex)\0updateDictionaries()\0"
};

void FTS::FullTextSearchDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        FullTextSearchDialog *_t = static_cast<FullTextSearchDialog *>(_o);
        switch (_id) {
        case 0: _t->showTranslationFor((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QStringList(*)>(_a[2])),(*reinterpret_cast< const QRegExp(*)>(_a[3]))); break;
        case 1: _t->closeDialog(); break;
        case 2: _t->setNewIndexingName((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->saveData(); break;
        case 4: _t->accept(); break;
        case 5: _t->setLimitsUsing(); break;
        case 6: _t->searchReqFinished(); break;
        case 7: _t->reject(); break;
        case 8: _t->itemClicked((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 9: _t->updateDictionaries(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData FTS::FullTextSearchDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject FTS::FullTextSearchDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_FTS__FullTextSearchDialog,
      qt_meta_data_FTS__FullTextSearchDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &FTS::FullTextSearchDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *FTS::FullTextSearchDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *FTS::FullTextSearchDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FTS__FullTextSearchDialog))
        return static_cast<void*>(const_cast< FullTextSearchDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int FTS::FullTextSearchDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void FTS::FullTextSearchDialog::showTranslationFor(QString const & _t1, QStringList const & _t2, QRegExp const & _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void FTS::FullTextSearchDialog::closeDialog()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
QT_END_MOC_NAMESPACE
