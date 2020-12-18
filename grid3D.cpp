
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



//  grid3D class.

/***************************************************************************\

    Module :        grid3D

    Programmer :    Jan C. Depner

    Date :          08/25/10

    Purpose :       Simple 3D CHRTR2 bin viewer.

    Caveats :       This application only runs from within gridEdit.  That
                    is, it doesn't run standalone.

\***************************************************************************/

#include "grid3D.hpp"
#include "grid3DHelp.hpp"
#include "acknowledgments.hpp"


grid3D::grid3D (int *argc, char **argv, QWidget * parent):
  QMainWindow (parent, 0)
{
  extern char     *optarg;


  uint8_t envin (OPTIONS *options, QMainWindow *mainWindow);
  void set_defaults (MISC *misc, OPTIONS *options, uint8_t restore);


  strcpy (misc.progname, argv[0]);
  got_geotiff = NVFalse;
  int32_t option_index = 0;
  int32_t shmid = 0;
  popup_active = NVFalse;
  prefs_dialog = NULL;


  strcpy (misc.GeoTIFF_name, " ");

  for (int32_t layer = 0 ; layer < MAX_CHRTR2_FILES ; layer++) misc.chrtr2_handle[layer] = -1;


  while (NVTrue) 
    {
      static struct option long_options[] = {{"shared_memory_key", required_argument, 0, 0},
                                             {0, no_argument, 0, 0}};

      char c = (char) getopt_long (*argc, argv, "G:", long_options, &option_index);
      if (c == -1) break;

      switch (c) 
        {
        case 0:

          switch (option_index)
            {
            case 0:
              sscanf (optarg, "%d", &shmid);
              break;
            }
          break;

        case 'G':
          strcpy (misc.GeoTIFF_name, optarg);
          got_geotiff = NVTrue;
          break;
        }
    }


  //  We must have the shared memory ID.

  if (!shmid)
    {
      QMessageBox::critical (this, "grid3D", tr ("grid3D can only be run from gridEdit using shared memory."),
                             QMessageBox::Close);
      exit (-1);
    }


  //  Get the shared memory area.  If it doesn't exist, quit.  It should have already been created by gridEdit.  The key is the process ID of the parent
  //  plus "_chrtr2".  The parent process ID is a required command line argument (--shared_memory_id).

  QString skey;
  skey.sprintf ("%d_chrtr2", shmid);


  misc.chrtr2Share = new QSharedMemory (skey);

  if (misc.chrtr2Share->attach (QSharedMemory::ReadWrite)) misc.chrtr2_share = (CHRTR2_SHARE *) misc.chrtr2Share->data ();



  options.position_form = misc.chrtr2_share->position_form;
  options.z_factor = misc.chrtr2_share->z_factor;
  options.z_offset = misc.chrtr2_share->z_offset;


  //  Have to set the focus policy or keypress events don't work properly at first in Focus Follows Mouse mode

  setFocusPolicy (Qt::WheelFocus);


  //  Set a few defaults for startup

  mv_marker = -1;
  mv_tracker = -1;
  active_window_id = getpid ();
  start_ctrl_x = -1;
  start_ctrl_y = -1;
  double_click = NVFalse;


  //  Set up the marker data list

  marker[0].x = -MARKER_W;
  marker[0].y = -MARKER_H;
  marker[1].x = MARKER_W;
  marker[1].y = -MARKER_H;
  marker[2].x = MARKER_W;
  marker[2].y = -MARKER_H;
  marker[3].x = MARKER_W;
  marker[3].y = MARKER_H;
  marker[4].x = MARKER_W;
  marker[4].y = MARKER_H;
  marker[5].x = -MARKER_W;
  marker[5].y = MARKER_H;
  marker[6].x = -MARKER_W;
  marker[6].y = MARKER_H;
  marker[7].x = -MARKER_W;
  marker[7].y = -MARKER_H;
  marker[8].x = -MARKER_W;
  marker[8].y = 0;
  marker[9].x = -(MARKER_W / 5);
  marker[9].y = 0;
  marker[10].x = (MARKER_W / 5);
  marker[10].y = 0;
  marker[11].x = MARKER_W;
  marker[11].y = 0;
  marker[12].x = 0;
  marker[12].y = -MARKER_H;
  marker[13].x = 0;
  marker[13].y = -(MARKER_W / 5);
  marker[14].x = 0;
  marker[14].y = (MARKER_W / 5);
  marker[15].x = 0;
  marker[15].y = MARKER_H;


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/grid3D.jpg"));


  rotateCursor = QCursor (QPixmap (":/icons/rotate_cursor.png"), 17, 17);
  zoomCursor = QCursor (QPixmap (":/icons/zoom_cursor.png"), 11, 11);


  this->setWindowTitle (QString (VERSION));


  //  This is the "tools" toolbar.  We have to do this here so that we can restore the toolbar location(s).

  QToolBar *tools = addToolBar (tr ("Tools"));
  tools->setObjectName (tr ("grid3D main toolbar"));


  //  Set all of the defaults

  set_defaults (&misc, &options, NVFalse);


  //  Get the user's defaults if available

  if (!envin (&options, this))
    {
      //  Set the geometry from defaults since we didn't get any from the saved settings.

      this->resize (misc.width, misc.height);
      this->move (misc.window_x, misc.window_y);
    }


  // Set the application font

  QApplication::setFont (options.font);


  statusBar ()->setSizeGripEnabled (false);
  statusBar ()->showMessage (VERSION);


  QFrame *frame = new QFrame (this, 0);

  setCentralWidget (frame);


  //  Set the map values from the defaults

  mapdef.draw_width = misc.width;
  mapdef.draw_height = misc.height;
  mapdef.zoom_percent = options.zoom_percent;
  mapdef.exaggeration = options.exaggeration;
  mapdef.mode = NVMAPGL_BIN_MODE;
  mapdef.light_model = GL_LIGHT_MODEL_TWO_SIDE;
  mapdef.auto_scale = NVFalse;
  mapdef.projected = 0;

  mapdef.background_color = options.background_color;
  mapdef.scale_color = options.scale_color;
  mapdef.draw_scale = options.draw_scale;
  mapdef.initial_zx_rotation = 0.0;
  mapdef.initial_y_rotation = 0.0;


  //  Make the map.

  map = new nvMapGL (this, &mapdef, "grid3D");

  map->setWhatsThis (mapText);


  //  Connect to the signals from the map class.

  connect (map, SIGNAL (mousePressSignal (QMouseEvent *, double, double, double)), this, 
           SLOT (slotMousePress (QMouseEvent *, double, double, double)));
  connect (map, SIGNAL (mouseDoubleClickSignal (QMouseEvent *, double, double, double)), this,
           SLOT (slotMouseDoubleClick (QMouseEvent *, double, double, double)));
  connect (map, SIGNAL (mouseReleaseSignal (QMouseEvent *, double, double, double)), this, 
           SLOT (slotMouseRelease (QMouseEvent *, double, double, double)));
  connect (map, SIGNAL (mouseMoveSignal (QMouseEvent *, double, double, double, NVMAPGL_DEF)), this, 
           SLOT (slotMouseMove (QMouseEvent *, double, double, double, NVMAPGL_DEF)));
  connect (map, SIGNAL (wheelSignal (QWheelEvent *, double, double, double)), this, 
           SLOT (slotWheel (QWheelEvent *, double, double, double)));
  connect (map, SIGNAL (resizeSignal (QResizeEvent *)), this, SLOT (slotResize (QResizeEvent *)));
  connect (map, SIGNAL (exaggerationChanged (float, float)), this, SLOT (slotExaggerationChanged (float, float)));


  exagBar = new QScrollBar (Qt::Vertical);
  exagBar->setTracking (true);
  exagBar->setInvertedAppearance (true);
  exagBar->setToolTip (tr ("Vertical exaggeration"));
  exagBar->setWhatsThis (exagBarText);
  exagPalette.setColor (QPalette::Normal, QPalette::Base, options.background_color);
  exagPalette.setColor (QPalette::Normal, QPalette::Window, options.background_color);
  exagPalette.setColor (QPalette::Inactive, QPalette::Base, options.background_color);
  exagPalette.setColor (QPalette::Inactive, QPalette::Window, options.background_color);
  exagBar->setPalette (exagPalette);
  exagBar->setMinimum (100);
  exagBar->setMaximum (10000);
  exagBar->setSingleStep (100);
  exagBar->setPageStep (500);
  exagBar->setValue (NINT (options.exaggeration * 100.0));
  connect (exagBar, SIGNAL (actionTriggered (int)), this, SLOT (slotExagTriggered (int)));
  connect (exagBar, SIGNAL (sliderReleased ()), this, SLOT (slotExagReleased ()));


  //  Layouts, what fun!

  QVBoxLayout *vBox = new QVBoxLayout (frame);


  QHBoxLayout *hBox = new QHBoxLayout ();
  hBox->addWidget (exagBar);
  hBox->addWidget (map, 1);
  vBox->addLayout (hBox, 1);


  //  Button, button, who's got the buttons?

  bQuit = new QToolButton (this);
  bQuit->setIcon (QIcon (":/icons/quit.png"));
  bQuit->setToolTip (tr ("Quit"));
  bQuit->setWhatsThis (quitText);
  connect (bQuit, SIGNAL (clicked ()), this, SLOT (slotQuit ()));
  tools->addWidget (bQuit);


  tools->addSeparator ();
  tools->addSeparator ();


  bGeoOpen = new QToolButton (this);
  bGeoOpen->setIcon (QIcon (":/icons/geotiffopen.png"));
  bGeoOpen->setToolTip (tr ("Open a GeoTIFF file"));
  bGeoOpen->setWhatsThis (geoOpenText);
  connect (bGeoOpen, SIGNAL (clicked ()), this, SLOT (slotOpenGeotiff ()));
  tools->addWidget (bGeoOpen);


  tools->addSeparator ();
  tools->addSeparator ();


  bReset = new QToolButton (this);
  bReset->setIcon (QIcon (":/icons/reset_view.png"));
  bReset->setToolTip (tr ("Reset view"));
  bReset->setWhatsThis (resetText);
  connect (bReset, SIGNAL (clicked ()), this, SLOT (slotReset ()));
  tools->addWidget (bReset);


  tools->addSeparator ();
  tools->addSeparator ();


  bGeotiff = new QToolButton (this);
  bGeotiff->setIcon (QIcon (":/icons/geo_off.png"));
  bGeotiff->setToolTip (tr ("Toggle GeoTIFF display"));
  bGeotiff->setWhatsThis (geoText);
  connect (bGeotiff, SIGNAL (clicked ()), this, SLOT (slotGeotiff ()));
  tools->addWidget (bGeotiff);


  tools->addSeparator ();
  tools->addSeparator ();


  bPrefs = new QToolButton (this);
  bPrefs->setIcon (QIcon (":/icons/prefs.png"));
  bPrefs->setToolTip (tr ("Change application preferences"));
  bPrefs->setWhatsThis (prefsText);
  connect (bPrefs, SIGNAL (clicked ()), this, SLOT (slotPrefs ()));
  tools->addWidget (bPrefs);


  tools->addSeparator ();
  tools->addSeparator ();


  QAction *bHelp = QWhatsThis::createAction (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.png"));
  tools->addAction (bHelp);


  //  Set the default function

  misc.function = NOOP;


  //  Right click popup menu

  popupMenu = new QMenu (map);

  popup0 = popupMenu->addAction ("");
  connect (popup0, SIGNAL (triggered ()), this, SLOT (slotPopupMenu0 ()));
  popup1 = popupMenu->addAction ("");
  connect (popup1, SIGNAL (triggered ()), this, SLOT (slotPopupMenu1 ()));
  popup2 = popupMenu->addAction ("");
  connect (popup2, SIGNAL (triggered ()), this, SLOT (slotPopupMenu2 ()));
  popupMenu->addSeparator ();
  popup3 = popupMenu->addAction (tr ("Help"));
  connect (popup3, SIGNAL (triggered ()), this, SLOT (slotPopupHelp ()));


  //  Setup the file menu.

  QAction *fileQuitAction = new QAction (tr ("&Quit"), this);
  fileQuitAction->setShortcut (tr ("Ctrl+Q"));
  fileQuitAction->setStatusTip (tr ("Exit from application"));
  connect (fileQuitAction, SIGNAL (triggered ()), qApp, SLOT (closeAllWindows ()));

  QMenu *fileMenu = menuBar ()->addMenu (tr ("&File"));
  fileMenu->addAction (fileQuitAction);


  //  Setup the help menu.

  QAction *whatsThisAct = QWhatsThis::createAction (this);
  whatsThisAct->setIcon (QIcon (":/icons/contextHelp.png"));

  QAction *hotHelp = new QAction (tr ("&GUI control"), this);
  hotHelp->setShortcut (tr ("Ctrl+G"));
  hotHelp->setStatusTip (tr ("Help on GUI control"));
  connect (hotHelp, SIGNAL (triggered ()), this, SLOT (slotHotkeyHelp ()));

  QAction *aboutAct = new QAction (tr ("&About"), this);
  aboutAct->setShortcut (tr ("Ctrl+A"));
  aboutAct->setStatusTip (tr ("Information about grid3D"));
  connect (aboutAct, SIGNAL (triggered ()), this, SLOT (about ()));

  QAction *acknowledgments = new QAction (tr ("A&cknowledgments"), this);
  acknowledgments->setShortcut (tr ("Ctrl+c"));
  acknowledgments->setStatusTip (tr ("Information about supporting libraries"));
  connect (acknowledgments, SIGNAL (triggered ()), this, SLOT (slotAcknowledgments ()));

  QAction *aboutQtAct = new QAction (tr ("About Qt"), this);
  aboutQtAct->setStatusTip (tr ("Information about Qt"));
  connect (aboutQtAct, SIGNAL (triggered ()), this, SLOT (aboutQt ()));

  QMenu *helpMenu = menuBar ()->addMenu (tr ("&Help"));
  helpMenu->addAction (whatsThisAct);
  helpMenu->addSeparator ();
  helpMenu->addAction (hotHelp);
  helpMenu->addSeparator ();
  helpMenu->addAction (aboutAct);
  helpMenu->addSeparator ();
  helpMenu->addAction (acknowledgments);
  helpMenu->addAction (aboutQtAct);


  //  Set the tracking timer to every 50 milliseconds.

  trackCursor = new QTimer (this);
  connect (trackCursor, SIGNAL (timeout ()), this, SLOT (slotTrackCursor ()));
  trackCursor->start (50);


  misc.GeoTIFF_open = NVFalse;
  misc.display_GeoTIFF = NVFalse;


  //  Set the GUI buttons and the cursor.

  setFunctionCursor (misc.function);
  setMainButtons (NVFalse);


  //  Open all of the CHRTR2 files.

  for (int32_t chrtr2 = misc.chrtr2_share->chrtr2_count - 1 ; chrtr2 >= 0 ; chrtr2--)
    {
      if (misc.chrtr2_handle[chrtr2] >= 0) chrtr2_close_file (misc.chrtr2_handle[chrtr2]);

      if ((misc.chrtr2_handle[chrtr2] = chrtr2_open_file (misc.chrtr2_share->chrtr2_file[chrtr2], &misc.chrtr2_header[chrtr2], CHRTR2_UPDATE)) < 0)
        {
          QMessageBox::warning (this, tr ("Open CHRTR2 File"),
                                tr ("The file %1 is not a CHRTR2 file or there was an error reading the file.\n"
                                    "The error message returned was:\n\n%2").arg 
                                (QDir::toNativeSeparators (QString (misc.chrtr2_share->chrtr2_file[chrtr2]))).arg (chrtr2_strerror ()));
          clean_exit (-1);
        }


      if (misc.chrtr2_header[chrtr2].z_units != CHRTR2_METERS)
        {
          QMessageBox::warning (this, tr ("Open CHRTR2 File"), tr ("The file %1 does not use CHRTR2_METERS units.  This is not allowed.").arg
                                (QDir::toNativeSeparators (QString (misc.chrtr2_share->chrtr2_file[chrtr2]))));
          chrtr2_close_file (misc.chrtr2_handle[chrtr2]);
          clean_exit (-1);
        }
    }


  show ();
}



grid3D::~grid3D ()
{
}



void 
grid3D::setMainButtons (uint8_t enabled)
{
  bGeoOpen->setEnabled (enabled);
  bReset->setEnabled (enabled);
  bPrefs->setEnabled (enabled);


  //  Only enable the GeoTIFF display button if we have opened a GeoTIFF file

  if (enabled && misc.GeoTIFF_open)
    {
      bGeotiff->setEnabled (enabled);
    }
  else
    {
      bGeotiff->setEnabled (false);
    }
}



void 
grid3D::initializeMaps (uint8_t reset)
{
  void paint_surface (grid3D *parent, MISC *misc, nvMapGL *map, uint8_t reset);


  static uint8_t first = NVTrue;


  setMainButtons (NVFalse);

  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();


  misc.drawing = NVTrue;


  map->enableSignals ();


  //  Clear out any overlays.

  map->resetMap ();


  //  Build and display the 3D surface.

  paint_surface (this, &misc, map, reset);


  misc.drawing = NVFalse;


  setMainButtons (NVTrue);


  if (first)
    {
      map->rotateZX (30.0);
      map->flush ();
      first = NVFalse;
    }


  qApp->restoreOverrideCursor ();
}



void 
grid3D::slotOpenGeotiff ()
{
  static QDir dir = QDir (".");
  QStringList files, filters;
  QString file;

  QFileDialog *fd = new QFileDialog (this, tr ("grid3D Open GeoTIFF"));
  fd->setViewMode (QFileDialog::List);


  fd->setDirectory (dir);
  filters << tr ("GeoTIFF (*.tif)");

  fd->setNameFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectNameFilter (tr ("GeoTIFF (*.tif)"));

  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      file = files.at (0);


      if (!file.isEmpty())
        {
          //  Check the file to make sure it is in the area

          GDALDataset        *poDataset;
          double             adfGeoTransform[6];
          double             GeoTIFF_wlon, GeoTIFF_nlat, GeoTIFF_lon_step, 
                             GeoTIFF_lat_step, GeoTIFF_elon, GeoTIFF_slat;
          int32_t            width, height;


          GDALAllRegister ();


          char path[512];
          strcpy (path, file.toLatin1 ());

          poDataset = (GDALDataset *) GDALOpen (path, GA_ReadOnly);
          if (poDataset != NULL)
            {
              if (poDataset->GetProjectionRef ()  != NULL)
                {
                  QString projRef = QString (poDataset->GetProjectionRef ());

                  if (projRef.contains ("GEOGCS"))
                    {
                      if (poDataset->GetGeoTransform (adfGeoTransform) == CE_None)
                        {
                          GeoTIFF_lon_step = adfGeoTransform[1];
                          GeoTIFF_lat_step = -adfGeoTransform[5];


                          width = poDataset->GetRasterXSize ();
                          height = poDataset->GetRasterYSize ();


                          GeoTIFF_wlon = adfGeoTransform[0];
                          GeoTIFF_nlat = adfGeoTransform[3];


                          GeoTIFF_slat = GeoTIFF_nlat - height * GeoTIFF_lat_step;
                          GeoTIFF_elon = GeoTIFF_wlon + width * GeoTIFF_lon_step;


                          if (GeoTIFF_nlat < misc.chrtr2_share->viewer_displayed_area.min_y || 
                              GeoTIFF_slat > misc.chrtr2_share->viewer_displayed_area.max_y ||
                              GeoTIFF_elon < misc.chrtr2_share->viewer_displayed_area.min_x || 
                              GeoTIFF_wlon > misc.chrtr2_share->viewer_displayed_area.max_x)
                            {
                              QMessageBox::warning (this, tr ("Open GeoTIFF"), tr ("No part of this GeoTIFF file falls within the displayed area."));
                              return;
                            }
                        }
                      else
                        {
                          QMessageBox::warning (this, tr ("Open GeoTIFF"), tr ("This program only handles unprojected GeoTIFF files."));
                          return;
                        }
                    }
                  else
                    {
                      QMessageBox::warning (this, tr ("Open GeoTIFF"), tr ("This program only handles geographic GeoTIFF files."));
                      return;
                    }
                }
              else
                {
                  QMessageBox::warning (this, tr ("Open GeoTIFF"), tr ("This file has no datum/projection information."));
                  return;
                }
            }
          else
            {
              QMessageBox::warning (this, tr ("Open GeoTIFF"), tr ("Unable to open file."));
              return;
            }

          delete poDataset;

          misc.GeoTIFF_open = NVTrue;
          bGeotiff->setEnabled (true);

          strcpy (misc.GeoTIFF_name, file.toLatin1 ());
          misc.GeoTIFF_init = NVTrue;

          bGeotiff->setEnabled (true);
          bGeotiff->setIcon (QIcon (":/icons/geo_decal.png"));
          misc.display_GeoTIFF = 1;

          initializeMaps (NVFalse);

          dir = fd->directory ().path ();
        }
    }
}



