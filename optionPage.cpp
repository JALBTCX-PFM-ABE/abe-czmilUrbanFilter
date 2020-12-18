
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



#include "optionPage.hpp"
#include "optionPageHelp.hpp"

optionPage::optionPage (QWidget *parent, OPTIONS *op):
  QWizardPage (parent)
{
  options = op;


  setTitle (tr ("Filter options"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/czmilUrbanFilterWatermark.png"));


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);
  QHBoxLayout *hbox = new QHBoxLayout;
  vbox->addLayout (hbox);


  lbox[0] = new QGroupBox (tr ("Urban filter vicinity options"), this);
  lboxLayout[0] = new QHBoxLayout;
  lbox[0]->setLayout (lboxLayout[0]);


  rbox[0] = new QGroupBox (tr ("Radius"), this);
  rboxLayout[0] = new QHBoxLayout;
  rbox[0]->setLayout (rboxLayout[0]);
  hpRadiusValue[0] = new QLineEdit ("1.0", rbox[0]);
  hpRadiusValue[0]->setAlignment (Qt::AlignCenter);
  QString rad;
  rad.sprintf ("%0.2f", options->vicinityRadius);
  hpRadiusValue[0]->setText (rad);
  hpRadiusValue[0]->setToolTip (tr ("Filter radius value"));
  hpRadiusValue[0]->setWhatsThis (hpRadiusValueText0);
  rboxLayout[0]->addWidget (hpRadiusValue[0]);
  lboxLayout[0]->addWidget (rbox[0]);

  htbox[0] = new QGroupBox (tr ("Height"), this);
  htboxLayout[0] = new QHBoxLayout;
  htbox[0]->setLayout (htboxLayout[0]);
  hpHeightValue[0] = new QLineEdit ("1.0", htbox[0]);
  hpHeightValue[0]->setAlignment (Qt::AlignCenter);
  QString ht;
  ht.sprintf ("%0.2f", options->vicinityHeight);
  hpHeightValue[0]->setText (ht);
  hpHeightValue[0]->setToolTip (tr ("Filter height value"));
  hpHeightValue[0]->setWhatsThis (hpHeightValueText0);
  htboxLayout[0]->addWidget (hpHeightValue[0]);
  lboxLayout[0]->addWidget (htbox[0]);


  vbox->addWidget (lbox[0]);


  lbox[1] = new QGroupBox (tr ("Urban filter D_index options"), this);
  lboxLayout[1] = new QHBoxLayout;
  lbox[1]->setLayout (lboxLayout[1]);


  rbox[1] = new QGroupBox (tr ("Radius"), this);
  rboxLayout[1] = new QHBoxLayout;
  rbox[1]->setLayout (rboxLayout[1]);
  hpRadiusValue[1] = new QLineEdit ("1.0", rbox[1]);
  hpRadiusValue[1]->setAlignment (Qt::AlignCenter);
  rad.sprintf ("%0.2f", options->hpRadius);
  hpRadiusValue[1]->setText (rad);
  hpRadiusValue[1]->setToolTip (tr ("Filter radius value"));
  hpRadiusValue[1]->setWhatsThis (hpRadiusValueText1);
  rboxLayout[1]->addWidget (hpRadiusValue[1]);
  lboxLayout[1]->addWidget (rbox[1]);

  htbox[1] = new QGroupBox (tr ("Height"), this);
  htboxLayout[1] = new QHBoxLayout;
  htbox[1]->setLayout (htboxLayout[1]);
  hpHeightValue[1] = new QLineEdit ("1.0", htbox[1]);
  hpHeightValue[1]->setAlignment (Qt::AlignCenter);
  ht.sprintf ("%0.2f", options->hpHeight);
  hpHeightValue[1]->setText (ht);
  hpHeightValue[1]->setToolTip (tr ("Filter height value"));
  hpHeightValue[1]->setWhatsThis (hpHeightValueText1);
  htboxLayout[1]->addWidget (hpHeightValue[1]);
  lboxLayout[1]->addWidget (htbox[1]);

  dibox[0] = new QGroupBox (tr ("D_index lower cutoff value"), this);
  diboxLayout[0] = new QHBoxLayout;
  dibox[0]->setLayout (diboxLayout[0]);
  D_indexValue[0] = new QLineEdit (" 000 ", dibox[0]);
  D_indexValue[0]->setAlignment (Qt::AlignCenter);
  ht.sprintf ("%d", options->hpD_index[0]);
  D_indexValue[0]->setText (ht);
  D_indexValue[0]->setToolTip (tr ("D_index lower cutoff value"));
  D_indexValue[0]->setWhatsThis (D_indexValueText0);
  diboxLayout[0]->addWidget (D_indexValue[0]);
  lboxLayout[1]->addWidget (dibox[0]);


  dibox[1] = new QGroupBox (tr ("D_index upper cutoff value"), this);
  diboxLayout[1] = new QHBoxLayout;
  dibox[1]->setLayout (diboxLayout[1]);
  D_indexValue[1] = new QLineEdit (" 000 ", dibox[1]);
  D_indexValue[1]->setAlignment (Qt::AlignCenter);
  ht.sprintf ("%d", options->hpD_index[1]);
  D_indexValue[1]->setText (ht);
  D_indexValue[1]->setToolTip (tr ("D_index upper cutoff value"));
  D_indexValue[1]->setWhatsThis (D_indexValueText1);
  diboxLayout[1]->addWidget (D_indexValue[1]);
  lboxLayout[1]->addWidget (dibox[1]);


  vbox->addWidget (lbox[1]);


  QGroupBox *invBox = new QGroupBox (tr ("Invalidate"), this);
  QHBoxLayout *invBoxLayout = new QHBoxLayout;
  invBox->setLayout (invBoxLayout);


  inv = new QCheckBox (invBox);
  inv->setToolTip (tr ("Check this box if you want to invalidate points instead of marking as suspect"));
  inv->setWhatsThis (invText);
  connect (inv, SIGNAL (stateChanged (int)), this, SLOT (slotInvClicked (int)));
  invBoxLayout->addWidget (inv);
  inv->setChecked (options->invalidate);


  vbox->addWidget (invBox);


  registerField ("hpRadiusValue0", hpRadiusValue[0]);
  registerField ("hpHeightValue0", hpHeightValue[0]);
  registerField ("hpRadiusValue1", hpRadiusValue[1]);
  registerField ("hpHeightValue1", hpHeightValue[1]);
  registerField ("D_indexValue0", D_indexValue[0]);
  registerField ("D_indexValue1", D_indexValue[1]);
}



void 
optionPage::setFileCenter (QString file)
{
  int32_t pfm_handle;
  PFM_OPEN_ARGS open_args;

  strcpy (open_args.list_path, file.toLatin1 ());

  open_args.checkpoint = 0;
  if ((pfm_handle = open_existing_pfm_file (&open_args)) < 0) pfm_error_exit (pfm_error);

  mbr_center.y = open_args.head.mbr.min_y + (open_args.head.mbr.max_y - open_args.head.mbr.min_y) / 2.0;
  mbr_center.x = open_args.head.mbr.min_x + (open_args.head.mbr.max_x - open_args.head.mbr.min_x) / 2.0;

  close_pfm_file (pfm_handle);
}



bool 
optionPage::validatePage ()
{
  return (true);
}



void
optionPage::slotInvClicked (int state)
{
  if (!state)
    {
      options->invalidate = NVFalse;
    }
  else
    {
      options->invalidate = NVTrue;
    }
}



void
optionPage::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}
