/********************************************************************************
** Form generated from reading UI file 'groupselectorwidget.ui'
**
** Created: Wed Nov 19 16:01:34 2014
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GROUPSELECTORWIDGET_H
#define UI_GROUPSELECTORWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QWidget>

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
            GroupSelectorWidget->setObjectName(QString::fromUtf8("GroupSelectorWidget"));
        GroupSelectorWidget->resize(134, 40);
        horizontalLayout = new QHBoxLayout(GroupSelectorWidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(GroupSelectorWidget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        group = new QComboBox(GroupSelectorWidget);
        group->setObjectName(QString::fromUtf8("group"));
        group->setSizeAdjustPolicy(QComboBox::AdjustToContents);

        horizontalLayout->addWidget(group);


        retranslateUi(GroupSelectorWidget);

        QMetaObject::connectSlotsByName(GroupSelectorWidget);
    } // setupUi

    void retranslateUi(QWidget *GroupSelectorWidget)
    {
        GroupSelectorWidget->setWindowTitle(QApplication::translate("GroupSelectorWidget", "Form", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("GroupSelectorWidget", "Look in", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class GroupSelectorWidget: public Ui_GroupSelectorWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GROUPSELECTORWIDGET_H
