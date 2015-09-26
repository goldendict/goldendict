/********************************************************************************
** Form generated from reading UI file 'articleview.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ARTICLEVIEW_H
#define UI_ARTICLEVIEW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
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
            ArticleView->setObjectName(QStringLiteral("ArticleView"));
        ArticleView->resize(833, 634);
        verticalLayout_2 = new QVBoxLayout(ArticleView);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        frame = new QFrame(ArticleView);
        frame->setObjectName(QStringLiteral("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        verticalLayout = new QVBoxLayout(frame);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        definition = new ArticleWebView(frame);
        definition->setObjectName(QStringLiteral("definition"));
        definition->setProperty("url", QVariant(QUrl(QStringLiteral("about:blank"))));

        verticalLayout->addWidget(definition);


        verticalLayout_2->addWidget(frame);

        ftsSearchFrame = new QFrame(ArticleView);
        ftsSearchFrame->setObjectName(QStringLiteral("ftsSearchFrame"));
        ftsSearchFrame->setFrameShadow(QFrame::Raised);
        horizontalLayout_3 = new QHBoxLayout(ftsSearchFrame);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        ftsSearchPrevious = new QToolButton(ftsSearchFrame);
        ftsSearchPrevious->setObjectName(QStringLiteral("ftsSearchPrevious"));
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/previous.png"), QSize(), QIcon::Normal, QIcon::Off);
        ftsSearchPrevious->setIcon(icon);
        ftsSearchPrevious->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

        horizontalLayout_3->addWidget(ftsSearchPrevious);

        ftsSearchNext = new QToolButton(ftsSearchFrame);
        ftsSearchNext->setObjectName(QStringLiteral("ftsSearchNext"));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/icons/next.png"), QSize(), QIcon::Normal, QIcon::Off);
        ftsSearchNext->setIcon(icon1);
        ftsSearchNext->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

        horizontalLayout_3->addWidget(ftsSearchNext);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_2);


        verticalLayout_2->addWidget(ftsSearchFrame);

        searchFrame = new QFrame(ArticleView);
        searchFrame->setObjectName(QStringLiteral("searchFrame"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(searchFrame->sizePolicy().hasHeightForWidth());
        searchFrame->setSizePolicy(sizePolicy);
        searchFrame->setFrameShape(QFrame::NoFrame);
        searchFrame->setFrameShadow(QFrame::Raised);
        gridLayout = new QGridLayout(searchFrame);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        label = new QLabel(searchFrame);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout_2->addWidget(label);

        searchText = new QLineEdit(searchFrame);
        searchText->setObjectName(QStringLiteral("searchText"));

        horizontalLayout_2->addWidget(searchText);

        searchCloseButton = new QToolButton(searchFrame);
        searchCloseButton->setObjectName(QStringLiteral("searchCloseButton"));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/icons/closetab.png"), QSize(), QIcon::Normal, QIcon::Off);
        searchCloseButton->setIcon(icon2);
        searchCloseButton->setAutoRaise(true);

        horizontalLayout_2->addWidget(searchCloseButton);


        gridLayout->addLayout(horizontalLayout_2, 0, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        searchPrevious = new QToolButton(searchFrame);
        searchPrevious->setObjectName(QStringLiteral("searchPrevious"));
        searchPrevious->setIcon(icon);
        searchPrevious->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        searchPrevious->setAutoRaise(true);

        horizontalLayout->addWidget(searchPrevious);

        searchNext = new QToolButton(searchFrame);
        searchNext->setObjectName(QStringLiteral("searchNext"));
        searchNext->setIcon(icon1);
        searchNext->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        searchNext->setAutoRaise(true);

        horizontalLayout->addWidget(searchNext);

        highlightAllButton = new QToolButton(searchFrame);
        highlightAllButton->setObjectName(QStringLiteral("highlightAllButton"));
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/icons/highlighter.png"), QSize(), QIcon::Normal, QIcon::Off);
        highlightAllButton->setIcon(icon3);
        highlightAllButton->setCheckable(true);
        highlightAllButton->setChecked(false);
        highlightAllButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        highlightAllButton->setAutoRaise(true);

        horizontalLayout->addWidget(highlightAllButton);

        searchCaseSensitive = new QCheckBox(searchFrame);
        searchCaseSensitive->setObjectName(QStringLiteral("searchCaseSensitive"));

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
        ArticleView->setWindowTitle(QApplication::translate("ArticleView", "Form", 0));
        ftsSearchPrevious->setText(QApplication::translate("ArticleView", "&Previous", 0));
        ftsSearchNext->setText(QApplication::translate("ArticleView", "&Next", 0));
        label->setText(QApplication::translate("ArticleView", "Find:", 0));
        searchCloseButton->setText(QApplication::translate("ArticleView", "x", 0));
        searchPrevious->setText(QApplication::translate("ArticleView", "&Previous", 0));
        searchNext->setText(QApplication::translate("ArticleView", "&Next", 0));
        searchNext->setShortcut(QApplication::translate("ArticleView", "Ctrl+G", 0));
        highlightAllButton->setText(QApplication::translate("ArticleView", "Highlight &all", 0));
        searchCaseSensitive->setText(QApplication::translate("ArticleView", "&Case Sensitive", 0));
    } // retranslateUi

};

namespace Ui {
    class ArticleView: public Ui_ArticleView {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ARTICLEVIEW_H
