
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



#include "czmilUrbanFilter.hpp"
#include "czmilUrbanFilterHelp.hpp"

#include <getopt.h>

double settings_version = 1.0;


czmilUrbanFilter::czmilUrbanFilter (int32_t *argc, char **argv, QWidget *parent)
  : QWizard (parent, 0)
{
  QResource::registerResource ("/icons.rcc");


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/czmilUrbanFilterWatermark.png"));


  //  Get the command line arguments.  This is a placeholder.

  int32_t option_index = 0;

  while (NVTrue) 
    {
      static struct option long_options[] = {{"dummy", no_argument, 0, 0},
                                             {0, no_argument, 0, 0}};

      char c = (char) getopt_long (*argc, argv, "d", long_options, &option_index);
      if (c == -1) break;

      switch (c) 
        {
        case 0:

          switch (option_index)
            {
            case 0:
              //  dummy
              break;
            }
          break;

        case 'd':
          //  dummy
          break;
        }
    }


  //  Get the user's defaults if available

  envin (&options);


  // Set the application font

  QApplication::setFont (options.font);


  options.replace = NVTrue;


  setWizardStyle (QWizard::ClassicStyle);


  setOption (HaveHelpButton, true);
  setOption (ExtendedWatermarkPixmap, false);


  connect (this, SIGNAL (helpRequested ()), this, SLOT (slotHelpClicked ()));


  //  Set the window size and location from the defaults

  this->resize (options.window_width, options.window_height);
  this->move (options.window_x, options.window_y);


  setPage (0, new startPage (argc, argv, this, &options));

  setPage (1, oPage = new optionPage (this, &options));

  setPage (2, new runPage (this, &progress));


  setButtonText (QWizard::CustomButton1, tr("&Run"));
  setOption (QWizard::HaveCustomButton1, true);
  button (QWizard::CustomButton1)->setToolTip (tr ("Start filtering the PFM"));
  button (QWizard::CustomButton1)->setWhatsThis (runText);
  connect (this, SIGNAL (customButtonClicked (int)), this, SLOT (slotCustomButtonClicked (int)));


  setStartId (0);
}



czmilUrbanFilter::~czmilUrbanFilter ()
{
}



void czmilUrbanFilter::initializePage (int id)
{
  button (QWizard::HelpButton)->setIcon (QIcon (":/icons/contextHelp.png"));
  button (QWizard::CustomButton1)->setEnabled (false);


  switch (id)
    {
    case 0:
      break;

    case 1:
      pfm_file = field ("pfm_file_edit").toString ();
      area_file = field ("area_file_edit").toString ();
      ex_file = field ("ex_file_edit").toString ();
      oPage->setFileCenter (pfm_file);
      break;

    case 2:
      button (QWizard::CustomButton1)->setEnabled (true);


      options.vicinityRadius = field ("hpRadiusValue0").toDouble ();
      options.vicinityHeight = field ("hpHeightValue0").toDouble ();
      options.hpRadius = field ("hpRadiusValue1").toDouble ();
      options.hpHeight = field ("hpHeightValue1").toDouble ();
      options.hpD_index[0] = field ("D_indexValue0").toInt ();
      options.hpD_index[1] = field ("D_indexValue1").toInt ();


      //  Use frame geometry to get the absolute x and y.

      QRect tmp = this->frameGeometry ();
      options.window_x = tmp.x ();
      options.window_y = tmp.y ();


      //  Use geometry to get the width and height.

      tmp = this->geometry ();
      options.window_width = tmp.width ();
      options.window_height = tmp.height ();


      //  Save the options.

      envout (&options);


      QString string;

      progress.list->clear ();

      string = tr ("PFM file : ") + pfm_file;
      progress.list->addItem (string);

      if (!area_file.isEmpty ())
        {
          string = tr ("Area file : ") + area_file;
          progress.list->addItem (string);
        }

      if (!ex_file.isEmpty ())
        {
          string = tr ("Exclusion area file : ") + ex_file;
          progress.list->addItem (string);
        }



      string.sprintf (tr ("Urban vicinity radius = %.2f").toLatin1 (), options.vicinityRadius);
      progress.list->addItem (string);

      string.sprintf (tr ("Urban vicinity height = %.2f").toLatin1 (), options.vicinityHeight);
      progress.list->addItem (string);

      break;
    }
}



