/********************************************************************************
** Form generated from reading UI file 'preferences.ui'
**
** Created: Wed Nov 19 16:48:27 2014
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PREFERENCES_H
#define UI_PREFERENCES_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QRadioButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "hotkeyedit.hh"
#include "stylescombobox.hh"

QT_BEGIN_NAMESPACE

class Ui_Preferences
{
public:
    QVBoxLayout *verticalLayout_8;
    QTabWidget *tabWidget;
    QWidget *tab;
    QGridLayout *gridLayout_2;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_2;
    QCheckBox *cbAutostart;
    QHBoxLayout *horizontalLayout_4;
    QLabel *addonStylesLabel;
    StylesComboBox *addonStyles;
    QSpacerItem *horizontalSpacer_6;
    QGroupBox *enableTrayIcon;
    QVBoxLayout *verticalLayout;
    QCheckBox *startToTray;
    QCheckBox *closeToTray;
    QCheckBox *doubleClickTranslates;
    QCheckBox *escKeyHidesMainWindow;
    QCheckBox *selectBySingleClick;
    QSpacerItem *verticalSpacer_8;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_5;
    QCheckBox *newTabsOpenInBackground;
    QCheckBox *newTabsOpenAfterCurrentOne;
    QCheckBox *hideSingleTab;
    QCheckBox *mruTabOrder;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_6;
    QComboBox *interfaceLanguage;
    QSpacerItem *horizontalSpacer_3;
    QLabel *label_9;
    QComboBox *displayStyle;
    QSpacerItem *horizontalSpacer_7;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout_13;
    QLabel *maxDictsInContextMenuLabel;
    QSpinBox *maxDictsInContextMenu;
    QLabel *label_12;
    QWidget *tab_4;
    QVBoxLayout *verticalLayout_10;
    QSpacerItem *verticalSpacer_16;
    QGroupBox *enableScanPopup;
    QVBoxLayout *verticalLayout_4;
    QCheckBox *startWithScanPopupOn;
    QCheckBox *enableScanPopupModifiers;
    QFrame *scanPopupModifiers;
    QVBoxLayout *verticalLayout_5;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QGridLayout *gridLayout;
    QCheckBox *leftCtrl;
    QCheckBox *rightShift;
    QCheckBox *altKey;
    QCheckBox *ctrlKey;
    QCheckBox *leftAlt;
    QCheckBox *shiftKey;
    QCheckBox *rightAlt;
    QCheckBox *rightCtrl;
    QCheckBox *leftShift;
    QCheckBox *winKey;
    QFrame *frame;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_5;
    QCheckBox *scanPopupAltMode;
    QSpinBox *scanPopupAltModeSecs;
    QLabel *label_7;
    QSpacerItem *horizontalSpacer_5;
    QCheckBox *scanToMainWindow;
    QSpacerItem *verticalSpacer_6;
    QWidget *tab_5;
    QVBoxLayout *verticalLayout_12;
    QSpacerItem *verticalSpacer_11;
    QCheckBox *enableMainWindowHotkey;
    QHBoxLayout *horizontalLayout_7;
    QSpacerItem *horizontalSpacer_2;
    HotKeyEdit *mainWindowHotkey;
    QCheckBox *enableClipboardHotkey;
    QHBoxLayout *horizontalLayout_8;
    QSpacerItem *horizontalSpacer_4;
    HotKeyEdit *clipboardHotkey;
    QSpacerItem *verticalSpacer_13;
    QLabel *label_8;
    QLabel *brokenXRECORD;
    QSpacerItem *verticalSpacer_12;
    QWidget *tab_2;
    QVBoxLayout *verticalLayout_7;
    QSpacerItem *verticalSpacer_4;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_15;
    QCheckBox *pronounceOnLoadMain;
    QCheckBox *pronounceOnLoadPopup;
    QGroupBox *groupBox_4;
    QVBoxLayout *verticalLayout_16;
    QRadioButton *useInternalPlayer;
    QHBoxLayout *horizontalLayout_12;
    QRadioButton *useExternalPlayer;
    QLineEdit *audioPlaybackProgram;
    QSpacerItem *verticalSpacer_2;
    QWidget *tab_3;
    QVBoxLayout *verticalLayout_9;
    QSpacerItem *verticalSpacer_5;
    QGroupBox *useProxyServer;
    QVBoxLayout *verticalLayout_6;
    QRadioButton *systemProxy;
    QRadioButton *customProxy;
    QGroupBox *customSettingsGroup;
    QVBoxLayout *verticalLayout_18;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label;
    QComboBox *proxyType;
    QLabel *label_3;
    QLineEdit *proxyHost;
    QLabel *label_2;
    QSpinBox *proxyPort;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_4;
    QLineEdit *proxyUser;
    QLabel *label_5;
    QLineEdit *proxyPassword;
    QSpacerItem *verticalSpacer_14;
    QCheckBox *disallowContentFromOtherSites;
    QCheckBox *enableWebPlugins;
    QCheckBox *hideGoldenDictHeader;
    QSpacerItem *verticalSpacer_10;
    QCheckBox *checkForNewReleases;
    QSpacerItem *verticalSpacer_3;
    QWidget *tab_FTS;
    QVBoxLayout *verticalLayout_19;
    QSpacerItem *verticalSpacer_9;
    QGroupBox *ftsGroupBox;
    QGridLayout *gridLayout_4;
    QCheckBox *allowAard;
    QCheckBox *allowBGL;
    QCheckBox *allowDictD;
    QCheckBox *allowDSL;
    QCheckBox *allowMDict;
    QCheckBox *allowSDict;
    QCheckBox *allowStardict;
    QCheckBox *allowXDXF;
    QCheckBox *allowZim;
    QHBoxLayout *horizontalLayout_14;
    QLabel *label_14;
    QSpinBox *maxDictionarySize;
    QLabel *label_15;
    QSpacerItem *horizontalSpacer_10;
    QSpacerItem *verticalSpacer_7;
    QWidget *tab_Advanced;
    QVBoxLayout *verticalLayout_11;
    QGroupBox *groupBox_ScanPopupTechnologies;
    QVBoxLayout *verticalLayout_13;
    QCheckBox *scanPopupUseIAccessibleEx;
    QCheckBox *scanPopupUseUIAutomation;
    QCheckBox *scanPopupUseGDMessage;
    QGroupBox *historyBox;
    QVBoxLayout *verticalLayout_14;
    QCheckBox *storeHistory;
    QHBoxLayout *horizontalLayout_9;
    QLabel *historySizeLabel;
    QSpinBox *historyMaxSizeField;
    QSpacerItem *horizontalSpacer_8;
    QHBoxLayout *horizontalLayout_10;
    QLabel *label_10;
    QSpinBox *historySaveIntervalField;
    QLabel *label_11;
    QSpacerItem *horizontalSpacer_9;
    QGroupBox *groupBox_6;
    QVBoxLayout *verticalLayout_17;
    QCheckBox *alwaysExpandOptionalParts;
    QHBoxLayout *horizontalLayout_11;
    QCheckBox *collapseBigArticles;
    QSpinBox *articleSizeLimit;
    QLabel *label_13;
    QSpacerItem *horizontalSpacer_11;
    QSpacerItem *verticalSpacer_17;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *Preferences)
    {
        if (Preferences->objectName().isEmpty())
            Preferences->setObjectName(QString::fromUtf8("Preferences"));
        Preferences->resize(742, 456);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/configure.png"), QSize(), QIcon::Normal, QIcon::Off);
        Preferences->setWindowIcon(icon);
        Preferences->setModal(true);
        verticalLayout_8 = new QVBoxLayout(Preferences);
        verticalLayout_8->setObjectName(QString::fromUtf8("verticalLayout_8"));
        tabWidget = new QTabWidget(Preferences);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setIconSize(QSize(15, 15));
        tabWidget->setUsesScrollButtons(false);
        tabWidget->setDocumentMode(false);
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        gridLayout_2 = new QGridLayout(tab);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        groupBox_2 = new QGroupBox(tab);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        verticalLayout_2 = new QVBoxLayout(groupBox_2);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        cbAutostart = new QCheckBox(groupBox_2);
        cbAutostart->setObjectName(QString::fromUtf8("cbAutostart"));

        verticalLayout_2->addWidget(cbAutostart);


        gridLayout_2->addWidget(groupBox_2, 2, 1, 1, 1);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        addonStylesLabel = new QLabel(tab);
        addonStylesLabel->setObjectName(QString::fromUtf8("addonStylesLabel"));

        horizontalLayout_4->addWidget(addonStylesLabel);

        addonStyles = new StylesComboBox(tab);
        addonStyles->setObjectName(QString::fromUtf8("addonStyles"));

        horizontalLayout_4->addWidget(addonStyles);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_6);


        gridLayout_2->addLayout(horizontalLayout_4, 8, 0, 1, 2);

        enableTrayIcon = new QGroupBox(tab);
        enableTrayIcon->setObjectName(QString::fromUtf8("enableTrayIcon"));
        enableTrayIcon->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        enableTrayIcon->setFlat(false);
        enableTrayIcon->setCheckable(true);
        enableTrayIcon->setChecked(false);
        verticalLayout = new QVBoxLayout(enableTrayIcon);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        startToTray = new QCheckBox(enableTrayIcon);
        startToTray->setObjectName(QString::fromUtf8("startToTray"));

        verticalLayout->addWidget(startToTray);

        closeToTray = new QCheckBox(enableTrayIcon);
        closeToTray->setObjectName(QString::fromUtf8("closeToTray"));

        verticalLayout->addWidget(closeToTray);


        gridLayout_2->addWidget(enableTrayIcon, 2, 0, 1, 1);

        doubleClickTranslates = new QCheckBox(tab);
        doubleClickTranslates->setObjectName(QString::fromUtf8("doubleClickTranslates"));

        gridLayout_2->addWidget(doubleClickTranslates, 3, 0, 1, 1);

        escKeyHidesMainWindow = new QCheckBox(tab);
        escKeyHidesMainWindow->setObjectName(QString::fromUtf8("escKeyHidesMainWindow"));

        gridLayout_2->addWidget(escKeyHidesMainWindow, 5, 0, 1, 1);

        selectBySingleClick = new QCheckBox(tab);
        selectBySingleClick->setObjectName(QString::fromUtf8("selectBySingleClick"));

        gridLayout_2->addWidget(selectBySingleClick, 4, 0, 1, 1);

        verticalSpacer_8 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_2->addItem(verticalSpacer_8, 0, 0, 1, 2);

        groupBox = new QGroupBox(tab);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        gridLayout_5 = new QGridLayout(groupBox);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        newTabsOpenInBackground = new QCheckBox(groupBox);
        newTabsOpenInBackground->setObjectName(QString::fromUtf8("newTabsOpenInBackground"));

        gridLayout_5->addWidget(newTabsOpenInBackground, 0, 0, 1, 1);

        newTabsOpenAfterCurrentOne = new QCheckBox(groupBox);
        newTabsOpenAfterCurrentOne->setObjectName(QString::fromUtf8("newTabsOpenAfterCurrentOne"));

        gridLayout_5->addWidget(newTabsOpenAfterCurrentOne, 1, 0, 1, 1);

        hideSingleTab = new QCheckBox(groupBox);
        hideSingleTab->setObjectName(QString::fromUtf8("hideSingleTab"));

        gridLayout_5->addWidget(hideSingleTab, 0, 1, 1, 1);

        mruTabOrder = new QCheckBox(groupBox);
        mruTabOrder->setObjectName(QString::fromUtf8("mruTabOrder"));

        gridLayout_5->addWidget(mruTabOrder, 1, 1, 1, 1);


        gridLayout_2->addWidget(groupBox, 1, 0, 1, 2);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_6 = new QLabel(tab);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        horizontalLayout_6->addWidget(label_6);

        interfaceLanguage = new QComboBox(tab);
        interfaceLanguage->setObjectName(QString::fromUtf8("interfaceLanguage"));

        horizontalLayout_6->addWidget(interfaceLanguage);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_3);

        label_9 = new QLabel(tab);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        horizontalLayout_6->addWidget(label_9);

        displayStyle = new QComboBox(tab);
        displayStyle->setObjectName(QString::fromUtf8("displayStyle"));

        horizontalLayout_6->addWidget(displayStyle);

        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_7);


        gridLayout_2->addLayout(horizontalLayout_6, 6, 0, 1, 2);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_2->addItem(verticalSpacer, 10, 0, 1, 2);

        horizontalLayout_13 = new QHBoxLayout();
        horizontalLayout_13->setObjectName(QString::fromUtf8("horizontalLayout_13"));
        maxDictsInContextMenuLabel = new QLabel(tab);
        maxDictsInContextMenuLabel->setObjectName(QString::fromUtf8("maxDictsInContextMenuLabel"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(maxDictsInContextMenuLabel->sizePolicy().hasHeightForWidth());
        maxDictsInContextMenuLabel->setSizePolicy(sizePolicy);

        horizontalLayout_13->addWidget(maxDictsInContextMenuLabel);

        maxDictsInContextMenu = new QSpinBox(tab);
        maxDictsInContextMenu->setObjectName(QString::fromUtf8("maxDictsInContextMenu"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(maxDictsInContextMenu->sizePolicy().hasHeightForWidth());
        maxDictsInContextMenu->setSizePolicy(sizePolicy1);
        maxDictsInContextMenu->setMaximum(9999);
        maxDictsInContextMenu->setValue(20);

        horizontalLayout_13->addWidget(maxDictsInContextMenu);

        label_12 = new QLabel(tab);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        sizePolicy.setHeightForWidth(label_12->sizePolicy().hasHeightForWidth());
        label_12->setSizePolicy(sizePolicy);

        horizontalLayout_13->addWidget(label_12);


        gridLayout_2->addLayout(horizontalLayout_13, 3, 1, 1, 1);

        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/interface.png"), QSize(), QIcon::Normal, QIcon::Off);
        tabWidget->addTab(tab, icon1, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName(QString::fromUtf8("tab_4"));
        verticalLayout_10 = new QVBoxLayout(tab_4);
        verticalLayout_10->setObjectName(QString::fromUtf8("verticalLayout_10"));
        verticalSpacer_16 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_10->addItem(verticalSpacer_16);

        enableScanPopup = new QGroupBox(tab_4);
        enableScanPopup->setObjectName(QString::fromUtf8("enableScanPopup"));
        enableScanPopup->setCheckable(true);
        enableScanPopup->setChecked(false);
        verticalLayout_4 = new QVBoxLayout(enableScanPopup);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        startWithScanPopupOn = new QCheckBox(enableScanPopup);
        startWithScanPopupOn->setObjectName(QString::fromUtf8("startWithScanPopupOn"));

        verticalLayout_4->addWidget(startWithScanPopupOn);

        enableScanPopupModifiers = new QCheckBox(enableScanPopup);
        enableScanPopupModifiers->setObjectName(QString::fromUtf8("enableScanPopupModifiers"));

        verticalLayout_4->addWidget(enableScanPopupModifiers);

        scanPopupModifiers = new QFrame(enableScanPopup);
        scanPopupModifiers->setObjectName(QString::fromUtf8("scanPopupModifiers"));
        scanPopupModifiers->setFrameShape(QFrame::NoFrame);
        scanPopupModifiers->setFrameShadow(QFrame::Plain);
        scanPopupModifiers->setLineWidth(0);
        verticalLayout_5 = new QVBoxLayout(scanPopupModifiers);
        verticalLayout_5->setContentsMargins(0, 0, 0, 0);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        leftCtrl = new QCheckBox(scanPopupModifiers);
        leftCtrl->setObjectName(QString::fromUtf8("leftCtrl"));

        gridLayout->addWidget(leftCtrl, 1, 1, 1, 1);

        rightShift = new QCheckBox(scanPopupModifiers);
        rightShift->setObjectName(QString::fromUtf8("rightShift"));

        gridLayout->addWidget(rightShift, 2, 2, 1, 1);

        altKey = new QCheckBox(scanPopupModifiers);
        altKey->setObjectName(QString::fromUtf8("altKey"));

        gridLayout->addWidget(altKey, 0, 0, 1, 1);

        ctrlKey = new QCheckBox(scanPopupModifiers);
        ctrlKey->setObjectName(QString::fromUtf8("ctrlKey"));

        gridLayout->addWidget(ctrlKey, 0, 1, 1, 1);

        leftAlt = new QCheckBox(scanPopupModifiers);
        leftAlt->setObjectName(QString::fromUtf8("leftAlt"));

        gridLayout->addWidget(leftAlt, 1, 0, 1, 1);

        shiftKey = new QCheckBox(scanPopupModifiers);
        shiftKey->setObjectName(QString::fromUtf8("shiftKey"));

        gridLayout->addWidget(shiftKey, 0, 2, 1, 1);

        rightAlt = new QCheckBox(scanPopupModifiers);
        rightAlt->setObjectName(QString::fromUtf8("rightAlt"));

        gridLayout->addWidget(rightAlt, 2, 0, 1, 1);

        rightCtrl = new QCheckBox(scanPopupModifiers);
        rightCtrl->setObjectName(QString::fromUtf8("rightCtrl"));

        gridLayout->addWidget(rightCtrl, 2, 1, 1, 1);

        leftShift = new QCheckBox(scanPopupModifiers);
        leftShift->setObjectName(QString::fromUtf8("leftShift"));

        gridLayout->addWidget(leftShift, 1, 2, 1, 1);

        winKey = new QCheckBox(scanPopupModifiers);
        winKey->setObjectName(QString::fromUtf8("winKey"));

        gridLayout->addWidget(winKey, 0, 3, 1, 1);


        horizontalLayout->addLayout(gridLayout);

        frame = new QFrame(scanPopupModifiers);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::NoFrame);
        frame->setFrameShadow(QFrame::Raised);
        frame->setLineWidth(0);
        verticalLayout_3 = new QVBoxLayout(frame);
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));

        horizontalLayout->addWidget(frame);


        verticalLayout_5->addLayout(horizontalLayout);


        verticalLayout_4->addWidget(scanPopupModifiers);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        scanPopupAltMode = new QCheckBox(enableScanPopup);
        scanPopupAltMode->setObjectName(QString::fromUtf8("scanPopupAltMode"));

        horizontalLayout_5->addWidget(scanPopupAltMode);

        scanPopupAltModeSecs = new QSpinBox(enableScanPopup);
        scanPopupAltModeSecs->setObjectName(QString::fromUtf8("scanPopupAltModeSecs"));
        scanPopupAltModeSecs->setWrapping(false);
        scanPopupAltModeSecs->setFrame(true);
        scanPopupAltModeSecs->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        scanPopupAltModeSecs->setMinimum(1);
        scanPopupAltModeSecs->setMaximum(99);

        horizontalLayout_5->addWidget(scanPopupAltModeSecs);

        label_7 = new QLabel(enableScanPopup);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        horizontalLayout_5->addWidget(label_7);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_5);


        verticalLayout_4->addLayout(horizontalLayout_5);

        scanToMainWindow = new QCheckBox(enableScanPopup);
        scanToMainWindow->setObjectName(QString::fromUtf8("scanToMainWindow"));

        verticalLayout_4->addWidget(scanToMainWindow);


        verticalLayout_10->addWidget(enableScanPopup);

        verticalSpacer_6 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_10->addItem(verticalSpacer_6);

        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/wizard.png"), QSize(), QIcon::Normal, QIcon::Off);
        tabWidget->addTab(tab_4, icon2, QString());
        tab_5 = new QWidget();
        tab_5->setObjectName(QString::fromUtf8("tab_5"));
        verticalLayout_12 = new QVBoxLayout(tab_5);
        verticalLayout_12->setObjectName(QString::fromUtf8("verticalLayout_12"));
        verticalSpacer_11 = new QSpacerItem(20, 19, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_12->addItem(verticalSpacer_11);

        enableMainWindowHotkey = new QCheckBox(tab_5);
        enableMainWindowHotkey->setObjectName(QString::fromUtf8("enableMainWindowHotkey"));

        verticalLayout_12->addWidget(enableMainWindowHotkey);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_2);

        mainWindowHotkey = new HotKeyEdit(tab_5);
        mainWindowHotkey->setObjectName(QString::fromUtf8("mainWindowHotkey"));
        mainWindowHotkey->setEnabled(false);

        horizontalLayout_7->addWidget(mainWindowHotkey);


        verticalLayout_12->addLayout(horizontalLayout_7);

        enableClipboardHotkey = new QCheckBox(tab_5);
        enableClipboardHotkey->setObjectName(QString::fromUtf8("enableClipboardHotkey"));

        verticalLayout_12->addWidget(enableClipboardHotkey);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_8->addItem(horizontalSpacer_4);

        clipboardHotkey = new HotKeyEdit(tab_5);
        clipboardHotkey->setObjectName(QString::fromUtf8("clipboardHotkey"));
        clipboardHotkey->setEnabled(false);

        horizontalLayout_8->addWidget(clipboardHotkey);


        verticalLayout_12->addLayout(horizontalLayout_8);

        verticalSpacer_13 = new QSpacerItem(20, 16, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout_12->addItem(verticalSpacer_13);

        label_8 = new QLabel(tab_5);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setWordWrap(true);

        verticalLayout_12->addWidget(label_8);

        brokenXRECORD = new QLabel(tab_5);
        brokenXRECORD->setObjectName(QString::fromUtf8("brokenXRECORD"));
        brokenXRECORD->setTextFormat(Qt::RichText);
        brokenXRECORD->setWordWrap(true);
        brokenXRECORD->setOpenExternalLinks(true);

        verticalLayout_12->addWidget(brokenXRECORD);

        verticalSpacer_12 = new QSpacerItem(20, 53, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_12->addItem(verticalSpacer_12);

        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/hotkeys.png"), QSize(), QIcon::Normal, QIcon::Off);
        tabWidget->addTab(tab_5, icon3, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        verticalLayout_7 = new QVBoxLayout(tab_2);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_7->addItem(verticalSpacer_4);

        groupBox_3 = new QGroupBox(tab_2);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        verticalLayout_15 = new QVBoxLayout(groupBox_3);
        verticalLayout_15->setObjectName(QString::fromUtf8("verticalLayout_15"));
        pronounceOnLoadMain = new QCheckBox(groupBox_3);
        pronounceOnLoadMain->setObjectName(QString::fromUtf8("pronounceOnLoadMain"));

        verticalLayout_15->addWidget(pronounceOnLoadMain);

        pronounceOnLoadPopup = new QCheckBox(groupBox_3);
        pronounceOnLoadPopup->setObjectName(QString::fromUtf8("pronounceOnLoadPopup"));

        verticalLayout_15->addWidget(pronounceOnLoadPopup);


        verticalLayout_7->addWidget(groupBox_3);

        groupBox_4 = new QGroupBox(tab_2);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        verticalLayout_16 = new QVBoxLayout(groupBox_4);
        verticalLayout_16->setObjectName(QString::fromUtf8("verticalLayout_16"));
        useInternalPlayer = new QRadioButton(groupBox_4);
        useInternalPlayer->setObjectName(QString::fromUtf8("useInternalPlayer"));

        verticalLayout_16->addWidget(useInternalPlayer);

        horizontalLayout_12 = new QHBoxLayout();
        horizontalLayout_12->setObjectName(QString::fromUtf8("horizontalLayout_12"));
        useExternalPlayer = new QRadioButton(groupBox_4);
        useExternalPlayer->setObjectName(QString::fromUtf8("useExternalPlayer"));

        horizontalLayout_12->addWidget(useExternalPlayer);

        audioPlaybackProgram = new QLineEdit(groupBox_4);
        audioPlaybackProgram->setObjectName(QString::fromUtf8("audioPlaybackProgram"));
        audioPlaybackProgram->setEnabled(false);

        horizontalLayout_12->addWidget(audioPlaybackProgram);


        verticalLayout_16->addLayout(horizontalLayout_12);


        verticalLayout_7->addWidget(groupBox_4);

        verticalSpacer_2 = new QSpacerItem(20, 46, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_7->addItem(verticalSpacer_2);

        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icons/playsound_color.png"), QSize(), QIcon::Normal, QIcon::Off);
        tabWidget->addTab(tab_2, icon4, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QString::fromUtf8("tab_3"));
        verticalLayout_9 = new QVBoxLayout(tab_3);
        verticalLayout_9->setObjectName(QString::fromUtf8("verticalLayout_9"));
        verticalSpacer_5 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_9->addItem(verticalSpacer_5);

        useProxyServer = new QGroupBox(tab_3);
        useProxyServer->setObjectName(QString::fromUtf8("useProxyServer"));
        useProxyServer->setCheckable(true);
        useProxyServer->setChecked(false);
        verticalLayout_6 = new QVBoxLayout(useProxyServer);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        systemProxy = new QRadioButton(useProxyServer);
        systemProxy->setObjectName(QString::fromUtf8("systemProxy"));

        verticalLayout_6->addWidget(systemProxy);

        customProxy = new QRadioButton(useProxyServer);
        customProxy->setObjectName(QString::fromUtf8("customProxy"));

        verticalLayout_6->addWidget(customProxy);

        customSettingsGroup = new QGroupBox(useProxyServer);
        customSettingsGroup->setObjectName(QString::fromUtf8("customSettingsGroup"));
        verticalLayout_18 = new QVBoxLayout(customSettingsGroup);
        verticalLayout_18->setObjectName(QString::fromUtf8("verticalLayout_18"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label = new QLabel(customSettingsGroup);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_3->addWidget(label);

        proxyType = new QComboBox(customSettingsGroup);
        proxyType->setObjectName(QString::fromUtf8("proxyType"));

        horizontalLayout_3->addWidget(proxyType);

        label_3 = new QLabel(customSettingsGroup);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_3->addWidget(label_3);

        proxyHost = new QLineEdit(customSettingsGroup);
        proxyHost->setObjectName(QString::fromUtf8("proxyHost"));

        horizontalLayout_3->addWidget(proxyHost);

        label_2 = new QLabel(customSettingsGroup);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_3->addWidget(label_2);

        proxyPort = new QSpinBox(customSettingsGroup);
        proxyPort->setObjectName(QString::fromUtf8("proxyPort"));
        proxyPort->setMaximum(65535);
        proxyPort->setValue(8080);

        horizontalLayout_3->addWidget(proxyPort);


        verticalLayout_18->addLayout(horizontalLayout_3);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_4 = new QLabel(customSettingsGroup);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_2->addWidget(label_4);

        proxyUser = new QLineEdit(customSettingsGroup);
        proxyUser->setObjectName(QString::fromUtf8("proxyUser"));

        horizontalLayout_2->addWidget(proxyUser);

        label_5 = new QLabel(customSettingsGroup);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        horizontalLayout_2->addWidget(label_5);

        proxyPassword = new QLineEdit(customSettingsGroup);
        proxyPassword->setObjectName(QString::fromUtf8("proxyPassword"));
        proxyPassword->setEchoMode(QLineEdit::Password);

        horizontalLayout_2->addWidget(proxyPassword);


        verticalLayout_18->addLayout(horizontalLayout_2);


        verticalLayout_6->addWidget(customSettingsGroup);


        verticalLayout_9->addWidget(useProxyServer);

        verticalSpacer_14 = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout_9->addItem(verticalSpacer_14);

        disallowContentFromOtherSites = new QCheckBox(tab_3);
        disallowContentFromOtherSites->setObjectName(QString::fromUtf8("disallowContentFromOtherSites"));

        verticalLayout_9->addWidget(disallowContentFromOtherSites);

        enableWebPlugins = new QCheckBox(tab_3);
        enableWebPlugins->setObjectName(QString::fromUtf8("enableWebPlugins"));

        verticalLayout_9->addWidget(enableWebPlugins);

        hideGoldenDictHeader = new QCheckBox(tab_3);
        hideGoldenDictHeader->setObjectName(QString::fromUtf8("hideGoldenDictHeader"));

        verticalLayout_9->addWidget(hideGoldenDictHeader);

        verticalSpacer_10 = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout_9->addItem(verticalSpacer_10);

        checkForNewReleases = new QCheckBox(tab_3);
        checkForNewReleases->setObjectName(QString::fromUtf8("checkForNewReleases"));

        verticalLayout_9->addWidget(checkForNewReleases);

        verticalSpacer_3 = new QSpacerItem(20, 39, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_9->addItem(verticalSpacer_3);

        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/icons/network.png"), QSize(), QIcon::Normal, QIcon::Off);
        tabWidget->addTab(tab_3, icon5, QString());
        tab_FTS = new QWidget();
        tab_FTS->setObjectName(QString::fromUtf8("tab_FTS"));
        verticalLayout_19 = new QVBoxLayout(tab_FTS);
        verticalLayout_19->setObjectName(QString::fromUtf8("verticalLayout_19"));
        verticalSpacer_9 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_19->addItem(verticalSpacer_9);

        ftsGroupBox = new QGroupBox(tab_FTS);
        ftsGroupBox->setObjectName(QString::fromUtf8("ftsGroupBox"));
        ftsGroupBox->setCheckable(true);
        ftsGroupBox->setChecked(true);
        gridLayout_4 = new QGridLayout(ftsGroupBox);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        allowAard = new QCheckBox(ftsGroupBox);
        allowAard->setObjectName(QString::fromUtf8("allowAard"));
        allowAard->setText(QString::fromUtf8("Aard"));

        gridLayout_4->addWidget(allowAard, 0, 0, 1, 1);

        allowBGL = new QCheckBox(ftsGroupBox);
        allowBGL->setObjectName(QString::fromUtf8("allowBGL"));
        allowBGL->setText(QString::fromUtf8("BGL"));

        gridLayout_4->addWidget(allowBGL, 1, 0, 1, 1);

        allowDictD = new QCheckBox(ftsGroupBox);
        allowDictD->setObjectName(QString::fromUtf8("allowDictD"));
        allowDictD->setText(QString::fromUtf8("DictD"));

        gridLayout_4->addWidget(allowDictD, 2, 0, 1, 1);

        allowDSL = new QCheckBox(ftsGroupBox);
        allowDSL->setObjectName(QString::fromUtf8("allowDSL"));
        allowDSL->setText(QString::fromUtf8("DSL"));

        gridLayout_4->addWidget(allowDSL, 3, 0, 1, 1);

        allowMDict = new QCheckBox(ftsGroupBox);
        allowMDict->setObjectName(QString::fromUtf8("allowMDict"));
        allowMDict->setText(QString::fromUtf8("MDict"));

        gridLayout_4->addWidget(allowMDict, 4, 0, 1, 1);

        allowSDict = new QCheckBox(ftsGroupBox);
        allowSDict->setObjectName(QString::fromUtf8("allowSDict"));
        allowSDict->setText(QString::fromUtf8("SDict"));

        gridLayout_4->addWidget(allowSDict, 0, 1, 1, 1);

        allowStardict = new QCheckBox(ftsGroupBox);
        allowStardict->setObjectName(QString::fromUtf8("allowStardict"));
        allowStardict->setText(QString::fromUtf8("Stardict"));

        gridLayout_4->addWidget(allowStardict, 1, 1, 1, 1);

        allowXDXF = new QCheckBox(ftsGroupBox);
        allowXDXF->setObjectName(QString::fromUtf8("allowXDXF"));
        allowXDXF->setText(QString::fromUtf8("XDXF"));

        gridLayout_4->addWidget(allowXDXF, 2, 1, 1, 1);

        allowZim = new QCheckBox(ftsGroupBox);
        allowZim->setObjectName(QString::fromUtf8("allowZim"));
        allowZim->setText(QString::fromUtf8("Zim"));

        gridLayout_4->addWidget(allowZim, 3, 1, 1, 1);

        horizontalLayout_14 = new QHBoxLayout();
        horizontalLayout_14->setObjectName(QString::fromUtf8("horizontalLayout_14"));
        label_14 = new QLabel(ftsGroupBox);
        label_14->setObjectName(QString::fromUtf8("label_14"));

        horizontalLayout_14->addWidget(label_14);

        maxDictionarySize = new QSpinBox(ftsGroupBox);
        maxDictionarySize->setObjectName(QString::fromUtf8("maxDictionarySize"));
        maxDictionarySize->setMaximum(10000000);
        maxDictionarySize->setSingleStep(10000);

        horizontalLayout_14->addWidget(maxDictionarySize);

        label_15 = new QLabel(ftsGroupBox);
        label_15->setObjectName(QString::fromUtf8("label_15"));

        horizontalLayout_14->addWidget(label_15);

        horizontalSpacer_10 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_14->addItem(horizontalSpacer_10);


        gridLayout_4->addLayout(horizontalLayout_14, 5, 0, 1, 2);


        verticalLayout_19->addWidget(ftsGroupBox);

        verticalSpacer_7 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_19->addItem(verticalSpacer_7);

        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/icons/system-search.png"), QSize(), QIcon::Normal, QIcon::Off);
        tabWidget->addTab(tab_FTS, icon6, QString());
        tab_Advanced = new QWidget();
        tab_Advanced->setObjectName(QString::fromUtf8("tab_Advanced"));
        verticalLayout_11 = new QVBoxLayout(tab_Advanced);
        verticalLayout_11->setObjectName(QString::fromUtf8("verticalLayout_11"));
        groupBox_ScanPopupTechnologies = new QGroupBox(tab_Advanced);
        groupBox_ScanPopupTechnologies->setObjectName(QString::fromUtf8("groupBox_ScanPopupTechnologies"));
        verticalLayout_13 = new QVBoxLayout(groupBox_ScanPopupTechnologies);
        verticalLayout_13->setObjectName(QString::fromUtf8("verticalLayout_13"));
        scanPopupUseIAccessibleEx = new QCheckBox(groupBox_ScanPopupTechnologies);
        scanPopupUseIAccessibleEx->setObjectName(QString::fromUtf8("scanPopupUseIAccessibleEx"));

        verticalLayout_13->addWidget(scanPopupUseIAccessibleEx);

        scanPopupUseUIAutomation = new QCheckBox(groupBox_ScanPopupTechnologies);
        scanPopupUseUIAutomation->setObjectName(QString::fromUtf8("scanPopupUseUIAutomation"));

        verticalLayout_13->addWidget(scanPopupUseUIAutomation);

        scanPopupUseGDMessage = new QCheckBox(groupBox_ScanPopupTechnologies);
        scanPopupUseGDMessage->setObjectName(QString::fromUtf8("scanPopupUseGDMessage"));

        verticalLayout_13->addWidget(scanPopupUseGDMessage);


        verticalLayout_11->addWidget(groupBox_ScanPopupTechnologies);

        historyBox = new QGroupBox(tab_Advanced);
        historyBox->setObjectName(QString::fromUtf8("historyBox"));
        verticalLayout_14 = new QVBoxLayout(historyBox);
        verticalLayout_14->setObjectName(QString::fromUtf8("verticalLayout_14"));
        storeHistory = new QCheckBox(historyBox);
        storeHistory->setObjectName(QString::fromUtf8("storeHistory"));
        storeHistory->setChecked(true);

        verticalLayout_14->addWidget(storeHistory);

        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        horizontalLayout_9->setContentsMargins(-1, 0, -1, -1);
        historySizeLabel = new QLabel(historyBox);
        historySizeLabel->setObjectName(QString::fromUtf8("historySizeLabel"));

        horizontalLayout_9->addWidget(historySizeLabel);

        historyMaxSizeField = new QSpinBox(historyBox);
        historyMaxSizeField->setObjectName(QString::fromUtf8("historyMaxSizeField"));
        historyMaxSizeField->setAccelerated(true);
        historyMaxSizeField->setMaximum(9999);
        historyMaxSizeField->setValue(1000);

        horizontalLayout_9->addWidget(historyMaxSizeField);

        horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_9->addItem(horizontalSpacer_8);


        verticalLayout_14->addLayout(horizontalLayout_9);

        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setObjectName(QString::fromUtf8("horizontalLayout_10"));
        label_10 = new QLabel(historyBox);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        horizontalLayout_10->addWidget(label_10);

        historySaveIntervalField = new QSpinBox(historyBox);
        historySaveIntervalField->setObjectName(QString::fromUtf8("historySaveIntervalField"));
        historySaveIntervalField->setMaximum(120);

        horizontalLayout_10->addWidget(historySaveIntervalField);

        label_11 = new QLabel(historyBox);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        horizontalLayout_10->addWidget(label_11);

        horizontalSpacer_9 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_10->addItem(horizontalSpacer_9);


        verticalLayout_14->addLayout(horizontalLayout_10);


        verticalLayout_11->addWidget(historyBox);

        groupBox_6 = new QGroupBox(tab_Advanced);
        groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
        verticalLayout_17 = new QVBoxLayout(groupBox_6);
        verticalLayout_17->setObjectName(QString::fromUtf8("verticalLayout_17"));
        alwaysExpandOptionalParts = new QCheckBox(groupBox_6);
        alwaysExpandOptionalParts->setObjectName(QString::fromUtf8("alwaysExpandOptionalParts"));

        verticalLayout_17->addWidget(alwaysExpandOptionalParts);

        horizontalLayout_11 = new QHBoxLayout();
        horizontalLayout_11->setObjectName(QString::fromUtf8("horizontalLayout_11"));
        collapseBigArticles = new QCheckBox(groupBox_6);
        collapseBigArticles->setObjectName(QString::fromUtf8("collapseBigArticles"));

        horizontalLayout_11->addWidget(collapseBigArticles);

        articleSizeLimit = new QSpinBox(groupBox_6);
        articleSizeLimit->setObjectName(QString::fromUtf8("articleSizeLimit"));
        articleSizeLimit->setMaximum(100000);
        articleSizeLimit->setSingleStep(50);
        articleSizeLimit->setValue(2000);

        horizontalLayout_11->addWidget(articleSizeLimit);

        label_13 = new QLabel(groupBox_6);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        horizontalLayout_11->addWidget(label_13);

        horizontalSpacer_11 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_11->addItem(horizontalSpacer_11);


        verticalLayout_17->addLayout(horizontalLayout_11);


        verticalLayout_11->addWidget(groupBox_6);

        verticalSpacer_17 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_11->addItem(verticalSpacer_17);

        tabWidget->addTab(tab_Advanced, QString());

        verticalLayout_8->addWidget(tabWidget);

        buttonBox = new QDialogButtonBox(Preferences);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout_8->addWidget(buttonBox);

        QWidget::setTabOrder(tabWidget, buttonBox);
        QWidget::setTabOrder(buttonBox, newTabsOpenInBackground);
        QWidget::setTabOrder(newTabsOpenInBackground, newTabsOpenAfterCurrentOne);
        QWidget::setTabOrder(newTabsOpenAfterCurrentOne, enableTrayIcon);
        QWidget::setTabOrder(enableTrayIcon, startToTray);
        QWidget::setTabOrder(startToTray, closeToTray);
        QWidget::setTabOrder(closeToTray, cbAutostart);
        QWidget::setTabOrder(cbAutostart, interfaceLanguage);
        QWidget::setTabOrder(interfaceLanguage, startWithScanPopupOn);
        QWidget::setTabOrder(startWithScanPopupOn, enableScanPopupModifiers);
        QWidget::setTabOrder(enableScanPopupModifiers, leftCtrl);
        QWidget::setTabOrder(leftCtrl, rightShift);
        QWidget::setTabOrder(rightShift, altKey);
        QWidget::setTabOrder(altKey, ctrlKey);
        QWidget::setTabOrder(ctrlKey, leftAlt);
        QWidget::setTabOrder(leftAlt, shiftKey);
        QWidget::setTabOrder(shiftKey, rightAlt);
        QWidget::setTabOrder(rightAlt, rightCtrl);
        QWidget::setTabOrder(rightCtrl, leftShift);
        QWidget::setTabOrder(leftShift, winKey);
        QWidget::setTabOrder(winKey, scanPopupAltMode);
        QWidget::setTabOrder(scanPopupAltMode, scanPopupAltModeSecs);
        QWidget::setTabOrder(scanPopupAltModeSecs, useProxyServer);
        QWidget::setTabOrder(useProxyServer, proxyType);
        QWidget::setTabOrder(proxyType, proxyHost);
        QWidget::setTabOrder(proxyHost, proxyPort);
        QWidget::setTabOrder(proxyPort, proxyUser);
        QWidget::setTabOrder(proxyUser, proxyPassword);

        retranslateUi(Preferences);
        QObject::connect(buttonBox, SIGNAL(accepted()), Preferences, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), Preferences, SLOT(reject()));

        tabWidget->setCurrentIndex(6);


        QMetaObject::connectSlotsByName(Preferences);
    } // setupUi

    void retranslateUi(QDialog *Preferences)
    {
        Preferences->setWindowTitle(QApplication::translate("Preferences", "Preferences", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("Preferences", "Startup", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        cbAutostart->setToolTip(QApplication::translate("Preferences", "Automatically starts GoldenDict after operation system bootup.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        cbAutostart->setText(QApplication::translate("Preferences", "Start with system", 0, QApplication::UnicodeUTF8));
        addonStylesLabel->setText(QApplication::translate("Preferences", "Add-on style:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        enableTrayIcon->setToolTip(QApplication::translate("Preferences", "When enabled, an icon appears in the sytem tray area which can be used\n"
"to open main window and perform other tasks.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        enableTrayIcon->setTitle(QApplication::translate("Preferences", "Enable system tray icon", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        startToTray->setToolTip(QApplication::translate("Preferences", "With this on, the application starts directly to system tray without showing\n"
"its main window.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        startToTray->setText(QApplication::translate("Preferences", "Start to system tray", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        closeToTray->setToolTip(QApplication::translate("Preferences", "With this on, an attempt to close main window would hide it instead of closing\n"
"the application.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        closeToTray->setText(QApplication::translate("Preferences", "Close to system tray", 0, QApplication::UnicodeUTF8));
        doubleClickTranslates->setText(QApplication::translate("Preferences", "Double-click translates the word clicked", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        escKeyHidesMainWindow->setToolTip(QApplication::translate("Preferences", "Normally, pressing ESC key moves focus to the translation line.\n"
"With this on however, it will hide the main window.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        escKeyHidesMainWindow->setText(QApplication::translate("Preferences", "ESC key hides main window", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        selectBySingleClick->setToolTip(QApplication::translate("Preferences", "Turn this option on if you want to select words by single mouse click", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        selectBySingleClick->setText(QApplication::translate("Preferences", "Select word by single click", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("Preferences", "Tabbed browsing", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        newTabsOpenInBackground->setToolTip(QApplication::translate("Preferences", "Normally, opening a new tab switches to it immediately.\n"
"With this on however, new tabs will be opened without\n"
"switching to them.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        newTabsOpenInBackground->setText(QApplication::translate("Preferences", "Open new tabs in background", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        newTabsOpenAfterCurrentOne->setToolTip(QApplication::translate("Preferences", "With this on, new tabs are opened just after the\n"
"current, active one. Otherwise they are added to\n"
"be the last ones.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        newTabsOpenAfterCurrentOne->setText(QApplication::translate("Preferences", "Open new tabs after the current one", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        hideSingleTab->setToolTip(QApplication::translate("Preferences", "Select this option if you don't want to see the main tab bar when only a single tab is opened.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        hideSingleTab->setText(QApplication::translate("Preferences", "Hide single tab", 0, QApplication::UnicodeUTF8));
        mruTabOrder->setText(QApplication::translate("Preferences", "Ctrl-Tab navigates tabs in MRU order", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("Preferences", "Interface language:", 0, QApplication::UnicodeUTF8));
        label_9->setText(QApplication::translate("Preferences", "Display style:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        maxDictsInContextMenuLabel->setToolTip(QApplication::translate("Preferences", "Adjust this value to avoid huge context menus.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        maxDictsInContextMenuLabel->setText(QApplication::translate("Preferences", "Context menu dictionaries limit:", 0, QApplication::UnicodeUTF8));
        label_12->setText(QString());
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("Preferences", "&Interface", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        enableScanPopup->setToolTip(QApplication::translate("Preferences", "When enabled, a translation popup window would be shown each time\n"
"you point your mouse on any word on the screen (Windows) or select\n"
"any word with mouse (Linux). When enabled, you can switch it on and\n"
"off from main window or tray icon.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        enableScanPopup->setTitle(QApplication::translate("Preferences", "Enable scan popup functionality", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        startWithScanPopupOn->setToolTip(QApplication::translate("Preferences", "Chooses whether the scan popup mode is on by default or not. If checked,\n"
"the program would always start with the scan popup active.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        startWithScanPopupOn->setText(QApplication::translate("Preferences", "Start with scan popup turned on", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        enableScanPopupModifiers->setToolTip(QApplication::translate("Preferences", "With this enabled, the popup would only show up if all chosen keys are\n"
"in the pressed state when the word selection changes.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        enableScanPopupModifiers->setText(QApplication::translate("Preferences", "Only show popup when all selected keys are kept pressed:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        leftCtrl->setToolTip(QApplication::translate("Preferences", "Left Ctrl only", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        leftCtrl->setText(QApplication::translate("Preferences", "Left Ctrl", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        rightShift->setToolTip(QApplication::translate("Preferences", "Right Shift only", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        rightShift->setText(QApplication::translate("Preferences", "Right Shift", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        altKey->setToolTip(QApplication::translate("Preferences", "Alt key", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        altKey->setText(QApplication::translate("Preferences", "Alt", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        ctrlKey->setToolTip(QApplication::translate("Preferences", "Ctrl key", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        ctrlKey->setText(QApplication::translate("Preferences", "Ctrl", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        leftAlt->setToolTip(QApplication::translate("Preferences", "Left Alt only", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        leftAlt->setText(QApplication::translate("Preferences", "Left Alt", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        shiftKey->setToolTip(QApplication::translate("Preferences", "Shift key", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        shiftKey->setText(QApplication::translate("Preferences", "Shift", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        rightAlt->setToolTip(QApplication::translate("Preferences", "Right Alt only", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        rightAlt->setText(QApplication::translate("Preferences", "Right Alt", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        rightCtrl->setToolTip(QApplication::translate("Preferences", "Right Ctrl only", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        rightCtrl->setText(QApplication::translate("Preferences", "Right Ctrl", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        leftShift->setToolTip(QApplication::translate("Preferences", "Left Shift only", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        leftShift->setText(QApplication::translate("Preferences", "Left Shift", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        winKey->setToolTip(QApplication::translate("Preferences", "Windows key or Meta key", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        winKey->setText(QApplication::translate("Preferences", "Win/Meta", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        scanPopupAltMode->setToolTip(QApplication::translate("Preferences", "Normally, in order to activate a popup you have to\n"
"maintain the chosen keys pressed while you select\n"
"a word. With this enabled, the chosen keys may also\n"
"be pressed shortly after the selection is done.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        scanPopupAltMode->setText(QApplication::translate("Preferences", "Keys may also be pressed afterwards, within", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        scanPopupAltModeSecs->setToolTip(QApplication::translate("Preferences", "To avoid false positives, the keys are only monitored\n"
"after the selection's done for a limited amount of\n"
"seconds, which is specified here.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_7->setText(QApplication::translate("Preferences", "secs", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        scanToMainWindow->setToolTip(QApplication::translate("Preferences", "Send translated word to main window instead of to show it in popup window", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        scanToMainWindow->setText(QApplication::translate("Preferences", "Send translated word to main window", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_4), QApplication::translate("Preferences", "&Scan Popup", 0, QApplication::UnicodeUTF8));
        enableMainWindowHotkey->setText(QApplication::translate("Preferences", "Use the following hotkey to show or hide the main window:", 0, QApplication::UnicodeUTF8));
        enableClipboardHotkey->setText(QApplication::translate("Preferences", "Use the following hotkey to translate a word from clipboard:", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("Preferences", "The hotkeys are global and work from any program and within any context as long as GoldenDict is running in background.", 0, QApplication::UnicodeUTF8));
        brokenXRECORD->setText(QApplication::translate("Preferences", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'DejaVu Sans'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" color:#ff0000;\">Note: You appear to be running an X.Org XServer release which has the RECORD extension broken. Hotkeys in GoldenDict will probably not work. This must be fixed in the server itself. Please refer to the following </span><a href=\"https://bugs.freedesktop.org/show_bug.cgi?id=20500\"><span style=\" text-decoration: underline; color:#0000ff;\">bug entry</span></a><span style=\" color:#ff0000;\"> and leave a comment there if you like.</span></p></body></html>", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_5), QApplication::translate("Preferences", "Hotkeys", 0, QApplication::UnicodeUTF8));
        groupBox_3->setTitle(QApplication::translate("Preferences", "Pronunciation", 0, QApplication::UnicodeUTF8));
        pronounceOnLoadMain->setText(QApplication::translate("Preferences", "Auto-pronounce words in main window", 0, QApplication::UnicodeUTF8));
        pronounceOnLoadPopup->setText(QApplication::translate("Preferences", "Auto-pronounce words in scan popup", 0, QApplication::UnicodeUTF8));
        groupBox_4->setTitle(QApplication::translate("Preferences", "Playback", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        useInternalPlayer->setToolTip(QApplication::translate("Preferences", "Play audio files via FFmpeg(libav) and libao", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        useInternalPlayer->setText(QApplication::translate("Preferences", "Use internal player", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        useExternalPlayer->setToolTip(QApplication::translate("Preferences", "Use any external program to play audio files", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        useExternalPlayer->setText(QApplication::translate("Preferences", "Use external program:", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("Preferences", "&Audio", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        useProxyServer->setToolTip(QApplication::translate("Preferences", "Enable if you wish to use a proxy server\n"
"for all program's network requests.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        useProxyServer->setTitle(QApplication::translate("Preferences", "Use proxy server", 0, QApplication::UnicodeUTF8));
        systemProxy->setText(QApplication::translate("Preferences", "System proxy", 0, QApplication::UnicodeUTF8));
        customProxy->setText(QApplication::translate("Preferences", "Custom proxy", 0, QApplication::UnicodeUTF8));
        customSettingsGroup->setTitle(QApplication::translate("Preferences", "Custom settings", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Preferences", "Type:", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("Preferences", "Host:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("Preferences", "Port:", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("Preferences", "User:", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("Preferences", "Password:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        disallowContentFromOtherSites->setToolTip(QApplication::translate("Preferences", "Enabling this would make GoldenDict block most advertisements\n"
"by disallowing content (images, frames) not originating from the site\n"
"you are browsing. If some site breaks because of this, try disabling this.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        disallowContentFromOtherSites->setText(QApplication::translate("Preferences", "Disallow loading content from other sites (hides most advertisements)", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        enableWebPlugins->setToolTip(QApplication::translate("Preferences", "Enabling this would allow to listen to sound pronunciations from\n"
"online dictionaries that rely on Flash or other web plugins.\n"
"Plugin must be installed for this option to work.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        enableWebPlugins->setText(QApplication::translate("Preferences", "Enable web plugins", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        hideGoldenDictHeader->setToolTip(QApplication::translate("Preferences", "Some sites detect GoldenDict via HTTP headers and block the requests.\n"
"Enable this option to workaround the problem.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        hideGoldenDictHeader->setText(QApplication::translate("Preferences", "Do not identify GoldenDict in HTTP headers", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        checkForNewReleases->setToolTip(QApplication::translate("Preferences", "When this is enabled, the program periodically\n"
"checks if a new, updated version of GoldenDict\n"
"is available for download. If it is so, the program\n"
"informs the user about it and prompts to open a\n"
"download page.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        checkForNewReleases->setText(QApplication::translate("Preferences", "Check for new program releases periodically", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QApplication::translate("Preferences", "&Network", 0, QApplication::UnicodeUTF8));
        ftsGroupBox->setTitle(QApplication::translate("Preferences", "Allow full-text search for:", 0, QApplication::UnicodeUTF8));
        label_14->setText(QApplication::translate("Preferences", "Don't search in dictionaries contains more than", 0, QApplication::UnicodeUTF8));
        label_15->setText(QApplication::translate("Preferences", "articles (0 - unlimited)", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_FTS), QApplication::translate("Preferences", "Full-text search", 0, QApplication::UnicodeUTF8));
        groupBox_ScanPopupTechnologies->setTitle(QApplication::translate("Preferences", "ScanPopup extra technologies", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        scanPopupUseIAccessibleEx->setToolTip(QApplication::translate("Preferences", "Try to use IAccessibleEx technology to retrieve word under cursor.\n"
"This technology works only with some programs that support it\n"
" (for example Internet Explorer 9).\n"
"It is not needed to select this option if you don't use such programs.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        scanPopupUseIAccessibleEx->setText(QApplication::translate("Preferences", "Use &IAccessibleEx", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        scanPopupUseUIAutomation->setToolTip(QApplication::translate("Preferences", "Try to use UI Automation technology to retrieve word under cursor.\n"
"This technology works only with some programs that support it.\n"
"It is not needed to select this option if you don't use such programs.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        scanPopupUseUIAutomation->setText(QApplication::translate("Preferences", "Use &UIAutomation", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        scanPopupUseGDMessage->setToolTip(QApplication::translate("Preferences", "Try to use special GoldenDict message to retrieve word under cursor.\n"
"This technology works only with some programs that support it.\n"
"It is not needed to select this option if you don't use such programs.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        scanPopupUseGDMessage->setText(QApplication::translate("Preferences", "Use &GoldenDict message", 0, QApplication::UnicodeUTF8));
        historyBox->setTitle(QApplication::translate("Preferences", "History", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        storeHistory->setToolTip(QApplication::translate("Preferences", "Turn this option on to store history of the translated words", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        storeHistory->setText(QApplication::translate("Preferences", "Store &history", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        historySizeLabel->setToolTip(QApplication::translate("Preferences", "Specify the maximum number of entries to keep in history.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        historySizeLabel->setText(QApplication::translate("Preferences", "Maximum history size:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        label_10->setToolTip(QApplication::translate("Preferences", "History saving interval. If set to 0 history will be saved only during exit.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        label_10->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
        label_10->setText(QApplication::translate("Preferences", "Save every", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_STATUSTIP
        historySaveIntervalField->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
        label_11->setText(QApplication::translate("Preferences", "minutes", 0, QApplication::UnicodeUTF8));
        groupBox_6->setTitle(QApplication::translate("Preferences", "Articles", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        alwaysExpandOptionalParts->setToolTip(QApplication::translate("Preferences", "Turn this option on to always expand optional parts of articles", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        alwaysExpandOptionalParts->setText(QApplication::translate("Preferences", "Expand optional &parts", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        collapseBigArticles->setToolTip(QApplication::translate("Preferences", "Select this option to automatic collapse big articles", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        collapseBigArticles->setText(QApplication::translate("Preferences", "Collapse articles more than", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        articleSizeLimit->setToolTip(QApplication::translate("Preferences", "Articles longer than this size will be collapsed", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_13->setText(QApplication::translate("Preferences", "symbols", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_Advanced), QApplication::translate("Preferences", "Ad&vanced", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Preferences: public Ui_Preferences {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PREFERENCES_H