void 
grid3D::slotPrefs ()
{
  if (prefs_dialog) prefs_dialog->close ();
  prefs_dialog = new prefs (this, &options, &misc);

  connect (prefs_dialog, SIGNAL (dataChangedSignal ()), this, SLOT (slotPrefDataChanged ()));
}



//  Changed some of the preferences

void 
grid3D::slotPrefDataChanged ()
{
  map->setZoomPercent (options.zoom_percent);
  map->setExaggeration (options.exaggeration);
  
  map->setBackgroundColor (options.background_color);
  map->setScaleColor (options.scale_color);
  map->enableScale (options.draw_scale);

  initializeMaps (NVFalse);
}



void 
grid3D::discardMovableObjects ()
{
  map->closeMovingList (&mv_marker);
  map->closeMovingList (&mv_tracker);
}



void 
grid3D::leftMouse (int32_t mouse_x __attribute__ ((unused)), int32_t mouse_y __attribute__ ((unused)), double lon __attribute__ ((unused)),
                   double lat __attribute__ ((unused)), double z __attribute__ ((unused)))
{
  //  If the popup menu is up discard this mouse press

  if (popup_active)
    {
      popup_active = NVFalse;
      return;
    }


  QString file, string;
  static QDir dir = QDir (".");


  //  Actions based on the active function

  switch (misc.function)
    {
      //  Placeholder

    default:
      break;
    }
}



