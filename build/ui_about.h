/********************************************************************************
** Form generated from reading UI file 'about.ui'
**
** Created: Mon Apr 28 13:32:14 2014
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ABOUT_H
#define UI_ABOUT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QTextBrowser>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_About
{
public:
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QLabel *icon;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QLabel *version;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *qtVersion;
    QLabel *label_5;
    QTextBrowser *credits;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *About)
    {
        if (About->objectName().isEmpty())
            About->setObjectName(QString::fromUtf8("About"));
        About->setWindowModality(Qt::NonModal);
        About->resize(400, 400);
        About->setModal(true);
        verticalLayout_2 = new QVBoxLayout(About);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        icon = new QLabel(About);
        icon->setObjectName(QString::fromUtf8("icon"));
        icon->setMinimumSize(QSize(80, 80));
        icon->setMaximumSize(QSize(80, 80));
        icon->setPixmap(QPixmap(QString::fromUtf8(":/icons/programicon.png")));
        icon->setScaledContents(true);
        icon->setAlignment(Qt::AlignCenter);
        icon->setMargin(10);

        horizontalLayout->addWidget(icon);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label = new QLabel(About);
        label->setObjectName(QString::fromUtf8("label"));
        label->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        horizontalLayout_2->addWidget(label);

        version = new QLabel(About);
        version->setObjectName(QString::fromUtf8("version"));
        version->setText(QString::fromUtf8("#.#"));
        version->setTextInteractionFlags(Qt::TextEditorInteraction);

        horizontalLayout_2->addWidget(version);


        verticalLayout->addLayout(horizontalLayout_2);

        label_2 = new QLabel(About);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        verticalLayout->addWidget(label_2);

        label_3 = new QLabel(About);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        verticalLayout->addWidget(label_3);

        qtVersion = new QLabel(About);
        qtVersion->setObjectName(QString::fromUtf8("qtVersion"));
        qtVersion->setText(QString::fromUtf8("Based on Qt #.#.# (GCC #.#, 32/64 bit)"));
        qtVersion->setTextInteractionFlags(Qt::TextEditorInteraction);

        verticalLayout->addWidget(qtVersion);


        horizontalLayout->addLayout(verticalLayout);


        verticalLayout_2->addLayout(horizontalLayout);

        label_5 = new QLabel(About);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        verticalLayout_2->addWidget(label_5);

        credits = new QTextBrowser(About);
        credits->setObjectName(QString::fromUtf8("credits"));
        credits->setEnabled(true);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(1);
        sizePolicy.setHeightForWidth(credits->sizePolicy().hasHeightForWidth());
        credits->setSizePolicy(sizePolicy);

        verticalLayout_2->addWidget(credits);

        buttonBox = new QDialogButtonBox(About);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Ok);
        buttonBox->setCenterButtons(true);

        verticalLayout_2->addWidget(buttonBox);


        retranslateUi(About);
        QObject::connect(buttonBox, SIGNAL(accepted()), About, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), About, SLOT(reject()));

        QMetaObject::connectSlotsByName(About);
    } // setupUi

    void retranslateUi(QDialog *About)
    {
        About->setWindowTitle(QApplication::translate("About", "About", 0, QApplication::UnicodeUTF8));
        icon->setText(QString());
        label->setText(QApplication::translate("About", "GoldenDict dictionary lookup program, version ", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_ACCESSIBILITY
        version->setAccessibleDescription(QString());
#endif // QT_NO_ACCESSIBILITY
        label_2->setText(QApplication::translate("About", "(c) 2008-2013 Konstantin Isakov (ikm@goldendict.org)", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("About", "Licensed under GNU GPLv3 or later", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("About", "Credits:", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class About: public Ui_About {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUT_H
