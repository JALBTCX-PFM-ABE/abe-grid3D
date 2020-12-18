
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

void adjust_bounds (MISC *misc, int32_t chrtr2)
{
  //  Adjust input bounds to nearest grid point.

  misc->displayed_area[chrtr2].min_y = misc->chrtr2_header[chrtr2].mbr.slat +
    (NINT ((misc->chrtr2_share->viewer_displayed_area.min_y - misc->chrtr2_header[chrtr2].mbr.slat) /
           misc->chrtr2_header[chrtr2].lat_grid_size_degrees)) * misc->chrtr2_header[chrtr2].lat_grid_size_degrees;
  misc->displayed_area[chrtr2].max_y = misc->chrtr2_header[chrtr2].mbr.slat + 
    (NINT ((misc->chrtr2_share->viewer_displayed_area.max_y - misc->chrtr2_header[chrtr2].mbr.slat) /
           misc->chrtr2_header[chrtr2].lat_grid_size_degrees)) * misc->chrtr2_header[chrtr2].lat_grid_size_degrees;
  misc->displayed_area[chrtr2].min_x = misc->chrtr2_header[chrtr2].mbr.wlon + 
    (NINT ((misc->chrtr2_share->viewer_displayed_area.min_x - misc->chrtr2_header[chrtr2].mbr.wlon) /
           misc->chrtr2_header[chrtr2].lon_grid_size_degrees)) * misc->chrtr2_header[chrtr2].lon_grid_size_degrees;
  misc->displayed_area[chrtr2].max_x = misc->chrtr2_header[chrtr2].mbr.wlon + 
    (NINT ((misc->chrtr2_share->viewer_displayed_area.max_x - misc->chrtr2_header[chrtr2].mbr.wlon) /
           misc->chrtr2_header[chrtr2].lon_grid_size_degrees)) * misc->chrtr2_header[chrtr2].lon_grid_size_degrees;


  misc->displayed_area[chrtr2].min_y = qMax (misc->displayed_area[chrtr2].min_y, misc->chrtr2_header[chrtr2].mbr.slat);
  misc->displayed_area[chrtr2].max_y = qMin (misc->displayed_area[chrtr2].max_y, misc->chrtr2_header[chrtr2].mbr.nlat);
  misc->displayed_area[chrtr2].min_x = qMax (misc->displayed_area[chrtr2].min_x, misc->chrtr2_header[chrtr2].mbr.wlon);
  misc->displayed_area[chrtr2].max_x = qMin (misc->displayed_area[chrtr2].max_x, misc->chrtr2_header[chrtr2].mbr.elon);


  misc->displayed_area_width[chrtr2] = (NINT ((misc->displayed_area[chrtr2].max_x - misc->displayed_area[chrtr2].min_x) / 
                                              misc->chrtr2_header[chrtr2].lon_grid_size_degrees));
  misc->displayed_area_height[chrtr2] = (NINT ((misc->displayed_area[chrtr2].max_y - misc->displayed_area[chrtr2].min_y) / 
                                               misc->chrtr2_header[chrtr2].lat_grid_size_degrees));
  misc->displayed_area_row[chrtr2] = NINT ((misc->displayed_area[chrtr2].min_y - misc->chrtr2_header[chrtr2].mbr.slat) / 
                                           misc->chrtr2_header[chrtr2].lat_grid_size_degrees);
  misc->displayed_area_column[chrtr2] = NINT ((misc->displayed_area[chrtr2].min_x - misc->chrtr2_header[chrtr2].mbr.wlon) / 
                                              misc->chrtr2_header[chrtr2].lon_grid_size_degrees);

  if (misc->displayed_area_column[chrtr2] + misc->displayed_area_width[chrtr2] > misc->chrtr2_header[chrtr2].width)
    {
      misc->displayed_area_width[chrtr2] = misc->chrtr2_header[chrtr2].width - misc->displayed_area_column[chrtr2];
      misc->displayed_area[chrtr2].max_x = misc->chrtr2_header[chrtr2].mbr.elon;
    }

  if (misc->displayed_area_row[chrtr2] + misc->displayed_area_height[chrtr2] > misc->chrtr2_header[chrtr2].height)
    {
      misc->displayed_area_height[chrtr2] = misc->chrtr2_header[chrtr2].height - misc->displayed_area_row[chrtr2];
      misc->displayed_area[chrtr2].max_y = misc->chrtr2_header[chrtr2].mbr.nlat;
    }
}
