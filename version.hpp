
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




#ifndef VERSION

#define     VERSION     "PFM Software - grid3D V1.1 - 10/23/17"

#endif

/*! <pre>

    Version 1.00
    Jan C. Depner
    08/25/10

    First working version.


    Version 1.01
    Jan C. Depner
    11/08/10

    Minor mods for auto scale changes to nvMapGL.cpp in nvutility library.


    Version 1.02
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


    Version 1.03
    Jan C. Depner
    01/15/11

    Added an exaggeration scrollbar to the left side of the window.


    Version 1.04
    Jan C. Depner
    04/15/11

    Fixed the geoTIFF reading by switching to using GDAL instead of Qt.  Hopefully Qt will get fixed eventually.


    Version 1.05
    Jan C. Depner
    11/30/11

    Converted .xpm icons to .png icons.


    Version 1.06
    Jan C. Depner
    01/20/12

    Now checks for CHRTR2_METERS units.  If the chrtr2 file is not in CHRTR2_METERS we abort.


    Version 1.07
    Jan C. Depner (PFM Software)
    12/09/13

    Switched to using .ini file in $HOME (Linux) or $USERPROFILE (Windows) in the ABE.config directory.  Now
    the applications qsettings will not end up in unknown places like ~/.config/navo.navy.mil/blah_blah_blah on
    Linux or, in the registry (shudder) on Windows.


    Version 1.08
    Jan C. Depner (PFM Software)
    02/28/14

    Reverted to using Qt to read geoTIFFs.  It quit working in Qt 4.7.2 but is working again.  It's
    much faster than using GDAL and it's probably a lot more bulletproof.


    Version 1.09
    Jan C. Depner (PFM Software)
    03/19/14

    - Straightened up the Open Source acknowledgments.


    Version 1.10
    Jan C. Depner (PFM Software)
    05/27/14

    - Added the new LGPL licensed GSF library to the acknowledgments.


    Version 1.11
    Jan C. Depner (PFM Software)
    07/23/14

    - Switched from using the old NV_INT64 and NV_U_INT32 type definitions to the C99 standard stdint.h and
      inttypes.h sized data types (e.g. int64_t and uint32_t).


    Version 1.12
    Jan C. Depner (PFM Software)
    02/16/15

    - To give better feedback to shelling programs in the case of errors I've added the program name to all
      output to stderr.


    Version 1.13
    Jan C. Depner (PFM Software)
    08/09/16

    - Changed scroll wheel zoom behavior to work like pfm3D.  In other words, scroll to zoom/stop scroll to stop zoom.
    - Now gets font from globalABE.ini as set in Preferences by pfmView.
    - To avoid possible confusion, removed translation setup from QSettings in env_in_out.cpp.


    Version 1.14
    Jan C. Depner (PFM Software)
    10/23/17

    - A bunch of changes to support doing translations in the future.  There is a generic
      grid3D_xx.ts file that can be run through Qt's "linguist" to translate to another language.

</pre>*/
