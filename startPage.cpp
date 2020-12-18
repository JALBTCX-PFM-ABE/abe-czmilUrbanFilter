
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



#include "startPage.hpp"
#include "startPageHelp.hpp"

startPage::startPage (int32_t *argc, char **argv, QWidget *parent, OPTIONS *op):
  QWizardPage (parent)
{
  options = op;


  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/czmilUrbanFilterWatermark.png"));


  setTitle (tr ("Introduction"));

  QLabel *label = new QLabel (tr ("czmilUrbanFilter is a tool for filtering a PFM that has been built from CZMIL data. "
                                  "This filter runs in two passes.  First, it searches the data for any topo points that are in "
                                  "the vicinity of urban feature points and marks them as being in the vicinity of an urban feature.  "
                                  "Urban feature points are marked by pfmLoad as CZMIL_URBAN_MAJORITY if a majority of the 7 channels "
                                  "of a topo shot had less than 4 valid returns.<br><br>"
                                  "The second pass uses the Hockey Puck filter to mark points (excluding CZMIL_URBAN_SOFT_HIT points) as "
                                  "invalid or suspect if their D_index value is less than the lower D_index cutoff and they are within "
                                  "the filter radius/height of urban features (CZMIL_URBAN_MAJORITY or CZMIL_URBAN_VICINITY) and whose "
                                  "D_index value is greater than or equal to the upper D_index value.<br><br>"
                                  "Data that does not meet the Hockey Puck filter criteria will be flagged as PFM_SUSPECT or marked as "
                                  "PFM_FILTER_INVAL depending on the requested option.  If you mark points as suspect "
                                  "they can be seen in pfmView by turning on the suspect flags option.  Help is available "
                                  "by clicking on the Help button and then clicking on the item for which you want help.  For a "
                                  "ridiculously long explanation of the CZMIL Urban Noise Filter algorithms, click on the Help button "
                                  "and then click on this text.<br><br>"
                                  "Select a PFM file below.  You may then, optionally, select an area file "
                                  "to limit the area to be filtered and/or select an area file to define an "
                                  "area to be excluded from filtering.  Click <b>Next</b> to continue or "
                                  "<b>Cancel</b> to exit."));

  label->setWhatsThis (tr ("The CZMIL Urban Noise Filter (also known as the Nick Johnson Filter) algorithms for loading, marking, searching, "
                           "and filtering (which are, by necessity, linked) are as follows:<br><br>"
                           "<ul>"
                           "<li>All returns from shots that are processed as topo (Optech processing mode = 1) will be marked as "
                           "CZMIL_URBAN_TOPO (bit 1 set, returns from hybrid or bathy processed shots will be marked as 0).</li>"
                           "<li>All returns from shots that are processed as topo that have 5 or more valid returns on that channel will "
                           "be marked as CZMIL_URBAN_SOFT_HIT (bit 2 set).</li>"
                           "<li>All returns from shots that are processed as topo that have 2  or fewer valid returns will be marked as "
                           "CZMIL_URBAN_HARD_HIT (bit 3 set).  Note that CZMIL_URBAN_SOFT_HIT and CZMIL_URBAN_HARD_HIT are mutually "
                           "exclusive.</li>"
                           "<li>Returns on channels with 3 or fewer returns from shots that are processed as topo that have 3 or fewer "
                           "valid returns on a majority (4 or more of 7, IR and deep channels are ignored) of the channels will be marked "
                           "as CZMIL_URBAN_MAJORITY (bit 4 set).  Pay close attention here - returns on channels with 4 or more valid "
                           "returns are <font color=\"#ff0000\">NOT</font> marked as CZMIL_URBAN_MAJORITY.</li>"
                           "<li>The filter, either in czmilUrbanFilter or in pfmEdit3D, will perform a hockey puck (user specified radius "
                           "and height) vicinity search based on the criteria set in the program.  Any valid CZMIL_URBAN_HARD_HIT return "
                           "from a topo processed shot that is within the specified radius and height of a valid CZMIL_URBAN_MAJORITY "
                           "return will be marked as CZMIL_URBAN_VICINITY.  In czmilUrbanFilter this is written to the CZMIL Urban noise "
                           "flags attribute.  In pfmEdit3D it is not written to the attribute since the search is always run (due to "
                           "possible changes in validity).</li>"
                           "<li>The filter, either in czmilUrbanFilter or pfmEdit3D, will perform a hockey puck search for valid "
                           "CZMIL_URBAN_TOPO returns within the user specified radius and height of any valid CZMIL_URBAN_MAJORITY or "
                           "CZMIL_URBAN_VICINITY return with a D_index value that meets or exceeds the user specified upper D_index cutoff. "
                           "Prior to the search, CZMIL_URBAN_TOPO points that exceed the user specified lower D_index cutoff are excluded "
                           "(no point in looking at them since they won't be invalidated).  Also excluded are CZMIL_URBAN_TOPO returns that "
                           "are also marked as CZMIL_URBAN_SOFT_HIT.  Once it gets through all that testing, any remaining valid "
                           "CZMIL_URBAN_TOPO points with a D_index value less than the user specified lower D_index cutoff will be marked as "
                           "invalid.  In czmilUrbanFilter they will be marked as PFM_FILTER_INVAL with PFM_USER_FLAG_06 set (unless the user "
                           "specified setting to PFM_SUSPECT) and in pfmEdit3D they will be set to PFM_MANUALLY_INVAL.</li>"
                           "</ul><br><br>"
                           "The binary flags that will be displayed in pfmEdit3D (as a series of 1s and 0s) will be in this order, left to "
                           "right:<br><br>"
                           "CZMIL_URBAN_VICINITY, CZMIL_URBAN_MAJORITY, CZMIL_URBAN_HARD_HIT, CZMIL_URBAN_SOFT_HIT, CZMIL_URBAN_TOPO"));


  label->setWordWrap (true);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->addWidget (label);


  QHBoxLayout *pfm_file_box = new QHBoxLayout (0);
  pfm_file_box->setSpacing (8);

  vbox->addLayout (pfm_file_box);


  QLabel *pfm_file_label = new QLabel (tr ("PFM File"), this);
  pfm_file_box->addWidget (pfm_file_label, 1);

  pfm_file_edit = new QLineEdit (this);
  pfm_file_edit->setReadOnly (true);
  pfm_file_box->addWidget (pfm_file_edit, 10);

  QPushButton *pfm_file_browse = new QPushButton (tr ("Browse..."), this);
  pfm_file_box->addWidget (pfm_file_browse, 1);

  pfm_file_label->setWhatsThis (pfm_fileText);
  pfm_file_edit->setWhatsThis (pfm_fileText);
  pfm_file_browse->setWhatsThis (pfm_fileBrowseText);

  connect (pfm_file_browse, SIGNAL (clicked ()), this, SLOT (slotPFMFileBrowse ()));



  QHBoxLayout *area_box = new QHBoxLayout (0);
  area_box->setSpacing (8);

  vbox->addLayout (area_box);


  QLabel *area_file_label = new QLabel (tr ("Optional Area File"), this);
  area_box->addWidget (area_file_label, 1);

  area_file_edit = new QLineEdit (this);
  area_file_edit->setReadOnly (true);
  area_box->addWidget (area_file_edit, 10);

  QPushButton *area_file_browse = new QPushButton (tr ("Browse..."), this);
  area_box->addWidget (area_file_browse, 1);


  area_file_label->setWhatsThis (area_fileText);
  area_file_edit->setWhatsThis (area_fileText);
  area_file_browse->setWhatsThis (area_fileBrowseText);

  connect (area_file_browse, SIGNAL (clicked ()), this, SLOT (slotAreaFileBrowse ()));


  QHBoxLayout *ex_box = new QHBoxLayout (0);
  ex_box->setSpacing (8);

  vbox->addLayout (ex_box);


  QLabel *ex_file_label = new QLabel (tr ("Optional Exclusion Area File"), this);
  ex_box->addWidget (ex_file_label, 1);

  ex_file_edit = new QLineEdit (this);
  ex_file_edit->setReadOnly (true);
  ex_box->addWidget (ex_file_edit, 10);

  QPushButton *ex_file_browse = new QPushButton (tr ("Browse..."), this);
  ex_box->addWidget (ex_file_browse, 1);


  ex_file_label->setWhatsThis (ex_fileText);
  ex_file_edit->setWhatsThis (ex_fileText);
  ex_file_browse->setWhatsThis (ex_fileBrowseText);

  connect (ex_file_browse, SIGNAL (clicked ()), this, SLOT (slotExFileBrowse ()));


  if (*argc == 2)
    {
      PFM_OPEN_ARGS open_args;
      int32_t pfm_handle = -1;

      QString pfm_file_name = QString (argv[1]);

      strcpy (open_args.list_path, argv[1]);

      open_args.checkpoint = 0;
      pfm_handle = open_existing_pfm_file (&open_args);

      if (pfm_handle >= 0)
        {
          if (!open_args.head.proj_data.projection)
            {
              pfm_file_edit->setText (pfm_file_name);
            }

          close_pfm_file (pfm_handle);
        }
    }


  if (!pfm_file_edit->text ().isEmpty ())
    {
      registerField ("pfm_file_edit", pfm_file_edit);
    }
  else
    {
      registerField ("pfm_file_edit*", pfm_file_edit);
    }


  registerField ("area_file_edit", area_file_edit);
  registerField ("ex_file_edit", ex_file_edit);
}



