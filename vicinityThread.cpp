
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



#include "vicinityThread.hpp"

vicinityThread::vicinityThread (QObject *parent)
  : QThread(parent)
{
}



vicinityThread::~vicinityThread ()
{
}



void vicinityThread::vicinity (OPTIONS *op, int32_t ph, PFM_OPEN_ARGS *oa, int32_t sr, int32_t er, int32_t o, int32_t p)
{
  QMutexLocker locker (&mutex);

  l_options = op;
  l_pfm_handle = ph;
  l_open_args = *oa;
  l_start_row = sr;
  l_end_row = er;
  l_overlap = o;
  l_pass = p;

  if (!isRunning ()) start ();
}



void vicinityThread::run ()
{
  int32_t             percent = 0, old_percent = -1;
  QString             string;


  mutex.lock ();

  OPTIONS *options = l_options;
  int32_t pfm_handle = l_pfm_handle;
  PFM_OPEN_ARGS open_args = l_open_args;
  int32_t start_row = l_start_row;
  int32_t end_row = l_end_row;
  int32_t overlap = l_overlap;
  int32_t pass = l_pass;
  int32_t urban_attr = options->urban_attr;


  mutex.unlock ();


  //qDebug () << __LINE__ << pass;


  BIN_RECORD bin;
  DEPTH_RECORD *depth;
  std::vector<POS_DATA> tpos;
  int32_t recnum;
  int32_t tcount = 0;


  for (int32_t i = start_row ; i <= end_row ; i++)
    {
      NV_I32_COORD2 ref_coord;

      ref_coord.y = i;

      int32_t i_start = qMax (i - overlap, 0);
      int32_t i_end = qMin (i + overlap, open_args.head.bin_height - 1);

      for (int32_t j = 0 ; j < open_args.head.bin_width ; j++)
        {
          ref_coord.x = j;

          tcount = 0;

          int32_t j_start = qMax (j - overlap, 0);
          int32_t j_end = qMin (j + overlap, open_args.head.bin_width - 1);


          //  Load the point array.

          for (int32_t k = i_start ; k <= i_end ; k++)
            {
              NV_I32_COORD2 coord;

              coord.y = k;


              for (int32_t m = j_start ; m <= j_end ; m++)
                {
                  coord.x = m;

                  read_bin_record_index (pfm_handle, coord, &bin);


                  if (!read_depth_array_index (pfm_handle, coord, &depth, &recnum))
                    {
                      for (int32_t n = 0 ; n < recnum ; n++)
                        {
                          int32_t urban = NINT (depth[n].attr[urban_attr]);


                          //  Count the CZMIL_URBAN_MAJORITY points.  If we don't have any after the vicinity search, there is no point in
                          //  running the filter.

                          if (urban & CZMIL_URBAN_MAJORITY) options->majority_count[pass]++;


                          //  Only use valid data flagged as CZMIL_URBAN_MAJORITY or CZMIL_URBAN_HARD_HIT.

                          if (!(depth[n].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE)) &&
                              (urban & (CZMIL_URBAN_MAJORITY | CZMIL_URBAN_HARD_HIT)))
                            {
                              try
                                {
                                  tpos.resize (tcount + 1);
                                }
                              catch (std::bad_alloc&)
                                {
                                  fprintf (stderr, "%s %s %d - tpos - %s\n", __FILE__, __FUNCTION__, __LINE__, strerror (errno));
                                  exit (-1);
                                }


                              //  Populate the position array with the position of the point in meters from the southwest corner of
                              //  the area.  This allows us to directly compute distance between points using just the sqrt function.

                              double dist_x, dist_y;

                              pfm_geo_distance (pfm_handle, open_args.head.mbr.min_y, depth[n].xyz.x, depth[n].xyz.y, depth[n].xyz.x, &dist_y);
                              pfm_geo_distance (pfm_handle, depth[n].xyz.y, open_args.head.mbr.min_x, depth[n].xyz.y, depth[n].xyz.x, &dist_x);

                              tpos[tcount].x = (float) dist_x;
                              tpos[tcount].y = (float) dist_y;
                              tpos[tcount].z = depth[n].xyz.z;
                              tpos[tcount].h = depth[n].horizontal_error;
                              tpos[tcount].v = depth[n].vertical_error;
                              tpos[tcount].coord = coord;
                              tpos[tcount].array_index = n;
                              tpos[tcount].urban_attr = urban;
                              tpos[tcount].saved = NVFalse;
                              tpos[tcount].flagged = NVFalse;
                              tcount++;
                            }
                        }

                      free (depth);
                    }
                }
            }



          //  If we don't have a count, we don't have any data.

          if (tcount)
            {
              float hp_half_height = options->vicinityHeight / 2.0;


              //  Determine which points need to be flagged.  This uses the dreaded Hockey Puck of Confidence (TM).

              uint8_t inside_ref = NVFalse;

              for (int32_t k = 0 ; k < tcount ; k++)
                {
                  //  Check to see if this point is in our referenced bin and it is a CZMIL_URBAN_HARD_HIT flagged point.  Why am I
                  //  using reverse logic to determine this?  Because, some points will be marked as CZMIL_URBAN_MAJORITY *AND*
                  //  CZMIL_URBAN_HARD_HIT.  Since all points in the array are flagged as CZMIL_URBAN_HARD_HIT and/or
                  //  CZMIL_URBAN_MAJORITY, any point that is not flagged as CZMIL_URBAN_MAJORITY must, by definition, be flagged
                  //  as *ONLY* CZMIL_URBAN_HARD_HIT.  In that case we need to check for CZMIL_URBAN_MAJORITY flagged points in the
                  //  immediate vicinity.  If it's already marked as CZMIL_URBAN_MAJORITY we don't need to set CZMIL_URBAN_VICINITY
                  //  because the filter will use either one.

                  if (tpos[k].coord.x == ref_coord.x && tpos[k].coord.y == ref_coord.y && (!(tpos[k].urban_attr & CZMIL_URBAN_MAJORITY)))
                    {
                      //  Set flag when we get a point inside our reference bin.

                      inside_ref = NVTrue;


                      //  Make sure we haven't already marked it for flagging.

                      if (!tpos[k].flagged)
                        {
                          //  Check for inclusion and/or exclusion.

                          uint8_t flag_point = NVTrue;
                          if (options->in_polygon_count)
                            {
                              if (!inside_polygon2 (options->in_polygon_x, options->in_polygon_y, options->in_polygon_count,
                                                    tpos[k].x, tpos[k].y)) flag_point = NVFalse;
                            }

                          if (options->ex_polygon_count)
                            {
                              if (inside_polygon2 (options->ex_polygon_x, options->ex_polygon_y, options->ex_polygon_count,
                                                   tpos[k].x, tpos[k].y)) flag_point = NVFalse;
                            }


                          if (flag_point)
                            {
                              //  Increment the count of total points.

                              options->point_count[pass]++;


                              for (int32_t m = 0 ; m < tcount ; m++)
                                {
                                  //  Don't check against itself.

                                  if (m != k)
                                    {
                                      //  Simple check for exceeding distance in X or Y direction (prior to a radius check).

                                      double diff_x = fabs (tpos[k].x - tpos[m].x);
                                      double diff_y = fabs (tpos[k].y - tpos[m].y);


                                      //  The radius.

                                      double dist = options->vicinityRadius;  // +  horizontal error  ???


                                      //  The depth offset.

                                      float height_offset = hp_half_height;  // + vertical error ???


                                      //  Simple check for either X or Y exceeding our distance (so we don't do too many SQRTs)

                                      if (diff_x <= dist && diff_y <= dist)
                                        {
                                          double pnt_dist = sqrt (diff_x * diff_x + diff_y * diff_y);


                                          //  Check the distance.

                                          if (pnt_dist <= dist)
                                            {
                                              //  Check the Z difference.

                                              if (fabs (tpos[k].z - tpos[m].z) < height_offset)
                                                {
                                                  //  All we need to look for is a single point in the hockey puck that is flagged as 
                                                  //  CZMIL_URBAN_MAJORITY.  When we find one of those we can mark this point to be
                                                  //  flagged as CZMIL_URBAN_VICINITY so that it can be used for filtering later.

                                                  if (tpos[m].urban_attr & CZMIL_URBAN_MAJORITY)
                                                    {
                                                      tpos[k].flagged = NVTrue;


                                                      //  Increment the count of flagged points.

                                                      options->filter_count[pass]++;

                                                      break;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }


                  //  If we were inside the reference bin and now we've moved out, we can break the loop.

                  else
                    {
                      if (inside_ref) break;
                    }
                }


              uint8_t done = NVFalse;

              for (int32_t k = 0 ; k < tcount ; k++)
                {
                  //  Check to see if this point needs to be flagged as CZMIL_URBAN_VICINITY.  Make sure it hasn't already been saved.

                  if (tpos[k].flagged && (!tpos[k].saved))
                    {
                      bin.coord = tpos[k].coord;
                      read_bin_depth_array_index (pfm_handle, &bin, &depth);


                      //  Now we loop through the array from the first "flagged" point until the coordinates change
                      //  (since they were loaded by bin cell).  This allows us to see if there are any more
                      //  points in this particular depth array that also need to be "flagged".

                      for (int32_t m = k ; m < tcount ; m++)
                        {
                          if (tpos[m].coord.x != ref_coord.x || tpos[m].coord.y != ref_coord.y)
                            {
                              done = NVTrue;
                              break;
                            }

                          if (tpos[m].flagged)
                            {
                              tpos[m].saved = NVTrue;


                              depth[tpos[m].array_index].attr[urban_attr] =
                                (float) (NINT (depth[tpos[m].array_index].attr[urban_attr]) | CZMIL_URBAN_VICINITY);


                              //  Note that we have to use change_depth_record instead of update_depth_record because we
                              //  need to modify the attribute instead of the status.

                              change_depth_record_index (pfm_handle, &depth[tpos[m].array_index]);
                            }
                        }

                      free (depth);
                      if (done) break;
                    }
                }


              //  Clear the topo vector.

              tpos.clear ();
            }


          percent = NINT (((float) ((i - start_row) * open_args.head.bin_width + j) /
                           (float) (open_args.head.bin_width * (end_row - start_row))) * 100.0);
          if (percent != old_percent)
            {
              emit percentValue (percent, pass);
              old_percent = percent;
            }
        }
    }


  emit percentValue (100, pass);


  //qDebug () << __LINE__ << pass;


  emit completed (pass);
  qApp->processEvents ();
}
