/********************************************************************************
** Form generated from reading UI file 'editdictionaries.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDITDICTIONARIES_H
#define UI_EDITDICTIONARIES_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_EditDictionaries
{
public:
    QVBoxLayout *verticalLayout;
    QTabWidget *tabs;
    QWidget *dummy;
    QDialogButtonBox *buttons;

    void setupUi(QDialog *EditDictionaries)
    {
        if (EditDictionaries->objectName().isEmpty())
            EditDictionaries->setObjectName(QStringLiteral("EditDictionaries"));
        EditDictionaries->resize(839, 532);
        verticalLayout = new QVBoxLayout(EditDictionaries);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        tabs = new QTabWidget(EditDictionaries);
        tabs->setObjectName(QStringLiteral("tabs"));
        dummy = new QWidget();
        dummy->setObjectName(QStringLiteral("dummy"));
        tabs->addTab(dummy, QString());
        tabs->setTabText(tabs->indexOf(dummy), QStringLiteral("Dummy"));

        verticalLayout->addWidget(tabs);

        buttons = new QDialogButtonBox(EditDictionaries);
        buttons->setObjectName(QStringLiteral("buttons"));
        buttons->setOrientation(Qt::Horizontal);
        buttons->setStandardButtons(QDialogButtonBox::Apply|QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttons);


        retranslateUi(EditDictionaries);
        QObject::connect(buttons, SIGNAL(accepted()), EditDictionaries, SLOT(accept()));
        QObject::connect(buttons, SIGNAL(rejected()), EditDictionaries, SLOT(reject()));

        tabs->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(EditDictionaries);
    } // setupUi

    void retranslateUi(QDialog *EditDictionaries)
    {
        EditDictionaries->setWindowTitle(QApplication::translate("EditDictionaries", "Dictionaries", 0));
    } // retranslateUi

};

namespace Ui {
    class EditDictionaries: public Ui_EditDictionaries {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDITDICTIONARIES_H
