/********************************************************************************
** Form generated from reading UI file 'dictgroupwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DICTGROUPWIDGET_H
#define UI_DICTGROUPWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
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
            DictGroupWidget->setObjectName(QStringLiteral("DictGroupWidget"));
        DictGroupWidget->resize(403, 333);
        verticalLayout = new QVBoxLayout(DictGroupWidget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(2, 4, 2, 3);
        dictionaries = new DictListWidget(DictGroupWidget);
        dictionaries->setObjectName(QStringLiteral("dictionaries"));

        verticalLayout->addWidget(dictionaries);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label = new QLabel(DictGroupWidget);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label);

        groupIcon = new QComboBox(DictGroupWidget);
        groupIcon->setObjectName(QStringLiteral("groupIcon"));

        horizontalLayout->addWidget(groupIcon);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        label_3 = new QLabel(DictGroupWidget);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setMinimumSize(QSize(16, 16));
        label_3->setMaximumSize(QSize(16, 16));
        label_3->setText(QStringLiteral(""));
        label_3->setPixmap(QPixmap(QString::fromUtf8(":/icons/hotkeys.png")));
        label_3->setScaledContents(true);

        horizontalLayout->addWidget(label_3);

        label_2 = new QLabel(DictGroupWidget);
        label_2->setObjectName(QStringLiteral("label_2"));

        horizontalLayout->addWidget(label_2);

        shortcut = new HotKeyEdit(DictGroupWidget);
        shortcut->setObjectName(QStringLiteral("shortcut"));

        horizontalLayout->addWidget(shortcut);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(DictGroupWidget);

        QMetaObject::connectSlotsByName(DictGroupWidget);
    } // setupUi

    void retranslateUi(QWidget *DictGroupWidget)
    {
        DictGroupWidget->setWindowTitle(QApplication::translate("DictGroupWidget", "Form", 0));
        label->setText(QApplication::translate("DictGroupWidget", "Group icon:", 0));
        label_2->setText(QApplication::translate("DictGroupWidget", "Shortcut:", 0));
    } // retranslateUi

};

namespace Ui {
    class DictGroupWidget: public Ui_DictGroupWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DICTGROUPWIDGET_H
