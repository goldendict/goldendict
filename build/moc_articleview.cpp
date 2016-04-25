/****************************************************************************
** Meta object code from reading C++ file 'articleview.hh'
**
** Created: Wed Nov 19 16:06:17 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../articleview.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'articleview.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ArticleView[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      51,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      19,       // signalCount

 // signals: signature, parameters, type, tag, flags
      19,   13,   12,   12, 0x05,
      58,   51,   12,   12, 0x05,
      93,   12,   12,   12, 0x05,
     149,  118,   12,   12, 0x05,
     239,  207,   12,   12, 0x05,
     311,  306,   12,   12, 0x05,
     343,  338,   12,   12, 0x05,
     387,  364,   12,   12, 0x05,
     441,  425,   12,   12, 0x25,
     479,  471,   12,   12, 0x25,
     505,   12,   12,   12, 0x05,
     525,  521,   12,   12, 0x05,
     574,  306,   12,   12, 0x05,
     605,   12,   12,   12, 0x05,
     629,  622,   12,   12, 0x05,
     649,  306,   12,   12, 0x05,
     678,   12,   12,   12, 0x05,
     709,   12,   12,   12, 0x05,
     718,   12,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
     728,   12,   12,   12, 0x0a,
     735,   12,   12,   12, 0x0a,
     745,   12,   12,   12, 0x0a,
     773,   12,   12,   12, 0x0a,
     800,  797,   12,   12, 0x0a,
     846,  839,  834,   12, 0x0a,
     873,  622,   12,   12, 0x0a,
     906,   12,   12,   12, 0x0a,
     934,   12,   12,   12, 0x0a,
     960,  957,   12,   12, 0x08,
     985,  979,   12,   12, 0x08,
    1017, 1013,   12,   12, 0x08,
    1040,   12,   12,   12, 0x08,
    1061,   12,   12,   12, 0x08,
    1102, 1079,   12,   12, 0x08,
    1139,   12,   12,   12, 0x08,
    1168,   12,   12,   12, 0x08,
    1195,   12,   12,   12, 0x08,
    1212,   12,   12,   12, 0x08,
    1231,   12,   12,   12, 0x08,
    1252,   12,   12,   12, 0x08,
    1265,   12,   12,   12, 0x08,
    1292,   12,   12,   12, 0x08,
    1322,   12,   12,   12, 0x08,
    1353,   12,   12,   12, 0x08,
    1386,   12,   12,   12, 0x08,
    1418,   12,   12,   12, 0x08,
    1449,   12,   12,   12, 0x08,
    1476,   12,   12,   12, 0x08,
    1492,  471,   12,   12, 0x08,
    1518,   12,   12,   12, 0x08,
    1531,   12,   12,   12, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ArticleView[] = {
    "ArticleView\0\0,icon\0iconChanged(ArticleView*,QIcon)\0"
    ",title\0titleChanged(ArticleView*,QString)\0"
    "pageLoaded(ArticleView*)\0"
    ",referrer,fromArticle,contexts\0"
    "openLinkInNewTab(QUrl,QUrl,QString,ArticleView::Contexts)\0"
    "word,group,fromArticle,contexts\0"
    "showDefinitionInNewTab(QString,uint,QString,ArticleView::Contexts)\0"
    "word\0sendWordToHistory(QString)\0text\0"
    "typingEvent(QString)\0message,timeout,pixmap\0"
    "statusBarMessage(QString,int,QPixmap)\0"
    "message,timeout\0statusBarMessage(QString,int)\0"
    "message\0statusBarMessage(QString)\0"
    "showDictsPane()\0,id\0"
    "activeArticleChanged(const ArticleView*,QString)\0"
    "forceAddWordToHistory(QString)\0"
    "closePopupMenu()\0expand\0setExpandMode(bool)\0"
    "sendWordToInputLine(QString)\0"
    "storeResourceSavePath(QString)\0zoomIn()\0"
    "zoomOut()\0back()\0forward()\0"
    "on_searchPrevious_clicked()\0"
    "on_searchNext_clicked()\0id\0"
    "onJsActiveArticleChanged(QString)\0"
    "bool\0obj,ev\0handleF3(QObject*,QEvent*)\0"
    "receiveExpandOptionalParts(bool)\0"
    "switchExpandOptionalParts()\0"
    "selectCurrentArticle()\0ok\0loadFinished(bool)\0"
    "title\0handleTitleChanged(QString)\0url\0"
    "handleUrlChanged(QUrl)\0attachToJavaScript()\0"
    "linkClicked(QUrl)\0link,title,textContent\0"
    "linkHovered(QString,QString,QString)\0"
    "contextMenuRequested(QPoint)\0"
    "resourceDownloadFinished()\0pasteTriggered()\0"
    "moveOneArticleUp()\0moveOneArticleDown()\0"
    "openSearch()\0on_searchText_textEdited()\0"
    "on_searchText_returnPressed()\0"
    "on_searchCloseButton_clicked()\0"
    "on_searchCaseSensitive_clicked()\0"
    "on_highlightAllButton_clicked()\0"
    "on_ftsSearchPrevious_clicked()\0"
    "on_ftsSearchNext_clicked()\0doubleClicked()\0"
    "audioPlayerError(QString)\0copyAsText()\0"
    "inspect()\0"
};

void ArticleView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ArticleView *_t = static_cast<ArticleView *>(_o);
        switch (_id) {
        case 0: _t->iconChanged((*reinterpret_cast< ArticleView*(*)>(_a[1])),(*reinterpret_cast< const QIcon(*)>(_a[2]))); break;
        case 1: _t->titleChanged((*reinterpret_cast< ArticleView*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 2: _t->pageLoaded((*reinterpret_cast< ArticleView*(*)>(_a[1]))); break;
        case 3: _t->openLinkInNewTab((*reinterpret_cast< const QUrl(*)>(_a[1])),(*reinterpret_cast< const QUrl(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const ArticleView::Contexts(*)>(_a[4]))); break;
        case 4: _t->showDefinitionInNewTab((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const ArticleView::Contexts(*)>(_a[4]))); break;
        case 5: _t->sendWordToHistory((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->typingEvent((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->statusBarMessage((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< const QPixmap(*)>(_a[3]))); break;
        case 8: _t->statusBarMessage((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 9: _t->statusBarMessage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: _t->showDictsPane(); break;
        case 11: _t->activeArticleChanged((*reinterpret_cast< const ArticleView*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 12: _t->forceAddWordToHistory((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 13: _t->closePopupMenu(); break;
        case 14: _t->setExpandMode((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 15: _t->sendWordToInputLine((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 16: _t->storeResourceSavePath((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 17: _t->zoomIn(); break;
        case 18: _t->zoomOut(); break;
        case 19: _t->back(); break;
        case 20: _t->forward(); break;
        case 21: _t->on_searchPrevious_clicked(); break;
        case 22: _t->on_searchNext_clicked(); break;
        case 23: _t->onJsActiveArticleChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 24: { bool _r = _t->handleF3((*reinterpret_cast< QObject*(*)>(_a[1])),(*reinterpret_cast< QEvent*(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 25: _t->receiveExpandOptionalParts((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 26: _t->switchExpandOptionalParts(); break;
        case 27: _t->selectCurrentArticle(); break;
        case 28: _t->loadFinished((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 29: _t->handleTitleChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 30: _t->handleUrlChanged((*reinterpret_cast< const QUrl(*)>(_a[1]))); break;
        case 31: _t->attachToJavaScript(); break;
        case 32: _t->linkClicked((*reinterpret_cast< const QUrl(*)>(_a[1]))); break;
        case 33: _t->linkHovered((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        case 34: _t->contextMenuRequested((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 35: _t->resourceDownloadFinished(); break;
        case 36: _t->pasteTriggered(); break;
        case 37: _t->moveOneArticleUp(); break;
        case 38: _t->moveOneArticleDown(); break;
        case 39: _t->openSearch(); break;
        case 40: _t->on_searchText_textEdited(); break;
        case 41: _t->on_searchText_returnPressed(); break;
        case 42: _t->on_searchCloseButton_clicked(); break;
        case 43: _t->on_searchCaseSensitive_clicked(); break;
        case 44: _t->on_highlightAllButton_clicked(); break;
        case 45: _t->on_ftsSearchPrevious_clicked(); break;
        case 46: _t->on_ftsSearchNext_clicked(); break;
        case 47: _t->doubleClicked(); break;
        case 48: _t->audioPlayerError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 49: _t->copyAsText(); break;
        case 50: _t->inspect(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ArticleView::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ArticleView::staticMetaObject = {
    { &QFrame::staticMetaObject, qt_meta_stringdata_ArticleView,
      qt_meta_data_ArticleView, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ArticleView::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ArticleView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ArticleView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ArticleView))
        return static_cast<void*>(const_cast< ArticleView*>(this));
    return QFrame::qt_metacast(_clname);
}

int ArticleView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 51)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 51;
    }
    return _id;
}

// SIGNAL 0
void ArticleView::iconChanged(ArticleView * _t1, QIcon const & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ArticleView::titleChanged(ArticleView * _t1, QString const & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void ArticleView::pageLoaded(ArticleView * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void ArticleView::openLinkInNewTab(QUrl const & _t1, QUrl const & _t2, QString const & _t3, ArticleView::Contexts const & _t4)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void ArticleView::showDefinitionInNewTab(QString const & _t1, unsigned  _t2, QString const & _t3, ArticleView::Contexts const & _t4)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void ArticleView::sendWordToHistory(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void ArticleView::typingEvent(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void ArticleView::statusBarMessage(QString const & _t1, int _t2, QPixmap const & _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 10
void ArticleView::showDictsPane()
{
    QMetaObject::activate(this, &staticMetaObject, 10, 0);
}

// SIGNAL 11
void ArticleView::activeArticleChanged(ArticleView const * _t1, QString const & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 11, _a);
}

// SIGNAL 12
void ArticleView::forceAddWordToHistory(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}

// SIGNAL 13
void ArticleView::closePopupMenu()
{
    QMetaObject::activate(this, &staticMetaObject, 13, 0);
}

// SIGNAL 14
void ArticleView::setExpandMode(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 14, _a);
}

// SIGNAL 15
void ArticleView::sendWordToInputLine(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 15, _a);
}

// SIGNAL 16
void ArticleView::storeResourceSavePath(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 16, _a);
}

// SIGNAL 17
void ArticleView::zoomIn()
{
    QMetaObject::activate(this, &staticMetaObject, 17, 0);
}

// SIGNAL 18
void ArticleView::zoomOut()
{
    QMetaObject::activate(this, &staticMetaObject, 18, 0);
}
static const uint qt_meta_data_ResourceToSaveHandler[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
      23,   22,   22,   22, 0x05,
      53,   30,   22,   22, 0x05,
     107,   91,   22,   22, 0x25,
     145,  137,   22,   22, 0x25,

 // slots: signature, parameters, type, tag, flags
     171,   22,   22,   22, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ResourceToSaveHandler[] = {
    "ResourceToSaveHandler\0\0done()\0"
    "message,timeout,pixmap\0"
    "statusBarMessage(QString,int,QPixmap)\0"
    "message,timeout\0statusBarMessage(QString,int)\0"
    "message\0statusBarMessage(QString)\0"
    "downloadFinished()\0"
};

void ResourceToSaveHandler::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ResourceToSaveHandler *_t = static_cast<ResourceToSaveHandler *>(_o);
        switch (_id) {
        case 0: _t->done(); break;
        case 1: _t->statusBarMessage((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< const QPixmap(*)>(_a[3]))); break;
        case 2: _t->statusBarMessage((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->statusBarMessage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->downloadFinished(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ResourceToSaveHandler::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ResourceToSaveHandler::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ResourceToSaveHandler,
      qt_meta_data_ResourceToSaveHandler, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ResourceToSaveHandler::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ResourceToSaveHandler::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ResourceToSaveHandler::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ResourceToSaveHandler))
        return static_cast<void*>(const_cast< ResourceToSaveHandler*>(this));
    return QObject::qt_metacast(_clname);
}

int ResourceToSaveHandler::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void ResourceToSaveHandler::done()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void ResourceToSaveHandler::statusBarMessage(QString const & _t1, int _t2, QPixmap const & _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
