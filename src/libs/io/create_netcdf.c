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

#include <io.h>

short int create_netcdf(char *title, char *title_french, char *summary, char *summary_french,
                        char *keywords, char *processor, char *description, char *institution,
                        char *creator_email, char *creator_url, char *creator_name,
                        char *version, char *scenario, char *scenario_co2, char *model,
                        char *institution_model, char *country, char *member, char *downscaling_forcing,
                        char *contact_email, char *contact_name, char *other_contact_email, char *other_contact_name,
                        char *filename) {

  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  int istat;
  int ncoutid;
  char *tmpstr = NULL;

  (void) fprintf(stdout, "%s: Creating NetCDF file %s.\n", __FILE__, filename);

  tmpstr = (char *) malloc(5000 * sizeof(char));
  if (tmpstr == NULL) alloc_error(__FILE__, __LINE__);

  /* Open NetCDF file */
  istat = nc_create(filename, NC_CLOBBER, &ncoutid);  /* open NetCDF file */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Set global attributes */
  // global attributes:
  /*                :Conventions = "CF-1.0" ;
                    :Metadata_Conventions = "Unidata Dataset Discovery v1.0" ;
                    :title = "Downscaling data from CERFACS" ;
                    :title_french = "Donnees de desagregation produites par le CERFACS" ;
                    :summary_french = "Donnees de desagregation produites par le CERFACS" ;
                    :summary = "Donnees de desagregation produites par le CERFACS" ;
                    :keywords = "climat,scenarios,desagregation,downscaling,CERFACS" ;
                    :processor = "IDL/NCL scripts" ;
                    :description = "Downscaling data from CERFACS" ;
                    :cdm_datatype = "Grid" ;
                    :institution = "CERFACS" ;
                    :creator_email = "globc@cerfacs.fr" ;
                    :creator_url = "http://www.cerfacs.fr/globc/" ;
                    :creator_name = "Global Change Team" ;
                    :time_coverage_start = "2087-08-01T00:00:00Z" ;
                    :time_coverage_end = "2088-07-31T23:59:59Z" ;
                    :version = "1.0" ;
                    :scenario = "SRESA1B" ;
                    :scenario_co2 = "A1B" ;
                    :model = "Coupled Model 3" ;
                    :institution_model = "Centre National de Recherches Meteorologiques" ;
                    :country = "France" ;
                    :member = "1" ;
                    :downscaling_forcing = "SAFRAN 1970-2005" ;
                    :timestep = "daily" ;
                    :contact_email = "christian.page@cerfacs.fr" ;
                    :contact_name = "Christian PAGE" ;
                    :other_contact_email = "laurent.terray@cerfacs.fr" ;
                    :other_contact_name = "Laurent TERRAY" ;
                    :geospatial_lat_max = 51.1215157941104 ;
                    :geospatial_lat_min = 41.3181733938225 ;
                    :geospatial_lon_max = 10.7928123474121 ;
                    :geospatial_lon_min = -5.32915830612183 ; */

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

  /* Close the output netCDF file. */
  istat = ncclose(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  (void) free(tmpstr);

  return 0;
}