void czmilUrbanFilter::cleanupPage (int id)
{
  switch (id)
    {
    case 0:
      break;

    case 1:
      break;

    case 2:
      break;
    }
}



void czmilUrbanFilter::slotHelpClicked ()
{
  QWhatsThis::enterWhatsThisMode ();
}



//  This is where the fun stuff happens.

void 
czmilUrbanFilter::slotCustomButtonClicked (int id __attribute__ ((unused)))
{
  vicinityThread      vicinity_thread[THREADS];
  filterThread        filter_thread[THREADS];
  static int32_t      old_percent = -1;
  QString             qstring;


  QApplication::setOverrideCursor (Qt::WaitCursor);


  button (QWizard::FinishButton)->setEnabled (false);
  button (QWizard::BackButton)->setEnabled (false);
  button (QWizard::CustomButton1)->setEnabled (false);


  //  We open a different handle for each thread so that we don't have any collision problems in the PFM library.

  for (int32_t i = 0 ; i < THREADS ; i++)
    {
      strcpy (open_args[i].list_path, pfm_file.toLatin1 ());


      //  Open the PFM file.

      open_args[i].checkpoint = 0;
      if ((pfm_handle[i] = open_existing_pfm_file (&open_args[i])) < 0) pfm_error_exit (pfm_error);

      int32_t row_size = NINT (open_args[i].head.bin_height / THREADS);

      start_row[i] = i * row_size;
      end_row[i] = qMin ((i + 1) * row_size - 1, open_args[i].head.bin_height - 1);
    }


  //  Check for an exclusion area file.

  options.ex_polygon_count = 0;
  if (!ex_file.isEmpty ())
    {
      char ef[512];
      strcpy (ef, ex_file.toLatin1 ());

      get_area_mbr (ef, &options.ex_polygon_count, options.ex_polygon_x, options.ex_polygon_y, &options.ex_mbr);


      //  Convert the polygon to the pseudo X/Y that we will be using in the filter threads (we don't have to worry about the MBR
      //  because we don't use it).

      for (int32_t i = 0 ; i < options.ex_polygon_count ; i++)
        {
          double dist_x, dist_y;
          pfm_geo_distance (pfm_handle[0], open_args[0].head.mbr.min_y, options.ex_polygon_x[i], options.ex_polygon_y[i],
                            options.ex_polygon_x[i], &dist_y);
          pfm_geo_distance (pfm_handle[0], options.ex_polygon_y[i], open_args[0].head.mbr.min_x, options.ex_polygon_y[i],
                            options.ex_polygon_x[i], &dist_x);

          options.ex_polygon_y[i] = dist_y;
          options.ex_polygon_x[i] = dist_x;
        }
    }


  //  Check for an inclusion area file.

  options.in_polygon_count = 0;
  if (!area_file.isEmpty ())
    {
      char af[512];
      strcpy (af, area_file.toLatin1 ());

      get_area_mbr (af, &options.in_polygon_count, options.in_polygon_x, options.in_polygon_y, &options.in_mbr);


      //  Convert the polygon to the pseudo X/Y that we will be using in the filter threads (we don't have to worry about the MBR
      //  because we don't use it).

      for (int32_t i = 0 ; i < options.in_polygon_count ; i++)
        {
          double dist_x, dist_y;
          pfm_geo_distance (pfm_handle[0], open_args[0].head.mbr.min_y, options.in_polygon_x[i], options.in_polygon_y[i],
                            options.in_polygon_x[i], &dist_y);
          pfm_geo_distance (pfm_handle[0], options.in_polygon_y[i], open_args[0].head.mbr.min_x, options.in_polygon_y[i],
                            options.in_polygon_x[i], &dist_x);

          options.in_polygon_y[i] = dist_y;
          options.in_polygon_x[i] = dist_x;
        }
    }



  progress.sbox->setTitle (tr ("Searching for hard topo hits in the vicinity of marked urban returns"));
  progress.sbar->setWhatsThis (tr ("Progress of the search process."));


  overlap = NINT64 (options.hpRadius / open_args[0].head.bin_size_xy) + 1;


  //  We're starting all THREADS vicinity passes concurrently using threads.  Note that we're using the Qt::DirectConnection type
  //  for the signal/slot connections.  This causes all of the signals emitted from the threads to be serviced immediately.

  qRegisterMetaType<int32_t> ("int32_t");

  complete = NVFalse;
  for (int32_t i = 0 ; i < THREADS ; i++)
    {
      options.filter_count[i] = options.point_count[i] = options.majority_count[i] = 0;
      pass_complete[i] = NVFalse;
      connect (&vicinity_thread[i], SIGNAL (percentValue (int32_t, int32_t)), this, SLOT (slotPercentValue (int32_t, int32_t)),
               Qt::DirectConnection);
      connect (&vicinity_thread[i], SIGNAL (completed (int32_t)), this, SLOT (slotCompleted (int32_t)), Qt::DirectConnection);
    }


  for (int32_t i = 0 ; i < THREADS ; i++)
    {
      vicinity_thread[i].vicinity (&options, pfm_handle[i], &open_args[i], start_row[i], end_row[i], overlap, i);
    }


  //  We can't exit from this method until the threads are complete but we want to keep our progress bar updated.  This is a bit tricky 
  //  because you can't update the progress bar from within the slots connected to the thread signals.  Those slots are considered part
  //  of the filterion thread and not part of the GUI thread.  In the percent slot we just update the minimum percentage value (from
  //  any of the four threads) and monitor it here every second (updating the progress bar accordingly).  When all of the threads are
  //  finished we move on.

  while (!complete)
    {
#ifdef NVWIN3X
      Sleep (1000);
#else
      sleep (1);
#endif

      if (min_percent != old_percent)
        {
          progress.sbar->setValue (min_percent);
          old_percent = min_percent;
        }

      qApp->processEvents ();
    }


  progress.sbar->setValue (100);
  qApp->processEvents ();


  progress.list->addItem (" ");

  int32_t filter_count = 0;
  int32_t point_count = 0;
  int32_t majority_count = 0;
  for (int32_t i = 0 ; i < THREADS ; i++)
    {
      filter_count += options.filter_count[i];
      point_count += options.point_count[i];
      majority_count += options.majority_count[i];
    }

  if (!majority_count)
    {
      qstring = tr ("No CZMIL_URBAN_MAJORITY points have been found in the PFM.  Filter aborted, press Finish to exit.");

      QListWidgetItem *cur = new QListWidgetItem (qstring);
      progress.list->addItem (cur);
      progress.list->setCurrentItem (cur);
      progress.list->scrollToItem (cur);


      progress.sbar->setRange (0, 100);
      progress.sbar->setValue (100);
      qApp->processEvents ();


      button (QWizard::FinishButton)->setEnabled (true);
      button (QWizard::CancelButton)->setEnabled (false);


      QApplication::restoreOverrideCursor ();

      return;
    }


  float percent = 0;
  if (point_count) percent = ((float) filter_count / (float) point_count) * 100.0;

  qstring.sprintf (tr ("Urban expansion complete, %d of %d CZMIL_URBAN_HARD_HIT points (%0.2f%%) marked as CZMIL_URBAN_VICINITY.").toLatin1 (),
                   filter_count, point_count, percent);

  QListWidgetItem *cur = new QListWidgetItem (qstring);
  progress.list->addItem (cur);
  progress.list->setCurrentItem (cur);
  progress.list->scrollToItem (cur);


  qstring.sprintf (tr ("\nUrban filter radius = %.2f").toLatin1 (), options.hpRadius);
  progress.list->addItem (qstring);

  qstring.sprintf (tr ("Urban filter height = %.2f").toLatin1 (), options.hpHeight);
  progress.list->addItem (qstring);

  qstring.sprintf (tr ("Urban filter lower D_index cutoff = %d").toLatin1 (), options.hpD_index[0]);
  progress.list->addItem (qstring);

  qstring.sprintf (tr ("Urban filter upper D_index cutoff = %d").toLatin1 (), options.hpD_index[1]);
  progress.list->addItem (qstring);


  progress.sbox->setTitle (tr ("Filtering points based on D_index cutoffs in the vicinity of urban features"));
  progress.sbar->setWhatsThis (tr ("Progress of the filter process."));



  progress.sbar->setRange (0, 100);
  progress.sbar->setValue (100);
  qApp->processEvents ();


  //  We're starting all THREADS filter passes concurrently using threads.  Note that we're using the Qt::DirectConnection type
  //  for the signal/slot connections.  This causes all of the signals emitted from the threads to be serviced immediately.

  qRegisterMetaType<int32_t> ("int32_t");

  complete = NVFalse;
  for (int32_t i = 0 ; i < THREADS ; i++)
    {
      options.filter_count[i] = options.point_count[i] = 0;
      pass_complete[i] = NVFalse;
      connect (&filter_thread[i], SIGNAL (percentValue (int32_t, int32_t)), this, SLOT (slotPercentValue (int32_t, int32_t)),
               Qt::DirectConnection);
      connect (&filter_thread[i], SIGNAL (completed (int32_t)), this, SLOT (slotCompleted (int32_t)), Qt::DirectConnection);
    }


  for (int32_t i = 0 ; i < THREADS ; i++)
    {
      filter_thread[i].filter (&options, pfm_handle[i], &open_args[i], start_row[i], end_row[i], overlap, i);
    }


  //  We can't exit from this method until the threads are complete but we want to keep our progress bar updated.  This is a bit tricky 
  //  because you can't update the progress bar from within the slots connected to the thread signals.  Those slots are considered part
  //  of the filterion thread and not part of the GUI thread.  In the percent slot we just update the minimum percentage value (from
  //  any of the four threads) and monitor it here every second (updating the progress bar accordingly).  When all of the threads are
  //  finished we move on.

  while (!complete)
    {
#ifdef NVWIN3X
      Sleep (1000);
#else
      sleep (1);
#endif

      if (min_percent != old_percent)
        {
          progress.sbar->setValue (min_percent);
          old_percent = min_percent;
        }

      qApp->processEvents ();
    }


  progress.sbar->setValue (100);
  qApp->processEvents ();


  for (int32_t i = 0 ; i < THREADS ; i++) close_pfm_file (pfm_handle[i]);


  progress.sbar->setRange (0, 100);
  progress.sbar->setValue (100);
  qApp->processEvents ();


  button (QWizard::FinishButton)->setEnabled (true);
  button (QWizard::CancelButton)->setEnabled (false);


  QApplication::restoreOverrideCursor ();


  progress.list->addItem (" ");

  filter_count = 0;
  point_count = 0;
  for (int32_t i = 0 ; i < THREADS ; i++)
    {
      filter_count += options.filter_count[i];
      point_count += options.point_count[i];
    }

  percent = 0;
  if (point_count) percent = ((float) filter_count / (float) point_count) * 100.0;

  if (options.invalidate)
    {
      qstring.sprintf (tr ("Filtering complete, %d of %d points (%0.2f%%) invalidated, press Finish to exit.").toLatin1 (), filter_count,
                      point_count, percent);
    }
  else
    {
      qstring.sprintf (tr ("Filtering complete, %d of %d points (%0.2f%%) marked suspect, press Finish to exit.").toLatin1 (), filter_count,
                      point_count, percent);
    }

  QListWidgetItem *ncur = new QListWidgetItem (qstring);
  progress.list->addItem (ncur);
  progress.list->setCurrentItem (ncur);
  progress.list->scrollToItem (ncur);


  char string[1024];
  FILE *hfp = fopen (open_args[0].list_path, "r+");
  if (fgets (string, sizeof (string), hfp) == NULL)
    {
      strcpy (string, tr ("Error reading the handle file %1 in file %2, function %3, line %4.  This shouldn't happen!\n").arg 
              (open_args[0].list_path).arg (__FILE__).arg (__FUNCTION__).arg (__LINE__).toUtf8 ());
      fprintf (stderr, "%s", string);
      exit (-1);
    }

  if (!strstr (string, "PFM Handle File"))
    {
      strcpy (string, tr ("Error reading the handle file %1 in file %2, function %3, line %4.  This shouldn't happen!\n").arg 
              (open_args[0].list_path).arg (__FILE__).arg (__FUNCTION__).arg (__LINE__).toUtf8 ());
      fprintf (stderr, "%s", string);
      exit (-1);
    }

  QString msg;

  if (options.invalidate)
    {
      msg = tr ("# --------------------------------------\n"
                "# CZMIL Urban Filter Statistics\n"
                "# --------------------------------------\n"
                "# Topo Filter radius = %1\n"
                "# Topo Filter height = %2\n"
                "# Topo lower D_index cutoff = %3\n"
                "# Topo upper D_index cutoff = %4\n"
                "# Total number of points processed %5\n"
                "# Number of soundings invalidated : %6\n"
                "# Percent of soundings invalidated : %7\n"
                "# --------------------------------------\n").arg
        (options.hpRadius).arg (options.hpHeight).arg (options.hpD_index[0]).arg (options.hpD_index[1]).arg
        (point_count).arg (filter_count).arg (percent);
    }
  else
    {
      msg = tr ("# --------------------------------------\n"
                "# CZMIL Urban Filter Statistics\n"
                "# --------------------------------------\n"
                "# Topo Filter radius = %1\n"
                "# Topo Filter height = %2\n"
                "# Topo lower D_index cutoff = %3\n"
                "# Topo upper D_index cutoff = %4\n"
                "# Total number of points processed %5\n"
                "# Number of soundings marked suspect : %6\n"
                "# Percent of soundings suspect : %7\n"
                "# --------------------------------------\n").arg
        (options.hpRadius).arg (options.hpHeight).arg (options.hpD_index[0]).arg (options.hpD_index[1]).arg
        (point_count).arg (filter_count).arg (percent);
    }

  strcpy (string, msg.toUtf8 ());

  fseek (hfp, 0, SEEK_END);
  fprintf (hfp, "%s", string);
  fclose (hfp);
}



