
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



#ifndef _CHRTR23D_DEF_H_
#define _CHRTR23D_DEF_H_


#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <cerrno>
#include <cmath>


#include "nvutility.h"
#include "nvutility.hpp"


#include <gdal.h>
#include <gdal_priv.h>
#include <cpl_string.h>
#include <ogr_spatialref.h>
#include <gdalwarper.h>
#include <ogr_spatialref.h>


#include "chrtr2.h"
#include "chrtr2_shared.h"

#include <QtCore>
#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
#include <QSharedMemory>


#ifdef NVWIN3X
#include "windows_getuid.h"
using namespace std;  // Windoze bullshit - God forbid they should follow a standard
#endif


//  Pointer interaction functions.

#define         NOOP                        0
#define         ROTATE                      1
#define         ZOOM                        2
//#define         DRAG                        3


#define         H_NONE                      0
#define         H_ALL                       1
#define         H_CHECKED                   2
#define         H_01                        3
#define         H_02                        4
#define         H_03                        5
#define         H_04                        6
#define         H_05                        7
#define         H_INT                       8


#define         NUMSHADES                   256
#define         NUMHUES                     128
#define         LINE_WIDTH                  2
#define         POLYGON_POINTS              NVMAP_POLYGON_POINTS
#define         HOTKEYS                     10


#define         MARKER_W                    15
#define         MARKER_H                    10


//  The OPTIONS structure contains all those variables that can be saved to the users gridEdit QSettings.


typedef struct
{
  int32_t       position_form;              //  Position format number
  QColor        background_color;           //  Color to be used for background
  QColor        tracker_color;              //  Color to be used for track cursor
  QColor        edit_color;                 //  Color to be used for edit rectangles and polygons
  QColor        scale_color;                //  Color to be used for the "scale"
  QFont         font;                       //  Font used for all ABE map GUI applications
  int32_t       zoom_percent;               //  Zoom in/out percentage
  double        exaggeration;               //  Z exaggeration
  float         z_factor;                   //  Coversion factor for Z values. May be used to convert m to ft...
  float         z_offset;                   //  Offset value for Z values.
  int32_t       highlight;                  //  See H_NONE and others above for definitions
  int32_t       edit_mode;                  //  Saved "misc.function"
  uint8_t       draw_scale;                 //  Set this to draw the scale in the display.
} OPTIONS;


//  General stuff.

typedef struct
{
  int32_t       function;                   //  Active edit function
  int32_t       save_function;              //  Save last edit function
  char          GeoTIFF_name[512];          //  GeoTIFF file name
  uint8_t       GeoTIFF_open;               //  Set if GeoTIFF file has been opened.
  uint8_t       GeoTIFF_init;               //  Set if GeoTIFF is new
  int32_t       display_GeoTIFF;            //  0 - no display, 1 - display as decal, -1 display modulated with depth color
  uint8_t       drawing;                    //  set if we are drawing the surface
  uint8_t       drawing_canceled;           //  set to cancel drawing
  NV_F64_XYMBR  displayed_area[MAX_CHRTR2_FILES]; //  displayed area for each of the CHRTR2(s)
  NV_F64_XYMBR  total_displayed_area;       //  displayed area for all of the displayed CHRTR2s
  NV_F64_XYMBR  prev_mbr;                   //  previously displayed area
  int32_t       displayed_area_width[MAX_CHRTR2_FILES]; //  displayed area width in columns
  int32_t       displayed_area_height[MAX_CHRTR2_FILES]; //  displayed area height in rows
  int32_t       displayed_area_column[MAX_CHRTR2_FILES]; //  displayed area start column
  int32_t       displayed_area_row[MAX_CHRTR2_FILES]; //  displayed area start row
  float         displayed_area_min;         //  min Z value for displayed area
  float         displayed_area_max;         //  max Z value for displayed area
  float         displayed_area_range;       //  Z range for displayed area
  QColor        color_array[2][NUMHUES + 1][NUMSHADES]; //  arrays of surface colors
  QColor        widgetBackgroundColor;      //  The normal widget background color.
  QSharedMemory *chrtr2Share;               //  CHRTR2's shared memory pointer.
  CHRTR2_SHARE   *chrtr2_share;             //  Pointer to the CHRTR2_SHARE structure in shared memory.
  double        map_center_x;               //  Center of the map to be used for view reset.
  double        map_center_y;               //  Center of the map to be used for view reset.
  double        map_center_z;               //  Center of the map to be used for view reset.
  int32_t       width;                      //  Main window width
  int32_t       height;                     //  Main window height
  int32_t       window_x;                   //  Main window x position
  int32_t       window_y;                   //  Main window y position
  char          progname[256];              /*  This is the program name.  It will be used in all output to stderr so that shelling programs
                                                will know what program printed out the error message.  */


  //  The following concern CHRTR2s as layers.  There are a few things from CHRTR2_SHARE that also need to be 
  //  addressed when playing with layers - open_args, display_chrtr2, and chrtr2_count.

  NV_F64_XYMBR  total_mbr;                  //  MBR of all of the displayed CHRTR2s
  int32_t       chrtr2_handle[MAX_CHRTR2_FILES];   //  CHRTR2 file handle
  CHRTR2_HEADER chrtr2_header[MAX_CHRTR2_FILES];   //  CHRTR2 file header structures
  int32_t       layer[MAX_CHRTR2_FILES];
} MISC;


#endif
