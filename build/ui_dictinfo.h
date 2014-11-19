/********************************************************************************
** Form generated from reading UI file 'dictinfo.ui'
**
** Created: Mon Apr 28 13:32:14 2014
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DICTINFO_H
#define UI_DICTINFO_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DictInfo
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QLabel *label_2;
    QLabel *dictionaryTotalArticles;
    QLabel *label_4;
    QLabel *dictionaryTranslatesFrom;
    QLabel *label_6;
    QLabel *dictionaryTotalWords;
    QLabel *label_8;
    QLabel *dictionaryTranslatesTo;
    QPushButton *openFolder;
    QPushButton *editDictionary;
    QLabel *label;
    QPlainTextEdit *dictionaryFileList;
    QLabel *dictionaryDescriptionLabel;
    QPlainTextEdit *infoLabel;
    QHBoxLayout *buttonsLayout;
    QPushButton *headwordsButton;
    QSpacerItem *horizontalSpacer;
    QPushButton *OKButton;
    QSpacerItem *horizontalSpacer_2;

    void setupUi(QDialog *DictInfo)
    {
        if (DictInfo->objectName().isEmpty())
            DictInfo->setObjectName(QString::fromUtf8("DictInfo"));
        DictInfo->resize(600, 400);
        DictInfo->setModal(true);
        verticalLayout = new QVBoxLayout(DictInfo);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        groupBox = new QGroupBox(DictInfo);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 0, 0, 1, 1);

        dictionaryTotalArticles = new QLabel(groupBox);
        dictionaryTotalArticles->setObjectName(QString::fromUtf8("dictionaryTotalArticles"));
        dictionaryTotalArticles->setText(QString::fromUtf8(""));
        dictionaryTotalArticles->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(dictionaryTotalArticles, 0, 1, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 0, 2, 1, 1);

        dictionaryTranslatesFrom = new QLabel(groupBox);
        dictionaryTranslatesFrom->setObjectName(QString::fromUtf8("dictionaryTranslatesFrom"));
        dictionaryTranslatesFrom->setText(QString::fromUtf8(""));

        gridLayout->addWidget(dictionaryTranslatesFrom, 0, 3, 1, 1);

        label_6 = new QLabel(groupBox);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout->addWidget(label_6, 1, 0, 1, 1);

        dictionaryTotalWords = new QLabel(groupBox);
        dictionaryTotalWords->setObjectName(QString::fromUtf8("dictionaryTotalWords"));
        dictionaryTotalWords->setText(QString::fromUtf8(""));
        dictionaryTotalWords->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(dictionaryTotalWords, 1, 1, 1, 1);

        label_8 = new QLabel(groupBox);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        gridLayout->addWidget(label_8, 1, 2, 1, 1);

        dictionaryTranslatesTo = new QLabel(groupBox);
        dictionaryTranslatesTo->setObjectName(QString::fromUtf8("dictionaryTranslatesTo"));
        dictionaryTranslatesTo->setText(QString::fromUtf8(""));

        gridLayout->addWidget(dictionaryTranslatesTo, 1, 3, 1, 1);

        openFolder = new QPushButton(groupBox);
        openFolder->setObjectName(QString::fromUtf8("openFolder"));
        openFolder->setAutoDefault(false);
        openFolder->setDefault(false);
        openFolder->setFlat(false);

        gridLayout->addWidget(openFolder, 0, 4, 1, 1);

        editDictionary = new QPushButton(groupBox);
        editDictionary->setObjectName(QString::fromUtf8("editDictionary"));
        editDictionary->setEnabled(true);

        gridLayout->addWidget(editDictionary, 1, 4, 1, 1);


        verticalLayout->addWidget(groupBox);

        label = new QLabel(DictInfo);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout->addWidget(label);

        dictionaryFileList = new QPlainTextEdit(DictInfo);
        dictionaryFileList->setObjectName(QString::fromUtf8("dictionaryFileList"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(70);
        sizePolicy.setHeightForWidth(dictionaryFileList->sizePolicy().hasHeightForWidth());
        dictionaryFileList->setSizePolicy(sizePolicy);
        dictionaryFileList->setMinimumSize(QSize(0, 70));
        dictionaryFileList->setMaximumSize(QSize(16777215, 70));
        QPalette palette;
        QBrush brush(QColor(224, 223, 223, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Base, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Base, brush);
        QBrush brush1(QColor(212, 208, 200, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Disabled, QPalette::Base, brush1);
        dictionaryFileList->setPalette(palette);
#ifndef QT_NO_TOOLTIP
        dictionaryFileList->setToolTip(QString::fromUtf8(""));
#endif // QT_NO_TOOLTIP
        dictionaryFileList->setUndoRedoEnabled(false);
        dictionaryFileList->setLineWrapMode(QPlainTextEdit::NoWrap);
        dictionaryFileList->setReadOnly(true);
        dictionaryFileList->setPlainText(QString::fromUtf8(""));

        verticalLayout->addWidget(dictionaryFileList);

        dictionaryDescriptionLabel = new QLabel(DictInfo);
        dictionaryDescriptionLabel->setObjectName(QString::fromUtf8("dictionaryDescriptionLabel"));

        verticalLayout->addWidget(dictionaryDescriptionLabel);

        infoLabel = new QPlainTextEdit(DictInfo);
        infoLabel->setObjectName(QString::fromUtf8("infoLabel"));
        QPalette palette1;
        palette1.setBrush(QPalette::Active, QPalette::Base, brush);
        palette1.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette1.setBrush(QPalette::Disabled, QPalette::Base, brush1);
        infoLabel->setPalette(palette1);
#ifndef QT_NO_TOOLTIP
        infoLabel->setToolTip(QString::fromUtf8(""));
#endif // QT_NO_TOOLTIP
        infoLabel->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        infoLabel->setUndoRedoEnabled(false);
        infoLabel->setReadOnly(true);

        verticalLayout->addWidget(infoLabel);

        buttonsLayout = new QHBoxLayout();
        buttonsLayout->setObjectName(QString::fromUtf8("buttonsLayout"));
        headwordsButton = new QPushButton(DictInfo);
        headwordsButton->setObjectName(QString::fromUtf8("headwordsButton"));
        headwordsButton->setAutoDefault(false);

        buttonsLayout->addWidget(headwordsButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        buttonsLayout->addItem(horizontalSpacer);

        OKButton = new QPushButton(DictInfo);
        OKButton->setObjectName(QString::fromUtf8("OKButton"));
        OKButton->setText(QString::fromUtf8("OK"));
        OKButton->setDefault(true);

        buttonsLayout->addWidget(OKButton);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        buttonsLayout->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(buttonsLayout);


        retranslateUi(DictInfo);

        QMetaObject::connectSlotsByName(DictInfo);
    } // setupUi

    void retranslateUi(QDialog *DictInfo)
    {
        DictInfo->setWindowTitle(QString());
        groupBox->setTitle(QString());
        label_2->setText(QApplication::translate("DictInfo", "Total articles:", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("DictInfo", "Translates from:", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("DictInfo", "Total words:", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("DictInfo", "Translates to:", 0, QApplication::UnicodeUTF8));
        openFolder->setText(QApplication::translate("DictInfo", "Open folder", 0, QApplication::UnicodeUTF8));
        editDictionary->setText(QApplication::translate("DictInfo", "Edit dictionary", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("DictInfo", "Files comprising this dictionary:", 0, QApplication::UnicodeUTF8));
        dictionaryDescriptionLabel->setText(QApplication::translate("DictInfo", "Description:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        headwordsButton->setToolTip(QApplication::translate("DictInfo", "Show all unique dictionary headwords", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        headwordsButton->setText(QApplication::translate("DictInfo", "Headwords", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DictInfo: public Ui_DictInfo {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DICTINFO_H
