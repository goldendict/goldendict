/********************************************************************************
** Form generated from reading UI file 'groups.ui'
**
** Created: Mon Apr 28 13:32:14 2014
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GROUPS_H
#define UI_GROUPS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "groups_widgets.hh"

QT_BEGIN_NAMESPACE

class Ui_Groups
{
public:
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout_3;
    QLabel *label_2;
    QuickFilterLine *searchLine;
    DictListWidget *dictionaries;
    QVBoxLayout *verticalLayout_2;
    QSpacerItem *verticalSpacer_2;
    QPushButton *addDictsToGroup;
    QPushButton *removeDictsFromGroup;
    QSpacerItem *verticalSpacer;
    QVBoxLayout *verticalLayout;
    QLabel *label_3;
    DictGroupsWidget *groups;
    QWidget *tab_4;
    QHBoxLayout *horizontalLayout;
    QPushButton *addGroup;
    QPushButton *autoGroups;
    QPushButton *renameGroup;
    QPushButton *removeGroup;
    QPushButton *removeAllGroups;
    QLabel *label_4;

    void setupUi(QWidget *Groups)
    {
        if (Groups->objectName().isEmpty())
            Groups->setObjectName(QString::fromUtf8("Groups"));
        Groups->resize(662, 319);
        Groups->setWindowTitle(QString::fromUtf8("Groups"));
        gridLayout = new QGridLayout(Groups);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        label_2 = new QLabel(Groups);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        verticalLayout_3->addWidget(label_2);

        searchLine = new QuickFilterLine(Groups);
        searchLine->setObjectName(QString::fromUtf8("searchLine"));

        verticalLayout_3->addWidget(searchLine);

        dictionaries = new DictListWidget(Groups);
        dictionaries->setObjectName(QString::fromUtf8("dictionaries"));

        verticalLayout_3->addWidget(dictionaries);


        gridLayout->addLayout(verticalLayout_3, 0, 0, 1, 1);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_2);

        addDictsToGroup = new QPushButton(Groups);
        addDictsToGroup->setObjectName(QString::fromUtf8("addDictsToGroup"));
        addDictsToGroup->setMinimumSize(QSize(32, 32));
        addDictsToGroup->setMaximumSize(QSize(32, 32));

        verticalLayout_2->addWidget(addDictsToGroup);

        removeDictsFromGroup = new QPushButton(Groups);
        removeDictsFromGroup->setObjectName(QString::fromUtf8("removeDictsFromGroup"));
        removeDictsFromGroup->setMinimumSize(QSize(32, 32));
        removeDictsFromGroup->setMaximumSize(QSize(32, 32));

        verticalLayout_2->addWidget(removeDictsFromGroup);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);


        gridLayout->addLayout(verticalLayout_2, 0, 1, 1, 1);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label_3 = new QLabel(Groups);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        verticalLayout->addWidget(label_3);

        groups = new DictGroupsWidget(Groups);
        groups->setObjectName(QString::fromUtf8("groups"));
        groups->setTabPosition(QTabWidget::North);
        groups->setElideMode(Qt::ElideRight);
        tab_4 = new QWidget();
        tab_4->setObjectName(QString::fromUtf8("tab_4"));
        groups->addTab(tab_4, QString());

        verticalLayout->addWidget(groups);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        addGroup = new QPushButton(Groups);
        addGroup->setObjectName(QString::fromUtf8("addGroup"));

        horizontalLayout->addWidget(addGroup);

        autoGroups = new QPushButton(Groups);
        autoGroups->setObjectName(QString::fromUtf8("autoGroups"));

        horizontalLayout->addWidget(autoGroups);

        renameGroup = new QPushButton(Groups);
        renameGroup->setObjectName(QString::fromUtf8("renameGroup"));

        horizontalLayout->addWidget(renameGroup);

        removeGroup = new QPushButton(Groups);
        removeGroup->setObjectName(QString::fromUtf8("removeGroup"));

        horizontalLayout->addWidget(removeGroup);

        removeAllGroups = new QPushButton(Groups);
        removeAllGroups->setObjectName(QString::fromUtf8("removeAllGroups"));

        horizontalLayout->addWidget(removeAllGroups);


        verticalLayout->addLayout(horizontalLayout);


        gridLayout->addLayout(verticalLayout, 0, 2, 1, 1);

        label_4 = new QLabel(Groups);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setWordWrap(true);

        gridLayout->addWidget(label_4, 1, 0, 1, 3);

        gridLayout->setColumnStretch(0, 1);
        QWidget::setTabOrder(dictionaries, addDictsToGroup);
        QWidget::setTabOrder(addDictsToGroup, removeDictsFromGroup);
        QWidget::setTabOrder(removeDictsFromGroup, groups);
        QWidget::setTabOrder(groups, addGroup);
        QWidget::setTabOrder(addGroup, autoGroups);
        QWidget::setTabOrder(autoGroups, renameGroup);
        QWidget::setTabOrder(renameGroup, removeGroup);
        QWidget::setTabOrder(removeGroup, removeAllGroups);
        QWidget::setTabOrder(removeAllGroups, searchLine);

        retranslateUi(Groups);

        QMetaObject::connectSlotsByName(Groups);
    } // setupUi

    void retranslateUi(QWidget *Groups)
    {
        label_2->setText(QApplication::translate("Groups", "Dictionaries available:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        addDictsToGroup->setToolTip(QApplication::translate("Groups", "Add selected dictionaries to group (Ins)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        addDictsToGroup->setText(QApplication::translate("Groups", ">", 0, QApplication::UnicodeUTF8));
        addDictsToGroup->setShortcut(QApplication::translate("Groups", "Ins", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        removeDictsFromGroup->setToolTip(QApplication::translate("Groups", "Remove selected dictionaries from group (Del)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        removeDictsFromGroup->setText(QApplication::translate("Groups", "<", 0, QApplication::UnicodeUTF8));
        removeDictsFromGroup->setShortcut(QApplication::translate("Groups", "Del", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("Groups", "Groups:", 0, QApplication::UnicodeUTF8));
        groups->setTabText(groups->indexOf(tab_4), QApplication::translate("Groups", "Tab 2", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        addGroup->setToolTip(QApplication::translate("Groups", "Create new dictionary group", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        addGroup->setText(QApplication::translate("Groups", "&Add group", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        autoGroups->setToolTip(QApplication::translate("Groups", "Create language-based groups", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        autoGroups->setText(QApplication::translate("Groups", "Auto groups", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        renameGroup->setToolTip(QApplication::translate("Groups", "Rename current dictionary group", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        renameGroup->setText(QApplication::translate("Groups", "Re&name group", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        removeGroup->setToolTip(QApplication::translate("Groups", "Remove current dictionary group", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        removeGroup->setText(QApplication::translate("Groups", "&Remove group", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        removeAllGroups->setToolTip(QApplication::translate("Groups", "Remove all dictionary groups", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        removeAllGroups->setText(QApplication::translate("Groups", "Remove all groups", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("Groups", "Drag&drop dictionaries to and from the groups, move them inside the groups, reorder the groups using your mouse.", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(Groups);
    } // retranslateUi

};

namespace Ui {
    class Groups: public Ui_Groups {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GROUPS_H
