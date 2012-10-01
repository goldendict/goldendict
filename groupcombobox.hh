/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __GROUPCOMBOBOX_HH_INCLUDED__
#define __GROUPCOMBOBOX_HH_INCLUDED__

#include <QComboBox>
#include <QAction>
#include "instances.hh"

/// This is a combo box which is for choosing the dictionary group
class GroupComboBox: public QComboBox
{
  Q_OBJECT

public:

  GroupComboBox( QWidget * parent );

  /// Fills combo-box with the given groups
  void fill( Instances::Groups const & );

  /// Chooses the given group in the combobox. If there's no such group,
  /// does nothing.
  void setCurrentGroup( unsigned id );


  /// Returns current group.
  unsigned getCurrentGroup() const;

protected:

  /// We handle shortcut events here.
  virtual bool event( QEvent * event );

private slots:

  void popupGroups();
  void selectNextGroup();
  void selectPreviousGroup();

private:

  QAction popupAction;
  QAction selectNextAction, selectPreviousAction;
  QMap< int, int > shortcuts;
};

#endif

