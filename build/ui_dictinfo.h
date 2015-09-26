/********************************************************************************
** Form generated from reading UI file 'dictinfo.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DICTINFO_H
#define UI_DICTINFO_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

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
            DictInfo->setObjectName(QStringLiteral("DictInfo"));
        DictInfo->resize(600, 400);
        DictInfo->setModal(true);
        verticalLayout = new QVBoxLayout(DictInfo);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(DictInfo);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 0, 0, 1, 1);

        dictionaryTotalArticles = new QLabel(groupBox);
        dictionaryTotalArticles->setObjectName(QStringLiteral("dictionaryTotalArticles"));
        dictionaryTotalArticles->setText(QStringLiteral(""));
        dictionaryTotalArticles->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(dictionaryTotalArticles, 0, 1, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout->addWidget(label_4, 0, 2, 1, 1);

        dictionaryTranslatesFrom = new QLabel(groupBox);
        dictionaryTranslatesFrom->setObjectName(QStringLiteral("dictionaryTranslatesFrom"));
        dictionaryTranslatesFrom->setText(QStringLiteral(""));

        gridLayout->addWidget(dictionaryTranslatesFrom, 0, 3, 1, 1);

        label_6 = new QLabel(groupBox);
        label_6->setObjectName(QStringLiteral("label_6"));

        gridLayout->addWidget(label_6, 1, 0, 1, 1);

        dictionaryTotalWords = new QLabel(groupBox);
        dictionaryTotalWords->setObjectName(QStringLiteral("dictionaryTotalWords"));
        dictionaryTotalWords->setText(QStringLiteral(""));
        dictionaryTotalWords->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(dictionaryTotalWords, 1, 1, 1, 1);

        label_8 = new QLabel(groupBox);
        label_8->setObjectName(QStringLiteral("label_8"));

        gridLayout->addWidget(label_8, 1, 2, 1, 1);

        dictionaryTranslatesTo = new QLabel(groupBox);
        dictionaryTranslatesTo->setObjectName(QStringLiteral("dictionaryTranslatesTo"));
        dictionaryTranslatesTo->setText(QStringLiteral(""));

        gridLayout->addWidget(dictionaryTranslatesTo, 1, 3, 1, 1);

        openFolder = new QPushButton(groupBox);
        openFolder->setObjectName(QStringLiteral("openFolder"));
        openFolder->setAutoDefault(false);
        openFolder->setDefault(false);
        openFolder->setFlat(false);

        gridLayout->addWidget(openFolder, 0, 4, 1, 1);

        editDictionary = new QPushButton(groupBox);
        editDictionary->setObjectName(QStringLiteral("editDictionary"));
        editDictionary->setEnabled(true);

        gridLayout->addWidget(editDictionary, 1, 4, 1, 1);


        verticalLayout->addWidget(groupBox);

        label = new QLabel(DictInfo);
        label->setObjectName(QStringLiteral("label"));

        verticalLayout->addWidget(label);

        dictionaryFileList = new QPlainTextEdit(DictInfo);
        dictionaryFileList->setObjectName(QStringLiteral("dictionaryFileList"));
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
        dictionaryFileList->setToolTip(QStringLiteral(""));
#endif // QT_NO_TOOLTIP
        dictionaryFileList->setUndoRedoEnabled(false);
        dictionaryFileList->setLineWrapMode(QPlainTextEdit::NoWrap);
        dictionaryFileList->setReadOnly(true);
        dictionaryFileList->setPlainText(QStringLiteral(""));

        verticalLayout->addWidget(dictionaryFileList);

        dictionaryDescriptionLabel = new QLabel(DictInfo);
        dictionaryDescriptionLabel->setObjectName(QStringLiteral("dictionaryDescriptionLabel"));

        verticalLayout->addWidget(dictionaryDescriptionLabel);

        infoLabel = new QPlainTextEdit(DictInfo);
        infoLabel->setObjectName(QStringLiteral("infoLabel"));
        QPalette palette1;
        palette1.setBrush(QPalette::Active, QPalette::Base, brush);
        palette1.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette1.setBrush(QPalette::Disabled, QPalette::Base, brush1);
        infoLabel->setPalette(palette1);
#ifndef QT_NO_TOOLTIP
        infoLabel->setToolTip(QStringLiteral(""));
#endif // QT_NO_TOOLTIP
        infoLabel->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        infoLabel->setUndoRedoEnabled(false);
        infoLabel->setReadOnly(true);

        verticalLayout->addWidget(infoLabel);

        buttonsLayout = new QHBoxLayout();
        buttonsLayout->setObjectName(QStringLiteral("buttonsLayout"));
        headwordsButton = new QPushButton(DictInfo);
        headwordsButton->setObjectName(QStringLiteral("headwordsButton"));
        headwordsButton->setAutoDefault(false);

        buttonsLayout->addWidget(headwordsButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        buttonsLayout->addItem(horizontalSpacer);

        OKButton = new QPushButton(DictInfo);
        OKButton->setObjectName(QStringLiteral("OKButton"));
        OKButton->setText(QStringLiteral("OK"));
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
        label_2->setText(QApplication::translate("DictInfo", "Total articles:", 0));
        label_4->setText(QApplication::translate("DictInfo", "Translates from:", 0));
        label_6->setText(QApplication::translate("DictInfo", "Total words:", 0));
        label_8->setText(QApplication::translate("DictInfo", "Translates to:", 0));
        openFolder->setText(QApplication::translate("DictInfo", "Open folder", 0));
        editDictionary->setText(QApplication::translate("DictInfo", "Edit dictionary", 0));
        label->setText(QApplication::translate("DictInfo", "Files comprising this dictionary:", 0));
        dictionaryDescriptionLabel->setText(QApplication::translate("DictInfo", "Description:", 0));
#ifndef QT_NO_TOOLTIP
        headwordsButton->setToolTip(QApplication::translate("DictInfo", "Show all unique dictionary headwords", 0));
#endif // QT_NO_TOOLTIP
        headwordsButton->setText(QApplication::translate("DictInfo", "Headwords", 0));
    } // retranslateUi

};

namespace Ui {
    class DictInfo: public Ui_DictInfo {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DICTINFO_H
