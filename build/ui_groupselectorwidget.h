/********************************************************************************
** Form generated from reading UI file 'groupselectorwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GROUPSELECTORWIDGET_H
#define UI_GROUPSELECTORWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GroupSelectorWidget
{
public:
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QComboBox *group;

    void setupUi(QWidget *GroupSelectorWidget)
    {
        if (GroupSelectorWidget->objectName().isEmpty())
            GroupSelectorWidget->setObjectName(QStringLiteral("GroupSelectorWidget"));
        GroupSelectorWidget->resize(134, 40);
        horizontalLayout = new QHBoxLayout(GroupSelectorWidget);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label = new QLabel(GroupSelectorWidget);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label);

        group = new QComboBox(GroupSelectorWidget);
        group->setObjectName(QStringLiteral("group"));
        group->setSizeAdjustPolicy(QComboBox::AdjustToContents);

        horizontalLayout->addWidget(group);


        retranslateUi(GroupSelectorWidget);

        QMetaObject::connectSlotsByName(GroupSelectorWidget);
    } // setupUi

    void retranslateUi(QWidget *GroupSelectorWidget)
    {
        GroupSelectorWidget->setWindowTitle(QApplication::translate("GroupSelectorWidget", "Form", 0));
        label->setText(QApplication::translate("GroupSelectorWidget", "Look in", 0));
    } // retranslateUi

};

namespace Ui {
    class GroupSelectorWidget: public Ui_GroupSelectorWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GROUPSELECTORWIDGET_H
