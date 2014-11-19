/********************************************************************************
** Form generated from reading UI file 'orderandprops.ui'
**
** Created: Wed Nov 19 16:01:34 2014
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ORDERANDPROPS_H
#define UI_ORDERANDPROPS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QSpacerItem>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "groups_widgets.hh"

QT_BEGIN_NAMESPACE

class Ui_OrderAndProps
{
public:
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout_2;
    QVBoxLayout *verticalLayout_3;
    QGridLayout *gridLayout_2;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QToolButton *moveToActive;
    QToolButton *moveToInactive;
    QSpacerItem *horizontalSpacer_2;
    QLabel *label;
    DictListWidget *dictionaryOrder;
    QVBoxLayout *verticalLayout_2;
    QSpacerItem *verticalSpacer;
    QToolButton *moveActiveUp;
    QToolButton *moveActiveDown;
    QSpacerItem *verticalSpacer_2;
    DictListWidget *inactiveDictionaries;
    QLabel *label_2;
    QuickFilterLine *searchLine;
    QGroupBox *dictionaryInformation;
    QVBoxLayout *verticalLayout_5;
    QGridLayout *gridLayout;
    QLabel *label_5;
    QLabel *dictionaryName;
    QLabel *dictionaryTotalArticles;
    QLabel *label_6;
    QLabel *label_7;
    QLabel *dictionaryTotalWords;
    QLabel *label_9;
    QLabel *label_10;
    QLabel *dictionaryTranslatesFrom;
    QLabel *dictionaryTranslatesTo;
    QLabel *dictionaryDescriptionLabel;
    QPlainTextEdit *dictionaryDescription;
    QSpacerItem *verticalSpacer_4;
    QSpacerItem *horizontalSpacer_5;
    QLabel *label_11;
    QPlainTextEdit *dictionaryFileList;
    QLabel *label_3;

    void setupUi(QWidget *OrderAndProps)
    {
        if (OrderAndProps->objectName().isEmpty())
            OrderAndProps->setObjectName(QString::fromUtf8("OrderAndProps"));
        OrderAndProps->resize(606, 585);
        verticalLayout_4 = new QVBoxLayout(OrderAndProps);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        moveToActive = new QToolButton(OrderAndProps);
        moveToActive->setObjectName(QString::fromUtf8("moveToActive"));
        moveToActive->setArrowType(Qt::UpArrow);

        horizontalLayout->addWidget(moveToActive);

        moveToInactive = new QToolButton(OrderAndProps);
        moveToInactive->setObjectName(QString::fromUtf8("moveToInactive"));
        moveToInactive->setArrowType(Qt::DownArrow);

        horizontalLayout->addWidget(moveToInactive);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);


        gridLayout_2->addLayout(horizontalLayout, 3, 0, 1, 1);

        label = new QLabel(OrderAndProps);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout_2->addWidget(label, 0, 0, 1, 1);

        dictionaryOrder = new DictListWidget(OrderAndProps);
        dictionaryOrder->setObjectName(QString::fromUtf8("dictionaryOrder"));

        gridLayout_2->addWidget(dictionaryOrder, 2, 0, 1, 1);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);

        moveActiveUp = new QToolButton(OrderAndProps);
        moveActiveUp->setObjectName(QString::fromUtf8("moveActiveUp"));
        moveActiveUp->setArrowType(Qt::UpArrow);

        verticalLayout_2->addWidget(moveActiveUp);

        moveActiveDown = new QToolButton(OrderAndProps);
        moveActiveDown->setObjectName(QString::fromUtf8("moveActiveDown"));
        moveActiveDown->setArrowType(Qt::DownArrow);

        verticalLayout_2->addWidget(moveActiveDown);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_2);


        gridLayout_2->addLayout(verticalLayout_2, 2, 1, 1, 1);

        inactiveDictionaries = new DictListWidget(OrderAndProps);
        inactiveDictionaries->setObjectName(QString::fromUtf8("inactiveDictionaries"));

        gridLayout_2->addWidget(inactiveDictionaries, 5, 0, 1, 1);

        label_2 = new QLabel(OrderAndProps);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout_2->addWidget(label_2, 4, 0, 1, 1);

        searchLine = new QuickFilterLine(OrderAndProps);
        searchLine->setObjectName(QString::fromUtf8("searchLine"));

        gridLayout_2->addWidget(searchLine, 1, 0, 1, 1);

        gridLayout_2->setRowStretch(1, 10);
        gridLayout_2->setRowStretch(2, 10);
        gridLayout_2->setRowStretch(5, 2);

        verticalLayout_3->addLayout(gridLayout_2);


        horizontalLayout_2->addLayout(verticalLayout_3);

        dictionaryInformation = new QGroupBox(OrderAndProps);
        dictionaryInformation->setObjectName(QString::fromUtf8("dictionaryInformation"));
        verticalLayout_5 = new QVBoxLayout(dictionaryInformation);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_5 = new QLabel(dictionaryInformation);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 0, 0, 1, 1);

        dictionaryName = new QLabel(dictionaryInformation);
        dictionaryName->setObjectName(QString::fromUtf8("dictionaryName"));
        dictionaryName->setText(QString::fromUtf8("TextLabel"));
        dictionaryName->setTextFormat(Qt::PlainText);

        gridLayout->addWidget(dictionaryName, 0, 1, 1, 1);

        dictionaryTotalArticles = new QLabel(dictionaryInformation);
        dictionaryTotalArticles->setObjectName(QString::fromUtf8("dictionaryTotalArticles"));
        dictionaryTotalArticles->setText(QString::fromUtf8("TextLabel"));
        dictionaryTotalArticles->setTextFormat(Qt::PlainText);

        gridLayout->addWidget(dictionaryTotalArticles, 1, 1, 1, 1);

        label_6 = new QLabel(dictionaryInformation);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout->addWidget(label_6, 1, 0, 1, 1);

        label_7 = new QLabel(dictionaryInformation);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        gridLayout->addWidget(label_7, 2, 0, 1, 1);

        dictionaryTotalWords = new QLabel(dictionaryInformation);
        dictionaryTotalWords->setObjectName(QString::fromUtf8("dictionaryTotalWords"));
        dictionaryTotalWords->setText(QString::fromUtf8("TextLabel"));
        dictionaryTotalWords->setTextFormat(Qt::PlainText);

        gridLayout->addWidget(dictionaryTotalWords, 2, 1, 1, 1);

        label_9 = new QLabel(dictionaryInformation);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        gridLayout->addWidget(label_9, 3, 0, 1, 1);

        label_10 = new QLabel(dictionaryInformation);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        gridLayout->addWidget(label_10, 4, 0, 1, 1);

        dictionaryTranslatesFrom = new QLabel(dictionaryInformation);
        dictionaryTranslatesFrom->setObjectName(QString::fromUtf8("dictionaryTranslatesFrom"));
        dictionaryTranslatesFrom->setText(QString::fromUtf8("TextLabel"));
        dictionaryTranslatesFrom->setTextFormat(Qt::RichText);

        gridLayout->addWidget(dictionaryTranslatesFrom, 3, 1, 1, 1);

        dictionaryTranslatesTo = new QLabel(dictionaryInformation);
        dictionaryTranslatesTo->setObjectName(QString::fromUtf8("dictionaryTranslatesTo"));
        dictionaryTranslatesTo->setText(QString::fromUtf8("TextLabel"));
        dictionaryTranslatesTo->setTextFormat(Qt::RichText);

        gridLayout->addWidget(dictionaryTranslatesTo, 4, 1, 1, 1);


        verticalLayout_5->addLayout(gridLayout);

        dictionaryDescriptionLabel = new QLabel(dictionaryInformation);
        dictionaryDescriptionLabel->setObjectName(QString::fromUtf8("dictionaryDescriptionLabel"));

        verticalLayout_5->addWidget(dictionaryDescriptionLabel);

        dictionaryDescription = new QPlainTextEdit(dictionaryInformation);
        dictionaryDescription->setObjectName(QString::fromUtf8("dictionaryDescription"));
        QPalette palette;
        QBrush brush(QColor(224, 223, 223, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Base, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Base, brush);
        QBrush brush1(QColor(212, 208, 200, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Disabled, QPalette::Base, brush1);
        dictionaryDescription->setPalette(palette);
        dictionaryDescription->setFrameShape(QFrame::NoFrame);
        dictionaryDescription->setReadOnly(true);

        verticalLayout_5->addWidget(dictionaryDescription);

        verticalSpacer_4 = new QSpacerItem(20, 5, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_5->addItem(verticalSpacer_4);

        horizontalSpacer_5 = new QSpacerItem(256, 5, QSizePolicy::Expanding, QSizePolicy::Minimum);

        verticalLayout_5->addItem(horizontalSpacer_5);

        label_11 = new QLabel(dictionaryInformation);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        verticalLayout_5->addWidget(label_11);

        dictionaryFileList = new QPlainTextEdit(dictionaryInformation);
        dictionaryFileList->setObjectName(QString::fromUtf8("dictionaryFileList"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(dictionaryFileList->sizePolicy().hasHeightForWidth());
        dictionaryFileList->setSizePolicy(sizePolicy);
        dictionaryFileList->setMinimumSize(QSize(0, 65));
        dictionaryFileList->setMaximumSize(QSize(16777215, 65));
        QPalette palette1;
        QBrush brush2(QColor(224, 223, 222, 255));
        brush2.setStyle(Qt::SolidPattern);
        palette1.setBrush(QPalette::Active, QPalette::Base, brush2);
        palette1.setBrush(QPalette::Inactive, QPalette::Base, brush2);
        palette1.setBrush(QPalette::Disabled, QPalette::Base, brush2);
        dictionaryFileList->setPalette(palette1);
        dictionaryFileList->setFrameShape(QFrame::NoFrame);
        dictionaryFileList->setLineWrapMode(QPlainTextEdit::NoWrap);
        dictionaryFileList->setReadOnly(true);

        verticalLayout_5->addWidget(dictionaryFileList);


        horizontalLayout_2->addWidget(dictionaryInformation);


        verticalLayout_4->addLayout(horizontalLayout_2);

        label_3 = new QLabel(OrderAndProps);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setWordWrap(true);

        verticalLayout_4->addWidget(label_3);

        QWidget::setTabOrder(dictionaryOrder, moveActiveUp);
        QWidget::setTabOrder(moveActiveUp, moveActiveDown);
        QWidget::setTabOrder(moveActiveDown, moveToActive);
        QWidget::setTabOrder(moveToActive, moveToInactive);
        QWidget::setTabOrder(moveToInactive, inactiveDictionaries);
        QWidget::setTabOrder(inactiveDictionaries, dictionaryDescription);
        QWidget::setTabOrder(dictionaryDescription, dictionaryFileList);
        QWidget::setTabOrder(dictionaryFileList, searchLine);

        retranslateUi(OrderAndProps);

        QMetaObject::connectSlotsByName(OrderAndProps);
    } // setupUi

    void retranslateUi(QWidget *OrderAndProps)
    {
        OrderAndProps->setWindowTitle(QApplication::translate("OrderAndProps", "Form", 0, QApplication::UnicodeUTF8));
        moveToActive->setText(QApplication::translate("OrderAndProps", "...", 0, QApplication::UnicodeUTF8));
        moveToInactive->setText(QApplication::translate("OrderAndProps", "...", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("OrderAndProps", "Dictionary order:", 0, QApplication::UnicodeUTF8));
        moveActiveUp->setText(QApplication::translate("OrderAndProps", "...", 0, QApplication::UnicodeUTF8));
        moveActiveDown->setText(QApplication::translate("OrderAndProps", "...", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("OrderAndProps", "Inactive (disabled) dictionaries:", 0, QApplication::UnicodeUTF8));
        dictionaryInformation->setTitle(QApplication::translate("OrderAndProps", "Dictionary information", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("OrderAndProps", "Name:", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("OrderAndProps", "Total articles:", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("OrderAndProps", "Total words:", 0, QApplication::UnicodeUTF8));
        label_9->setText(QApplication::translate("OrderAndProps", "Translates from:", 0, QApplication::UnicodeUTF8));
        label_10->setText(QApplication::translate("OrderAndProps", "Translates to:", 0, QApplication::UnicodeUTF8));
        dictionaryDescriptionLabel->setText(QApplication::translate("OrderAndProps", "Description:", 0, QApplication::UnicodeUTF8));
        label_11->setText(QApplication::translate("OrderAndProps", "Files comprising this dictionary:", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("OrderAndProps", "Adjust the order by dragging and dropping items in it. Drop dictionaries to the inactive group to disable their use.", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class OrderAndProps: public Ui_OrderAndProps {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ORDERANDPROPS_H