void 
grid3D::midMouse (int32_t mouse_x __attribute__ ((unused)), int32_t mouse_y __attribute__ ((unused)),
                 double lon __attribute__ ((unused)), double lat __attribute__ ((unused)),
                 double z __attribute__ ((unused)))
{
  //  Actions based on the active function

  switch (misc.function)
    {
      //  Placeholder

    default:
      break;
    }
}



void 
grid3D::slotMouseDoubleClick (QMouseEvent *e __attribute__ ((unused)), double lon __attribute__ ((unused)),
                             double lat __attribute__ ((unused)), double z __attribute__ ((unused)))
{
  //  Flip the double_click flag.  The right-click menu sets this to NVTrue so it will flip to NVFalse.
  //  Left-click sets it to NVFalse so it will filp to NVTrue;

  double_click = !double_click;


  //  Actions based on the active function

  switch (misc.function)
    {
      //  Placeholder

    default:
      break;
    }

  double_click = NVFalse;
}



void 
grid3D::rightMouse (int32_t mouse_x, int32_t mouse_y, double lon, double lat,
                   double z __attribute__ ((unused)))
{
  QString tmp;

  menu_cursor_lon = lon;
  menu_cursor_lat = lat;
  menu_cursor_x = mouse_x;
  menu_cursor_y = mouse_y;


  QPoint pos (mouse_x, mouse_y);


  //  Popups need global positioning

  QPoint global_pos = map->mapToGlobal (pos);

  popup0->setVisible (true);
  popup1->setVisible (true);
  popup2->setVisible (true);

  switch (misc.function)
    {
    case ROTATE:
      popup0->setVisible (false);
      popup1->setVisible (false);
      popup2->setVisible (false);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case ZOOM:
      start_ctrl_y = mouse_y;
      break;
    }
}



