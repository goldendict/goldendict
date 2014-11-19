/********************************************************************************
** Form generated from reading UI file 'dictgroupwidget.ui'
**
** Created: Wed Nov 19 16:01:34 2014
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DICTGROUPWIDGET_H
#define UI_DICTGROUPWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "groups_widgets.hh"
#include "hotkeyedit.hh"

QT_BEGIN_NAMESPACE

class Ui_DictGroupWidget
{
public:
    QVBoxLayout *verticalLayout;
    DictListWidget *dictionaries;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QComboBox *groupIcon;
    QSpacerItem *horizontalSpacer;
    QLabel *label_3;
    QLabel *label_2;
    HotKeyEdit *shortcut;

    void setupUi(QWidget *DictGroupWidget)
    {
        if (DictGroupWidget->objectName().isEmpty())
            DictGroupWidget->setObjectName(QString::fromUtf8("DictGroupWidget"));
        DictGroupWidget->resize(403, 333);
        verticalLayout = new QVBoxLayout(DictGroupWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(2, 4, 2, 3);
        dictionaries = new DictListWidget(DictGroupWidget);
        dictionaries->setObjectName(QString::fromUtf8("dictionaries"));

        verticalLayout->addWidget(dictionaries);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(DictGroupWidget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        groupIcon = new QComboBox(DictGroupWidget);
        groupIcon->setObjectName(QString::fromUtf8("groupIcon"));

        horizontalLayout->addWidget(groupIcon);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        label_3 = new QLabel(DictGroupWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setMinimumSize(QSize(16, 16));
        label_3->setMaximumSize(QSize(16, 16));
        label_3->setText(QString::fromUtf8(""));
        label_3->setPixmap(QPixmap(QString::fromUtf8(":/icons/hotkeys.png")));
        label_3->setScaledContents(true);

        horizontalLayout->addWidget(label_3);

        label_2 = new QLabel(DictGroupWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout->addWidget(label_2);

        shortcut = new HotKeyEdit(DictGroupWidget);
        shortcut->setObjectName(QString::fromUtf8("shortcut"));

        horizontalLayout->addWidget(shortcut);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(DictGroupWidget);

        QMetaObject::connectSlotsByName(DictGroupWidget);
    } // setupUi

    void retranslateUi(QWidget *DictGroupWidget)
    {
        DictGroupWidget->setWindowTitle(QApplication::translate("DictGroupWidget", "Form", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("DictGroupWidget", "Group icon:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("DictGroupWidget", "Shortcut:", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DictGroupWidget: public Ui_DictGroupWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DICTGROUPWIDGET_H