void startPage::slotPFMFileBrowse ()
{
  PFM_OPEN_ARGS       open_args;
  QStringList         files, filters;
  QString             file;
  int32_t             pfm_handle = -1;


  QFileDialog *fd = new QFileDialog (this, tr ("czmilUrbanFilter Open PFM Structure"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options->input_dir);


  filters << tr ("PFM (*.pfm)");

  fd->setNameFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectNameFilter (tr ("PFM (*.pfm)"));

  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      QString pfm_file_name = files.at (0);


      if (!pfm_file_name.isEmpty())
        {
          strcpy (open_args.list_path, pfm_file_name.toLatin1 ());

          open_args.checkpoint = 0;
          pfm_handle = open_existing_pfm_file (&open_args);

          if (pfm_handle < 0)
            {
              QMessageBox::warning (this, tr ("Open PFM Structure"),
                                    tr ("The file ") + QDir::toNativeSeparators (QString (open_args.list_path)) + 
                                    tr (" is not a PFM structure or there was an error reading the file.") +
                                    tr ("  The error message returned was:\n\n") +
                                    QString (pfm_error_str (pfm_error)));

              if (pfm_error == CHECKPOINT_FILE_EXISTS_ERROR)
                {
                  fprintf (stderr, "\n\n%s\n", pfm_error_str (pfm_error));
                  exit (-1);
                }

              return;
            }


          //  Make sure this file has CZMIL data.

          char path[2048];
          int16_t type;

          read_list_file (pfm_handle, 0, path, &type);

          if (type != PFM_CZMIL_DATA)
            {
              QMessageBox::warning (this, tr ("Open PFM Structure"),
                                    tr ("The file ") + QDir::toNativeSeparators (QString (open_args.list_path)) + 
                                    tr (" does not contain only CZMIL data."));

              exit (-1);
            }


          //  Make sure this file has CZMIL Classification as an attribute.

          options->class_attr = -1;
          for (int32_t i = 0 ; i < open_args.head.num_ndx_attr ; i++)
            {
              if (!strncmp (open_args.head.ndx_attr_name[i], "CZMIL Classification", 20))
                {
                  options->class_attr = i;
                  break;
                }
            }

          if (options->class_attr < 0)
            {
              QMessageBox::warning (this, tr ("Open PFM Structure"),
                                    tr ("The file ") + QDir::toNativeSeparators (QString (open_args.list_path)) + 
                                    tr (" does not have CZMIL Classification as an attribute."));

              exit (-1);
            }



          //  Make sure this file has CZMIL Urban noise flags as an attribute.

          options->urban_attr = -1;
          for (int32_t i = 0 ; i < open_args.head.num_ndx_attr ; i++)
            {
              if (!strncmp (open_args.head.ndx_attr_name[i], "CZMIL Urban noise flags", 23))
                {
                  options->urban_attr = i;
                  break;
                }
            }

          if (options->urban_attr < 0)
            {
              QMessageBox::warning (this, tr ("Open PFM Structure"),
                                    tr ("The file ") + QDir::toNativeSeparators (QString (open_args.list_path)) + 
                                    tr (" does not have CZMIL Urban noise flags as an attribute."));

              exit (-1);
            }


          //  Make sure this file has CZMIL D_index as an attribute.

          options->d_index_attr = -1;
          for (int32_t i = 0 ; i < open_args.head.num_ndx_attr ; i++)
            {
              if (!strncmp (open_args.head.ndx_attr_name[i], "CZMIL D_index", 13))
                {
                  options->d_index_attr = i;
                  break;
                }
            }

          if (options->d_index_attr < 0)
            {
              QMessageBox::warning (this, tr ("Open PFM Structure"),
                                    tr ("The file ") + QDir::toNativeSeparators (QString (open_args.list_path)) + 
                                    tr (" does not have CZMIL D_index as an attribute."));

              exit (-1);
            }


          //  Make sure this file has the CZMIL HP Filtered user flag

          options->hp_flag = -1;
          for (int32_t i = 0 ; i < PFM_USER_FLAGS ; i++)
            {
              if (strstr (open_args.head.user_flag_name[i], "CZMIL HP Filtered"))
                {
                  options->hp_flag = i;
                  break;
                }
            }

          if (options->hp_flag < 0)
            {
              QMessageBox::warning (this, tr ("Open PFM Structure"),
                                    tr ("The file ") + QDir::toNativeSeparators (QString (open_args.list_path)) + 
                                    tr (" does not have CZMIL HP Filtered as a user flag."));

              exit (-1);
            }

          if (options->hp_flag != 5)
            {
              QMessageBox::warning (this, tr ("Open PFM Structure"),
                                    tr ("The file ") + QDir::toNativeSeparators (QString (open_args.list_path)) + 
                                    tr (" does not have CZMIL HP Filtered as PFM_USER_06.  Get Jan to fix this crap!"));

              exit (-1);
            }
        }

      if (open_args.head.proj_data.projection)
        {
          QMessageBox::warning (this, tr ("Open PFM Structure"),
                                tr ("Sorry, czmilUrbanFilter only handles geographic PFM structures."));
          close_pfm_file (pfm_handle);
        }


      pfm_file_edit->setText (pfm_file_name);


      options->input_dir = fd->directory ().absolutePath ();
    }
}



