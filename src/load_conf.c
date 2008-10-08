/* ***************************************************** */
/* Read and set variables from XML configuration file.   */
/* load_conf.c                                           */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file load_conf.c
    \brief Read and set variables from XML configuration file.
*/

#include <libs/xml_utils/xml_utils.h>
#include <dsclim.h>

short int load_conf(data_struct *data, char *fileconf) {

  xmlConfig_t *conf;
  char setting_name[1000];
  xmlChar *val;
  int i;
  char *path = NULL;

  (void) fprintf(stdout, "%s: *** Current Configuration ***\n\n", __FILE__);

  (void) strcpy(setting_name, "setting");
  
  /* Load XML configuration file into memory */
  conf = xml_load_config(fileconf);
  if (conf == NULL) return -1;

  data->conf = (conf_struct *) malloc(sizeof(conf_struct));
  if (data->conf == NULL) alloc_error(__FILE__, __LINE__);
  data->conf->proj = (proj_struct *) malloc(sizeof(proj_struct));
  if (data->conf->proj == NULL) alloc_error(__FILE__, __LINE__);
  data->info = (info_struct *) malloc(sizeof(info_struct));
  if (data->info == NULL) alloc_error(__FILE__, __LINE__);
  data->field = (field_struct *) malloc(sizeof(field_struct));
  if (data->field == NULL) alloc_error(__FILE__, __LINE__);

  /* Get needed settings */
  /* Set default value if not in configuration file */
  path = (char *) malloc(5000 * sizeof(char));
  if (path == NULL) alloc_error(__FILE__, __LINE__);

  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "debug");
  val = xml_get_setting(conf, path);
  if ( !xmlStrcmp(val, "On") )
    data->conf->debug = 1;
  else
    data->conf->debug = 0;
  (void) fprintf(stdout, "%s: debug = %d\n", __FILE__, data->conf->debug);

  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "clim_filter_width");
  val = xml_get_setting(conf, path);
  data->conf->clim_filter_width = (int) xmlXPathCastStringToNumber(val);
  if ( data->conf->clim_filter_width < 4 || data->conf->clim_filter_width > 365 )
    data->conf->clim_filter_width = 60;
  (void) fprintf(stdout, "%s: clim_filter_width = %d\n", __FILE__, data->conf->clim_filter_width);

  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "clim_filter_type");
  val = xml_get_setting(conf, path);
  if ( !xmlStrcmp(val, "hanning") )
    data->conf->clim_filter_type = strdup("hanning");
  else {
    (void) fprintf(stderr, "%s: Invalid clim_filter_type value %s in configuration file. Aborting.", __FILE__, val);
    (void) abort();
  }
  (void) fprintf(stdout, "%s: clim_filter_type = %s\n", __FILE__, data->conf->clim_filter_type);

  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "base_time_units");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->time_units = strdup(val);
  else
    data->conf->time_units = strdup("days since 1900-01-01 12:00:00");
  (void) fprintf(stdout, "%s: base_time_units = %s\n", __FILE__, data->conf->time_units);

  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "base_calendar_type");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->cal_type = strdup(val);
  else
    data->conf->cal_type = strdup("gregorian");
  (void) fprintf(stdout, "%s: base_calendar_type = %s\n", __FILE__, data->conf->cal_type);

  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "longitude_name");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->lonname = strdup(val);
  else
    data->conf->lonname = strdup("lon");  
  (void) fprintf(stdout, "%s: longitude_name = %s\n", __FILE__, data->conf->lonname);

  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "latitude_name");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->latname = strdup(val);
  else
    data->conf->latname = strdup("lat");
  (void) fprintf(stdout, "%s: latitude_name = %s\n", __FILE__, data->conf->latname);

  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "longitude_name_eof");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->lonname_eof = strdup(val);
  else
    data->conf->lonname_eof = strdup("lon");  
  (void) fprintf(stdout, "%s: longitude_name_eof = %s\n", __FILE__, data->conf->lonname_eof);

  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "latitude_name_eof");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->latname_eof = strdup(val);
  else
    data->conf->latname_eof = strdup("lat");
  (void) fprintf(stdout, "%s: latitude_name_eof = %s\n", __FILE__, data->conf->latname_eof);

  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "eof_name");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->eofname = strdup(val);
  else
    data->conf->eofname = strdup("pc");
  (void) fprintf(stdout, "%s: eof_name = %s\n", __FILE__, data->conf->eofname);

  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "time_name");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->timename = strdup(val);
  else
    data->conf->timename = strdup("time");
  (void) fprintf(stdout, "%s: time_name = %s\n", __FILE__, data->conf->timename);


  /*** Output projection info ***/
  (void) sprintf(path, "/configuration/%s[@name=\"projection\"]/%s", "setting", "name");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->proj->name = strdup(val);
  else
    data->conf->proj->name = strdup("Lambert_Conformal");
  (void) fprintf(stdout, "%s: output projection name = %s\n", __FILE__, data->conf->proj->name);

  (void) sprintf(path, "/configuration/%s[@name=\"projection\"]/%s", "setting", "coordinates");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->proj->coords = strdup(val);
  else
    data->conf->proj->coords = strdup("2D");
  (void) fprintf(stdout, "%s: output projection coords = %s\n", __FILE__, data->conf->proj->coords);

  (void) sprintf(path, "/configuration/%s[@name=\"projection\"]/%s", "setting", "grid_mapping_name");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->proj->grid_mapping_name = strdup(val);
  else
    data->conf->proj->grid_mapping_name = strdup("lambert_conformal_conic");
  (void) fprintf(stdout, "%s: output projection grid_mapping_name = %s\n", __FILE__, data->conf->proj->grid_mapping_name);

  (void) sprintf(path, "/configuration/%s[@name=\"projection\"]/%s", "setting", "latin1");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    (void) sscanf(val, "%lf", &(data->conf->proj->latin1));
  else
    data->conf->proj->latin1 = 45.89892;
  (void) fprintf(stdout, "%s: output projection latin1 = %lf\n", __FILE__, data->conf->proj->latin1);

  (void) sprintf(path, "/configuration/%s[@name=\"projection\"]/%s", "setting", "latin2");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    (void) sscanf(val, "%lf", &(data->conf->proj->latin2));
  else
    data->conf->proj->latin2 = 47.69601;
  (void) fprintf(stdout, "%s: output projection latin2 = %lf\n", __FILE__, data->conf->proj->latin2);

  (void) sprintf(path, "/configuration/%s[@name=\"projection\"]/%s", "setting", "lonc");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    (void) sscanf(val, "%lf", &(data->conf->proj->lonc));
  else
    data->conf->proj->lonc = 2.337229;
  (void) fprintf(stdout, "%s: output projection lonc = %lf\n", __FILE__, data->conf->proj->lonc);

  (void) sprintf(path, "/configuration/%s[@name=\"projection\"]/%s", "setting", "lat0");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    (void) sscanf(val, "%lf", &(data->conf->proj->lat0));
  else
    data->conf->proj->lat0 = 46.8;
  (void) fprintf(stdout, "%s: output projection lat0 = %lf\n", __FILE__, data->conf->proj->lat0);

  (void) sprintf(path, "/configuration/%s[@name=\"projection\"]/%s", "setting", "false_easting");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    (void) sscanf(val, "%lf", &(data->conf->proj->false_easting));
  else
    data->conf->proj->false_easting = 600000.0;
  (void) fprintf(stdout, "%s: output projection false_easting = %lf\n", __FILE__, data->conf->proj->false_easting);

  (void) sprintf(path, "/configuration/%s[@name=\"projection\"]/%s", "setting", "false_northing");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    (void) sscanf(val, "%lf", &(data->conf->proj->false_northing));
  else
    data->conf->proj->false_northing = 2200000.0;
  (void) fprintf(stdout, "%s: output projection false_northing = %lf\n", __FILE__, data->conf->proj->false_northing);

  /* Large-scale domain */
  (void) sprintf(path, "/configuration/%s/%s[@type=\"%s\"]", "setting", "longitude", "min");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->longitude_min = xmlXPathCastStringToNumber(val);
  else
    data->conf->longitude_min = -15.0;
  (void) fprintf(stdout, "%s: Large-scale domain longitude min = %lf\n", __FILE__, data->conf->longitude_min);

  (void) sprintf(path, "/configuration/%s/%s[@type=\"%s\"]", "setting", "longitude", "max");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->longitude_max = xmlXPathCastStringToNumber(val);
  else
    data->conf->longitude_max = 20.0;
  (void) fprintf(stdout, "%s: Large-scale domain longitude max = %lf\n", __FILE__, data->conf->longitude_max);

  (void) sprintf(path, "/configuration/%s/%s[@type=\"%s\"]", "setting", "latitude", "min");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->latitude_min = xmlXPathCastStringToNumber(val);
  else
    data->conf->latitude_min = 35.0;
  (void) fprintf(stdout, "%s: Large-scale domain latitude min = %lf\n", __FILE__, data->conf->latitude_min);

  (void) sprintf(path, "/configuration/%s/%s[@type=\"%s\"]", "setting", "latitude", "max");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->latitude_max = xmlXPathCastStringToNumber(val);
  else
    data->conf->latitude_max = 60.0;
  (void) fprintf(stdout, "%s: Large-scale domain latitude max = %lf\n", __FILE__, data->conf->latitude_max);


  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "number_of_large_scale_fields");
  val = xml_get_setting(conf, path);
  if (val != NULL) {

    data->field->n_ls = (int) strtol(val, (char **)NULL, 10);

    data->field->nomvar_ls = (char **) malloc(data->field->n_ls * sizeof(char *));
    if (data->field->nomvar_ls == NULL) alloc_error(__FILE__, __LINE__);
    data->field->filename_ls = (char **) malloc(data->field->n_ls * sizeof(char *));
    if (data->field->filename_ls == NULL) alloc_error(__FILE__, __LINE__);
    data->field->field_ls = (double **) malloc(data->field->n_ls * sizeof(double *));
    if (data->field->field_ls == NULL) alloc_error(__FILE__, __LINE__);

    data->field->clim_provided = (short int *) malloc(data->field->n_ls * sizeof(short int));
    if (data->field->clim_provided == NULL) alloc_error(__FILE__, __LINE__);
    data->field->clim_save = (short int *) malloc(data->field->n_ls * sizeof(short int));
    if (data->field->clim_save == NULL) alloc_error(__FILE__, __LINE__);
    data->field->clim_nomvar_ls = (char **) malloc(data->field->n_ls * sizeof(char *));
    if (data->field->clim_nomvar_ls == NULL) alloc_error(__FILE__, __LINE__);
    data->field->clim_filein_ls = (char **) malloc(data->field->n_ls * sizeof(char *));
    if (data->field->clim_filein_ls == NULL) alloc_error(__FILE__, __LINE__);
    data->field->clim_fileout_ls = (char **) malloc(data->field->n_ls * sizeof(char *));
    if (data->field->clim_fileout_ls == NULL) alloc_error(__FILE__, __LINE__);

    data->field->eof_ls = (double **) malloc(data->field->n_ls * sizeof(double *));
    if (data->field->eof_ls == NULL) alloc_error(__FILE__, __LINE__);
    data->field->sing_ls = (double **) malloc(data->field->n_ls * sizeof(double *));
    if (data->field->sing_ls == NULL) alloc_error(__FILE__, __LINE__);

    for (i=0; i<data->field->n_ls; i++) {
      data->field->field_ls[i] = NULL;
      data->field->eof_ls[i] = NULL;
      data->field->sing_ls[i] = NULL;
    }

    data->field->eof_provided = (short int *) malloc(data->field->n_ls * sizeof(short int));
    if (data->field->eof_provided == NULL) alloc_error(__FILE__, __LINE__);
    data->field->eof_save = (short int *) malloc(data->field->n_ls * sizeof(short int));
    if (data->field->eof_save == NULL) alloc_error(__FILE__, __LINE__);
    data->field->eof_nomvar_ls = (char **) malloc(data->field->n_ls * sizeof(char *));
    if (data->field->eof_nomvar_ls == NULL) alloc_error(__FILE__, __LINE__);
    data->field->sing_nomvar_ls = (char **) malloc(data->field->n_ls * sizeof(char *));
    if (data->field->sing_nomvar_ls == NULL) alloc_error(__FILE__, __LINE__);
    data->field->eof_filein_ls = (char **) malloc(data->field->n_ls * sizeof(char *));
    if (data->field->eof_filein_ls == NULL) alloc_error(__FILE__, __LINE__);
    data->field->eof_fileout_ls = (char **) malloc(data->field->n_ls * sizeof(char *));
    if (data->field->eof_fileout_ls == NULL) alloc_error(__FILE__, __LINE__);

    data->field->proj = (proj_struct *) malloc(data->field->n_ls * sizeof(proj_struct));
    if (data->field->proj == NULL) alloc_error(__FILE__, __LINE__);
    data->field->info = (info_struct *) malloc(data->field->n_ls * sizeof(info_struct));
    if (data->field->info == NULL) alloc_error(__FILE__, __LINE__);
    data->field->info_field = (info_field_struct *) malloc(data->field->n_ls * sizeof(info_field_struct));
    if (data->field->info_field == NULL) alloc_error(__FILE__, __LINE__);
  }
  else
    data->field->n_ls = 0;

  if (data->field->n_ls > 0) {
    (void) fprintf(stdout, "%s: number_of_large_scale_fields = %d\n", __FILE__, data->field->n_ls);
    for (i=0; i<data->field->n_ls; i++) {
      
      (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "name", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->field->nomvar_ls[i] = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
        if (data->field->nomvar_ls[i] == NULL) alloc_error(__FILE__, __LINE__);
        (void) strcpy(data->field->nomvar_ls[i], BAD_CAST val);
      }
      else {
        (void) fprintf(stderr, "%s: Missing name setting. Aborting.\n", __FILE__);
        return -1;
      }

      (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "filename", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->field->filename_ls[i] = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
        if (data->field->filename_ls[i] == NULL) alloc_error(__FILE__, __LINE__);
        (void) strcpy(data->field->filename_ls[i], BAD_CAST val);
        (void) fprintf(stdout, "%s: Large-scale field #%d: name = %s filename = %s\n",
                       __FILE__, i, data->field->nomvar_ls[i], data->field->filename_ls[i]);
      }
      else {
        (void) fprintf(stderr, "%s: Missing filename setting. Aborting.\n", __FILE__);
        return -1;
      }

      /* Fallback projection type */
      (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "projection", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->field->proj[i].name = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
        if (data->field->proj[i].name == NULL) alloc_error(__FILE__, __LINE__);
        (void) strcpy(data->field->proj[i].name, BAD_CAST val);
        (void) fprintf(stdout, "%s: Large-scale field #%d: name = %s projection = %s\n",
                       __FILE__, i, data->field->nomvar_ls[i], data->field->proj[i].name);
      }
      else
        data->field->proj[i].name = strdup("unknown");

      /* Fallback coordinate system dimensions */
      (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "coordinates", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->field->proj[i].coords = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
        if (data->field->proj[i].coords == NULL) alloc_error(__FILE__, __LINE__);
        (void) strcpy(data->field->proj[i].coords, BAD_CAST val);
        (void) fprintf(stdout, "%s: Large-scale field #%d: name = %s coordinates = %s\n",
                       __FILE__, i, data->field->nomvar_ls[i], data->field->proj[i].coords);
      }
      else
        data->field->proj[i].coords = strdup("2D");

      /** Climatology values **/

      (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "clim_provided", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        if ( !xmlStrcmp(val, "1") )
          data->field->clim_provided[i] = 1;
        else
          data->field->clim_provided[i] = 0;
        (void) fprintf(stdout, "%s: clim_provided #%d = %d\n", __FILE__, i+1, data->field->clim_provided[i]);

        if (data->field->clim_provided[i] == 1) {

          (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "clim_openfilename", i+1);
          val = xml_get_setting(conf, path);
          if (val != NULL) {
            data->field->clim_filein_ls[i] = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
            if (data->field->clim_filein_ls[i] == NULL) alloc_error(__FILE__, __LINE__);
            (void) strcpy(data->field->clim_filein_ls[i], BAD_CAST val);
            (void) fprintf(stdout, "%s:  Climatology input filename #%d = %s\n", __FILE__, i+1, data->field->clim_filein_ls[i]);
          }
          else {
            (void) fprintf(stderr, "%s: Missing clim_openfilename setting. Aborting.\n", __FILE__);
            return -1;
          }
        }
      }

      (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "clim_save", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        if ( !xmlStrcmp(val, "1") )
          data->field->clim_save[i] = 1;
        else
          data->field->clim_save[i] = 0;
        (void) fprintf(stdout, "%s: clim_save #%d = %d\n", __FILE__, i+1, data->field->clim_save[i]);

        if (data->field->clim_save[i] == 1) {

          (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "clim_savefilename", i+1);
          val = xml_get_setting(conf, path);
          if (val != NULL) {
            data->field->clim_fileout_ls[i] = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
            if (data->field->clim_fileout_ls[i] == NULL) alloc_error(__FILE__, __LINE__);
            (void) strcpy(data->field->clim_fileout_ls[i], BAD_CAST val);
            (void) fprintf(stdout, "%s:  Climatology output filename #%d = %s\n", __FILE__, i+1, data->field->clim_fileout_ls[i]);
          }
          else {
            (void) fprintf(stderr, "%s: Missing clim_savefilename setting. Aborting.\n", __FILE__);
            return -1;
          }
        }

        if (data->field->clim_save[i] == 1 || data->field->clim_provided[i] == 1) {
          (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "clim_name", i+1);
          val = xml_get_setting(conf, path);
          if (val != NULL) {
            data->field->clim_nomvar_ls[i] = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
            if (data->field->clim_nomvar_ls[i] == NULL) alloc_error(__FILE__, __LINE__);
            (void) strcpy(data->field->clim_nomvar_ls[i], BAD_CAST val);
            (void) fprintf(stdout, "%s:  Climatology variable name #%d = %s\n", __FILE__, i+1, data->field->clim_nomvar_ls[i]);
          }
          else {
            (void) fprintf(stderr, "%s: Missing clim_name setting. Aborting.\n", __FILE__);
            return -1;
          }
        }
      }

      /** EOF and Singular values **/

      (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "eof_provided", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        if ( !xmlStrcmp(val, "1") )
          data->field->eof_provided[i] = 1;
        else
          data->field->eof_provided[i] = 0;
        (void) fprintf(stdout, "%s: eof_provided #%d = %d\n", __FILE__, i+1, data->field->eof_provided[i]);

        if (data->field->eof_provided[i] == 1) {

          (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "eof_openfilename", i+1);
          val = xml_get_setting(conf, path);
          if (val != NULL) {
            data->field->eof_filein_ls[i] = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
            if (data->field->eof_filein_ls[i] == NULL) alloc_error(__FILE__, __LINE__);
            (void) strcpy(data->field->eof_filein_ls[i], BAD_CAST val);
            (void) fprintf(stdout, "%s: EOF/Singular values input filename #%d = %s\n", __FILE__, i+1, data->field->eof_filein_ls[i]);
          }
          else {
            (void) fprintf(stderr, "%s: Missing eof_openfilename setting. Aborting.\n", __FILE__);
            return -1;
          }
        }
      }

      (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "eof_save", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        if ( !xmlStrcmp(val, "1") )
          data->field->eof_save[i] = 1;
        else
          data->field->eof_save[i] = 0;
        (void) fprintf(stdout, "%s: eof_save #%d = %d\n", __FILE__, i+1, data->field->eof_save[i]);

        if (data->field->eof_save[i] == 1) {

          (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "eof_savefilename", i+1);
          val = xml_get_setting(conf, path);
          if (val != NULL) {
            data->field->eof_fileout_ls[i] = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
            if (data->field->eof_fileout_ls[i] == NULL) alloc_error(__FILE__, __LINE__);
            (void) strcpy(data->field->eof_fileout_ls[i], BAD_CAST val);
            (void) fprintf(stdout, "%s: EOF/Singular values output filename #%d = %s\n", __FILE__, i+1, data->field->eof_fileout_ls[i]);
          }
          else {
            (void) fprintf(stderr, "%s: Missing eof_savefilename setting. Aborting.\n", __FILE__);
            return -1;
          }
        }

        if (data->field->eof_save[i] == 1 || data->field->eof_provided[i] == 1) {
          (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "eof_name", i+1);
          val = xml_get_setting(conf, path);
          if (val != NULL) {
            data->field->eof_nomvar_ls[i] = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
            if (data->field->eof_nomvar_ls[i] == NULL) alloc_error(__FILE__, __LINE__);
            (void) strcpy(data->field->eof_nomvar_ls[i], BAD_CAST val);
            (void) fprintf(stdout, "%s: EOF variable name #%d = %s\n", __FILE__, i+1, data->field->eof_nomvar_ls[i]);
          }
          else {
            (void) fprintf(stderr, "%s: Missing eof_name setting. Aborting.\n", __FILE__);
            return -1;
          }
          (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "sing_name", i+1);
          val = xml_get_setting(conf, path);
          if (val != NULL) {
            data->field->sing_nomvar_ls[i] = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
            if (data->field->sing_nomvar_ls[i] == NULL) alloc_error(__FILE__, __LINE__);
            (void) strcpy(data->field->sing_nomvar_ls[i], BAD_CAST val);
            (void) fprintf(stdout, "%s: Singular values variable name #%d = %s\n", __FILE__, i+1, data->field->sing_nomvar_ls[i]);
          }
          else {
            (void) fprintf(stderr, "%s: Missing sing_name setting. Aborting.\n", __FILE__);
            return -1;
          }

          /* Fallback coordinate system dimensions */
          (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "eof_coordinates", i+1);
          val = xml_get_setting(conf, path);
          if (val != NULL) {
            data->field->proj[i].eof_coords = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
            if (data->field->proj[i].eof_coords == NULL) alloc_error(__FILE__, __LINE__);
            (void) strcpy(data->field->proj[i].eof_coords, BAD_CAST val);
            (void) fprintf(stdout, "%s: EOF variable #%d: name = %s coordinates = %s\n",
                           __FILE__, i, data->field->eof_nomvar_ls[i], data->field->proj[i].eof_coords);
          }
          else
            data->field->proj[i].eof_coords = strdup("2D");

        }
      }
    }
  }
  else {
    (void) fprintf(stderr, "%s: Invalid number_of_large_scale_fields value %s in configuration file. Aborting.", __FILE__, val);
    return -1;
  }
  

  /* Free memory */
  (void) xml_free_config(conf);
  (void) free(path);

  return 0;
}
