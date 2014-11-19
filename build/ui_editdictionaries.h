/********************************************************************************
** Form generated from reading UI file 'editdictionaries.ui'
**
** Created: Mon Apr 28 13:32:14 2014
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDITDICTIONARIES_H
#define UI_EDITDICTIONARIES_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHeaderView>
#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

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
            EditDictionaries->setObjectName(QString::fromUtf8("EditDictionaries"));
        EditDictionaries->resize(839, 532);
        verticalLayout = new QVBoxLayout(EditDictionaries);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        tabs = new QTabWidget(EditDictionaries);
        tabs->setObjectName(QString::fromUtf8("tabs"));
        dummy = new QWidget();
        dummy->setObjectName(QString::fromUtf8("dummy"));
        tabs->addTab(dummy, QString());
        tabs->setTabText(tabs->indexOf(dummy), QString::fromUtf8("Dummy"));

        verticalLayout->addWidget(tabs);

        buttons = new QDialogButtonBox(EditDictionaries);
        buttons->setObjectName(QString::fromUtf8("buttons"));
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
        EditDictionaries->setWindowTitle(QApplication::translate("EditDictionaries", "Dictionaries", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class EditDictionaries: public Ui_EditDictionaries {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDITDICTIONARIES_H
