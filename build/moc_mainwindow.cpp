/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.hh'
**
** Created: Wed Nov 19 15:11:22 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../mainwindow.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ExpandableToolBar[] = {

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

static const char qt_meta_stringdata_ExpandableToolBar[] = {
    "ExpandableToolBar\0"
};

void ExpandableToolBar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData ExpandableToolBar::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ExpandableToolBar::staticMetaObject = {
    { &QToolBar::staticMetaObject, qt_meta_stringdata_ExpandableToolBar,
      qt_meta_data_ExpandableToolBar, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ExpandableToolBar::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ExpandableToolBar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ExpandableToolBar::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ExpandableToolBar))
        return static_cast<void*>(const_cast< ExpandableToolBar*>(this));
    return QToolBar::qt_metacast(_clname);
}

int ExpandableToolBar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QToolBar::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_MainWindow[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
     111,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      19,   12,   11,   11, 0x05,
      51,   48,   11,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
      76,   11,   11,   11, 0x0a,
     123,  120,   11,   11, 0x0a,
     165,   11,   11,   11, 0x0a,
     187,   12,   11,   11, 0x0a,
     207,   11,   11,   11, 0x08,
     228,   11,   11,   11, 0x08,
     254,   11,   11,   11, 0x08,
     275,   11,   11,   11, 0x08,
     306,  301,   11,   11, 0x08,
     350,  346,   11,   11, 0x08,
     389,   48,   11,   11, 0x08,
     417,   48,   11,   11, 0x08,
     450,   48,   11,   11, 0x08,
     485,  480,   11,   11, 0x08,
     520,   11,   11,   11, 0x08,
     532,   11,   11,   11, 0x08,
     555,   11,   11,   11, 0x08,
     573,   11,   11,   11, 0x08,
     588,   11,   11,   11, 0x08,
     604,   11,   11,   11, 0x08,
     622,   11,   11,   11, 0x08,
     640,   11,   11,   11, 0x08,
     655,   11,   11,   11, 0x08,
     687,   11,   11,   11, 0x08,
     703,   11,   11,   11, 0x08,
     725,  721,   11,   11, 0x08,
     750,   11,   11,   11, 0x08,
     764,   11,   11,   11, 0x08,
     783,  781,   11,   11, 0x08,
     818,  781,   11,   11, 0x08,
     850,   11,   11,   11, 0x08,
     875,   11,   11,   11, 0x08,
     892,  346,   11,   11, 0x08,
     925,  917,   11,   11, 0x08,
     957,  952,   11,   11, 0x08,
     981,   11,   11,   11, 0x28,
     993,   11,   11,   11, 0x08,
    1002,   11,   11,   11, 0x08,
    1012,   11,   11,   11, 0x08,
    1021,   11,   11,   11, 0x08,
    1037,   11,   11,   11, 0x08,
    1054,   11,   11,   11, 0x08,
    1072,   11,   11,   11, 0x08,
    1114, 1094,   11,   11, 0x08,
    1137,   11,   11,   11, 0x28,
    1156,   11,   11,   11, 0x08,
    1175,   11,   11,   11, 0x08,
    1193,   11,   11,   11, 0x08,
    1222,   11,   11,   11, 0x08,
    1268, 1253,   11,   11, 0x08,
    1297,   11,   11,   11, 0x28,
    1322,   11,   11,   11, 0x08,
    1334,   11,   11,   11, 0x08,
    1355,   11,   11,   11, 0x08,
    1395,   11,   11,   11, 0x08,
    1422,   11,   11,   11, 0x08,
    1463,   11,   11,   11, 0x08,
    1498, 1491,   11,   11, 0x08,
    1538,   11,   11,   11, 0x28,
    1573,   11,   11,   11, 0x08,
    1589,   11,   11,   11, 0x08,
    1651, 1635, 1622,   11, 0x08,
    1690, 1678,   11,   11, 0x08,
    1780, 1748,   11,   11, 0x08,
    1847,   11,   11,   11, 0x08,
    1872, 1868,   11,   11, 0x08,
    1921,   11,   11,   11, 0x08,
    1957, 1948,   11,   11, 0x08,
    1990,   11,   11,   11, 0x28,
    2040, 2018,   11,   11, 0x08,
    2088,   11,   11,   11, 0x08,
    2113,   11,   11,   11, 0x08,
    2132,   11,   11,   11, 0x08,
    2185,   11,   11,   11, 0x08,
    2209,   11,   11,   11, 0x08,
    2228,   11,   11,   11, 0x08,
    2245,   11,   11,   11, 0x08,
    2261,   11,   11,   11, 0x08,
    2274,   11,   11,   11, 0x08,
    2293,   11,   11,   11, 0x08,
    2305,   11,   11,   11, 0x08,
    2333,   11,   11,   11, 0x08,
    2377, 2368,   11,   11, 0x08,
    2406,   11,   11,   11, 0x28,
    2431,   11,   11,   11, 0x08,
    2459,   11,   11,   11, 0x08,
    2481,   11,   11,   11, 0x08,
    2514,   11,   11,   11, 0x08,
    2539,   11,   11,   11, 0x08,
    2567,   11,   11,   11, 0x08,
    2588,   11,   11,   11, 0x08,
    2626,   11,   11,   11, 0x08,
    2653,   11,   11,   11, 0x08,
    2680,   11,   11,   11, 0x08,
    2711,   11,   11,   11, 0x08,
    2740,   11,   11,   11, 0x08,
    2769,  917,   11,   11, 0x08,
    2800,   11,   11,   11, 0x08,
    2829, 2816,   11,   11, 0x08,
    2858,   11,   11,   11, 0x08,
    2883, 2878,   11,   11, 0x08,
    2909, 2878,   11,   11, 0x08,
    2940, 2878,   11,   11, 0x08,
    2969,   11,   11,   11, 0x08,
    3000,   11,   11,   11, 0x08,
    3023,   11,   11,   11, 0x08,
    3066, 3046,   11,   11, 0x08,
    3117,   11,   11,   11, 0x08,
    3144,   11,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MainWindow[] = {
    "MainWindow\0\0expand\0setExpandOptionalParts(bool)\0"
    "id\0clickOnDictPane(QString)\0"
    "messageFromAnotherInstanceReceived(QString)\0"
    ",,\0showStatusBarMessage(QString,int,QPixmap)\0"
    "wordReceived(QString)\0setExpandMode(bool)\0"
    "hotKeyActivated(int)\0prepareNewReleaseChecks()\0"
    "checkForNewRelease()\0latestReleaseReplyReady()\0"
    "item\0foundDictsPaneClicked(QListWidgetItem*)\0"
    "pos\0foundDictsContextMenuRequested(QPoint)\0"
    "showDictionaryInfo(QString)\0"
    "showDictionaryHeadwords(QString)\0"
    "openDictionaryFolder(QString)\0dict\0"
    "editDictionary(Dictionary::Class*)\0"
    "addNewTab()\0tabCloseRequested(int)\0"
    "closeCurrentTab()\0closeAllTabs()\0"
    "closeRestTabs()\0switchToNextTab()\0"
    "switchToPrevTab()\0ctrlReleased()\0"
    "switchExpandOptionalPartsMode()\0"
    "createTabList()\0fillWindowsMenu()\0act\0"
    "switchToWindow(QAction*)\0backClicked()\0"
    "forwardClicked()\0,\0"
    "titleChanged(ArticleView*,QString)\0"
    "iconChanged(ArticleView*,QIcon)\0"
    "pageLoaded(ArticleView*)\0tabSwitched(int)\0"
    "tabMenuRequested(QPoint)\0checked\0"
    "dictionaryBarToggled(bool)\0view\0"
    "pronounce(ArticleView*)\0pronounce()\0"
    "zoomin()\0zoomout()\0unzoom()\0doWordsZoomIn()\0"
    "doWordsZoomOut()\0doWordsZoomBase()\0"
    "applyWordsZoomLevel()\0editDictionaryGroup\0"
    "editDictionaries(uint)\0editDictionaries()\0"
    "editCurrentGroup()\0editPreferences()\0"
    "currentGroupChanged(QString)\0"
    "translateInputChanged(QString)\0"
    "checkModifiers\0translateInputFinished(bool)\0"
    "translateInputFinished()\0handleEsc()\0"
    "focusTranslateLine()\0"
    "wordListItemActivated(QListWidgetItem*)\0"
    "wordListSelectionChanged()\0"
    "dictsListItemActivated(QListWidgetItem*)\0"
    "dictsListSelectionChanged()\0,force\0"
    "jumpToDictionary(QListWidgetItem*,bool)\0"
    "jumpToDictionary(QListWidgetItem*)\0"
    "showDictsPane()\0dictsPaneVisibilityChanged(bool)\0"
    "ArticleView*\0switchToIt,name\0"
    "createNewTab(bool,QString)\0,,,contexts\0"
    "openLinkInNewTab(QUrl,QUrl,QString,ArticleView::Contexts)\0"
    "word,group,fromArticle,contexts\0"
    "showDefinitionInNewTab(QString,uint,QString,ArticleView::Contexts)\0"
    "typingEvent(QString)\0,id\0"
    "activeArticleChanged(const ArticleView*,QString)\0"
    "mutedDictionariesChanged()\0,inGroup\0"
    "showTranslationFor(QString,uint)\0"
    "showTranslationFor(QString)\0"
    ",dictIDs,searchRegExp\0"
    "showTranslationFor(QString,QStringList,QRegExp)\0"
    "showHistoryItem(QString)\0loginHistoryItem()\0"
    "trayIconActivated(QSystemTrayIcon::ActivationReason)\0"
    "scanEnableToggled(bool)\0setAutostart(bool)\0"
    "showMainWindow()\0visitHomepage()\0"
    "visitForum()\0openConfigFolder()\0"
    "showAbout()\0showDictBarNamesTriggered()\0"
    "useSmallIconsInToolbarsTriggered()\0"
    "announce\0toggleMenuBarTriggered(bool)\0"
    "toggleMenuBarTriggered()\0"
    "on_clearHistory_triggered()\0"
    "on_newTab_triggered()\0"
    "on_actionCloseToTray_triggered()\0"
    "on_pageSetup_triggered()\0"
    "on_printPreview_triggered()\0"
    "on_print_triggered()\0"
    "printPreviewPaintRequested(QPrinter*)\0"
    "on_saveArticle_triggered()\0"
    "on_rescanFiles_triggered()\0"
    "on_showHideHistory_triggered()\0"
    "on_exportHistory_triggered()\0"
    "on_importHistory_triggered()\0"
    "on_alwaysOnTop_triggered(bool)\0"
    "focusWordList()\0searchInDock\0"
    "updateSearchPaneAndBar(bool)\0"
    "updateHistoryMenu()\0word\0"
    "addWordToHistory(QString)\0"
    "forceAddWordToHistory(QString)\0"
    "sendWordToInputLine(QString)\0"
    "storeResourceSavePath(QString)\0"
    "closeHeadwordsDialog()\0focusHeadwordsDialog()\0"
    "proxy,authenticator\0"
    "proxyAuthentication(QNetworkProxy,QAuthenticator*)\0"
    "showFullTextSearchDialog()\0"
    "closeFullTextSearchDialog()\0"
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MainWindow *_t = static_cast<MainWindow *>(_o);
        switch (_id) {
        case 0: _t->setExpandOptionalParts((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->clickOnDictPane((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->messageFromAnotherInstanceReceived((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->showStatusBarMessage((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< const QPixmap(*)>(_a[3]))); break;
        case 4: _t->wordReceived((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->setExpandMode((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->hotKeyActivated((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->prepareNewReleaseChecks(); break;
        case 8: _t->checkForNewRelease(); break;
        case 9: _t->latestReleaseReplyReady(); break;
        case 10: _t->foundDictsPaneClicked((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 11: _t->foundDictsContextMenuRequested((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 12: _t->showDictionaryInfo((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 13: _t->showDictionaryHeadwords((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 14: _t->openDictionaryFolder((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 15: _t->editDictionary((*reinterpret_cast< Dictionary::Class*(*)>(_a[1]))); break;
        case 16: _t->addNewTab(); break;
        case 17: _t->tabCloseRequested((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 18: _t->closeCurrentTab(); break;
        case 19: _t->closeAllTabs(); break;
        case 20: _t->closeRestTabs(); break;
        case 21: _t->switchToNextTab(); break;
        case 22: _t->switchToPrevTab(); break;
        case 23: _t->ctrlReleased(); break;
        case 24: _t->switchExpandOptionalPartsMode(); break;
        case 25: _t->createTabList(); break;
        case 26: _t->fillWindowsMenu(); break;
        case 27: _t->switchToWindow((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 28: _t->backClicked(); break;
        case 29: _t->forwardClicked(); break;
        case 30: _t->titleChanged((*reinterpret_cast< ArticleView*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 31: _t->iconChanged((*reinterpret_cast< ArticleView*(*)>(_a[1])),(*reinterpret_cast< const QIcon(*)>(_a[2]))); break;
        case 32: _t->pageLoaded((*reinterpret_cast< ArticleView*(*)>(_a[1]))); break;
        case 33: _t->tabSwitched((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 34: _t->tabMenuRequested((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        case 35: _t->dictionaryBarToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 36: _t->pronounce((*reinterpret_cast< ArticleView*(*)>(_a[1]))); break;
        case 37: _t->pronounce(); break;
        case 38: _t->zoomin(); break;
        case 39: _t->zoomout(); break;
        case 40: _t->unzoom(); break;
        case 41: _t->doWordsZoomIn(); break;
        case 42: _t->doWordsZoomOut(); break;
        case 43: _t->doWordsZoomBase(); break;
        case 44: _t->applyWordsZoomLevel(); break;
        case 45: _t->editDictionaries((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 46: _t->editDictionaries(); break;
        case 47: _t->editCurrentGroup(); break;
        case 48: _t->editPreferences(); break;
        case 49: _t->currentGroupChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 50: _t->translateInputChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 51: _t->translateInputFinished((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 52: _t->translateInputFinished(); break;
        case 53: _t->handleEsc(); break;
        case 54: _t->focusTranslateLine(); break;
        case 55: _t->wordListItemActivated((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 56: _t->wordListSelectionChanged(); break;
        case 57: _t->dictsListItemActivated((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 58: _t->dictsListSelectionChanged(); break;
        case 59: _t->jumpToDictionary((*reinterpret_cast< QListWidgetItem*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 60: _t->jumpToDictionary((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 61: _t->showDictsPane(); break;
        case 62: _t->dictsPaneVisibilityChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 63: { ArticleView* _r = _t->createNewTab((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< ArticleView**>(_a[0]) = _r; }  break;
        case 64: _t->openLinkInNewTab((*reinterpret_cast< const QUrl(*)>(_a[1])),(*reinterpret_cast< const QUrl(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const ArticleView::Contexts(*)>(_a[4]))); break;
        case 65: _t->showDefinitionInNewTab((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const ArticleView::Contexts(*)>(_a[4]))); break;
        case 66: _t->typingEvent((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 67: _t->activeArticleChanged((*reinterpret_cast< const ArticleView*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 68: _t->mutedDictionariesChanged(); break;
        case 69: _t->showTranslationFor((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2]))); break;
        case 70: _t->showTranslationFor((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 71: _t->showTranslationFor((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QStringList(*)>(_a[2])),(*reinterpret_cast< const QRegExp(*)>(_a[3]))); break;
        case 72: _t->showHistoryItem((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 73: _t->loginHistoryItem(); break;
        case 74: _t->trayIconActivated((*reinterpret_cast< QSystemTrayIcon::ActivationReason(*)>(_a[1]))); break;
        case 75: _t->scanEnableToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 76: _t->setAutostart((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 77: _t->showMainWindow(); break;
        case 78: _t->visitHomepage(); break;
        case 79: _t->visitForum(); break;
        case 80: _t->openConfigFolder(); break;
        case 81: _t->showAbout(); break;
        case 82: _t->showDictBarNamesTriggered(); break;
        case 83: _t->useSmallIconsInToolbarsTriggered(); break;
        case 84: _t->toggleMenuBarTriggered((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 85: _t->toggleMenuBarTriggered(); break;
        case 86: _t->on_clearHistory_triggered(); break;
        case 87: _t->on_newTab_triggered(); break;
        case 88: _t->on_actionCloseToTray_triggered(); break;
        case 89: _t->on_pageSetup_triggered(); break;
        case 90: _t->on_printPreview_triggered(); break;
        case 91: _t->on_print_triggered(); break;
        case 92: _t->printPreviewPaintRequested((*reinterpret_cast< QPrinter*(*)>(_a[1]))); break;
        case 93: _t->on_saveArticle_triggered(); break;
        case 94: _t->on_rescanFiles_triggered(); break;
        case 95: _t->on_showHideHistory_triggered(); break;
        case 96: _t->on_exportHistory_triggered(); break;
        case 97: _t->on_importHistory_triggered(); break;
        case 98: _t->on_alwaysOnTop_triggered((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 99: _t->focusWordList(); break;
        case 100: _t->updateSearchPaneAndBar((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 101: _t->updateHistoryMenu(); break;
        case 102: _t->addWordToHistory((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 103: _t->forceAddWordToHistory((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 104: _t->sendWordToInputLine((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 105: _t->storeResourceSavePath((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 106: _t->closeHeadwordsDialog(); break;
        case 107: _t->focusHeadwordsDialog(); break;
        case 108: _t->proxyAuthentication((*reinterpret_cast< const QNetworkProxy(*)>(_a[1])),(*reinterpret_cast< QAuthenticator*(*)>(_a[2]))); break;
        case 109: _t->showFullTextSearchDialog(); break;
        case 110: _t->closeFullTextSearchDialog(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData MainWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow,
      qt_meta_data_MainWindow, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    if (!strcmp(_clname, "DataCommitter"))
        return static_cast< DataCommitter*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 111)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 111;
    }
    return _id;
}

// SIGNAL 0
void MainWindow::setExpandOptionalParts(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void MainWindow::clickOnDictPane(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_ArticleSaveProgressDialog[] = {

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
      27,   26,   26,   26, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_ArticleSaveProgressDialog[] = {
    "ArticleSaveProgressDialog\0\0perform()\0"
};

void ArticleSaveProgressDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ArticleSaveProgressDialog *_t = static_cast<ArticleSaveProgressDialog *>(_o);
        switch (_id) {
        case 0: _t->perform(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData ArticleSaveProgressDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ArticleSaveProgressDialog::staticMetaObject = {
    { &QProgressDialog::staticMetaObject, qt_meta_stringdata_ArticleSaveProgressDialog,
      qt_meta_data_ArticleSaveProgressDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ArticleSaveProgressDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ArticleSaveProgressDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ArticleSaveProgressDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ArticleSaveProgressDialog))
        return static_cast<void*>(const_cast< ArticleSaveProgressDialog*>(this));
    return QProgressDialog::qt_metacast(_clname);
}

int ArticleSaveProgressDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QProgressDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