void 
czmilUrbanFilter::slotPercentValue (int32_t percent, int32_t pass)
{
  static int32_t save_percent[THREADS];

  save_percent[pass] = percent;

  min_percent = 1001;
  for (int32_t i = 0 ; i < THREADS ; i++) min_percent = qMin (min_percent, save_percent[i]);
}



void 
czmilUrbanFilter::slotCompleted (int32_t pass)
{
  pass_complete[pass] = NVTrue;

  uint8_t done = NVTrue;
  for (int32_t i = 0 ; i < THREADS ; i++) if (!pass_complete[i]) done = NVFalse;


  if (done) complete = NVTrue;
}



//  Get the users defaults.

void czmilUrbanFilter::envin (OPTIONS *options)
{
  // Set defaults so that if keys don't exist the parameters are defined

  options->vicinityRadius = 5.0;
  options->vicinityHeight = 10.0;
  options->hpRadius = 3.0;
  options->hpHeight = 6.0;
  options->hpD_index[0] = 25;
  options->hpD_index[1] = 125;

  options->invalidate = NVFalse;

  options->input_dir = ".";
  options->area_dir = ".";
  options->exclude_dir = ".";
  options->window_x = 0;
  options->window_y = 0;
  options->window_width = 1100;
  options->window_height = 500;


  //  We need to get the font from the global settings.

#ifdef NVWIN3X
  QString ini_file2 = QString (getenv ("USERPROFILE")) + "/ABE.config/" + "globalABE.ini";
#else
  QString ini_file2 = QString (getenv ("HOME")) + "/ABE.config/" + "globalABE.ini";
#endif

  options->font = QApplication::font ();

  QSettings settings2 (ini_file2, QSettings::IniFormat);
  settings2.beginGroup ("globalABE");


  options->vicinityRadius = settings2.value (QString ("CZMIL urban vicinity radius"), options->vicinityRadius).toDouble ();
  options->hpRadius = settings2.value (QString ("CZMIL urban filter radius"), options->hpRadius).toDouble ();
  options->vicinityHeight = settings2.value (QString ("CZMIL urban vicinity height"), options->vicinityHeight).toDouble ();
  options->hpHeight = settings2.value (QString ("CZMIL urban filter height"), options->hpHeight).toDouble ();
  options->hpD_index[0] = settings2.value (QString ("CZMIL urban filter lower D_index"), options->hpD_index[0]).toInt ();
  options->hpD_index[1] = settings2.value (QString ("CZMIL urban filter upper D_index"), options->hpD_index[1]).toInt ();


  QString defaultFont = options->font.toString ();
  QString fontString = settings2.value (QString ("ABE map GUI font"), defaultFont).toString ();
  options->font.fromString (fontString);


  settings2.endGroup ();


  double saved_version = 1.0;


  //  Get the INI file name

#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/czmilUrbanFilter.ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/czmilUrbanFilter.ini";
#endif

  QSettings settings (ini_file, QSettings::IniFormat);
  settings.beginGroup ("czmilUrbanFilter");

  saved_version = settings.value (QString ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  options->vicinityRadius = settings.value (QString ("Vicinity Radius"), options->vicinityRadius).toDouble ();
  options->vicinityHeight = settings.value (QString ("Vicinity Height"), options->vicinityHeight).toDouble ();
  options->hpRadius = settings.value (QString ("HP Radius"), options->hpRadius).toDouble ();
  options->hpHeight = settings.value (QString ("HP Height"), options->hpHeight).toDouble ();
  options->hpD_index[0] = settings.value (QString ("HP D_index lower"), options->hpD_index[0]).toInt ();
  options->hpD_index[1] = settings.value (QString ("HP D_index upper"), options->hpD_index[1]).toInt ();

  options->invalidate = settings.value (QString ("Invalidate flag"), options->invalidate).toBool ();

  options->input_dir = settings.value (QString ("input directory"), options->input_dir).toString ();
  options->area_dir = settings.value (QString ("area directory"), options->area_dir).toString ();
  options->exclude_dir = settings.value (QString ("exclude directory"), options->exclude_dir).toString ();

  options->window_width = settings.value (QString ("width"), options->window_width).toInt ();
  options->window_height = settings.value (QString ("height"), options->window_height).toInt ();
  options->window_x = settings.value (QString ("x position"), options->window_x).toInt ();
  options->window_y = settings.value (QString ("y position"), options->window_y).toInt ();

  settings.endGroup ();
}




//  Save the users defaults.

void czmilUrbanFilter::envout (OPTIONS *options)
{
  //  Get the INI file name

#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/czmilUrbanFilter.ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/czmilUrbanFilter.ini";
#endif

  QSettings settings (ini_file, QSettings::IniFormat);
  settings.beginGroup ("czmilUrbanFilter");


  settings.setValue (QString ("settings version"), settings_version);

  settings.setValue (QString ("Invalidate flag"), options->invalidate);

  settings.setValue (QString ("input directory"), options->input_dir);
  settings.setValue (QString ("area directory"), options->area_dir);
  settings.setValue (QString ("exclude directory"), options->exclude_dir);

  settings.setValue (QString ("width"), options->window_width);
  settings.setValue (QString ("height"), options->window_height);
  settings.setValue (QString ("x position"), options->window_x);
  settings.setValue (QString ("y position"), options->window_y);

  settings.endGroup ();


  //  Saving urban filter parameters in global file since these are shared with pfmEdit3D

#ifdef NVWIN3X
  QString ini_file2 = QString (getenv ("USERPROFILE")) + "/ABE.config/" + "globalABE.ini";
#else
  QString ini_file2 = QString (getenv ("HOME")) + "/ABE.config/" + "globalABE.ini";
#endif

  QSettings settings2 (ini_file2, QSettings::IniFormat);
  settings2.beginGroup ("globalABE");


  settings2.setValue (QString ("CZMIL urban vicinity radius"), options->vicinityRadius);
  settings2.setValue (QString ("CZMIL urban filter radius"), options->hpRadius);
  settings2.setValue (QString ("CZMIL urban vicinity height"), options->vicinityHeight);
  settings2.setValue (QString ("CZMIL urban filter height"), options->hpHeight);
  settings2.setValue (QString ("CZMIL urban filter lower D_index"), options->hpD_index[0]);
  settings2.setValue (QString ("CZMIL urban filter upper D_index"), options->hpD_index[1]);


  settings2.endGroup ();
}
