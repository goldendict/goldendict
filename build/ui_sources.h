/********************************************************************************
** Form generated from reading UI file 'sources.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SOURCES_H
#define UI_SOURCES_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Sources
{
public:
    QVBoxLayout *verticalLayout_5;
    QTabWidget *tabWidget;
    QWidget *filesTab;
    QVBoxLayout *verticalLayout_2;
    QLabel *label;
    QHBoxLayout *horizontalLayout;
    QTreeView *paths;
    QVBoxLayout *verticalLayout;
    QPushButton *addPath;
    QPushButton *removePath;
    QSpacerItem *verticalSpacer;
    QPushButton *rescan;
    QWidget *tab;
    QVBoxLayout *verticalLayout_7;
    QLabel *label_3;
    QHBoxLayout *horizontalLayout_3;
    QTreeView *soundDirs;
    QVBoxLayout *verticalLayout_6;
    QPushButton *addSoundDir;
    QPushButton *removeSoundDir;
    QSpacerItem *verticalSpacer_3;
    QWidget *tab_2;
    QVBoxLayout *verticalLayout_8;
    QLabel *label_4;
    QHBoxLayout *horizontalLayout_4;
    QLineEdit *hunspellPath;
    QPushButton *changeHunspellPath;
    QLabel *label_5;
    QHBoxLayout *horizontalLayout_5;
    QTreeView *hunspellDictionaries;
    QLabel *label_6;
    QWidget *mediaWikisTab;
    QVBoxLayout *verticalLayout_4;
    QLabel *label_2;
    QHBoxLayout *horizontalLayout_2;
    QTreeView *mediaWikis;
    QVBoxLayout *verticalLayout_3;
    QPushButton *addMediaWiki;
    QPushButton *removeMediaWiki;
    QSpacerItem *verticalSpacer_2;
    QWidget *tab_4;
    QVBoxLayout *verticalLayout_11;
    QLabel *label_9;
    QHBoxLayout *horizontalLayout_6;
    QTreeView *webSites;
    QVBoxLayout *verticalLayout_10;
    QPushButton *addWebSite;
    QPushButton *removeWebSite;
    QSpacerItem *verticalSpacer_8;
    QLabel *label_10;
    QWidget *tab_6;
    QVBoxLayout *verticalLayout_15;
    QLabel *label_16;
    QHBoxLayout *horizontalLayout_7;
    QTreeView *programs;
    QVBoxLayout *verticalLayout_14;
    QPushButton *addProgram;
    QPushButton *removeProgram;
    QSpacerItem *verticalSpacer_12;
    QWidget *tab_5;
    QVBoxLayout *verticalLayout_13;
    QLabel *label_11;
    QGroupBox *forvoEnabled;
    QVBoxLayout *verticalLayout_12;
    QSpacerItem *verticalSpacer_9;
    QFormLayout *formLayout;
    QLabel *label_12;
    QLineEdit *forvoApiKey;
    QSpacerItem *horizontalSpacer_2;
    QLabel *label_13;
    QLabel *label_14;
    QLineEdit *forvoLanguageCodes;
    QSpacerItem *horizontalSpacer;
    QLabel *label_15;
    QSpacerItem *verticalSpacer_11;
    QSpacerItem *verticalSpacer_10;
    QWidget *tab_3;
    QVBoxLayout *verticalLayout_9;
    QSpacerItem *verticalSpacer_5;
    QGridLayout *gridLayout_2;
    QCheckBox *enableRussianTransliteration;
    QCheckBox *enableGreekTransliteration;
    QSpacerItem *verticalSpacer_6;
    QCheckBox *enableGermanTransliteration;
    QCheckBox *enableBelarusianTransliteration;
    QSpacerItem *verticalSpacer_7;
    QGroupBox *enableRomaji;
    QGridLayout *gridLayout;
    QLabel *label_7;
    QCheckBox *enableHepburn;
    QCheckBox *enableNihonShiki;
    QCheckBox *enableKunreiShiki;
    QLabel *label_8;
    QCheckBox *enableHiragana;
    QCheckBox *enableKatakana;
    QSpacerItem *verticalSpacer_4;

    void setupUi(QWidget *Sources)
    {
        if (Sources->objectName().isEmpty())
            Sources->setObjectName(QStringLiteral("Sources"));
        Sources->resize(690, 336);
        Sources->setWindowTitle(QStringLiteral("Sources"));
        verticalLayout_5 = new QVBoxLayout(Sources);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        tabWidget = new QTabWidget(Sources);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tabWidget->setIconSize(QSize(15, 15));
        tabWidget->setUsesScrollButtons(false);
        filesTab = new QWidget();
        filesTab->setObjectName(QStringLiteral("filesTab"));
        verticalLayout_2 = new QVBoxLayout(filesTab);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        label = new QLabel(filesTab);
        label->setObjectName(QStringLiteral("label"));

        verticalLayout_2->addWidget(label);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        paths = new QTreeView(filesTab);
        paths->setObjectName(QStringLiteral("paths"));

        horizontalLayout->addWidget(paths);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        addPath = new QPushButton(filesTab);
        addPath->setObjectName(QStringLiteral("addPath"));

        verticalLayout->addWidget(addPath);

        removePath = new QPushButton(filesTab);
        removePath->setObjectName(QStringLiteral("removePath"));

        verticalLayout->addWidget(removePath);

        verticalSpacer = new QSpacerItem(17, 68, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        rescan = new QPushButton(filesTab);
        rescan->setObjectName(QStringLiteral("rescan"));

        verticalLayout->addWidget(rescan);


        horizontalLayout->addLayout(verticalLayout);


        verticalLayout_2->addLayout(horizontalLayout);

        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/filesave.png"), QSize(), QIcon::Normal, QIcon::Off);
        tabWidget->addTab(filesTab, icon, QString());
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        verticalLayout_7 = new QVBoxLayout(tab);
        verticalLayout_7->setObjectName(QStringLiteral("verticalLayout_7"));
        label_3 = new QLabel(tab);
        label_3->setObjectName(QStringLiteral("label_3"));

        verticalLayout_7->addWidget(label_3);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        soundDirs = new QTreeView(tab);
        soundDirs->setObjectName(QStringLiteral("soundDirs"));

        horizontalLayout_3->addWidget(soundDirs);

        verticalLayout_6 = new QVBoxLayout();
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        addSoundDir = new QPushButton(tab);
        addSoundDir->setObjectName(QStringLiteral("addSoundDir"));

        verticalLayout_6->addWidget(addSoundDir);

        removeSoundDir = new QPushButton(tab);
        removeSoundDir->setObjectName(QStringLiteral("removeSoundDir"));

        verticalLayout_6->addWidget(removeSoundDir);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_6->addItem(verticalSpacer_3);


        horizontalLayout_3->addLayout(verticalLayout_6);


        verticalLayout_7->addLayout(horizontalLayout_3);

        QIcon icon1;
        icon1.addFile(QStringLiteral(":/icons/fileopen.png"), QSize(), QIcon::Normal, QIcon::Off);
        tabWidget->addTab(tab, icon1, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        verticalLayout_8 = new QVBoxLayout(tab_2);
        verticalLayout_8->setObjectName(QStringLiteral("verticalLayout_8"));
        label_4 = new QLabel(tab_2);
        label_4->setObjectName(QStringLiteral("label_4"));

        verticalLayout_8->addWidget(label_4);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        hunspellPath = new QLineEdit(tab_2);
        hunspellPath->setObjectName(QStringLiteral("hunspellPath"));
        hunspellPath->setReadOnly(true);

        horizontalLayout_4->addWidget(hunspellPath);

        changeHunspellPath = new QPushButton(tab_2);
        changeHunspellPath->setObjectName(QStringLiteral("changeHunspellPath"));

        horizontalLayout_4->addWidget(changeHunspellPath);


        verticalLayout_8->addLayout(horizontalLayout_4);

        label_5 = new QLabel(tab_2);
        label_5->setObjectName(QStringLiteral("label_5"));

        verticalLayout_8->addWidget(label_5);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        hunspellDictionaries = new QTreeView(tab_2);
        hunspellDictionaries->setObjectName(QStringLiteral("hunspellDictionaries"));

        horizontalLayout_5->addWidget(hunspellDictionaries);

        label_6 = new QLabel(tab_2);
        label_6->setObjectName(QStringLiteral("label_6"));
        label_6->setTextFormat(Qt::PlainText);
        label_6->setWordWrap(false);

        horizontalLayout_5->addWidget(label_6);


        verticalLayout_8->addLayout(horizontalLayout_5);

        QIcon icon2;
        icon2.addFile(QStringLiteral(":/icons/icon32_hunspell.png"), QSize(), QIcon::Normal, QIcon::Off);
        tabWidget->addTab(tab_2, icon2, QString());
        mediaWikisTab = new QWidget();
        mediaWikisTab->setObjectName(QStringLiteral("mediaWikisTab"));
        verticalLayout_4 = new QVBoxLayout(mediaWikisTab);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        label_2 = new QLabel(mediaWikisTab);
        label_2->setObjectName(QStringLiteral("label_2"));

        verticalLayout_4->addWidget(label_2);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        mediaWikis = new QTreeView(mediaWikisTab);
        mediaWikis->setObjectName(QStringLiteral("mediaWikis"));

        horizontalLayout_2->addWidget(mediaWikis);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        addMediaWiki = new QPushButton(mediaWikisTab);
        addMediaWiki->setObjectName(QStringLiteral("addMediaWiki"));

        verticalLayout_3->addWidget(addMediaWiki);

        removeMediaWiki = new QPushButton(mediaWikisTab);
        removeMediaWiki->setObjectName(QStringLiteral("removeMediaWiki"));

        verticalLayout_3->addWidget(removeMediaWiki);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(verticalSpacer_2);


        horizontalLayout_2->addLayout(verticalLayout_3);


        verticalLayout_4->addLayout(horizontalLayout_2);

        QIcon icon3;
        icon3.addFile(QStringLiteral(":/icons/icon32_wiki.png"), QSize(), QIcon::Normal, QIcon::Off);
        tabWidget->addTab(mediaWikisTab, icon3, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName(QStringLiteral("tab_4"));
        verticalLayout_11 = new QVBoxLayout(tab_4);
        verticalLayout_11->setObjectName(QStringLiteral("verticalLayout_11"));
        label_9 = new QLabel(tab_4);
        label_9->setObjectName(QStringLiteral("label_9"));

        verticalLayout_11->addWidget(label_9);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        webSites = new QTreeView(tab_4);
        webSites->setObjectName(QStringLiteral("webSites"));

        horizontalLayout_6->addWidget(webSites);

        verticalLayout_10 = new QVBoxLayout();
        verticalLayout_10->setObjectName(QStringLiteral("verticalLayout_10"));
        addWebSite = new QPushButton(tab_4);
        addWebSite->setObjectName(QStringLiteral("addWebSite"));

        verticalLayout_10->addWidget(addWebSite);

        removeWebSite = new QPushButton(tab_4);
        removeWebSite->setObjectName(QStringLiteral("removeWebSite"));

        verticalLayout_10->addWidget(removeWebSite);

        verticalSpacer_8 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_10->addItem(verticalSpacer_8);


        horizontalLayout_6->addLayout(verticalLayout_10);


        verticalLayout_11->addLayout(horizontalLayout_6);

        label_10 = new QLabel(tab_4);
        label_10->setObjectName(QStringLiteral("label_10"));

        verticalLayout_11->addWidget(label_10);

        QIcon icon4;
        icon4.addFile(QStringLiteral(":/icons/internet.png"), QSize(), QIcon::Normal, QIcon::Off);
        tabWidget->addTab(tab_4, icon4, QString());
        tab_6 = new QWidget();
        tab_6->setObjectName(QStringLiteral("tab_6"));
        verticalLayout_15 = new QVBoxLayout(tab_6);
        verticalLayout_15->setObjectName(QStringLiteral("verticalLayout_15"));
        label_16 = new QLabel(tab_6);
        label_16->setObjectName(QStringLiteral("label_16"));
        label_16->setWordWrap(true);

        verticalLayout_15->addWidget(label_16);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
        programs = new QTreeView(tab_6);
        programs->setObjectName(QStringLiteral("programs"));

        horizontalLayout_7->addWidget(programs);

        verticalLayout_14 = new QVBoxLayout();
        verticalLayout_14->setObjectName(QStringLiteral("verticalLayout_14"));
        addProgram = new QPushButton(tab_6);
        addProgram->setObjectName(QStringLiteral("addProgram"));

        verticalLayout_14->addWidget(addProgram);

        removeProgram = new QPushButton(tab_6);
        removeProgram->setObjectName(QStringLiteral("removeProgram"));

        verticalLayout_14->addWidget(removeProgram);

        verticalSpacer_12 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_14->addItem(verticalSpacer_12);


        horizontalLayout_7->addLayout(verticalLayout_14);


        verticalLayout_15->addLayout(horizontalLayout_7);

        QIcon icon5;
        icon5.addFile(QStringLiteral(":/icons/programs.png"), QSize(), QIcon::Normal, QIcon::Off);
        tabWidget->addTab(tab_6, icon5, QString());
        tab_5 = new QWidget();
        tab_5->setObjectName(QStringLiteral("tab_5"));
        verticalLayout_13 = new QVBoxLayout(tab_5);
        verticalLayout_13->setObjectName(QStringLiteral("verticalLayout_13"));
        label_11 = new QLabel(tab_5);
        label_11->setObjectName(QStringLiteral("label_11"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_11->sizePolicy().hasHeightForWidth());
        label_11->setSizePolicy(sizePolicy);
        label_11->setWordWrap(true);
        label_11->setOpenExternalLinks(true);

        verticalLayout_13->addWidget(label_11);

        forvoEnabled = new QGroupBox(tab_5);
        forvoEnabled->setObjectName(QStringLiteral("forvoEnabled"));
        forvoEnabled->setCheckable(true);
        forvoEnabled->setChecked(true);
        verticalLayout_12 = new QVBoxLayout(forvoEnabled);
        verticalLayout_12->setObjectName(QStringLiteral("verticalLayout_12"));
        verticalSpacer_9 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_12->addItem(verticalSpacer_9);

        formLayout = new QFormLayout();
        formLayout->setObjectName(QStringLiteral("formLayout"));
        formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
        label_12 = new QLabel(forvoEnabled);
        label_12->setObjectName(QStringLiteral("label_12"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label_12);

        forvoApiKey = new QLineEdit(forvoEnabled);
        forvoApiKey->setObjectName(QStringLiteral("forvoApiKey"));

        formLayout->setWidget(0, QFormLayout::FieldRole, forvoApiKey);

        horizontalSpacer_2 = new QSpacerItem(18, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        formLayout->setItem(1, QFormLayout::LabelRole, horizontalSpacer_2);

        label_13 = new QLabel(forvoEnabled);
        label_13->setObjectName(QStringLiteral("label_13"));
        label_13->setWordWrap(false);
        label_13->setOpenExternalLinks(true);

        formLayout->setWidget(1, QFormLayout::FieldRole, label_13);

        label_14 = new QLabel(forvoEnabled);
        label_14->setObjectName(QStringLiteral("label_14"));

        formLayout->setWidget(3, QFormLayout::LabelRole, label_14);

        forvoLanguageCodes = new QLineEdit(forvoEnabled);
        forvoLanguageCodes->setObjectName(QStringLiteral("forvoLanguageCodes"));

        formLayout->setWidget(3, QFormLayout::FieldRole, forvoLanguageCodes);

        horizontalSpacer = new QSpacerItem(18, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        formLayout->setItem(4, QFormLayout::LabelRole, horizontalSpacer);

        label_15 = new QLabel(forvoEnabled);
        label_15->setObjectName(QStringLiteral("label_15"));
        label_15->setOpenExternalLinks(true);

        formLayout->setWidget(4, QFormLayout::FieldRole, label_15);

        verticalSpacer_11 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        formLayout->setItem(2, QFormLayout::LabelRole, verticalSpacer_11);


        verticalLayout_12->addLayout(formLayout);

        verticalSpacer_10 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_12->addItem(verticalSpacer_10);


        verticalLayout_13->addWidget(forvoEnabled);

        QIcon icon6;
        icon6.addFile(QStringLiteral(":/icons/forvo.png"), QSize(), QIcon::Normal, QIcon::Off);
        tabWidget->addTab(tab_5, icon6, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QStringLiteral("tab_3"));
        verticalLayout_9 = new QVBoxLayout(tab_3);
        verticalLayout_9->setObjectName(QStringLiteral("verticalLayout_9"));
        verticalSpacer_5 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_9->addItem(verticalSpacer_5);

        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        enableRussianTransliteration = new QCheckBox(tab_3);
        enableRussianTransliteration->setObjectName(QStringLiteral("enableRussianTransliteration"));
        QIcon icon7;
        icon7.addFile(QStringLiteral(":/flags/ru.png"), QSize(), QIcon::Normal, QIcon::Off);
        enableRussianTransliteration->setIcon(icon7);

        gridLayout_2->addWidget(enableRussianTransliteration, 0, 0, 1, 1);

        enableGreekTransliteration = new QCheckBox(tab_3);
        enableGreekTransliteration->setObjectName(QStringLiteral("enableGreekTransliteration"));
        QIcon icon8;
        icon8.addFile(QStringLiteral(":/flags/gr.png"), QSize(), QIcon::Normal, QIcon::Off);
        enableGreekTransliteration->setIcon(icon8);

        gridLayout_2->addWidget(enableGreekTransliteration, 0, 1, 1, 1);

        verticalSpacer_6 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_2->addItem(verticalSpacer_6, 1, 0, 1, 1);

        enableGermanTransliteration = new QCheckBox(tab_3);
        enableGermanTransliteration->setObjectName(QStringLiteral("enableGermanTransliteration"));
        QIcon icon9;
        icon9.addFile(QStringLiteral(":/flags/de.png"), QSize(), QIcon::Normal, QIcon::Off);
        enableGermanTransliteration->setIcon(icon9);

        gridLayout_2->addWidget(enableGermanTransliteration, 2, 0, 1, 1);

        enableBelarusianTransliteration = new QCheckBox(tab_3);
        enableBelarusianTransliteration->setObjectName(QStringLiteral("enableBelarusianTransliteration"));
        QIcon icon10;
        icon10.addFile(QStringLiteral(":/flags/by.png"), QSize(), QIcon::Normal, QIcon::Off);
        enableBelarusianTransliteration->setIcon(icon10);

        gridLayout_2->addWidget(enableBelarusianTransliteration, 2, 1, 1, 1);


        verticalLayout_9->addLayout(gridLayout_2);

        verticalSpacer_7 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_9->addItem(verticalSpacer_7);

        enableRomaji = new QGroupBox(tab_3);
        enableRomaji->setObjectName(QStringLiteral("enableRomaji"));
        enableRomaji->setCheckable(true);
        enableRomaji->setChecked(true);
        gridLayout = new QGridLayout(enableRomaji);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label_7 = new QLabel(enableRomaji);
        label_7->setObjectName(QStringLiteral("label_7"));

        gridLayout->addWidget(label_7, 0, 0, 1, 1);

        enableHepburn = new QCheckBox(enableRomaji);
        enableHepburn->setObjectName(QStringLiteral("enableHepburn"));

        gridLayout->addWidget(enableHepburn, 0, 1, 1, 1);

        enableNihonShiki = new QCheckBox(enableRomaji);
        enableNihonShiki->setObjectName(QStringLiteral("enableNihonShiki"));
        enableNihonShiki->setEnabled(false);

        gridLayout->addWidget(enableNihonShiki, 0, 2, 1, 1);

        enableKunreiShiki = new QCheckBox(enableRomaji);
        enableKunreiShiki->setObjectName(QStringLiteral("enableKunreiShiki"));
        enableKunreiShiki->setEnabled(false);

        gridLayout->addWidget(enableKunreiShiki, 0, 3, 1, 1);

        label_8 = new QLabel(enableRomaji);
        label_8->setObjectName(QStringLiteral("label_8"));

        gridLayout->addWidget(label_8, 1, 0, 1, 1);

        enableHiragana = new QCheckBox(enableRomaji);
        enableHiragana->setObjectName(QStringLiteral("enableHiragana"));

        gridLayout->addWidget(enableHiragana, 1, 2, 1, 1);

        enableKatakana = new QCheckBox(enableRomaji);
        enableKatakana->setObjectName(QStringLiteral("enableKatakana"));

        gridLayout->addWidget(enableKatakana, 1, 3, 1, 1);


        verticalLayout_9->addWidget(enableRomaji);

        verticalSpacer_4 = new QSpacerItem(20, 80, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_9->addItem(verticalSpacer_4);

        tabWidget->addTab(tab_3, QString());

        verticalLayout_5->addWidget(tabWidget);

        QWidget::setTabOrder(enableRussianTransliteration, enableGermanTransliteration);
        QWidget::setTabOrder(enableGermanTransliteration, enableRomaji);
        QWidget::setTabOrder(enableRomaji, enableHepburn);
        QWidget::setTabOrder(enableHepburn, enableNihonShiki);
        QWidget::setTabOrder(enableNihonShiki, enableKunreiShiki);
        QWidget::setTabOrder(enableKunreiShiki, enableHiragana);
        QWidget::setTabOrder(enableHiragana, enableKatakana);
        QWidget::setTabOrder(enableKatakana, hunspellPath);
        QWidget::setTabOrder(hunspellPath, changeHunspellPath);
        QWidget::setTabOrder(changeHunspellPath, hunspellDictionaries);
        QWidget::setTabOrder(hunspellDictionaries, mediaWikis);
        QWidget::setTabOrder(mediaWikis, addMediaWiki);
        QWidget::setTabOrder(addMediaWiki, removeMediaWiki);
        QWidget::setTabOrder(removeMediaWiki, tabWidget);
        QWidget::setTabOrder(tabWidget, addPath);
        QWidget::setTabOrder(addPath, removePath);
        QWidget::setTabOrder(removePath, rescan);
        QWidget::setTabOrder(rescan, soundDirs);
        QWidget::setTabOrder(soundDirs, addSoundDir);
        QWidget::setTabOrder(addSoundDir, removeSoundDir);
        QWidget::setTabOrder(removeSoundDir, paths);

        retranslateUi(Sources);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(Sources);
    } // setupUi

    void retranslateUi(QWidget *Sources)
    {
        label->setText(QApplication::translate("Sources", "Paths to search for the dictionary files:", 0));
        addPath->setText(QApplication::translate("Sources", "&Add...", 0));
        removePath->setText(QApplication::translate("Sources", "&Remove", 0));
        rescan->setText(QApplication::translate("Sources", "Re&scan now", 0));
        tabWidget->setTabText(tabWidget->indexOf(filesTab), QApplication::translate("Sources", "Files", 0));
        label_3->setText(QApplication::translate("Sources", "Make dictionaries from bunches of audiofiles by adding paths here:", 0));
        addSoundDir->setText(QApplication::translate("Sources", "&Add...", 0));
        removeSoundDir->setText(QApplication::translate("Sources", "&Remove", 0));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("Sources", "Sound Dirs", 0));
        label_4->setText(QApplication::translate("Sources", "Path to a directory with Hunspell/Myspell dictionaries:", 0));
        changeHunspellPath->setText(QApplication::translate("Sources", "&Change...", 0));
        label_5->setText(QApplication::translate("Sources", "Available morphology dictionaries:", 0));
        label_6->setText(QApplication::translate("Sources", "Each morphology dictionary appears as a\n"
"separate auxiliary dictionary which\n"
"provides stem words for searches and\n"
"spelling suggestions for mistyped words.\n"
"Add appropriate dictionaries to the bottoms\n"
"of the appropriate groups to use them.", 0));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("Sources", "Morphology", 0));
        label_2->setText(QApplication::translate("Sources", "Wikipedia (MediaWiki) sites:", 0));
        addMediaWiki->setText(QApplication::translate("Sources", "&Add...", 0));
        removeMediaWiki->setText(QApplication::translate("Sources", "&Remove", 0));
        tabWidget->setTabText(tabWidget->indexOf(mediaWikisTab), QApplication::translate("Sources", "Wikipedia", 0));
        label_9->setText(QApplication::translate("Sources", "Any websites. A string %GDWORD% will be replaced with the query word:", 0));
        addWebSite->setText(QApplication::translate("Sources", "&Add...", 0));
        removeWebSite->setText(QApplication::translate("Sources", "&Remove", 0));
        label_10->setText(QApplication::translate("Sources", "Alternatively, use %GD1251% for CP1251, %GDISO1%...%GDISO16% for ISO 8859-1...ISO 8859-16 respectively,\n"
"%GDBIG5% for Big-5, %GDBIG5HKSCS% for Big5-HKSCS, %GDGBK% for GBK and GB18030, %GDSHIFTJIS% for Shift-JIS.", 0));
        tabWidget->setTabText(tabWidget->indexOf(tab_4), QApplication::translate("Sources", "Websites", 0));
        label_16->setText(QApplication::translate("Sources", "Any external programs. A string %GDWORD% will be replaced with the query word. If such string is not presented, the word will be fed into standard input.", 0));
        addProgram->setText(QApplication::translate("Sources", "&Add...", 0));
        removeProgram->setText(QApplication::translate("Sources", "&Remove", 0));
        tabWidget->setTabText(tabWidget->indexOf(tab_6), QApplication::translate("Sources", "Programs", 0));
        label_11->setText(QApplication::translate("Sources", "Live pronunciations from <a href=\"http://www.forvo.com/\">forvo.com</a>. The site allows people to record and share word pronunciations. You can listen to them from GoldenDict.", 0));
        forvoEnabled->setTitle(QApplication::translate("Sources", "Enable pronunciations from Forvo", 0));
        label_12->setText(QApplication::translate("Sources", "API Key:", 0));
#ifndef QT_NO_TOOLTIP
        forvoApiKey->setToolTip(QApplication::translate("Sources", "Use of Forvo currently requires an API key. Leave this field\n"
"blank to use the default key, which may become unavailable\n"
"in the future, or register on the site to get your own key.", 0));
#endif // QT_NO_TOOLTIP
        label_13->setText(QApplication::translate("Sources", "Get your own key <a href=\"http://api.forvo.com/key/\">here</a>, or leave blank to use the default one.", 0));
        label_14->setText(QApplication::translate("Sources", "Language codes (comma-separated):", 0));
#ifndef QT_NO_TOOLTIP
        forvoLanguageCodes->setToolTip(QApplication::translate("Sources", "List of language codes you would like to have. Example: \"en, ru\".", 0));
#endif // QT_NO_TOOLTIP
        label_15->setText(QApplication::translate("Sources", "Full list of language codes is available <a href=\"http://www.forvo.com/languages-codes/\">here</a>.", 0));
        tabWidget->setTabText(tabWidget->indexOf(tab_5), QApplication::translate("Sources", "Forvo", 0));
        enableRussianTransliteration->setText(QApplication::translate("Sources", "Russian transliteration", 0));
        enableGreekTransliteration->setText(QApplication::translate("Sources", "Greek transliteration", 0));
        enableGermanTransliteration->setText(QApplication::translate("Sources", "German transliteration", 0));
        enableBelarusianTransliteration->setText(QApplication::translate("Sources", "Belarusian transliteration", 0));
#ifndef QT_NO_TOOLTIP
        enableRomaji->setToolTip(QApplication::translate("Sources", "Enables to use the Latin alphabet to write the Japanese language", 0));
#endif // QT_NO_TOOLTIP
        enableRomaji->setTitle(QApplication::translate("Sources", "Japanese Romaji", 0));
        label_7->setText(QApplication::translate("Sources", "Systems:", 0));
#ifndef QT_NO_TOOLTIP
        enableHepburn->setToolTip(QApplication::translate("Sources", "The most widely used method of transcription of Japanese,\n"
"based on English phonology", 0));
#endif // QT_NO_TOOLTIP
        enableHepburn->setText(QApplication::translate("Sources", "Hepburn", 0));
#ifndef QT_NO_TOOLTIP
        enableNihonShiki->setToolTip(QApplication::translate("Sources", "The most regular system, having a one-to-one relation to the\n"
"kana writing systems. Standardized as ISO 3602\n"
"\n"
"Not implemented yet in GoldenDict.", 0));
#endif // QT_NO_TOOLTIP
        enableNihonShiki->setText(QApplication::translate("Sources", "Nihon-shiki", 0));
#ifndef QT_NO_TOOLTIP
        enableKunreiShiki->setToolTip(QApplication::translate("Sources", "Based on Nihon-shiki system, but modified for modern standard Japanese.\n"
"Standardized as ISO 3602\n"
"\n"
"Not implemented yet in GoldenDict.", 0));
#endif // QT_NO_TOOLTIP
        enableKunreiShiki->setText(QApplication::translate("Sources", "Kunrei-shiki", 0));
        label_8->setText(QApplication::translate("Sources", "Syllabaries:", 0));
#ifndef QT_NO_TOOLTIP
        enableHiragana->setToolTip(QApplication::translate("Sources", "Hiragana Japanese syllabary", 0));
#endif // QT_NO_TOOLTIP
        enableHiragana->setText(QApplication::translate("Sources", "Hiragana", 0));
#ifndef QT_NO_TOOLTIP
        enableKatakana->setToolTip(QApplication::translate("Sources", "Katakana Japanese syllabary", 0));
#endif // QT_NO_TOOLTIP
        enableKatakana->setText(QApplication::translate("Sources", "Katakana", 0));
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QApplication::translate("Sources", "Transliteration", 0));
        Q_UNUSED(Sources);
    } // retranslateUi

};

namespace Ui {
    class Sources: public Ui_Sources {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SOURCES_H
