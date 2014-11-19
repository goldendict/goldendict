/********************************************************************************
** Form generated from reading UI file 'initializing.ui'
**
** Created: Mon Apr 28 13:32:14 2014
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INITIALIZING_H
#define UI_INITIALIZING_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_Initializing
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *operation;
    QLabel *dictionary;
    QProgressBar *progressBar;

    void setupUi(QDialog *Initializing)
    {
        if (Initializing->objectName().isEmpty())
            Initializing->setObjectName(QString::fromUtf8("Initializing"));
        Initializing->setWindowModality(Qt::ApplicationModal);
        Initializing->resize(326, 84);
        Initializing->setMinimumSize(QSize(326, 0));
        verticalLayout = new QVBoxLayout(Initializing);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        operation = new QLabel(Initializing);
        operation->setObjectName(QString::fromUtf8("operation"));
        operation->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(operation);

        dictionary = new QLabel(Initializing);
        dictionary->setObjectName(QString::fromUtf8("dictionary"));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        dictionary->setFont(font);
        dictionary->setScaledContents(false);
        dictionary->setAlignment(Qt::AlignCenter);
        dictionary->setWordWrap(true);

        verticalLayout->addWidget(dictionary);

        progressBar = new QProgressBar(Initializing);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setMaximum(0);
        progressBar->setValue(-1);
        progressBar->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(progressBar);


        retranslateUi(Initializing);

        QMetaObject::connectSlotsByName(Initializing);
    } // setupUi

    void retranslateUi(QDialog *Initializing)
    {
        Initializing->setWindowTitle(QApplication::translate("Initializing", "GoldenDict - Initializing", 0, QApplication::UnicodeUTF8));
        operation->setText(QApplication::translate("Initializing", "Please wait while indexing dictionary", 0, QApplication::UnicodeUTF8));
        dictionary->setText(QApplication::translate("Initializing", "Dictionary Name", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Initializing: public Ui_Initializing {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INITIALIZING_H
