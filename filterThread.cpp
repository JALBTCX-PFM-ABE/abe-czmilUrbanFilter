
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



#include "filterThread.hpp"

filterThread::filterThread (QObject *parent)
  : QThread(parent)
{
}



filterThread::~filterThread ()
{
}



void filterThread::filter (OPTIONS *op, int32_t ph, PFM_OPEN_ARGS *oa, int32_t sr, int32_t er, int32_t o, int32_t p)
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



void filterThread::run ()
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
  int32_t d_index_attr = options->d_index_attr;


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


                          //  Only use valid data flagged as CZMIL_URBAN_TOPO.

                          if (!(depth[n].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE)) &&
                              (urban & CZMIL_URBAN_TOPO))
                            {
                              //  Don't use data flagged as CZMIL_URBAN_SOFT_HIT.

                              if (!(urban & (CZMIL_URBAN_SOFT_HIT)))
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

                                  pfm_geo_distance (pfm_handle, open_args.head.mbr.min_y, depth[n].xyz.x, depth[n].xyz.y, depth[n].xyz.x,
                                                    &dist_y);
                                  pfm_geo_distance (pfm_handle, depth[n].xyz.y, open_args.head.mbr.min_x, depth[n].xyz.y, depth[n].xyz.x,
                                                    &dist_x);

                                  tpos[tcount].x = (float) dist_x;
                                  tpos[tcount].y = (float) dist_y;
                                  tpos[tcount].z = depth[n].xyz.z;
                                  tpos[tcount].h = depth[n].horizontal_error;
                                  tpos[tcount].v = depth[n].vertical_error;
                                  tpos[tcount].coord = coord;
                                  tpos[tcount].array_index = n;
                                  tpos[tcount].d_index = NINT (depth[n].attr[d_index_attr]);
                                  tpos[tcount].urban_attr = urban;
                                  tpos[tcount].saved = NVFalse;
                                  tpos[tcount].killed = NVFalse;
                                  tcount++;
                                }
                            }
                        }

                      free (depth);
                    }
                }
            }


          uint8_t modified = NVFalse;


          //  If we don't have a count, we don't have any data.

          if (tcount)
            {
              float hp_half_height = options->hpHeight / 2.0;


              //  Determine which points need to be deleted.  This uses the dreaded Hockey Puck of Confidence (TM).  We have to
              //  run through this process iteratively since a point may be deemed good based on nearby points that might later
              //  be deemed invalid.  To do this we run the loop until we don't kill any more points.

              int32_t new_killed = 0;
              do
                {
                  new_killed = 0;
                  uint8_t inside_ref = NVFalse;

                  for (int32_t k = 0 ; k < tcount ; k++)
                    {
                      //  Check to see if this point is in our referenced bin.

                      if (tpos[k].coord.x == ref_coord.x && tpos[k].coord.y == ref_coord.y)
                        {
                          //  Set flag when we get a point inside our reference bin.

                          inside_ref = NVTrue;


                          //  Make sure we haven't already marked the point for killing.

                          if (!tpos[k].killed)
                            {
                              //  Check for inclusion and/or exclusion.

                              uint8_t filter_point = NVTrue;
                              if (options->in_polygon_count)
                                {
                                  if (!inside_polygon2 (options->in_polygon_x, options->in_polygon_y, options->in_polygon_count,
                                                        tpos[k].x, tpos[k].y)) filter_point = NVFalse;
                                }

                              if (options->ex_polygon_count)
                                {
                                  if (inside_polygon2 (options->ex_polygon_x, options->ex_polygon_y, options->ex_polygon_count,
                                                       tpos[k].x, tpos[k].y)) filter_point = NVFalse;
                                }


                              if (filter_point)
                                {
                                  //  Increment the count of total points.

                                  options->point_count[pass]++;


                                  //  Don't process points that exceed the lower D_index cutoff

                                  if (tpos[k].d_index < options->hpD_index[0])
                                    {
                                      for (int32_t m = 0 ; m < tcount ; m++)
                                        {
                                          //  Don't check against itself.

                                          if (m != k)
                                            {
                                              //  Simple check for exceeding distance in X or Y direction (prior to a radius check).

                                              double diff_x = fabs (tpos[k].x - tpos[m].x);
                                              double diff_y = fabs (tpos[k].y - tpos[m].y);


                                              //  The radius.

                                              double dist = options->hpRadius;  // +  horizontal error  ???


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
                                                          //  Now that we're inside the radius/height and we know that this point's
                                                          //  D_index value is less than the cutoff we need to make sure that at least one
                                                          //  of the points inside is marked as CZMIL_URBAN_MAJORITY or CZMIL_URBAN_VICINITY
                                                          //  and that it's D_index is greater than or equal to the D-index upper cutoff.

                                                          if ((tpos[m].urban_attr & (CZMIL_URBAN_MAJORITY | CZMIL_URBAN_VICINITY)) &&
                                                              tpos[m].d_index >= options->hpD_index[1])
                                                            {
                                                              tpos[k].killed = NVTrue;


                                                              //  Add one to the while loop's kill count so we'll know when to stop.

                                                              new_killed++;


                                                              //  Increment the count of filtered points.

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
                      //  Check to see if this point needs to be marked as suspect (or invalidated).  Make sure it hasn't already been saved.

                      if (tpos[k].killed && (!tpos[k].saved))
                        {
                          bin.coord = tpos[k].coord;
                          read_bin_depth_array_index (pfm_handle, &bin, &depth);


                          //  Obviously, the first one found with the killed flag will cause the record to be "modified".

                          modified = NVTrue;


                          //  Now we loop through the array from the first "killed" point until the coordinates change
                          //  (since they were loaded by bin cell).  This allows us to see if there are any more
                          //  points in this particular depth array that also need to be "killed".

                          for (int32_t m = k ; m < tcount ; m++)
                            {
                              if (tpos[m].coord.x != ref_coord.x || tpos[m].coord.y != ref_coord.y)
                                {
                                  done = NVTrue;
                                  break;
                                }

                              if (tpos[m].killed)
                                {
                                  tpos[m].saved = NVTrue;


                                  //  Note that we're using PFM_USER_06 to represent externally hockey puck filtered data.

                                  if (options->invalidate)
                                    {
                                      depth[tpos[m].array_index].validity |= (PFM_FILTER_INVAL | PFM_USER_06);
                                    }
                                  else
                                    {
                                      depth[tpos[m].array_index].validity |= PFM_SUSPECT;
                                    }

                                  update_depth_record_index (pfm_handle, &depth[tpos[m].array_index]);
                                }
                            }

                          if (modified)
                            {
                              if (options->invalidate)
                                {
                                  recompute_bin_values_from_depth_index (pfm_handle, &bin, 0, depth);
                                }
                              else
                                {
                                  bin.validity |= PFM_SUSPECT;
                                  recompute_bin_values_from_depth_index (pfm_handle, &bin, PFM_SUSPECT, depth);
                                }

                              modified = NVFalse;
                            }

                          free (depth);
                          if (done) break;
                        }
                    }


                  //  Hard wiring it to do only one pass.  Later on we might change this.

                  //  new_killed = 0;

                } while (new_killed);


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
