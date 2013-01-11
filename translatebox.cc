/* This file is (c) 2012 Tvangeste <i.4m.l33t@yandex.ru>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "translatebox.hh"

#include <QHBoxLayout>
#include <QEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QDebug>
#include <QModelIndex>

namespace
{
#define MAX_POPUP_ROWS 17
}

CompletionList::CompletionList(QWidget *parent) : QListWidget(parent)
{
  setWindowFlags(Qt::ToolTip);
  setMaximumWidth(1000);

  connect(this, SIGNAL( activated( QModelIndex ) ),
          this, SLOT( acceptCurrentEntry() ) );

  connect(this, SIGNAL( itemClicked( QListWidgetItem * ) ),
          this, SLOT( acceptCurrentEntry() ) );
}

int CompletionList::preferredHeight() const
{
  const QSize itemSizeHint = itemDelegate()->sizeHint(viewOptions(), model()->index( 0, 0 ) );
  return itemSizeHint.height() * MAX_POPUP_ROWS + frameWidth() * 2;
}

bool CompletionList::acceptCurrentEntry()
{
  if (!isVisible())
  {
    return false;
  }

  const QModelIndex index = currentIndex();
  if ( !index.isValid() )
  {
    return false;
  }

  emit doubleClicked(index);
  hide();

  return true;
}

TranslateBox::TranslateBox(QWidget *parent) : QWidget(parent),
  word_list(new CompletionList(this))
{
  // initially hidden
  word_list->hide();

  resize(200, 90);
  QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  setSizePolicy(sizePolicy);
  // setMinimumSize(QSize(800, 0));

  translate_line = new ExtLineEdit(this);
  setFocusProxy(translate_line);
  translate_line->setObjectName("translateLine");
#if QT_VERSION >= 0x040700
  translate_line->setPlaceholderText( tr( "Type a word or phrase to search dictionaries" ) );
#endif

  // completer = new QCompleter(m_completionList->model(), this);
  // completer->setCaseSensitivity(Qt::CaseInsensitive);
  // completer->setCompletionMode(QCompleter::InlineCompletion);

  QHBoxLayout *layout = new QHBoxLayout(this);
  setLayout(layout);
  layout->setMargin(0);
  layout->addWidget(translate_line);

  QPixmap image(":/icons/system-search.png");
  translate_line->setButtonPixmap(ExtLineEdit::Left, image.scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  // translate_line->setButtonToolTip(ExtLineEdit::Left, tr("Options"));
  translate_line->setButtonVisible(ExtLineEdit::Left, true);
  translate_line->setButtonFocusPolicy(ExtLineEdit::Left, Qt::ClickFocus);

  QPixmap right(":/icons/1downarrow.png");
  translate_line->setButtonPixmap(ExtLineEdit::Right, right);
  translate_line->setButtonToolTip(ExtLineEdit::Right, tr("Drop-down"));
  translate_line->setButtonVisible(ExtLineEdit::Right, true);
  translate_line->setButtonFocusPolicy(ExtLineEdit::Right, Qt::NoFocus);

  translate_line->setFocusPolicy(Qt::ClickFocus);

  translate_line->installEventFilter( this );
  this->installEventFilter( this );

  connect(translate_line, SIGNAL( textChanged( QString const & ) ),
          this, SLOT( showPopup() ) );

  connect(translate_line, SIGNAL( rightButtonClicked() ),
          this, SLOT( rightButtonClicked() ) );
}

bool TranslateBox::eventFilter(QObject *obj, QEvent *event)
{
    // hide the suggestions list when the window is not active
    if ( event->type() == QEvent::WindowDeactivate )
    {
      if (!word_list->isActiveWindow())
      {
        word_list->hide();
      }
      return false;
    }

    if (obj == translate_line && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
          if ( !word_list->isVisible() )
          {
            showPopup();
          }
          else
          {
            QApplication::sendEvent(word_list, event);
          }
          return true;
        case Qt::Key_Enter:
        case Qt::Key_Return:
          return word_list->acceptCurrentEntry();
        case Qt::Key_Escape:
          word_list->hide();
          return true;
        case Qt::Key_Tab:
          if ( !word_list->isVisible() )
          {
            showPopup();
          }
          else
          {
            QKeyEvent event( QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier );
            QApplication::sendEvent( word_list, &event );
          }
          return true;
        case Qt::Key_Backtab:
          if ( !word_list->isVisible() )
          {
            showPopup();
          }
          else
          {
            QKeyEvent event( QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier );
            QApplication::sendEvent( word_list, &event );
          }
          return true;
        default:
            break;
        }
    } else if (obj == translate_line && event->type() == QEvent::FocusOut) {
#if defined(Q_OS_WIN)
        QFocusEvent *fev = static_cast<QFocusEvent*>(event);
        if (fev->reason() != Qt::ActiveWindowFocusReason ||
            (fev->reason() == Qt::ActiveWindowFocusReason && !word_list->isActiveWindow()))
#endif
          word_list->hide();
    } else if (obj == translate_line && event->type() == QEvent::FocusIn) {
      // By default, focusing the traslate line does not show
      // the popup window.
      // showPopup();
    } else if (obj == this && event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_Escape && !ke->modifiers() && word_list->isVisible() ) {
            event->accept();
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void TranslateBox::showPopup()
{
  // completer->setCompletionPrefix( m_fileLineEdit->text() );
  // qDebug() << "COMPLETION:" << completer->currentCompletion();

  if (translate_line->text().trimmed().isEmpty())
  {
    // nothing to show
    if (word_list->isVisible())
    {
      hide();
      translate_line->setFocus();
    }

    return;
  }


  const QSize size(width(), word_list->preferredHeight());

  const QRect rect(mapToGlobal( QPoint( 0, translate_line->y() + translate_line->height() ) ), size );

  word_list->setGeometry(rect);
  word_list->show();
  translate_line->setFocus();
}

QLineEdit * TranslateBox::translateLine()
{
  return translate_line;
}

QListWidget * TranslateBox::wordList()
{
  return word_list;
}

void TranslateBox::rightButtonClicked()
{
  if ( word_list->isVisible() )
  {
    word_list->hide();
  }
  else
  {
    showPopup();
  }
}
