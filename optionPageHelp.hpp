
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



QString hpRadiusValueText0 = 
  optionPage::tr ("Change the X/Y <b>Radius</b> (in meters) to be used to define topo points that are in the vicinity of "
                  "urban features.  Urban feature points are marked by pfmLoad as CZMIL_URBAN_MAJORITY if a majority of the 7 channels of a "
                  "topo shot had less than 4 valid returns.");

QString hpHeightValueText0 = 
  optionPage::tr ("Change the vertical <b>Height</b> (in meters) to be used to define topo points that are in the vicinity of "
                  "urban features.  Urban feature points were marked by pfmLoad as CZMIL_URBAN_MAJORITY if a majority of the 7 channels of a "
                  "topo shot had less than 4 valid returns.");

QString hpRadiusValueText1 = 
  optionPage::tr ("Change the X/Y <b>Radius</b> (in meters) to be used for the Hockey Puck filter.  "
                  "Increasing this value will increase the amount of filtering.");

QString hpHeightValueText1 = 
  optionPage::tr ("Change the vertical <b>Height</b> (in meters) to be used for the Hockey Puck filter.  "
                  "Increasing this value will decrease the amount of filtering.");

QString D_indexValueText0 = 
  optionPage::tr ("Change the D_index lower cutoff value for the Hockey Puck filter.  Points with a D_index value less than this value "
                  "will be invalidated (or marked as suspect) if they are within the Hockey Puck filter radius/height of a point marked as "
                  "CZMIL_URBAN_MAJORITY or CZMIL_URBAN_VICINITY which meets or exceeds the D_index upper cutoff value.  Increasing this "
                  "value will increase the amount of filtering.");

QString D_indexValueText1 = 
  optionPage::tr ("Change the D_index upper cutoff value for the Hockey Puck filter.  Points with a D_index value greater than or equal to "
                  "this value that are marked as CZMIL_URBAN_MAJORITY or CZMIL_URBAN_VICINITY will be used to invalidate (or mark as "
                  "suspect) points within the Hockey Puck filter radius/height of the point and which have a D-index value less than "
                  "the D_index lower cutoff value.  Increasing this value will decrease the amount of filtering.");

QString invText = 
  optionPage::tr ("Check this box if you wish to mark filtered data points as PFM_FILTER_INVAL instead of PFM_SUSPECT.");
