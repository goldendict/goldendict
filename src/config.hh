/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __CONFIG_HH_INCLUDED__
#define __CONFIG_HH_INCLUDED__

#include <vector>
#include <QString>
#include "ex.hh"

/// GoldenDict's configuration
namespace Config {

using std::vector;

/// A list of paths where to search for the dictionaries
typedef vector< QString > Paths;

/// A dictionary group
struct Group
{
  QString name, icon;
  vector< QString > dictionaries; // consists of dictionary's ids
};

/// All the groups
typedef vector< Group > Groups;

/// Various user preferences
struct Preferences
{
  bool enableTrayIcon;
  bool startToTray;
  bool closeToTray;
  bool enableScanPopup;
  bool enableScanPopupModifiers;
  unsigned long scanPopupModifiers; // Combination of KeyboardState::Modifier

  Preferences();
};

struct Class
{
  Paths paths;
  Groups groups;
  Preferences preferences;

  QString lastMainGroup; // Last used group in main window
  QString lastPopupGroup; // Last used group in popup window
};

DEF_EX( exError, "Error with the program's configuration", std::exception )
DEF_EX( exCantUseHomeDir, "Can't use home directory to store GoldenDict preferences", exError )
DEF_EX( exCantUseIndexDir, "Can't use index directory to store GoldenDict index files", exError )
DEF_EX( exCantReadConfigFile, "Can't read the configuration file", exError )
DEF_EX( exCantWriteConfigFile, "Can't write the configuration file", exError )
DEF_EX( exMalformedConfigFile, "The configuration file is malformed", exError )

/// Loads the configuration, or creates the default one if none is present
Class load() throw( exError );

/// Saves the configuration
void save( Class const & ) throw( exError );

/// Returns the index directory, where the indices are to be stored.
QString getIndexDir() throw( exError );

/// Returns the user .css file name.
QString getUserCssFileName() throw( exError );

/// Returns the user .css file name for the Qt interface customization.
QString getUserQtCssFileName() throw( exError );

}

#endif