void startPage::slotAreaFileBrowse ()
{
  SHPHandle shpHandle;
  SHPObject *shape = NULL;
  int32_t type, numShapes;
  double minBounds[4], maxBounds[4];


  QFileDialog *fd = new QFileDialog (this, tr ("czmilUrbanFilter Area File"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options->area_dir);


  QStringList filters;
  filters << tr ("Area file (*.ARE *.are *.afs *.shp *.SHP)");

  fd->setNameFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectNameFilter (tr ("Area file (*.ARE *.are *.afs *.shp *.SHP)"));


  QStringList files;
  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      QString area_file_name = files.at (0);


      if (!area_file_name.isEmpty())
        {
          if (area_file_name.endsWith (".shp", Qt::CaseInsensitive))
            {
              char shpname[1024];
              strcpy (shpname, area_file_name.toLatin1 ());


              //  Open shape file

              shpHandle = SHPOpen (shpname, "rb");

              if (shpHandle == NULL)
                {
                  QMessageBox::warning (this, tr ("czmilUrbanFilter"), tr ("Cannot open shape file %1!").arg (area_file_name));
                  return;
                }
              else
                {
                  //  Get shape file header info

                  SHPGetInfo (shpHandle, &numShapes, &type, minBounds, maxBounds);


                  if (type != SHPT_POLYGON && type != SHPT_POLYGONZ && type != SHPT_POLYGONM &&
                      type != SHPT_ARC && type != SHPT_ARCZ && type != SHPT_ARCM)
                    {
                      QMessageBox::warning (this, tr ("czmilUrbanFilter"), tr ("Shape file %1 is not a polygon or polyline file!").arg (area_file_name));
                      return;
                    }
                  else
                    {
                      //  Read only the first shape.

                      shape = SHPReadObject (shpHandle, 0);


                      //  Check the number of vertices.

                      if (shape->nVertices < 3)
                        {
                          SHPClose (shpHandle);
                          QMessageBox::warning (this, tr ("czmilUrbanFilter"), tr ("Number of vertices (%1) of shape file %2 is too few for a polygon!").arg
                                                (shape->nVertices).arg (area_file_name));
                          return;
                        }


                      //  Read the vertices to take a shot at determining that this is a geographic polygon.

                      for (int32_t j = 0 ; j < shape->nVertices ; j++)
                        {
                          if (shape->padfX[j] < -360.0 || shape->padfX[j] > 360.0 || shape->padfY[j] < -90.0 || shape->padfY[j] > 90.0)
                            {
                              SHPDestroyObject (shape);
                              SHPClose (shpHandle);
                              QMessageBox::warning (this, tr ("czmilUrbanFilter"), tr ("Shape file %1 does not appear to be geographic!").arg (area_file_name));
                              return;
                            }
                        }


                      SHPDestroyObject (shape);
                      SHPClose (shpHandle);
                    }
                }
            }

          area_file_edit->setText (area_file_name);
        }

      options->area_dir = fd->directory ().absolutePath ();
    }
}



