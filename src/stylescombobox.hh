/* Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __STYLESCOMBOBOX_HH_INCLUDED__
#define __STYLESCOMBOBOX_HH_INCLUDED__

#include <QComboBox>
#include <QString>

/// This is a combo box which is for choosing the add-on styles
class StylesComboBox : public QComboBox
{
  Q_OBJECT

public:

  StylesComboBox( QWidget * parent );

  /// Fills combo-box with the given groups
  void fill();

  /// Chooses the given style in the combobox. If there's no such style,
  /// set to "None".
  void setCurrentStyle( QString const & style );

  /// Returns current style.
  QString getCurrentStyle() const;

};

#endif // __STYLESCOMBOBOX_HH_INCLUDED__
