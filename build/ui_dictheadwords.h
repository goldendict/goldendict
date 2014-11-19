/********************************************************************************
** Form generated from reading UI file 'dictheadwords.ui'
**
** Created: Mon Apr 28 13:32:14 2014
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DICTHEADWORDS_H
#define UI_DICTHEADWORDS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListView>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DictHeadwords
{
public:
    QVBoxLayout *verticalLayout_3;
    QGridLayout *gridLayout;
    QListView *headersListView;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QComboBox *searchModeCombo;
    QCheckBox *matchCase;
    QPushButton *exportButton;
    QSpacerItem *verticalSpacer;
    QPushButton *OKButton;
    QPushButton *applyButton;
    QCheckBox *autoApply;
    QLabel *label;
    QLineEdit *filterLine;
    QLabel *headersNumber;

    void setupUi(QDialog *DictHeadwords)
    {
        if (DictHeadwords->objectName().isEmpty())
            DictHeadwords->setObjectName(QString::fromUtf8("DictHeadwords"));
        DictHeadwords->resize(458, 550);
        DictHeadwords->setWindowTitle(QString::fromUtf8(""));
        verticalLayout_3 = new QVBoxLayout(DictHeadwords);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        headersListView = new QListView(DictHeadwords);
        headersListView->setObjectName(QString::fromUtf8("headersListView"));

        gridLayout->addWidget(headersListView, 3, 0, 1, 1);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        groupBox = new QGroupBox(DictHeadwords);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        searchModeCombo = new QComboBox(groupBox);
        searchModeCombo->setObjectName(QString::fromUtf8("searchModeCombo"));

        verticalLayout->addWidget(searchModeCombo);

        matchCase = new QCheckBox(groupBox);
        matchCase->setObjectName(QString::fromUtf8("matchCase"));

        verticalLayout->addWidget(matchCase);


        verticalLayout_2->addWidget(groupBox);

        exportButton = new QPushButton(DictHeadwords);
        exportButton->setObjectName(QString::fromUtf8("exportButton"));
        exportButton->setAutoDefault(false);

        verticalLayout_2->addWidget(exportButton);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);

        OKButton = new QPushButton(DictHeadwords);
        OKButton->setObjectName(QString::fromUtf8("OKButton"));
        OKButton->setAutoDefault(false);

        verticalLayout_2->addWidget(OKButton);


        gridLayout->addLayout(verticalLayout_2, 3, 1, 1, 1);

        applyButton = new QPushButton(DictHeadwords);
        applyButton->setObjectName(QString::fromUtf8("applyButton"));
        applyButton->setAutoDefault(false);
        applyButton->setDefault(true);

        gridLayout->addWidget(applyButton, 2, 1, 1, 1);

        autoApply = new QCheckBox(DictHeadwords);
        autoApply->setObjectName(QString::fromUtf8("autoApply"));

        gridLayout->addWidget(autoApply, 1, 1, 1, 1);

        label = new QLabel(DictHeadwords);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 1, 0, 1, 1);

        filterLine = new QLineEdit(DictHeadwords);
        filterLine->setObjectName(QString::fromUtf8("filterLine"));

        gridLayout->addWidget(filterLine, 2, 0, 1, 1);


        verticalLayout_3->addLayout(gridLayout);

        headersNumber = new QLabel(DictHeadwords);
        headersNumber->setObjectName(QString::fromUtf8("headersNumber"));

        verticalLayout_3->addWidget(headersNumber);


        retranslateUi(DictHeadwords);

        QMetaObject::connectSlotsByName(DictHeadwords);
    } // setupUi

    void retranslateUi(QDialog *DictHeadwords)
    {
        groupBox->setTitle(QApplication::translate("DictHeadwords", "Search mode", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        searchModeCombo->setToolTip(QApplication::translate("DictHeadwords", "This element determines how filter string will be interpreted", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        matchCase->setToolTip(QApplication::translate("DictHeadwords", "If checked on the symbols case will be take in account when filtering", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        matchCase->setText(QApplication::translate("DictHeadwords", "Match case", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        exportButton->setToolTip(QApplication::translate("DictHeadwords", "Exports headwords to file", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        exportButton->setText(QApplication::translate("DictHeadwords", "Export", 0, QApplication::UnicodeUTF8));
        OKButton->setText(QApplication::translate("DictHeadwords", "OK", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        applyButton->setToolTip(QApplication::translate("DictHeadwords", "Press this button to apply filter to headwords list", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        applyButton->setText(QApplication::translate("DictHeadwords", "Apply", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        autoApply->setToolTip(QApplication::translate("DictHeadwords", "If checked any filter changes will we immediately applied to headwords list", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        autoApply->setText(QApplication::translate("DictHeadwords", "Auto apply", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("DictHeadwords", "Filter:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        filterLine->setToolTip(QApplication::translate("DictHeadwords", "Filter string (fixed string, wildcards or regular expression)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        headersNumber->setText(QString());
        Q_UNUSED(DictHeadwords);
    } // retranslateUi

};

namespace Ui {
    class DictHeadwords: public Ui_DictHeadwords {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DICTHEADWORDS_H
