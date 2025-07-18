#ifndef LIONSUPPORT_H
#define LIONSUPPORT_H

#include "mainwindow.hh"

class LionSupport
{
public:
    /**
     * Returns whether the current system is Lion.
     */
    static bool isLion();

    /**
     * Adds fullscreen button to window for Lion.
     */
    static void addFullscreen(MainWindow *window);

    //Check for retina display
    static bool isRetinaDisplay();
};

#endif // LIONSUPPORT_H
