
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

void paint_surface (grid3D *parent, MISC *misc, nvMapGL *map, uint8_t reset)
{
  static NV_F64_XYMBR prev_mbr = {-999.0, -999.0, -999.0, -999.0};
  static int32_t prev_display_GeoTIFF = 0;
  float **data;
  NV_F64_XYMBC mbc;


  void adjust_bounds (MISC *misc, int32_t chrtr2);
  QImage *geotiff (NV_F64_XYMBR mbr, char *geotiff_file, NV_F64_XYMBR *geotiff_mbr);
  void geotiff_clear ();


  //  Make an MBC from the MBR and the data.

  mbc.min_x = misc->chrtr2_share->viewer_displayed_area.min_x;
  mbc.min_y = misc->chrtr2_share->viewer_displayed_area.min_y;
  mbc.max_x = misc->chrtr2_share->viewer_displayed_area.max_x;
  mbc.max_y = misc->chrtr2_share->viewer_displayed_area.max_y;
  mbc.max_z = -999999999.0;
  mbc.min_z = 999999999.0;


  //  First we have to compute the minimum bounding cube for all of the available CHRTR2s

  for (int32_t chrtr2 = misc->chrtr2_share->chrtr2_count - 1 ; chrtr2 >= 0 ; chrtr2--)
    {
      //  Only if we want to display it.

      if (misc->chrtr2_share->display_chrtr2[chrtr2])
        {
          //  Adjust bounds to nearest grid point (compute displayed_area).

          adjust_bounds (misc, chrtr2);


          //  If none of the area is in the displayed area the width and/or height will be 0 or negative.

          if (misc->displayed_area_width[chrtr2] > 0 && misc->displayed_area_height[chrtr2] > 0)
            {
              CHRTR2_RECORD *chrtr2_record;

              chrtr2_record = (CHRTR2_RECORD *) malloc (sizeof (CHRTR2_RECORD) * misc->displayed_area_width[chrtr2]);
              if (chrtr2_record == NULL)
                {
                  fprintf (stderr, "%s %s %s %d - chrtr2 - %s\n", misc->progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
                  parent->clean_exit (-1);
                }


              for (int32_t i = 0 ; i < misc->displayed_area_height[chrtr2] ; i++)
                {
                  chrtr2_read_row (misc->chrtr2_handle[chrtr2], misc->displayed_area_row[chrtr2] + i, misc->displayed_area_column[chrtr2],
                                   misc->displayed_area_width[chrtr2], chrtr2_record);

                  float z = -CHRTR2_NULL_Z_VALUE;

                  for (int32_t j = 0 ; j < misc->displayed_area_width[chrtr2] ; j++)
                    {
                      if (chrtr2_record[j].status) z = -chrtr2_record[j].z;

                      if (z != -CHRTR2_NULL_Z_VALUE)
                        {
                          mbc.max_z = qMax (mbc.max_z, (double) z);
                          mbc.min_z = qMin (mbc.min_z, (double) z);
                        }
                    }
                }

              free (chrtr2_record);
            }
        }
    }


  //  Have to set the bounds prior to doing anything

  if (prev_mbr.min_x != misc->chrtr2_share->viewer_displayed_area.min_x || prev_mbr.min_y != misc->chrtr2_share->viewer_displayed_area.min_y ||
      prev_mbr.max_x != misc->chrtr2_share->viewer_displayed_area.max_x || prev_mbr.max_y != misc->chrtr2_share->viewer_displayed_area.max_y)
    {
      map->setBounds (mbc);


      if (reset)
        {
          //  Get the map center so we can reset the view.

          map->getMapCenter (&misc->map_center_x, &misc->map_center_y, &misc->map_center_z);


          map->setMapCenter (misc->map_center_x, misc->map_center_y, misc->map_center_z);

          map->resetPOV ();
        }
    }


  //  If we want to display a GeoTIFF we must load the texture, unless it (or the displayed area) hasn't changed.

  if (misc->display_GeoTIFF && ((prev_mbr.min_x != misc->chrtr2_share->viewer_displayed_area.min_x ||
                                 prev_mbr.min_y != misc->chrtr2_share->viewer_displayed_area.min_y ||
                                 prev_mbr.max_x != misc->chrtr2_share->viewer_displayed_area.max_x ||
                                 prev_mbr.max_y != misc->chrtr2_share->viewer_displayed_area.max_y ||
                                 misc->display_GeoTIFF != prev_display_GeoTIFF) || misc->GeoTIFF_init))
    {
      NV_F64_XYMBR geotiff_mbr;
      QImage *subImage = geotiff (misc->chrtr2_share->viewer_displayed_area, misc->GeoTIFF_name, &geotiff_mbr);
      if (subImage != NULL)
        {
          map->setGeotiffTexture (subImage, geotiff_mbr, misc->display_GeoTIFF);
          geotiff_clear ();
          misc->GeoTIFF_init = NVFalse;
        }
      prev_display_GeoTIFF = misc->display_GeoTIFF;
    }
  prev_mbr = misc->chrtr2_share->viewer_displayed_area;


  //  Clear ALL of the data layers prior to loading.

  for (int32_t chrtr2 = 0 ; chrtr2 < MAX_CHRTR2_FILES ; chrtr2++) map->clearDataLayer (chrtr2);


  //  Now we load each of the CHRTR2s (setDataLayer) using the minimum bounding cube as the limits.

  for (int32_t chrtr2 = misc->chrtr2_share->chrtr2_count - 1 ; chrtr2 >= 0 ; chrtr2--)
    {
      //  Only if we want to display it.

      if (misc->chrtr2_share->display_chrtr2[chrtr2])
        {
          //  Adjust bounds to nearest grid point (compute displayed_area).

          adjust_bounds (misc, chrtr2);


          //  Make sure that each CHRTR2 is actually in the desired area.  If it's not then either the width
          //  or the height will be 0 or negative.

          if (misc->displayed_area_height[chrtr2] > 0 && misc->displayed_area_width[chrtr2] > 0)
            {
              data = (float **) malloc (sizeof (float *) * misc->displayed_area_height[chrtr2]);
              if (data == NULL) 
                {
                  fprintf (stderr, "%s %s %s %d - data - %s\n", misc->progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
                  parent->clean_exit (-1);
                }


              CHRTR2_RECORD *chrtr2_record;

              chrtr2_record = (CHRTR2_RECORD *) malloc (sizeof (CHRTR2_RECORD) * misc->displayed_area_width[chrtr2]);
              if (chrtr2_record == NULL)
                {
                  fprintf (stderr, "%s %s %s %d - chrtr2 - %s\n", misc->progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
                  parent->clean_exit (-1);
                }


              for (int32_t i = 0 ; i < misc->displayed_area_height[chrtr2] ; i++)
                {
                  data[i] = (float *) malloc (sizeof (float) * misc->displayed_area_width[chrtr2]);
                  if (data[i] == NULL) 
                    {
                      fprintf (stderr, "%s %s %s %d - data[i] - %s\n", misc->progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
                      parent->clean_exit (-1);
                    }


                  chrtr2_read_row (misc->chrtr2_handle[chrtr2], misc->displayed_area_row[chrtr2] + i, misc->displayed_area_column[chrtr2],
                                   misc->displayed_area_width[chrtr2], chrtr2_record);

                  for (int32_t j = 0 ; j < misc->displayed_area_width[chrtr2] ; j++)
                    {
                      data[i][j] = -CHRTR2_NULL_Z_VALUE;

                      if (chrtr2_record[j].status) data[i][j] = -chrtr2_record[j].z;
                    }
                }


              free (chrtr2_record);


              map->setDataLayer (chrtr2, data, NULL, 0, 0, misc->displayed_area_height[chrtr2], misc->displayed_area_width[chrtr2], 
                                 misc->chrtr2_header[chrtr2].lat_grid_size_degrees, misc->chrtr2_header[chrtr2].lon_grid_size_degrees,
                                 -CHRTR2_NULL_Z_VALUE, misc->displayed_area[chrtr2], 0);

              for (int32_t i = 0 ; i < misc->displayed_area_height[chrtr2] ; i++) free (data[i]);

              free (data);
            }
        }
    }
}
