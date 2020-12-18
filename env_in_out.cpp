
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



#include "grid3D.hpp"


double settings_version = 2.30;


/*!
  These functions store and retrieve the program settings (environment) from a .ini file.  On both Linux and Windows
  the file will be called grid3D.ini and will be stored in a directory called ABE.config.  On Linux, the ABE.config
  directory will be stored in your $HOME directory.  On Windows, it will be stored in your $USERPROFILE folder.  If
  you make a change to the way a variable is used and you want to force the defaults to be restored just change the
  settings_version to a newer number (I've been using the program version number from version.hpp - which you should
  be updating EVERY time you make a change to the program!).  You don't need to change the settings_version though
  unless you want to force the program to go back to the defaults (which can annoy your users).  So, the
  settings_version won't always match the program version.
*/

uint8_t envin (OPTIONS *options, QMainWindow *mainWindow)
{
  double saved_version = 0.0;
  QString string;


  //  Get the INI file name

#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/grid3D.ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/grid3D.ini";
#endif

  QSettings settings (ini_file, QSettings::IniFormat);
  settings.beginGroup ("grid3D");

  saved_version = settings.value (QString ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return (NVFalse);


  options->edit_mode = settings.value (QString ("edit mode"), options->edit_mode).toInt ();

  options->position_form = settings.value (QString ("position form"), options->position_form).toInt ();

  options->zoom_percent = settings.value (QString ("zoom percentage"), options->zoom_percent).toInt ();

  options->exaggeration = settings.value (QString ("z exaggeration"), options->exaggeration).toDouble ();

  options->draw_scale = settings.value (QString ("draw scale flag"), options->draw_scale).toBool ();

  int32_t red = settings.value (QString ("tracker color/red"), options->tracker_color.red ()).toInt ();
  int32_t green = settings.value (QString ("tracker color/green"), options->tracker_color.green ()).toInt ();
  int32_t blue = settings.value (QString ("tracker color/blue"), options->tracker_color.blue ()).toInt ();
  int32_t alpha = settings.value (QString ("tracker color/alpha"), options->tracker_color.alpha ()).toInt ();
  options->tracker_color.setRgb (red, green, blue, alpha);


  red = settings.value (QString ("background color/red"), options->background_color.red ()).toInt ();
  green = settings.value (QString ("background color/green"), options->background_color.green ()).toInt ();
  blue = settings.value (QString ("background color/blue"), options->background_color.blue ()).toInt ();
  alpha = settings.value (QString ("background color/alpha"), options->background_color.alpha ()).toInt ();
  options->background_color.setRgb (red, green, blue, alpha);

  red = settings.value (QString ("edit color/red"), options->edit_color.red ()).toInt ();
  green = settings.value (QString ("edit color/green"), options->edit_color.green ()).toInt ();
  blue = settings.value (QString ("edit color/blue"), options->edit_color.blue ()).toInt ();
  alpha = settings.value (QString ("edit color/alpha"), options->edit_color.alpha ()).toInt ();
  options->edit_color.setRgb (red, green, blue, alpha);

  red = settings.value (QString ("scale color/red"), options->scale_color.red ()).toInt ();
  green = settings.value (QString ("scale color/green"), options->scale_color.green ()).toInt ();
  blue = settings.value (QString ("scale color/blue"), options->scale_color.blue ()).toInt ();
  alpha = settings.value (QString ("scale color/alpha"), options->scale_color.alpha ()).toInt ();
  options->scale_color.setRgb (red, green, blue, alpha);


  mainWindow->restoreState (settings.value (QString ("main window state")).toByteArray (), 
                            (int32_t) (settings_version * 100.0));

  mainWindow->restoreGeometry (settings.value (QString ("main window geometry")).toByteArray ());

  settings.endGroup ();


  //  We need to get the font from the global settings since it will be used in (hopefully) all of the ABE map GUI applications.

#ifdef NVWIN3X
  QString ini_file2 = QString (getenv ("USERPROFILE")) + "/ABE.config/" + "globalABE.ini";
#else
  QString ini_file2 = QString (getenv ("HOME")) + "/ABE.config/" + "globalABE.ini";
#endif

  QSettings settings2 (ini_file2, QSettings::IniFormat);
  settings2.beginGroup ("globalABE");


  QString defaultFont = options->font.toString ();
  QString fontString = settings2.value (QString ("ABE map GUI font"), defaultFont).toString ();
  options->font.fromString (fontString);


  settings2.endGroup ();


  return (NVTrue);
}


void envout (OPTIONS *options, QMainWindow *mainWindow)
{
  //  Get the INI file name

#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/grid3D.ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/grid3D.ini";
#endif

  QSettings settings (ini_file, QSettings::IniFormat);
  settings.beginGroup ("grid3D");


  settings.setValue (QString ("settings version"), settings_version);


  settings.setValue (QString ("edit mode"), options->edit_mode);

  settings.setValue (QString ("position form"), options->position_form);

  settings.setValue (QString ("zoom percentage"), options->zoom_percent);

  settings.setValue (QString ("z exaggeration"), options->exaggeration);

  settings.setValue (QString ("draw scale flag"), options->draw_scale);

  settings.setValue (QString ("background color/red"), options->background_color.red ());
  settings.setValue (QString ("background color/green"), options->background_color.green ());
  settings.setValue (QString ("background color/blue"), options->background_color.blue ());
  settings.setValue (QString ("background color/alpha"), options->background_color.alpha ());

  settings.setValue (QString ("tracker color/red"), options->tracker_color.red ());
  settings.setValue (QString ("tracker color/green"), options->tracker_color.green ());
  settings.setValue (QString ("tracker color/blue"), options->tracker_color.blue ());
  settings.setValue (QString ("tracker color/alpha"), options->tracker_color.alpha ());


  settings.setValue (QString ("edit color/red"), options->edit_color.red ());
  settings.setValue (QString ("edit color/green"), options->edit_color.green ());
  settings.setValue (QString ("edit color/blue"), options->edit_color.blue ());
  settings.setValue (QString ("edit color/alpha"), options->edit_color.alpha ());


  settings.setValue (QString ("main window state"), mainWindow->saveState ((int32_t) (settings_version * 100.0)));

  settings.setValue (QString ("main window geometry"), mainWindow->saveGeometry ());

  settings.endGroup ();
}