void 
grid3D::slotPopupMenu0 ()
{
  popup_active = NVFalse;

  switch (misc.function)
    {
      //  Placeholder

    default:
      break;
    }
}



void 
grid3D::slotPopupMenu1 ()
{
  popup_active = NVFalse;
}



void 
grid3D::slotPopupMenu2 ()
{
  popup_active = NVFalse;
}



void 
grid3D::slotPopupHelp ()
{
  switch (misc.function)
    {
    case NOOP:
    case ROTATE:
    case ZOOM:
      QWhatsThis::showText (QCursor::pos ( ), mapText, map);
      break;
    }
  midMouse (menu_cursor_x, menu_cursor_y, menu_cursor_lon, menu_cursor_lat, menu_cursor_z);
}



//  Signal from the map class.

void 
grid3D::slotMousePress (QMouseEvent *e, double lon, double lat, double z)
{
  if (e->button () == Qt::LeftButton)
    {
      //  Check for the control key modifier.  If it's set, we want to rotate the image.

      if (e->modifiers () == Qt::ControlModifier)
        {
          //  Get the cursor position at this time.

          start_ctrl_x = e->x ();
          start_ctrl_y = e->y ();

          misc.function = ROTATE;
          setFunctionCursor (misc.function);
        }
    }

  if (e->button () == Qt::MidButton)
    {
      //  Check for the control key modifier.  If it's set, we want to move to center (in slotMouseRelease).

      if (e->modifiers () == Qt::ControlModifier)
        {
          //  Get the cursor position at this time.

          start_ctrl_x = e->x ();
          start_ctrl_y = e->y ();
          mid_lat = lat;
          mid_lon = lon;
          mid_z = z;
        }
    }

  if (e->button () == Qt::RightButton)
    {
      //  Check for the control key modifier.  If it's set, we want to zoom based on cursor movement.

      if (e->modifiers () == Qt::ControlModifier)
        {
          start_ctrl_y = e->y ();

          misc.function = ZOOM;
          setFunctionCursor (misc.function);
        }
    }
}



