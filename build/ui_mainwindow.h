/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "dictspanewidget.hh"
#include "historypanewidget.hh"
#include "maintabwidget.hh"
#include "searchpanewidget.hh"
#include "wordlist.hh"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *dictionaries;
    QAction *preferences;
    QAction *visitHomepage;
    QAction *about;
    QAction *quit;
    QAction *visitForum;
    QAction *actionCloseToTray;
    QAction *saveArticle;
    QAction *print;
    QAction *pageSetup;
    QAction *printPreview;
    QAction *rescanFiles;
    QAction *clearHistory;
    QAction *newTab;
    QAction *openConfigFolder;
    QAction *showHideHistory;
    QAction *exportHistory;
    QAction *importHistory;
    QAction *alwaysOnTop;
    QAction *menuOptions;
    QAction *searchInPageAction;
    QAction *fullTextSearchAction;
    QAction *loginHistory;
    QAction *synchronizeHistory;
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout_2;
    MainTabWidget *tabWidget;
    QWidget *tab;
    QHBoxLayout *horizontalLayout_4;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menu_Edit;
    QMenu *menu_Help;
    QMenu *menuView;
    QMenu *menuZoom;
    QMenu *menuHistory;
    QMenu *menuSearch;
    QDockWidget *searchPane;
    SearchPaneWidget *searchPaneWidget;
    QVBoxLayout *verticalLayout;
    QLineEdit *translateLine;
    WordList *wordList;
    QDockWidget *dictsPane;
    DictsPaneWidget *dictsPaneWidget;
    QVBoxLayout *verticalLayout_2;
    QListWidget *dictsList;
    QDockWidget *historyPane;
    HistoryPaneWidget *historyPaneWidget;
    QVBoxLayout *verticalLayout_3;
    QListView *historyList;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(653, 538);
        MainWindow->setWindowTitle(QStringLiteral("GoldenDict"));
        dictionaries = new QAction(MainWindow);
        dictionaries->setObjectName(QStringLiteral("dictionaries"));
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/book.png"), QSize(), QIcon::Normal, QIcon::Off);
        dictionaries->setIcon(icon);
        dictionaries->setMenuRole(QAction::NoRole);
        preferences = new QAction(MainWindow);
        preferences->setObjectName(QStringLiteral("preferences"));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/icons/configure.png"), QSize(), QIcon::Normal, QIcon::Off);
        preferences->setIcon(icon1);
        preferences->setMenuRole(QAction::PreferencesRole);
        visitHomepage = new QAction(MainWindow);
        visitHomepage->setObjectName(QStringLiteral("visitHomepage"));
        visitHomepage->setMenuRole(QAction::NoRole);
        about = new QAction(MainWindow);
        about->setObjectName(QStringLiteral("about"));
        about->setMenuRole(QAction::AboutRole);
        quit = new QAction(MainWindow);
        quit->setObjectName(QStringLiteral("quit"));
        quit->setMenuRole(QAction::QuitRole);
        visitForum = new QAction(MainWindow);
        visitForum->setObjectName(QStringLiteral("visitForum"));
        visitForum->setMenuRole(QAction::NoRole);
        actionCloseToTray = new QAction(MainWindow);
        actionCloseToTray->setObjectName(QStringLiteral("actionCloseToTray"));
        actionCloseToTray->setMenuRole(QAction::NoRole);
        saveArticle = new QAction(MainWindow);
        saveArticle->setObjectName(QStringLiteral("saveArticle"));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/icons/filesave.png"), QSize(), QIcon::Normal, QIcon::Off);
        saveArticle->setIcon(icon2);
        saveArticle->setMenuRole(QAction::NoRole);
        print = new QAction(MainWindow);
        print->setObjectName(QStringLiteral("print"));
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/icons/print.png"), QSize(), QIcon::Normal, QIcon::Off);
        print->setIcon(icon3);
        print->setMenuRole(QAction::NoRole);
        pageSetup = new QAction(MainWindow);
        pageSetup->setObjectName(QStringLiteral("pageSetup"));
        pageSetup->setMenuRole(QAction::NoRole);
        printPreview = new QAction(MainWindow);
        printPreview->setObjectName(QStringLiteral("printPreview"));
        printPreview->setMenuRole(QAction::NoRole);
        rescanFiles = new QAction(MainWindow);
        rescanFiles->setObjectName(QStringLiteral("rescanFiles"));
        rescanFiles->setMenuRole(QAction::NoRole);
        clearHistory = new QAction(MainWindow);
        clearHistory->setObjectName(QStringLiteral("clearHistory"));
        clearHistory->setMenuRole(QAction::NoRole);
        newTab = new QAction(MainWindow);
        newTab->setObjectName(QStringLiteral("newTab"));
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/icons/addtab.png"), QSize(), QIcon::Normal, QIcon::Off);
        newTab->setIcon(icon4);
        newTab->setShortcutContext(Qt::WidgetShortcut);
        newTab->setMenuRole(QAction::NoRole);
        openConfigFolder = new QAction(MainWindow);
        openConfigFolder->setObjectName(QStringLiteral("openConfigFolder"));
        openConfigFolder->setMenuRole(QAction::NoRole);
        showHideHistory = new QAction(MainWindow);
        showHideHistory->setObjectName(QStringLiteral("showHideHistory"));
        showHideHistory->setMenuRole(QAction::NoRole);
        exportHistory = new QAction(MainWindow);
        exportHistory->setObjectName(QStringLiteral("exportHistory"));
        exportHistory->setMenuRole(QAction::NoRole);
        importHistory = new QAction(MainWindow);
        importHistory->setObjectName(QStringLiteral("importHistory"));
        importHistory->setMenuRole(QAction::NoRole);
        alwaysOnTop = new QAction(MainWindow);
        alwaysOnTop->setObjectName(QStringLiteral("alwaysOnTop"));
        alwaysOnTop->setCheckable(true);
        menuOptions = new QAction(MainWindow);
        menuOptions->setObjectName(QStringLiteral("menuOptions"));
        QIcon icon5;
        icon5.addFile(QStringLiteral(":/icons/menu_button.png"), QSize(), QIcon::Normal, QIcon::Off);
        menuOptions->setIcon(icon5);
        searchInPageAction = new QAction(MainWindow);
        searchInPageAction->setObjectName(QStringLiteral("searchInPageAction"));
        searchInPageAction->setMenuRole(QAction::TextHeuristicRole);
        fullTextSearchAction = new QAction(MainWindow);
        fullTextSearchAction->setObjectName(QStringLiteral("fullTextSearchAction"));
        fullTextSearchAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        fullTextSearchAction->setMenuRole(QAction::TextHeuristicRole);
        loginHistory = new QAction(MainWindow);
        loginHistory->setObjectName(QStringLiteral("loginHistory"));
        synchronizeHistory = new QAction(MainWindow);
        synchronizeHistory->setObjectName(QStringLiteral("synchronizeHistory"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        horizontalLayout_2 = new QHBoxLayout(centralWidget);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(1, 2, 1, 0);
        tabWidget = new MainTabWidget(centralWidget);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tabWidget->setIconSize(QSize(16, 16));
        tabWidget->setElideMode(Qt::ElideRight);
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        horizontalLayout_4 = new QHBoxLayout(tab);
        horizontalLayout_4->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        tabWidget->addTab(tab, QString());

        horizontalLayout_2->addWidget(tabWidget);

        MainWindow->setCentralWidget(centralWidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 653, 25));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menu_Edit = new QMenu(menubar);
        menu_Edit->setObjectName(QStringLiteral("menu_Edit"));
        menu_Help = new QMenu(menubar);
        menu_Help->setObjectName(QStringLiteral("menu_Help"));
        menuView = new QMenu(menubar);
        menuView->setObjectName(QStringLiteral("menuView"));
        menuZoom = new QMenu(menuView);
        menuZoom->setObjectName(QStringLiteral("menuZoom"));
        menuHistory = new QMenu(menubar);
        menuHistory->setObjectName(QStringLiteral("menuHistory"));
        menuSearch = new QMenu(menubar);
        menuSearch->setObjectName(QStringLiteral("menuSearch"));
        MainWindow->setMenuBar(menubar);
        searchPane = new QDockWidget(MainWindow);
        searchPane->setObjectName(QStringLiteral("searchPane"));
        searchPaneWidget = new SearchPaneWidget();
        searchPaneWidget->setObjectName(QStringLiteral("searchPaneWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(searchPaneWidget->sizePolicy().hasHeightForWidth());
        searchPaneWidget->setSizePolicy(sizePolicy);
        verticalLayout = new QVBoxLayout(searchPaneWidget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(2, 1, 2, 1);
        translateLine = new QLineEdit(searchPaneWidget);
        translateLine->setObjectName(QStringLiteral("translateLine"));
        translateLine->setMinimumSize(QSize(200, 0));
        translateLine->setBaseSize(QSize(0, 0));
        QPalette palette;
        QBrush brush(QColor(254, 253, 235, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Base, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Base, brush);
        QBrush brush1(QColor(255, 255, 255, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Disabled, QPalette::Base, brush1);
        translateLine->setPalette(palette);
        translateLine->setFrame(true);

        verticalLayout->addWidget(translateLine);

        wordList = new WordList(searchPaneWidget);
        wordList->setObjectName(QStringLiteral("wordList"));
        QPalette palette1;
        palette1.setBrush(QPalette::Active, QPalette::Base, brush);
        palette1.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette1.setBrush(QPalette::Disabled, QPalette::Base, brush1);
        wordList->setPalette(palette1);

        verticalLayout->addWidget(wordList);

        searchPane->setWidget(searchPaneWidget);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(1), searchPane);
        dictsPane = new QDockWidget(MainWindow);
        dictsPane->setObjectName(QStringLiteral("dictsPane"));
        dictsPaneWidget = new DictsPaneWidget();
        dictsPaneWidget->setObjectName(QStringLiteral("dictsPaneWidget"));
        verticalLayout_2 = new QVBoxLayout(dictsPaneWidget);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(2, 1, 2, 1);
        dictsList = new QListWidget(dictsPaneWidget);
        dictsList->setObjectName(QStringLiteral("dictsList"));

        verticalLayout_2->addWidget(dictsList);

        dictsPane->setWidget(dictsPaneWidget);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(2), dictsPane);
        historyPane = new QDockWidget(MainWindow);
        historyPane->setObjectName(QStringLiteral("historyPane"));
        historyPaneWidget = new HistoryPaneWidget();
        historyPaneWidget->setObjectName(QStringLiteral("historyPaneWidget"));
        verticalLayout_3 = new QVBoxLayout(historyPaneWidget);
        verticalLayout_3->setSpacing(2);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(2, 1, 2, 1);
        historyList = new QListView(historyPaneWidget);
        historyList->setObjectName(QStringLiteral("historyList"));

        verticalLayout_3->addWidget(historyList);

        historyPane->setWidget(historyPaneWidget);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(2), historyPane);
        QWidget::setTabOrder(translateLine, tabWidget);
        QWidget::setTabOrder(tabWidget, dictsList);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuView->menuAction());
        menubar->addAction(menu_Edit->menuAction());
        menubar->addAction(menuSearch->menuAction());
        menubar->addAction(menuHistory->menuAction());
        menubar->addAction(menu_Help->menuAction());
        menuFile->addAction(newTab);
        menuFile->addSeparator();
        menuFile->addAction(pageSetup);
        menuFile->addAction(printPreview);
        menuFile->addAction(print);
        menuFile->addSeparator();
        menuFile->addAction(saveArticle);
        menuFile->addSeparator();
        menuFile->addAction(rescanFiles);
        menuFile->addSeparator();
        menuFile->addAction(actionCloseToTray);
        menuFile->addAction(quit);
        menu_Edit->addAction(dictionaries);
        menu_Edit->addAction(preferences);
        menu_Help->addAction(visitHomepage);
        menu_Help->addAction(visitForum);
        menu_Help->addSeparator();
        menu_Help->addAction(openConfigFolder);
        menu_Help->addSeparator();
        menu_Help->addAction(about);
        menuView->addAction(menuZoom->menuAction());
        menuHistory->addAction(showHideHistory);
        menuHistory->addAction(exportHistory);
        menuHistory->addAction(importHistory);
        menuHistory->addSeparator();
        menuHistory->addAction(clearHistory);
        menuHistory->addSeparator();
        menuHistory->addAction(loginHistory);
        menuHistory->addAction(synchronizeHistory);
        menuSearch->addAction(searchInPageAction);
        menuSearch->addAction(fullTextSearchAction);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        dictionaries->setText(QApplication::translate("MainWindow", "&Dictionaries...", 0));
        dictionaries->setShortcut(QApplication::translate("MainWindow", "F3", 0));
        preferences->setText(QApplication::translate("MainWindow", "&Preferences...", 0));
        preferences->setShortcut(QApplication::translate("MainWindow", "F4", 0));
        visitHomepage->setText(QApplication::translate("MainWindow", "&Homepage", 0));
        about->setText(QApplication::translate("MainWindow", "&About", 0));
#ifndef QT_NO_TOOLTIP
        about->setToolTip(QApplication::translate("MainWindow", "About GoldenDict", 0));
#endif // QT_NO_TOOLTIP
        about->setShortcut(QApplication::translate("MainWindow", "F1", 0));
        quit->setText(QApplication::translate("MainWindow", "&Quit", 0));
#ifndef QT_NO_TOOLTIP
        quit->setToolTip(QApplication::translate("MainWindow", "Quit from application", 0));
#endif // QT_NO_TOOLTIP
        quit->setShortcut(QApplication::translate("MainWindow", "Ctrl+Q", 0));
        visitForum->setText(QApplication::translate("MainWindow", "&Forum", 0));
        actionCloseToTray->setText(QApplication::translate("MainWindow", "&Close To Tray", 0));
#ifndef QT_NO_TOOLTIP
        actionCloseToTray->setToolTip(QApplication::translate("MainWindow", "Minimizes the window to tray", 0));
#endif // QT_NO_TOOLTIP
        actionCloseToTray->setShortcut(QApplication::translate("MainWindow", "Ctrl+F4", 0));
        saveArticle->setText(QApplication::translate("MainWindow", "&Save Article", 0));
#ifndef QT_NO_TOOLTIP
        saveArticle->setToolTip(QApplication::translate("MainWindow", "Save Article", 0));
#endif // QT_NO_TOOLTIP
        saveArticle->setShortcut(QApplication::translate("MainWindow", "F2", 0));
        print->setText(QApplication::translate("MainWindow", "&Print", 0));
        print->setShortcut(QApplication::translate("MainWindow", "Ctrl+P", 0));
        pageSetup->setText(QApplication::translate("MainWindow", "Page Set&up", 0));
        printPreview->setText(QApplication::translate("MainWindow", "Print Pre&view", 0));
        rescanFiles->setText(QApplication::translate("MainWindow", "&Rescan Files", 0));
        rescanFiles->setShortcut(QApplication::translate("MainWindow", "Ctrl+F5", 0));
        clearHistory->setText(QApplication::translate("MainWindow", "&Clear", 0));
        newTab->setText(QApplication::translate("MainWindow", "&New Tab", 0));
        newTab->setShortcut(QApplication::translate("MainWindow", "Ctrl+T", 0));
        openConfigFolder->setText(QApplication::translate("MainWindow", "&Configuration Folder", 0));
        showHideHistory->setText(QApplication::translate("MainWindow", "&Show", 0));
        showHideHistory->setShortcut(QApplication::translate("MainWindow", "Ctrl+H", 0));
        exportHistory->setText(QApplication::translate("MainWindow", "&Export", 0));
        importHistory->setText(QApplication::translate("MainWindow", "&Import", 0));
        alwaysOnTop->setText(QApplication::translate("MainWindow", "&Always on Top", 0));
#ifndef QT_NO_TOOLTIP
        alwaysOnTop->setToolTip(QApplication::translate("MainWindow", "Always on Top", 0));
#endif // QT_NO_TOOLTIP
        alwaysOnTop->setShortcut(QApplication::translate("MainWindow", "Ctrl+O", 0));
        menuOptions->setText(QApplication::translate("MainWindow", "Menu Button", 0));
#ifndef QT_NO_TOOLTIP
        menuOptions->setToolTip(QApplication::translate("MainWindow", "Menu Button", 0));
#endif // QT_NO_TOOLTIP
        searchInPageAction->setText(QApplication::translate("MainWindow", "Search in page", 0));
        searchInPageAction->setShortcut(QApplication::translate("MainWindow", "Ctrl+F", 0));
        fullTextSearchAction->setText(QApplication::translate("MainWindow", "Full-text search", 0));
        fullTextSearchAction->setShortcut(QApplication::translate("MainWindow", "Ctrl+Shift+F", 0));
        loginHistory->setText(QApplication::translate("MainWindow", "&Login", 0));
#ifndef QT_NO_TOOLTIP
        loginHistory->setToolTip(QApplication::translate("MainWindow", "Login to server to synchronize", 0));
#endif // QT_NO_TOOLTIP
        synchronizeHistory->setText(QApplication::translate("MainWindow", "S&ynchronize...", 0));
#ifndef QT_NO_TOOLTIP
        synchronizeHistory->setToolTip(QApplication::translate("MainWindow", "Synchronize history looking words up", 0));
#endif // QT_NO_TOOLTIP
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("MainWindow", "Welcome!", 0));
        menuFile->setTitle(QApplication::translate("MainWindow", "&File", 0));
        menu_Edit->setTitle(QApplication::translate("MainWindow", "&Edit", 0));
        menu_Help->setTitle(QApplication::translate("MainWindow", "&Help", 0));
        menuView->setTitle(QApplication::translate("MainWindow", "&View", 0));
        menuZoom->setTitle(QApplication::translate("MainWindow", "&Zoom", 0));
        menuHistory->setTitle(QApplication::translate("MainWindow", "H&istory", 0));
        menuSearch->setTitle(QApplication::translate("MainWindow", "Search", 0));
        searchPane->setWindowTitle(QApplication::translate("MainWindow", "&Search Pane", 0));
        dictsPane->setWindowTitle(QApplication::translate("MainWindow", "&Results Navigation Pane", 0));
        historyPane->setWindowTitle(QApplication::translate("MainWindow", "&History Pane", 0));
        Q_UNUSED(MainWindow);
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
