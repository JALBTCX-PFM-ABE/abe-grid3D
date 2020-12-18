
/*********************************************************************************************

    This is public domain software that was developed by or for the U.S. Naval Oceanographic
    Office and/or the U.S. Army Corps of Engineers.

    This is a work of the U.S. Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the U.S. Government.

    Neither the United States Government, nor any employees of the United States Government,
    nor the author, makes any warranty, express or implied, without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.
*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



/*  grid3D class definitions.  */

#ifndef _GRID3D_H_
#define _GRID3D_H_

#include <cmath>
#include <cerrno>
#include <getopt.h>

#include "grid3DDef.hpp"
#include "prefs.hpp"
#include "hotkeyHelp.hpp"
#include "version.hpp"




using namespace std;  // Windoze bullshit - God forbid they should follow a standard


class grid3D:public QMainWindow
{
  Q_OBJECT 


public:

  grid3D (int *argc = 0, char **argv = 0, QWidget *parent = 0);
  ~grid3D ();

  void initializeMaps (uint8_t reset);
  void clean_exit (int32_t ret);


protected:

  OPTIONS         options;

  MISC            misc;

  CHRTR2_SHARE    chrtr2_share;

  prefs           *prefs_dialog;

  int32_t         start_ctrl_x, start_ctrl_y;

  NV_I32_COORD2   marker[16];

  QTimer          *trackCursor;

  QCheckBox       *sMessage;

  QMouseEvent     *menu_mouse_event;

  nvMapGL         *map;

  int32_t         menu_cursor_x, menu_cursor_y;

  QCursor         rotateCursor, zoomCursor;

  NVMAPGL_DEF     mapdef;

  int32_t         prev_poly_x, prev_poly_y;

  QMenu           *popupMenu;

  QAction         *bHelp, *popup0, *popup1, *popup2, *popup3;

  QToolButton     *bQuit, *bGeoOpen, *bPrefs, *bReset, *bGeotiff;

  QScrollBar      *exagBar;

  QPalette        exagPalette;

  int32_t         mv_marker, mv_tracker;

  int32_t         active_window_id;

  double          menu_cursor_lon, menu_cursor_lat, menu_cursor_z;

  uint8_t         got_geotiff, popup_active, double_click;

  double          mid_lat, mid_lon, mid_z;



  void setMainButtons (uint8_t enabled);
  void discardMovableObjects ();
  void setFunctionCursor (int32_t function);
  void leftMouse (int32_t mouse_x, int32_t mouse_y, double lon, double lat, double z);
  void midMouse (int32_t mouse_x, int32_t mouse_y, double lon, double lat, double z);
  void rightMouse (int32_t mouse_x, int32_t mouse_y, double lon, double lat, double z);
  void focusInEvent (QFocusEvent *e);


protected slots:

  void slotMouseDoubleClick (QMouseEvent *e, double lon, double lat, double z);
  void slotMousePress (QMouseEvent *e, double lon, double lat, double z);
  void slotPreliminaryMousePress (QMouseEvent *e);
  void slotMouseRelease (QMouseEvent *e, double lon, double lat, double z);
  void slotWheel (QWheelEvent *e, double lon, double lat, double z);
  void slotMouseMove (QMouseEvent *e, double lon, double lat, double z, NVMAPGL_DEF l_mapdef);
  void slotResize (QResizeEvent *e);

  void slotExagTriggered (int action);
  void slotExagReleased ();
  void slotExaggerationChanged (float exaggeration, float apparent_exaggeration);

  void slotClose (QCloseEvent *e);

  void slotTrackCursor ();

  void slotPopupMenu0 ();
  void slotPopupMenu1 ();
  void slotPopupMenu2 ();
  void slotPopupHelp ();

  void slotGeotiff ();
  void slotReset ();

  void slotOpenGeotiff ();
  void slotPrefs ();
  void slotPrefDataChanged ();

  void slotHotkeyHelp ();
  void about ();
  void slotAcknowledgments ();
  void aboutQt ();

  void slotQuit ();


private:

  void keyPressEvent (QKeyEvent *e);

};

#endif