//  Mouse press signal prior to signals being enabled from the map class.

void 
grid3D::slotPreliminaryMousePress (QMouseEvent *e)
{
  QPoint pos = QPoint (e->x (), e->y ());

  QWhatsThis::showText (pos, mapText, map);
}



//  Signal from the map class.

void 
grid3D::slotMouseRelease (QMouseEvent * e, double lon __attribute__ ((unused)),
                         double lat __attribute__ ((unused)), double z __attribute__ ((unused)))
{
  if (e->button () == Qt::LeftButton)
    {
      popup_active = NVFalse;
      switch (misc.function)
        {
        case ROTATE:
          misc.function = NOOP;
          setFunctionCursor (misc.function);
          map->force_full_res ();
          break;
        }
    }

  if (e->button () == Qt::MidButton)
    {
      //  Check for the control key modifier.  If it's set, we want to center on the cursor.

      if (e->modifiers () == Qt::ControlModifier)
        {
          //if (abs (start_ctrl_x - e->x ()) < 5 && abs (start_ctrl_y - e->y ()) < 5)
            {
              map->setMapCenter (mid_lon, mid_lat, mid_z);
            }
        }

      setFunctionCursor (misc.function);

      popup_active = NVFalse;
    }

  if (e->button () == Qt::RightButton)
    {
      if (misc.function == ZOOM)
        {
          misc.function = NOOP;
          setFunctionCursor (misc.function);
          map->force_full_res ();
        }
    }
}



//  Mouse wheel signal from the map class.

