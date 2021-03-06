
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

static QImage      *subImage = NULL;


uint32_t getColorOffset (QString colorInt)
{
  static uint32_t mult;

  if (colorInt.contains ("Alpha"))
    {
      mult = 0x1000000;
    }
  else if (colorInt.contains ("Red"))
    {
      mult = 0x10000;
    }
  else if (colorInt.contains ("Green"))
    {
      mult = 0x100;
    }
  else if (colorInt.contains ("Blue"))
    {
      mult = 0x1;
    }

  return (mult);
}



QImage *geotiff (NV_F64_XYMBR mbr, char *geotiff_file, NV_F64_XYMBR *geotiff_mbr)
{
  double             GeoTIFF_wlon, GeoTIFF_nlat, GeoTIFF_lon_step, GeoTIFF_lat_step, GeoTIFF_elon, GeoTIFF_slat;
  int32_t            width, height, start_y, end_y, start_x, end_x;
  QImage             *full_res_image;
  GDALDataset        *poDataset;
  double             adfGeoTransform[6];
  static int32_t     powers_of_two[12] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048};


  GDALAllRegister ();


  poDataset = (GDALDataset *) GDALOpen (geotiff_file, GA_ReadOnly);
  if (poDataset != NULL)
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


          if (mbr.max_y >= GeoTIFF_slat && mbr.min_y <= GeoTIFF_nlat && mbr.max_x >= GeoTIFF_wlon && mbr.min_x <= GeoTIFF_elon)
            {
              full_res_image = new QImage (geotiff_file, "tiff");

              if (full_res_image == NULL || full_res_image->width () == 0 || full_res_image->height () == 0)
                {
                  QMessageBox::critical (0, "grid3D", grid3D::tr ("Unable to open or read GeoTIFF file!"));
                  delete poDataset;
                  return (NULL);
                }


              /*  This is how I had to read geoTIFFs from Qt 4.7.2 until 4.8.5.  For some unknown reason the above method quit working in
                  Qt 4.7.2 and I didn't check it again until !t 4.8.5 when it was again working.  I'm leaving this here for educational
                  purposes.

              GDALRasterBand *poBand[4];
              QString dataType[4], colorInt[4];
              uint32_t mult[4] = {0, 0, 0, 0};


              int32_t rasterCount = poDataset->GetRasterCount ();
              if (rasterCount < 3)
                {
                  delete poDataset;
                  QMessageBox::critical (0, "grid3D", grid3D::tr ("Not enough raster bands in geoTIFF"));
                  return (NULL);
                }

              for (int32_t i = 0 ; i < rasterCount ; i++)
                {
                  poBand[i] = poDataset->GetRasterBand (i + 1);

                  dataType[i] = QString (GDALGetDataTypeName (poBand[i]->GetRasterDataType ()));
                  colorInt[i] = QString (GDALGetColorInterpretationName (poBand[i]->GetColorInterpretation ()));


                  //  We can only handle Byte data (i.e. RGB or ARGB)

                  if (dataType[i] != "Byte")
                    {
                      delete poDataset;
                      QMessageBox::critical (0, "grid3D", grid3D::tr ("Cannot handle %1 data type").arg (dataType[i]));
                      return (NULL);
                    }

                  mult[i] = getColorOffset (colorInt[i]);
                }

              int32_t nXSize = poBand[0]->GetXSize ();
              int32_t nYSize = poBand[0]->GetYSize ();

              full_res_image = new QImage (nXSize, nYSize, QImage::Format_ARGB32);
              if (full_res_image == NULL || full_res_image->width () == 0 || full_res_image->height () == 0)
                {
                  QMessageBox::critical (0, "grid3D", grid3D::tr ("Unable to open image!"));
                  delete poDataset;
                  return (NULL);
                }

              uint32_t *color = new uint32_t[nXSize];
              uint8_t *pafScanline = (uint8_t *) CPLMalloc (sizeof (uint8_t) * nXSize);

              for (int32_t i = 0 ; i < nYSize ; i++)
                {
                  //  If we don't have an alpha band set it to 255.

                  for (int32_t k = 0 ; k < nXSize ; k++)
                    {
                      if (rasterCount < 4)
                        {
                          color[k] = 0xff000000;
                        }
                      else
                        {
                          color[k] = 0x0;
                        }
                    }


                  //  Read the raster bands.

                  for (int32_t j = 0 ; j < rasterCount ; j++)
                    {
                      poBand[j]->RasterIO (GF_Read, 0, i, nXSize, 1, pafScanline, nXSize, 1, GDT_Byte, 0, 0);
                      for (int32_t k = 0 ; k < nXSize ; k++) color[k] += ((uint32_t) pafScanline[k]) * mult[j];
                    }


                  //  Set the image pixels.

                  for (int32_t k = 0 ; k < nXSize ; k++)
                    {
                      full_res_image->setPixel (k, i, color[k]);
                    }
                }

              delete (color);
              CPLFree (pafScanline);
              */


              //  Set the bounds

              if (mbr.min_y < GeoTIFF_slat)
                {
                  start_y = 0;
                  geotiff_mbr->min_y = GeoTIFF_slat;
                }
              else
                {
                  start_y = (int32_t) ((GeoTIFF_nlat - mbr.max_y) / GeoTIFF_lat_step);
                  geotiff_mbr->min_y = mbr.min_y;
                }
              if (mbr.max_y > GeoTIFF_nlat)
                {
                  end_y = height;
                  geotiff_mbr->max_y = GeoTIFF_nlat;
                }
              else
                {
                  end_y = (int32_t) ((GeoTIFF_nlat - mbr.min_y) / GeoTIFF_lat_step);
                  geotiff_mbr->max_y = mbr.max_y;
                }

              if (mbr.min_x < GeoTIFF_wlon)
                {
                  start_x = 0;
                  geotiff_mbr->min_x = GeoTIFF_wlon;
                }
              else
                {
                  start_x = (int32_t) ((mbr.min_x - GeoTIFF_wlon) / GeoTIFF_lon_step);
                  geotiff_mbr->min_x = mbr.min_x;
                }
              if (mbr.max_x > GeoTIFF_elon)
                {
                  end_x = width;
                  geotiff_mbr->max_x = GeoTIFF_elon;
                }
              else
                {
                  end_x = (int32_t) ((mbr.max_x - GeoTIFF_wlon) / GeoTIFF_lon_step);
                  geotiff_mbr->max_x = mbr.max_x;
                }


              if (start_y < 0)
                {
                  end_y = (int32_t) ((GeoTIFF_nlat - geotiff_mbr->min_y) / GeoTIFF_lat_step);
                  start_y = 0;
                }

              if (end_y > height)
                {
                  end_y = height;
                  start_y = (int32_t) ((GeoTIFF_nlat - geotiff_mbr->max_y) / GeoTIFF_lat_step);
                }


              //  Number of rows and columns of the GeoTIFF to paint.

              int32_t rows = end_y - start_y;
              int32_t cols = end_x - start_x;


              //  I'm limiting the overlay to 2048 by 2048 max.

              int32_t wide = powers_of_two[11], high = powers_of_two[11];

              for (int32_t i = 0 ; i < 12 ; i++)
                {
                  if (powers_of_two[i] >= cols)
                    {
                      wide = powers_of_two[i];
                      break;
                    }
                }

              for (int32_t i = 0 ; i < 12 ; i++)
                {
                  if (powers_of_two[i] >= rows)
                    {
                      high = powers_of_two[i];
                      break;
                    }
                }


              //  First make sure we have enough resources to handle the texture.

              glTexImage2D (GL_PROXY_TEXTURE_2D, 0, GL_RGBA, wide, high, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

              int32_t w, h, d;
              glGetTexLevelParameteriv (GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
              glGetTexLevelParameteriv (GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
              glGetTexLevelParameteriv (GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_DEPTH, &d);

              if (!w || !h || !d)
                {
                  QMessageBox::critical (0, "grid3D", grid3D::tr ("The image is too large for OpenGL (%1 X %2)").arg (wide).arg (high));
                  delete full_res_image;
                  delete poDataset;
                  return (NULL);
                }


              //  Have to convert to a power of two prior to using as an OpenGL texture.  We're also mirroring 
              //  in the y direction (OpenGL Z direction).

              subImage = new QImage (full_res_image->copy (start_x, start_y, cols, rows).scaled
                                     (wide, high, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).mirrored
                                     (false, true));

              delete full_res_image;
            }
          delete poDataset;
        }
    }
  else
    {
      QMessageBox::warning (0, "grid3D", grid3D::tr ("Unable to read tiff file."));
    }

  return (subImage);
}



void geotiff_clear ()
{
  if (subImage != NULL) delete subImage;
  subImage = NULL;
}
