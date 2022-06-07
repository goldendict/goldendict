/* This file is (c) 2013 Timon Wong <timon86.wang@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __TEXTTOSPEECHSOURCE_HH_INCLUDED__
#define __TEXTTOSPEECHSOURCE_HH_INCLUDED__

#include "ui_texttospeechsource.h"
#include "config.hh"
#include "speechclient.hh"

#include <QComboBox>
#include <QStyledItemDelegate>

/// A model to be projected into the text to speech view, according to Qt's MVC model
class VoiceEnginesModel: public QAbstractItemModel
{
  Q_OBJECT

public:

  enum {
    kColumnEnabled  = 0,
    kColumnEngineId,
    kColumnEngineName,
    kColumnIcon,
    kColumnCount
  };

  VoiceEnginesModel( QWidget * parent, Config::VoiceEngines const & voiceEngines );

  void removeVoiceEngine( int index );
  void addNewVoiceEngine( QString const & id, QString const & name,
                          int volume, int rate );

  Config::VoiceEngines const & getCurrentVoiceEngines() const
  { return voiceEngines; }
  void setEngineParams( QModelIndex idx, int volume, int rate );

  QModelIndex index( int row, int column, QModelIndex const & parent ) const;
  QModelIndex parent( QModelIndex const & parent ) const;
  Qt::ItemFlags flags( QModelIndex const & index ) const;
  int rowCount( QModelIndex const & parent ) const;
  int columnCount( QModelIndex const & parent ) const;
  QVariant headerData( int section, Qt::Orientation orientation, int role ) const;
  QVariant data( QModelIndex const & index, int role ) const;
  bool setData( QModelIndex const & index, const QVariant & value, int role );

private:

  Config::VoiceEngines voiceEngines;
};

class VoiceEngineEditor: public QComboBox
{
  Q_OBJECT

public:
  VoiceEngineEditor( SpeechClient::Engines const & engines, QWidget * parent = 0 );

public:
  QString engineName() const;
  QString engineId() const;
  void setEngineId( QString const & engineId );
};

class VoiceEngineItemDelegate: public QStyledItemDelegate
{
  Q_OBJECT

public:
  VoiceEngineItemDelegate( SpeechClient::Engines const & engines, QObject * parent = 0 );

  virtual QWidget * createEditor( QWidget *parent,
                                  QStyleOptionViewItem const & option,
                                  QModelIndex const & index ) const;
  virtual void setEditorData( QWidget *uncastedEditor, const QModelIndex & index ) const;
  virtual void setModelData( QWidget *uncastedEditor, QAbstractItemModel * model,
                             const QModelIndex & index ) const;

private:
  SpeechClient::Engines engines;
};

class TextToSpeechSource: public QWidget
{
  Q_OBJECT

public:
  TextToSpeechSource( QWidget * parent, Config::VoiceEngines voiceEngines );

  const VoiceEnginesModel & getVoiceEnginesModel() const
  { return voiceEnginesModel; }

private slots:
  void on_addVoiceEngine_clicked();
  void on_removeVoiceEngine_clicked();
  void on_previewVoice_clicked();
  void previewVoiceFinished();
  void slidersChanged();
  void selectionChanged();

private:
  Ui::TextToSpeechSource ui;
  VoiceEnginesModel voiceEnginesModel;

  void fitSelectedVoiceEnginesColumns();
  void adjustSliders();
};

#endif // __TEXTTOSPEECHSOURCE_HH_INCLUDED__