void
grid3D::slotWheel (QWheelEvent *e, double lon __attribute__ ((unused)), double lat __attribute__ ((unused)),
                  double z __attribute__ ((unused)))
{
  if (e->delta () > 0)
    {
      //  Zoom in when pressing the Ctrl key and using the mouse wheel.

      if (e->modifiers () == Qt::ControlModifier)
        {
          map->zoomInPercent ();
        }
    }
  else
    {
      //  Zoom out when pressing the Ctrl key and using the mouse wheel.

      if (e->modifiers () == Qt::ControlModifier)
        {
          map->zoomOutPercent ();
        }
    }
}



//  Signal from the map class.

void
grid3D::slotMouseMove (QMouseEvent *e __attribute__ ((unused)), double lon, double lat, double z, NVMAPGL_DEF l_mapdef)
{
  char                 ltstring[25], lnstring[25], hem;
  QString              string, geo_string, exag_string;
  double               deg, min, sec;
  int32_t              ix, iy;
  NV_F64_COORD3        xyz, prev_xyz = {-1.0, -1.0, -1.0};
  CHRTR2_RECORD        chrtr2_record;


  //  Let other CHRTR2 programs know which window we're in.

  misc.chrtr2_share->active_window_id = active_window_id;


  //  Get rid of the tracking cursor from slotTrackCursor ().  But only if it already exists, otherwise we
  //  will be creating a new one (which we don't want to do).

  if (mv_tracker >= 0) map->closeMovingList (&mv_tracker);


  xyz.x = lon;
  xyz.y = lat;
  xyz.z = z;
  ix = e->x ();
  iy = e->y ();


  //  Track the cursor position for other CHRTR2 programs

  misc.chrtr2_share->cursor_position.y = lat;
  misc.chrtr2_share->cursor_position.x = lon;


  if (lat >= misc.chrtr2_share->viewer_displayed_area.min_y && lat <= misc.chrtr2_share->viewer_displayed_area.max_y && 
      lon >= misc.chrtr2_share->viewer_displayed_area.min_x && lon <= misc.chrtr2_share->viewer_displayed_area.max_x)
    {
      //  Try to find the highest layer with a valid value to display in the status bar.

      NV_F64_COORD2 xy;
      xy.y = lat;
      xy.x = lon;
      int32_t hit = -1;

      for (int32_t chrtr2 = 0 ; chrtr2 < misc.chrtr2_share->chrtr2_count ; chrtr2++)
        {
          if (misc.chrtr2_share->display_chrtr2[chrtr2] && inside_mbr_coord2 (&misc.chrtr2_header[chrtr2].mbr, xy))
            {
              chrtr2_read_record_lat_lon (misc.chrtr2_handle[chrtr2], lat, lon, &chrtr2_record);


              //  Check the status.

              if (chrtr2_record.status)
                {
                  hit = chrtr2;
                  break;
                }
            }
        }


      strcpy (ltstring, fixpos (lat, &deg, &min, &sec, &hem, POS_LAT, options.position_form));
      strcpy (lnstring, fixpos (lon, &deg, &min, &sec, &hem, POS_LON, options.position_form));


      exag_string = tr ("Z Exaggeration: %L1").arg (options.exaggeration, 0, 'f', 3);


      if (hit != -1)
        {
          double z = 0.0;

          z = chrtr2_record.z;


          int32_t pix_x, pix_y;

          NV_I32_COORD2 coord;
          double center_x, center_y;
          chrtr2_get_coord (misc.chrtr2_handle[hit], lat, lon, &coord);
          chrtr2_get_lat_lon (misc.chrtr2_handle[hit], &center_y, &center_x, coord);

          map->get2DCoords (center_x, center_y, -z, &pix_x, &pix_y);

          map->setMovingList (&mv_marker, marker, 16, pix_x, pix_y, 0.0, 2, options.tracker_color);

          geo_string = tr ("Lat: %1  Lon: %2  Z: %L3").arg (ltstring).arg (lnstring).arg (z * options.z_factor + options.z_offset, 0, 'f', 2);
        }
      else
        {
          geo_string = tr ("Lat: %1  Lon: %2").arg (ltstring).arg (lnstring);
        }


      statusBar ()->showMessage (geo_string + "      " + exag_string);
    }


  //  Actions based on the active function

  switch (misc.function)
    {
    case ROTATE:
      if (start_ctrl_x != ix || start_ctrl_y != iy)
        {
          int32_t diff_x = ix - start_ctrl_x;


          if (diff_x)
            {
              //  Flip the sign if we are above the center point looking at the top of the surface or below
              //  the center point looking at the bottom.  This allows the Y rotation from the mouse click/drag
              //  to be more intuitive.

              double zxrot = map->getZXRotation ();
              int32_t half = l_mapdef.draw_height / 2;


              //  If we're within 15 degrees of flat rotate normally for the front side.

              if ((zxrot < 15.0 && zxrot > -15.0) || (zxrot > 345.0) || (zxrot < -345.0))
                {
                  //  Don't do anything.
                }


              //  Reverse rotate reverse for the back side.

              else if ((zxrot > -195.0 && zxrot < -165.0) || (zxrot < 195.0 && zxrot > 165.0))
                {
                  diff_x = -diff_x;
                }


              //  Otherwise, check to see which side is up.

              else
                {
                  if ((iy < half && ((zxrot > 0.0 && zxrot < 180.0) || (zxrot < -180.0))) ||
                      (iy > half && ((zxrot < 0.0 && zxrot > -180.0) || (zxrot > 180.0)))) diff_x = -diff_x;
                }

              map->rotateY ((double) diff_x / 5.0);
            }

          int32_t diff_y = iy - start_ctrl_y;

          if (diff_y) map->rotateZX ((double) diff_y / 5.0);


          start_ctrl_x = ix;
          start_ctrl_y = iy;
        }
      break;

    case ZOOM:
      if (start_ctrl_y != xyz.y)
        {
          int32_t diff_y = iy - start_ctrl_y;


          if (diff_y < -5)
            {
              map->zoomInPercent ();
              start_ctrl_y = iy;
            }
          else if (diff_y > 5)
            {
              map->zoomOutPercent ();
              start_ctrl_y = iy;
            }
        }
      break;

    default:
      break;
    }


  //  Set the previous cursor.
             
  prev_xyz = xyz;
}