void startPage::slotExFileBrowse ()
{
  SHPHandle shpHandle;
  SHPObject *shape = NULL;
  int32_t type, numShapes;
  double minBounds[4], maxBounds[4];


  QFileDialog *fd = new QFileDialog (this, tr ("czmilUrbanFilter Exclusion Area File"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options->exclude_dir);


  QStringList filters;
  filters << tr ("Area file (*.ARE *.are *.afs *.shp *.SHP)");

  fd->setNameFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectNameFilter (tr ("Area file (*.ARE *.are *.afs *.shp *.SHP)"));


  QStringList files;
  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      QString ex_file_name = files.at (0);


      if (!ex_file_name.isEmpty())
        {
          if (ex_file_name.endsWith (".shp", Qt::CaseInsensitive))
            {
              char shpname[1024];
              strcpy (shpname, ex_file_name.toLatin1 ());


              //  Open shape file

              shpHandle = SHPOpen (shpname, "rb");

              if (shpHandle == NULL)
                {
                  QMessageBox::warning (this, tr ("czmilUrbanFilter"), tr ("Cannot open shape file %1!").arg (ex_file_name));
                  return;
                }
              else
                {
                  //  Get shape file header info

                  SHPGetInfo (shpHandle, &numShapes, &type, minBounds, maxBounds);


                  if (type != SHPT_POLYGON && type != SHPT_POLYGONZ && type != SHPT_POLYGONM &&
                      type != SHPT_ARC && type != SHPT_ARCZ && type != SHPT_ARCM)
                    {
                      QMessageBox::warning (this, tr ("czmilUrbanFilter"), tr ("Shape file %1 is not a polygon or polyline file!").arg (ex_file_name));
                      return;
                    }
                  else
                    {
                      //  Read only the first shape.

                      shape = SHPReadObject (shpHandle, 0);


                      //  Check the number of vertices.

                      if (shape->nVertices < 3)
                        {
                          SHPClose (shpHandle);
                          QMessageBox::warning (this, tr ("czmilUrbanFilter"), tr ("Number of vertices (%1) of shape file %2 is too few for a polygon!").arg
                                                (shape->nVertices).arg (ex_file_name));
                          return;
                        }


                      //  Read the vertices to take a shot at determining that this is a geographic polygon.

                      for (int32_t j = 0 ; j < shape->nVertices ; j++)
                        {
                          if (shape->padfX[j] < -360.0 || shape->padfX[j] > 360.0 || shape->padfY[j] < -90.0 || shape->padfY[j] > 90.0)
                            {
                              SHPDestroyObject (shape);
                              SHPClose (shpHandle);
                              QMessageBox::warning (this, tr ("czmilUrbanFilter"), tr ("Shape file %1 does not appear to be geographic!").arg (ex_file_name));
                              return;
                            }
                        }


                      SHPDestroyObject (shape);
                      SHPClose (shpHandle);
                    }
                }
            }

          ex_file_edit->setText (ex_file_name);
        }

      options->exclude_dir = fd->directory ().absolutePath ();
    }
}
