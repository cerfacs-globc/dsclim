/* ***************************************************** */
/* create_netcdf Create a new NetCDF file with global    */
/*               CF_1.0 attributes.                      */
/* create_netcdf.c                                       */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/* Date of creation: oct 2008                            */
/* Last date of modification: oct 2008                   */
/* ***************************************************** */
/* Original version: 1.0                                 */
/* Current revision:                                     */
/* ***************************************************** */
/* Revisions                                             */
/* ***************************************************** */
/*! \file create_netcdf.c
    \brief Create a new NetCDF file with global CF-1.0 attributes.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2010)

christian.page@cerfacs.fr

This software is a computer program whose purpose is to downscale climate
scenarios using a statistical methodology based on weather regimes.

This software is governed by the CeCILL license under French law and
abiding by the rules of distribution of free software. You can use, 
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty and the software's author, the holder of the
economic rights, and the successive licensors have only limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading, using, modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean that it is complicated to manipulate, and that also
therefore means that it is reserved for developers and experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and, more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.

LICENSE END */


#include <io.h>

/** Create a new NetCDF file with global CF-1.0 attributes. */
int
create_netcdf(char *title, char *title_french, char *summary, char *summary_french,
              char *keywords, char *processor, char *software, char *description, char *institution,
              char *creator_email, char *creator_url, char *creator_name,
              char *version, char *scenario, char *scenario_co2, char *model,
              char *institution_model, char *country, char *member, char *downscaling_forcing,
              char *contact_email, char *contact_name, char *other_contact_email, char *other_contact_name,
              char *filename, int outinfo, int format, int compression) {
  /**
     @param[in]  title                Title (english)
     @param[in]  title_french         Title (french)
     @param[in]  summary              Summary (english)
     @param[in]  summary_french       Summary *french)
     @param[in]  keywords             Keyword
     @param[in]  processor            Program, processor which have generated the data
     @param[in]  software             Software and version which have generated the data
     @param[in]  description          Main description of the data
     @param[in]  institution          Institution which generated the data
     @param[in]  creator_email        Contact email of the creator of the data
     @param[in]  creator_url          Website creator of the data
     @param[in]  creator_name         Name of the creator of the data
     @param[in]  version              Version of the data
     @param[in]  scenario             Climate scenario
     @param[in]  scenario_co2         CO2 scenario
     @param[in]  model                Numerical model used
     @param[in]  institution_model    Institution who developed the numerical model used
     @param[in]  country              Country of the institution who developed the numerical model used
     @param[in]  member               Member in the case of multi-member model configurations
     @param[in]  downscaling_forcing  Observations database used when downscaling
     @param[in]  contact_email        Contact email
     @param[in]  contact_name         Contact name
     @param[in]  other_contact_email  Other contact email
     @param[in]  other_contact_name   Other contact name
     @param[in]  filename             NetCDF output filename
     @param[in]  outinfo              TRUE if we want information output, FALSE if not
     @param[in]  format               File format version for NetCDF
     @param[in]  compression          Compression flag for NetCDF-4 file format
     
     \return           Status.
  */

  int istat = 0; /* Diagnostic status */
  int ncoutid; /* NetCDF output file handle ID */
  char *tmpstr = NULL; /* Temporary string */

  if (outinfo == TRUE)
    (void) fprintf(stdout, "%s: Creating NetCDF file %s\n", __FILE__, filename);

  /* Allocate memory */
  tmpstr = (char *) malloc(MAXPATH * sizeof(char));
  if (tmpstr == NULL) alloc_error(__FILE__, __LINE__);

  /* Open NetCDF file for writing, overwrite and truncate existing file if any */
  if (format == 4 && compression == TRUE)
#ifdef NC_NETCDF4
    istat = nc_create(filename, NC_CLOBBER | NC_NETCDF4 | NC_CLASSIC_MODEL, &ncoutid);
#else
    istat = nc_create(filename, NC_CLOBBER, &ncoutid);
#endif
  else
#ifdef NC_NETCDF4
    if (format == 4)
      istat = nc_create(filename, NC_CLOBBER | NC_NETCDF4 | NC_CLASSIC_MODEL, &ncoutid);
    else
      istat = nc_create(filename, NC_CLOBBER, &ncoutid);
#else
    istat = nc_create(filename, NC_CLOBBER, &ncoutid);
#endif
    //    istat = nc_create(filename, NC_CLOBBER, &ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Set global attributes */
  (void) strcpy(tmpstr, "CF-1.0");
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "Conventions", strlen(tmpstr), tmpstr);
  (void) strcpy(tmpstr, "Unidata Dataset Discovery v1.0");
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "Metadata_Conventions", strlen(tmpstr), tmpstr);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "title", strlen(title), title);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "title_french", strlen(title_french), title_french);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "summary", strlen(summary), summary);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "summary_french", strlen(summary_french), summary_french);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "keywords", strlen(keywords), keywords);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "processor", strlen(processor), processor);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "software", strlen(software), software);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "description", strlen(description), description);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "institution", strlen(institution), institution);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "creator_email", strlen(creator_email), creator_email);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "creator_url", strlen(creator_url), creator_url);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "creator_name", strlen(creator_name), creator_name);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "version", strlen(version), version);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "scenario", strlen(scenario), scenario);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "scenario_co2", strlen(scenario_co2), scenario_co2);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "model", strlen(model), model);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "institution_model", strlen(institution_model), institution_model);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "country", strlen(country), country);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "member", strlen(member), member);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "downscaling_forcing", strlen(downscaling_forcing), downscaling_forcing);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "contact_email", strlen(contact_email), contact_email);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "contact_name", strlen(contact_name), contact_name);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "other_contact_email", strlen(other_contact_email), other_contact_email);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "other_contact_name", strlen(other_contact_name), other_contact_name);

  /* End definition mode */
  istat = nc_enddef(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Close the output netCDF file */
  istat = ncclose(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Free memory */
  (void) free(tmpstr);

  /* Success status */
  return 0;
}