//  Timer - timeout signal.  Very much like an X workproc.  This tracks the cursor in the associated programs not in this window.
//  This is active whenever the mouse leaves this window and enters an associated program window.

void
grid3D::slotTrackCursor ()
{
  char                  ltstring[25], lnstring[25];
  QString               geo_string;
  QString               string;
  static NVMAP_DEF      mpdf;
  static NV_F64_COORD2  prev_xy;


  //  If we're in this window or we're drawing, return.

  if (misc.drawing) return;


  if (misc.chrtr2_share->active_window_id != active_window_id && mv_marker >= 0) map->closeMovingList (&mv_marker);


  //  If we have changed the view in gridEdit, we need to reload the data.

  if (misc.chrtr2_share->key == GRID3D_FORCE_RELOAD || misc.chrtr2_share->key == CHRTR2_LAYERS_CHANGED)
    {
      for (int32_t chrtr2 = misc.chrtr2_share->chrtr2_count - 1 ; chrtr2 >= 0 ; chrtr2--)
        {
          if (misc.chrtr2_handle[chrtr2] >= 0) chrtr2_close_file (misc.chrtr2_handle[chrtr2]);

          if ((misc.chrtr2_handle[chrtr2] = chrtr2_open_file (misc.chrtr2_share->chrtr2_file[chrtr2], &misc.chrtr2_header[chrtr2], CHRTR2_UPDATE)) < 0)
            {
              QMessageBox::warning (this, tr ("Open CHRTR2 File"),
                                    tr ("The file %1 is not a CHRTR2 file or there was an error reading the file.\n"
                                        "The error message returned was:\n\n%2").arg
                                    (QDir::toNativeSeparators (QString (misc.chrtr2_share->chrtr2_file[chrtr2]))).arg (chrtr2_strerror ()));
              clean_exit (-1);
            }


          if (misc.chrtr2_header[chrtr2].z_units != CHRTR2_METERS)
            {
              QMessageBox::warning (this, tr ("Open CHRTR2 File"), tr ("The file %1 does not use CHRTR2_METERS units.  This is not allowed.").arg
                                    (QDir::toNativeSeparators (QString (misc.chrtr2_share->chrtr2_file[chrtr2]))));
              chrtr2_close_file (misc.chrtr2_handle[chrtr2]);
              clean_exit (-1);
            }
        }


      misc.chrtr2_share->key = 0;

      initializeMaps (NVFalse);

      return;
    }


  NV_F64_COORD2 xy;
  xy.y = misc.chrtr2_share->cursor_position.y;
  xy.x = misc.chrtr2_share->cursor_position.x;


  if (misc.chrtr2_share->active_window_id != active_window_id && xy.y >= misc.chrtr2_share->viewer_displayed_area.min_y &&
      xy.y <= misc.chrtr2_share->viewer_displayed_area.max_y && xy.x >= misc.chrtr2_share->viewer_displayed_area.min_x &&
      xy.x <= misc.chrtr2_share->viewer_displayed_area.max_x && xy.y != prev_xy.y && xy.x != prev_xy.x)
    {
      double deg, min, sec;
      char       hem;

      strcpy (ltstring, fixpos (xy.y, &deg, &min, &sec, &hem, POS_LAT, options.position_form));
      strcpy (lnstring, fixpos (xy.x, &deg, &min, &sec, &hem, POS_LON, options.position_form));

      if (misc.chrtr2_share->cursor_position.z != CHRTR2_NULL_Z_VALUE)
        {
          int32_t pix_x, pix_y;
          map->get2DCoords (xy.x, xy.y, misc.chrtr2_share->cursor_position.z, &pix_x, &pix_y);

          map->setMovingList (&mv_tracker, marker, 16, pix_x, pix_y, 0.0, 2, options.tracker_color);
        }

      geo_string = tr ("Lat: %1  Lon: %2").arg (ltstring).arg (lnstring);

      statusBar ()->showMessage (geo_string);
    }

  prev_xy = xy;
}



//  Resize signal from the map class.

void
grid3D::slotResize (QResizeEvent *e __attribute__ ((unused)))
{
  //initializeMaps (NVFalse);
}



void 
grid3D::slotExagTriggered (int action)
{
  if (!misc.drawing)
    {
      switch (action)
        {
        case QAbstractSlider::SliderSingleStepAdd:
          if (options.exaggeration >= 1.0)
            {
              options.exaggeration -= 1.0;

              if (options.exaggeration < 1.0) options.exaggeration = 1.0;

              map->setExaggeration (options.exaggeration);

              initializeMaps (NVFalse);
            }
          break;

        case QAbstractSlider::SliderPageStepAdd:
          if (options.exaggeration >= 5.0)
            {
              options.exaggeration -= 5.0;

              if (options.exaggeration < 1.0) options.exaggeration = 1.0;

              map->setExaggeration (options.exaggeration);

              initializeMaps (NVFalse);
            }
          break;

        case QAbstractSlider::SliderSingleStepSub:
          options.exaggeration += 1.0;
          if (options.exaggeration > 100.0) options.exaggeration = 100.0;
          map->setExaggeration (options.exaggeration);

          initializeMaps (NVFalse);
          break;

        case QAbstractSlider::SliderPageStepSub:
          options.exaggeration += 5.0;
          if (options.exaggeration > 100.0) options.exaggeration = 100.0;
          map->setExaggeration (options.exaggeration);

          initializeMaps (NVFalse);
          break;

        case QAbstractSlider::SliderMove:
          QString lbl;
          lbl = tr ("Z Exaggeration: %L1").arg ((float) exagBar->value () / 100.0, 5, 'f', 3);
          statusBar ()->showMessage (lbl);
          break;
        }
    }
}



