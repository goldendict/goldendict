/********************************************************************************
** Form generated from reading UI file 'articleview.ui'
**
** Created: Wed Nov 19 16:01:34 2014
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ARTICLEVIEW_H
#define UI_ARTICLEVIEW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QSpacerItem>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "articlewebview.hh"

QT_BEGIN_NAMESPACE

class Ui_ArticleView
{
public:
    QVBoxLayout *verticalLayout_2;
    QFrame *frame;
    QVBoxLayout *verticalLayout;
    ArticleWebView *definition;
    QFrame *ftsSearchFrame;
    QHBoxLayout *horizontalLayout_3;
    QToolButton *ftsSearchPrevious;
    QToolButton *ftsSearchNext;
    QSpacerItem *horizontalSpacer_2;
    QFrame *searchFrame;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QLineEdit *searchText;
    QToolButton *searchCloseButton;
    QHBoxLayout *horizontalLayout;
    QToolButton *searchPrevious;
    QToolButton *searchNext;
    QToolButton *highlightAllButton;
    QCheckBox *searchCaseSensitive;
    QSpacerItem *horizontalSpacer;

    void setupUi(QWidget *ArticleView)
    {
        if (ArticleView->objectName().isEmpty())
            ArticleView->setObjectName(QString::fromUtf8("ArticleView"));
        ArticleView->resize(833, 634);
        verticalLayout_2 = new QVBoxLayout(ArticleView);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        frame = new QFrame(ArticleView);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        verticalLayout = new QVBoxLayout(frame);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        definition = new ArticleWebView(frame);
        definition->setObjectName(QString::fromUtf8("definition"));
        definition->setProperty("url", QVariant(QUrl(QString::fromUtf8("about:blank"))));

        verticalLayout->addWidget(definition);


        verticalLayout_2->addWidget(frame);

        ftsSearchFrame = new QFrame(ArticleView);
        ftsSearchFrame->setObjectName(QString::fromUtf8("ftsSearchFrame"));
        ftsSearchFrame->setFrameShadow(QFrame::Raised);
        horizontalLayout_3 = new QHBoxLayout(ftsSearchFrame);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        ftsSearchPrevious = new QToolButton(ftsSearchFrame);
        ftsSearchPrevious->setObjectName(QString::fromUtf8("ftsSearchPrevious"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/previous.png"), QSize(), QIcon::Normal, QIcon::Off);
        ftsSearchPrevious->setIcon(icon);
        ftsSearchPrevious->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

        horizontalLayout_3->addWidget(ftsSearchPrevious);

        ftsSearchNext = new QToolButton(ftsSearchFrame);
        ftsSearchNext->setObjectName(QString::fromUtf8("ftsSearchNext"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/next.png"), QSize(), QIcon::Normal, QIcon::Off);
        ftsSearchNext->setIcon(icon1);
        ftsSearchNext->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

        horizontalLayout_3->addWidget(ftsSearchNext);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_2);


        verticalLayout_2->addWidget(ftsSearchFrame);

        searchFrame = new QFrame(ArticleView);
        searchFrame->setObjectName(QString::fromUtf8("searchFrame"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(searchFrame->sizePolicy().hasHeightForWidth());
        searchFrame->setSizePolicy(sizePolicy);
        searchFrame->setFrameShape(QFrame::NoFrame);
        searchFrame->setFrameShadow(QFrame::Raised);
        gridLayout = new QGridLayout(searchFrame);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label = new QLabel(searchFrame);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_2->addWidget(label);

        searchText = new QLineEdit(searchFrame);
        searchText->setObjectName(QString::fromUtf8("searchText"));

        horizontalLayout_2->addWidget(searchText);

        searchCloseButton = new QToolButton(searchFrame);
        searchCloseButton->setObjectName(QString::fromUtf8("searchCloseButton"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/closetab.png"), QSize(), QIcon::Normal, QIcon::Off);
        searchCloseButton->setIcon(icon2);
        searchCloseButton->setAutoRaise(true);

        horizontalLayout_2->addWidget(searchCloseButton);


        gridLayout->addLayout(horizontalLayout_2, 0, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        searchPrevious = new QToolButton(searchFrame);
        searchPrevious->setObjectName(QString::fromUtf8("searchPrevious"));
        searchPrevious->setIcon(icon);
        searchPrevious->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        searchPrevious->setAutoRaise(true);

        horizontalLayout->addWidget(searchPrevious);

        searchNext = new QToolButton(searchFrame);
        searchNext->setObjectName(QString::fromUtf8("searchNext"));
        searchNext->setIcon(icon1);
        searchNext->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        searchNext->setAutoRaise(true);

        horizontalLayout->addWidget(searchNext);

        highlightAllButton = new QToolButton(searchFrame);
        highlightAllButton->setObjectName(QString::fromUtf8("highlightAllButton"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/highlighter.png"), QSize(), QIcon::Normal, QIcon::Off);
        highlightAllButton->setIcon(icon3);
        highlightAllButton->setCheckable(true);
        highlightAllButton->setChecked(false);
        highlightAllButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        highlightAllButton->setAutoRaise(true);

        horizontalLayout->addWidget(highlightAllButton);

        searchCaseSensitive = new QCheckBox(searchFrame);
        searchCaseSensitive->setObjectName(QString::fromUtf8("searchCaseSensitive"));

        horizontalLayout->addWidget(searchCaseSensitive);

        horizontalSpacer = new QSpacerItem(782, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        gridLayout->addLayout(horizontalLayout, 1, 0, 1, 1);


        verticalLayout_2->addWidget(searchFrame);

        verticalLayout_2->setStretch(0, 1000);

        retranslateUi(ArticleView);

        QMetaObject::connectSlotsByName(ArticleView);
    } // setupUi

    void retranslateUi(QWidget *ArticleView)
    {
        ArticleView->setWindowTitle(QApplication::translate("ArticleView", "Form", 0, QApplication::UnicodeUTF8));
        ftsSearchPrevious->setText(QApplication::translate("ArticleView", "&Previous", 0, QApplication::UnicodeUTF8));
        ftsSearchNext->setText(QApplication::translate("ArticleView", "&Next", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("ArticleView", "Find:", 0, QApplication::UnicodeUTF8));
        searchCloseButton->setText(QApplication::translate("ArticleView", "x", 0, QApplication::UnicodeUTF8));
        searchPrevious->setText(QApplication::translate("ArticleView", "&Previous", 0, QApplication::UnicodeUTF8));
        searchNext->setText(QApplication::translate("ArticleView", "&Next", 0, QApplication::UnicodeUTF8));
        searchNext->setShortcut(QApplication::translate("ArticleView", "Ctrl+G", 0, QApplication::UnicodeUTF8));
        highlightAllButton->setText(QApplication::translate("ArticleView", "Highlight &all", 0, QApplication::UnicodeUTF8));
        searchCaseSensitive->setText(QApplication::translate("ArticleView", "&Case Sensitive", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ArticleView: public Ui_ArticleView {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ARTICLEVIEW_H
