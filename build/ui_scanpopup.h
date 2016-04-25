/********************************************************************************
** Form generated from reading UI file 'scanpopup.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SCANPOPUP_H
#define UI_SCANPOPUP_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "groupcombobox.hh"
#include "translatebox.hh"

QT_BEGIN_NAMESPACE

class Ui_ScanPopup
{
public:
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout_222;
    QFrame *outerFrame;
    QVBoxLayout *mainLayout;
    QHBoxLayout *horizontalLayout_3;
    QHBoxLayout *horizontalLayout;
    GroupComboBox *groupList;
    TranslateBox *translateBox;
    QHBoxLayout *horizontalLayout_2;
    QLabel *queryError;
    QToolButton *goBackButton;
    QToolButton *goForwardButton;
    QToolButton *pronounceButton;
    QToolButton *sendWordButton;
    QSpacerItem *horizontalSpacer;
    QToolButton *showDictionaryBar;
    QToolButton *pinButton;

    void setupUi(QMainWindow *ScanPopup)
    {
        if (ScanPopup->objectName().isEmpty())
            ScanPopup->setObjectName(QStringLiteral("ScanPopup"));
        ScanPopup->resize(557, 403);
        centralWidget = new QWidget(ScanPopup);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        horizontalLayout_222 = new QHBoxLayout(centralWidget);
        horizontalLayout_222->setObjectName(QStringLiteral("horizontalLayout_222"));
        horizontalLayout_222->setContentsMargins(0, 0, 0, 0);
        outerFrame = new QFrame(centralWidget);
        outerFrame->setObjectName(QStringLiteral("outerFrame"));
        outerFrame->setFrameShape(QFrame::NoFrame);
        outerFrame->setFrameShadow(QFrame::Raised);
        outerFrame->setLineWidth(0);
        mainLayout = new QVBoxLayout(outerFrame);
        mainLayout->setSpacing(3);
        mainLayout->setObjectName(QStringLiteral("mainLayout"));
        mainLayout->setContentsMargins(3, 3, 3, 3);
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        groupList = new GroupComboBox(outerFrame);
        groupList->setObjectName(QStringLiteral("groupList"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(groupList->sizePolicy().hasHeightForWidth());
        groupList->setSizePolicy(sizePolicy);
        groupList->setSizeAdjustPolicy(QComboBox::AdjustToContents);

        horizontalLayout->addWidget(groupList);

        translateBox = new TranslateBox(outerFrame);
        translateBox->setObjectName(QStringLiteral("translateBox"));
        sizePolicy.setHeightForWidth(translateBox->sizePolicy().hasHeightForWidth());
        translateBox->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(translateBox);


        horizontalLayout_3->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        queryError = new QLabel(outerFrame);
        queryError->setObjectName(QStringLiteral("queryError"));
        queryError->setEnabled(true);
        queryError->setPixmap(QPixmap(QString::fromUtf8(":/icons/warning.png")));

        horizontalLayout_2->addWidget(queryError);

        goBackButton = new QToolButton(outerFrame);
        goBackButton->setObjectName(QStringLiteral("goBackButton"));
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/previous.png"), QSize(), QIcon::Normal, QIcon::Off);
        goBackButton->setIcon(icon);

        horizontalLayout_2->addWidget(goBackButton);

        goForwardButton = new QToolButton(outerFrame);
        goForwardButton->setObjectName(QStringLiteral("goForwardButton"));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/icons/next.png"), QSize(), QIcon::Normal, QIcon::Off);
        goForwardButton->setIcon(icon1);

        horizontalLayout_2->addWidget(goForwardButton);

        pronounceButton = new QToolButton(outerFrame);
        pronounceButton->setObjectName(QStringLiteral("pronounceButton"));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/icons/playsound.png"), QSize(), QIcon::Normal, QIcon::Off);
        pronounceButton->setIcon(icon2);
        pronounceButton->setAutoRaise(false);

        horizontalLayout_2->addWidget(pronounceButton);

        sendWordButton = new QToolButton(outerFrame);
        sendWordButton->setObjectName(QStringLiteral("sendWordButton"));
        sendWordButton->setText(QStringLiteral("..."));
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/icons/programicon.png"), QSize(), QIcon::Normal, QIcon::Off);
        sendWordButton->setIcon(icon3);

        horizontalLayout_2->addWidget(sendWordButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        showDictionaryBar = new QToolButton(outerFrame);
        showDictionaryBar->setObjectName(QStringLiteral("showDictionaryBar"));
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/icons/bookcase.png"), QSize(), QIcon::Normal, QIcon::Off);
        showDictionaryBar->setIcon(icon4);
        showDictionaryBar->setCheckable(true);
        showDictionaryBar->setChecked(false);
        showDictionaryBar->setAutoRaise(true);

        horizontalLayout_2->addWidget(showDictionaryBar);

        pinButton = new QToolButton(outerFrame);
        pinButton->setObjectName(QStringLiteral("pinButton"));
        QIcon icon5;
        icon5.addFile(QStringLiteral(":/icons/pushpin.png"), QSize(), QIcon::Normal, QIcon::Off);
        pinButton->setIcon(icon5);
        pinButton->setCheckable(true);
        pinButton->setAutoRaise(true);

        horizontalLayout_2->addWidget(pinButton);


        horizontalLayout_3->addLayout(horizontalLayout_2);


        mainLayout->addLayout(horizontalLayout_3);


        horizontalLayout_222->addWidget(outerFrame);

        ScanPopup->setCentralWidget(centralWidget);

        retranslateUi(ScanPopup);

        QMetaObject::connectSlotsByName(ScanPopup);
    } // setupUi

    void retranslateUi(QMainWindow *ScanPopup)
    {
        ScanPopup->setWindowTitle(QApplication::translate("ScanPopup", "Dialog", 0));
        queryError->setText(QString());
#ifndef QT_NO_TOOLTIP
        goBackButton->setToolTip(QApplication::translate("ScanPopup", "Back", 0));
#endif // QT_NO_TOOLTIP
        goBackButton->setText(QApplication::translate("ScanPopup", "...", 0));
#ifndef QT_NO_TOOLTIP
        goForwardButton->setToolTip(QApplication::translate("ScanPopup", "Forward", 0));
#endif // QT_NO_TOOLTIP
        goForwardButton->setText(QApplication::translate("ScanPopup", "...", 0));
#ifndef QT_NO_TOOLTIP
        pronounceButton->setToolTip(QApplication::translate("ScanPopup", "Pronounce Word (Alt+S)", 0));
#endif // QT_NO_TOOLTIP
        pronounceButton->setText(QApplication::translate("ScanPopup", "...", 0));
        pronounceButton->setShortcut(QApplication::translate("ScanPopup", "Alt+S", 0));
#ifndef QT_NO_TOOLTIP
        sendWordButton->setToolTip(QApplication::translate("ScanPopup", "Send word to main window (Alt+W)", 0));
#endif // QT_NO_TOOLTIP
        sendWordButton->setShortcut(QApplication::translate("ScanPopup", "Alt+W", 0));
#ifndef QT_NO_TOOLTIP
        showDictionaryBar->setToolTip(QApplication::translate("ScanPopup", "Shows or hides the dictionary bar", 0));
#endif // QT_NO_TOOLTIP
        showDictionaryBar->setText(QApplication::translate("ScanPopup", "...", 0));
#ifndef QT_NO_TOOLTIP
        pinButton->setToolTip(QApplication::translate("ScanPopup", "Use this to pin down the window so it would stay on screen,\n"
"could be resized or managed in other ways.", 0));
#endif // QT_NO_TOOLTIP
        pinButton->setText(QApplication::translate("ScanPopup", "...", 0));
    } // retranslateUi

};

namespace Ui {
    class ScanPopup: public Ui_ScanPopup {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SCANPOPUP_H