void 
grid3D::slotExagReleased ()
{
  options.exaggeration = (float) exagBar->value () / 100.0;
  map->setExaggeration (options.exaggeration);

  initializeMaps (NVFalse);
}



void 
grid3D::slotExaggerationChanged (float exaggeration, float apparent_exaggeration)
{
  if (exaggeration < 1.0)
    {
      exagBar->setEnabled (false);
    }
  else
    {
      exagBar->setEnabled (true);
    }

  QString lbl;
  lbl = tr ("Z Exaggeration: %L1").arg (apparent_exaggeration, 5, 'f', 3);
  statusBar ()->showMessage (lbl);

  options.exaggeration = exaggeration;

  disconnect (exagBar, SIGNAL (actionTriggered (int)), this, SLOT (slotExagTriggered (int)));
  exagBar->setValue (NINT (options.exaggeration * 100.0));
  connect (exagBar, SIGNAL (actionTriggered (int)), this, SLOT (slotExagTriggered (int)));
}



//  Using the keys to move around.

void
grid3D::keyPressEvent (QKeyEvent *e)
{
  if (!misc.drawing)
    {
      switch (e->key ())
        {
        case Qt::Key_Left:
          map->rotateY (-5.0);
          return;
          break;

        case Qt::Key_Up:
          map->rotateZX (-5.0);
          return;
          break;

        case Qt::Key_Right:
          map->rotateY (5.0);
          return;
          break;

        case Qt::Key_Down:
          map->rotateZX (5.0);
          return;
          break;

        case Qt::Key_PageUp:
          slotExagTriggered (QAbstractSlider::SliderSingleStepSub);
          return;
          break;

        case Qt::Key_PageDown:
          slotExagTriggered (QAbstractSlider::SliderSingleStepAdd);
          return;
          break;

        default:
          return;
        }
      e->accept ();
    }
}



void 
grid3D::slotClose (QCloseEvent *event __attribute__ ((unused)))
{
  slotQuit ();
}



void 
grid3D::slotQuit ()
{
  uint8_t envout (OPTIONS *options, QMainWindow *mainWindow);


  envout (&options, this);


  //  Get rid of the shared memory.

  misc.chrtr2Share->detach ();


  clean_exit (0);
}



void 
grid3D::clean_exit (int32_t ret)
{
  //  Have to close the GL widget or it stays on screen in VirtualBox

  map->close ();

  exit (ret);
}



void
grid3D::slotGeotiff ()
{
  setMainButtons (NVFalse);

  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();


  if (!misc.display_GeoTIFF)
    {
      misc.display_GeoTIFF = 1;
      map->setTextureType (misc.display_GeoTIFF);
      map->enableTexture (NVTrue);
      bGeotiff->setIcon (QIcon (":/icons/geo_decal.png"));
    }
  else if (misc.display_GeoTIFF > 0)
    {
      misc.display_GeoTIFF = -1;
      map->setTextureType (misc.display_GeoTIFF);
      map->enableTexture (NVTrue);
      bGeotiff->setIcon (QIcon (":/icons/geo_modulate.png"));
    }
  else
    {
      misc.display_GeoTIFF = 0;
      map->enableTexture (NVFalse);
      bGeotiff->setIcon (QIcon (":/icons/geo_off.png"));
    }
  qApp->processEvents ();

  setMainButtons (NVTrue);


  qApp->restoreOverrideCursor ();


  initializeMaps (NVFalse);
}



void
grid3D::slotReset ()
{
  map->setMapCenter (misc.map_center_x, misc.map_center_y, misc.map_center_z);


  map->resetPOV ();
}



void 
grid3D::setFunctionCursor (int32_t function)
{
  misc.function = function;


  discardMovableObjects ();


  switch (function)
    {
    case NOOP:
      map->setCursor (Qt::CrossCursor);
      break;

    case ROTATE:
      map->setCursor (rotateCursor);
      break;

    case ZOOM:
      map->setCursor (zoomCursor);
      break;

      /*
    case DRAG:
      map->setCursor (Qt::SizeAllCursor);
      break;
      */
    }
}



void
grid3D::slotHotkeyHelp ()
{
  hotkeyHelp *hk = new hotkeyHelp (this, &options, &misc);
  hk->show ();
}


void
grid3D::about ()
{
  QMessageBox::about (this, VERSION, tr ("grid3D - 3D CHRTR2 viewer.\n\nAuthor : Jan C. Depner (jan@pfmabe.software)"));
}


void
grid3D::slotAcknowledgments ()
{
  QMessageBox::about (this, VERSION, acknowledgmentsText);
}



void
grid3D::aboutQt ()
{
  QMessageBox::aboutQt (this, VERSION);
}



//  This triggers a paintevent in the QGLWidget on Windoze.  I have no idea why.

void 
grid3D::focusInEvent (QFocusEvent *e __attribute__ ((unused)))
{
  map->enableWindozePaintEvent (NVFalse);
}
