/****************************************************************************
** Meta object code from reading C++ file 'article_maker.hh'
**
** Created: Mon Apr 28 13:36:49 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../article_maker.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'article_maker.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ArticleMaker[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_ArticleMaker[] = {
    "ArticleMaker\0"
};

void ArticleMaker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData ArticleMaker::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ArticleMaker::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ArticleMaker,
      qt_meta_data_ArticleMaker, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ArticleMaker::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ArticleMaker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ArticleMaker::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ArticleMaker))
        return static_cast<void*>(const_cast< ArticleMaker*>(this));
    return QObject::qt_metacast(_clname);
}

int ArticleMaker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_ArticleRequest[] = {

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
      16,   15,   15,   15, 0x08,
      36,   15,   15,   15, 0x08,
      51,   15,   15,   15, 0x08,
      75,   15,   15,   15, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ArticleRequest[] = {
    "ArticleRequest\0\0altSearchFinished()\0"
    "bodyFinished()\0stemmedSearchFinished()\0"
    "individualWordFinished()\0"
};

void ArticleRequest::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ArticleRequest *_t = static_cast<ArticleRequest *>(_o);
        switch (_id) {
        case 0: _t->altSearchFinished(); break;
        case 1: _t->bodyFinished(); break;
        case 2: _t->stemmedSearchFinished(); break;
        case 3: _t->individualWordFinished(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData ArticleRequest::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ArticleRequest::staticMetaObject = {
    { &Dictionary::DataRequest::staticMetaObject, qt_meta_stringdata_ArticleRequest,
      qt_meta_data_ArticleRequest, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ArticleRequest::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ArticleRequest::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ArticleRequest::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ArticleRequest))
        return static_cast<void*>(const_cast< ArticleRequest*>(this));
    typedef Dictionary::DataRequest QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int ArticleRequest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Dictionary::DataRequest QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
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
