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

/** Read and set variables from XML configuration file. */
int load_conf(data_struct *data, char *fileconf) {
  /**
     @param[in]  data      MASTER data structure.
     @param[in]  fileconf  XML input filename
     
     \return           Status.
  */

  xmlConfig_t *conf; /* Pointer to XML Config */
  char setting_name[1000]; /* Setting name in XML file */
  xmlChar *val; /* Value in XML file */
  int i; /* Loop counter */
  int j; /* Loop counter */
  int cat; /* Loop counter for field category */
  char *path = NULL; /* XPath */

  char *token; /* Token for string decoding */
  char *saveptr; /* Pointer to save buffer data for thread-safe strtok use */
  char *catstr; /* Category string */
  char *catstrt; /* Category string */

  (void) fprintf(stdout, "%s: *** Current Configuration ***\n\n", __FILE__);

  (void) strcpy(setting_name, "setting");
  
  /* Load XML configuration file into memory */
  conf = xml_load_config(fileconf);
  if (conf == NULL) return -1;

  /* Allocate memory in main data structure */
  data->conf = (conf_struct *) malloc(sizeof(conf_struct));
  if (data->conf == NULL) alloc_error(__FILE__, __LINE__);
  data->conf->period_ctrl = (period_struct *) malloc(sizeof(period_struct));
  if (data->conf->period_ctrl == NULL) alloc_error(__FILE__, __LINE__);
  data->info = (info_struct *) malloc(sizeof(info_struct));
  if (data->info == NULL) alloc_error(__FILE__, __LINE__);
  data->info->title = (char *) NULL;
  data->learning = (learning_struct *) malloc(sizeof(learning_struct));
  if (data->learning == NULL) alloc_error(__FILE__, __LINE__);
  data->reg = (reg_struct *) malloc(sizeof(reg_struct));
  if (data->reg == NULL) alloc_error(__FILE__, __LINE__);

  data->field = (field_struct *) malloc(NCAT * sizeof(field_struct));
  if (data->field == NULL) alloc_error(__FILE__, __LINE__);

  /* Loop over field categories */
  /* Allocate memory in main data structure */
  for (i=0; i<NCAT; i++) {
    data->field[i].time_ls = (double *) malloc(sizeof(double));
    if (data->field[i].time_ls == NULL) alloc_error(__FILE__, __LINE__);
    data->field[i].time_s = (time_struct *) malloc(sizeof(time_struct));
    if (data->field[i].time_s == NULL) alloc_error(__FILE__, __LINE__);    

    data->field[i].lat_ls = NULL;
    data->field[i].lon_ls = NULL;

    data->field[i].lat_eof_ls = NULL;
    data->field[i].lon_eof_ls = NULL;
  }

  /*** Get needed settings ***/
  /* Set default value if not in configuration file */
  path = (char *) malloc(5000 * sizeof(char));
  if (path == NULL) alloc_error(__FILE__, __LINE__);

  /** debug **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "debug");
  val = xml_get_setting(conf, path);
  if ( !xmlStrcmp(val, (xmlChar *) "On") )
    data->conf->debug = 1;
  else
    data->conf->debug = 0;
  (void) fprintf(stdout, "%s: debug = %d\n", __FILE__, data->conf->debug);
  if (val != NULL)
    (void) xmlFree(val);

  /** clim_filter_width **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "clim_filter_width");
  val = xml_get_setting(conf, path);
  data->conf->clim_filter_width = (int) xmlXPathCastStringToNumber(val);
  if ( data->conf->clim_filter_width < 4 || data->conf->clim_filter_width > 365 )
    data->conf->clim_filter_width = 60;
  (void) fprintf(stdout, "%s: clim_filter_width = %d\n", __FILE__, data->conf->clim_filter_width);
  if (val != NULL)
    (void) xmlFree(val);

  /** clim_filter_type **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "clim_filter_type");
  val = xml_get_setting(conf, path);
  if ( !xmlStrcmp(val, (xmlChar *) "hanning") )
    data->conf->clim_filter_type = strdup("hanning");
  else {
    (void) fprintf(stderr, "%s: Invalid clim_filter_type value %s in configuration file. Aborting.\n", __FILE__, val);
    (void) abort();
  }
  (void) fprintf(stdout, "%s: clim_filter_type = %s\n", __FILE__, data->conf->clim_filter_type);
  if (val != NULL)
    (void) xmlFree(val);

  /** classif_type **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "classif_type");
  val = xml_get_setting(conf, path);
  if ( !xmlStrcmp(val, (xmlChar *) "euclidian") )
    data->conf->classif_type = strdup("euclidian");
  else {
    (void) fprintf(stderr, "%s: Invalid classif_type value %s in configuration file. Aborting.\n", __FILE__, val);
    (void) abort();
  }
  (void) fprintf(stdout, "%s: classif_type = %s\n", __FILE__, data->conf->classif_type);
  if (val != NULL)
    (void) xmlFree(val);

  /** base_time_units **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "base_time_units");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->time_units = strdup((char *) val);
  else
    data->conf->time_units = strdup("days since 1900-01-01 12:00:00");
  (void) fprintf(stdout, "%s: base_time_units = %s\n", __FILE__, data->conf->time_units);
  if (val != NULL)
    (void) xmlFree(val);

  /** base_calendar_type **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "base_calendar_type");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->cal_type = strdup((char *) val);
  else
    data->conf->cal_type = strdup("gregorian");
  (void) fprintf(stdout, "%s: base_calendar_type = %s\n", __FILE__, data->conf->cal_type);
  if (val != NULL)
    (void) xmlFree(val);

  /** longitude_name **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "longitude_name");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->lonname = strdup((char *) val);
  else
    data->conf->lonname = strdup("lon");  
  (void) fprintf(stdout, "%s: longitude_name = %s\n", __FILE__, data->conf->lonname);
  if (val != NULL)
    (void) xmlFree(val);

  /** latitude_name **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "latitude_name");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->latname = strdup((char *) val);
  else
    data->conf->latname = strdup("lat");
  (void) fprintf(stdout, "%s: latitude_name = %s\n", __FILE__, data->conf->latname);
  if (val != NULL)
    (void) xmlFree(val);

  /** coords **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "coordinates");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->coords = strdup((char *) val);
  else {
    (void) fprintf(stderr, "%s: Missing or invalid output coordinates setting. Aborting.\n", __FILE__);
    (void) xmlFree(val);
    return -1;
  }
  (void) fprintf(stdout, "%s: output coordinates = %s\n", __FILE__, data->conf->coords);
  if (val != NULL)
    (void) xmlFree(val);

  /** longitude_name_eof **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "longitude_name_eof");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->lonname_eof = strdup((char *) val);
  else
    data->conf->lonname_eof = strdup("lon");  
  (void) fprintf(stdout, "%s: longitude_name_eof = %s\n", __FILE__, data->conf->lonname_eof);
  if (val != NULL)
    (void) xmlFree(val);

  /** latitude_name_eof **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "latitude_name_eof");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->latname_eof = strdup((char *) val);
  else
    data->conf->latname_eof = strdup("lat");
  (void) fprintf(stdout, "%s: latitude_name_eof = %s\n", __FILE__, data->conf->latname_eof);
  if (val != NULL)
    (void) xmlFree(val);

  /** time_name **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "time_name");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->timename = strdup((char *) val);
  else
    data->conf->timename = strdup("time");
  (void) fprintf(stdout, "%s: time_name = %s\n", __FILE__, data->conf->timename);
  if (val != NULL)
    (void) xmlFree(val);

  /** eof_name **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "eof_name");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->eofname = strdup((char *) val);
  else
    data->conf->eofname = strdup("eof");
  (void) fprintf(stdout, "%s: eof_name = %s\n", __FILE__, data->conf->eofname);
  if (val != NULL)
    (void) xmlFree(val);

  /** pts_name **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "pts_name");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->ptsname = strdup((char *) val);
  else
    data->conf->ptsname = strdup("pts");
  (void) fprintf(stdout, "%s: pts_name = %s\n", __FILE__, data->conf->ptsname);
  if (val != NULL)
    (void) xmlFree(val);

  /** clust_name **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "clust_name");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->clustname = strdup((char *) val);
  else
    data->conf->clustname = strdup("clust");
  (void) fprintf(stdout, "%s: clust_name = %s\n", __FILE__, data->conf->clustname);
  if (val != NULL)
    (void) xmlFree(val);    

  /**** LARGE-SCALE DOMAIN CONFIGURATION ****/

  /** longitude min **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@type=\"%s\"]", "setting", "domain_large_scale", "longitude", "min");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->longitude_min = xmlXPathCastStringToNumber(val);
  else
    data->conf->longitude_min = -15.0;
  (void) fprintf(stdout, "%s: Large-scale domain longitude min = %lf\n", __FILE__, data->conf->longitude_min);
  if (val != NULL)
    (void) xmlFree(val);    

  /** longitude max **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@type=\"%s\"]", "setting", "domain_large_scale", "longitude", "max");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->longitude_max = xmlXPathCastStringToNumber(val);
  else
    data->conf->longitude_max = 20.0;
  (void) fprintf(stdout, "%s: Large-scale domain longitude max = %lf\n", __FILE__, data->conf->longitude_max);
  if (val != NULL)
    (void) xmlFree(val);    

  /** latitude min **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@type=\"%s\"]", "setting", "domain_large_scale", "latitude", "min");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->latitude_min = xmlXPathCastStringToNumber(val);
  else
    data->conf->latitude_min = 35.0;
  (void) fprintf(stdout, "%s: Large-scale domain latitude min = %lf\n", __FILE__, data->conf->latitude_min);
  if (val != NULL)
    (void) xmlFree(val);    

  /** latitude max **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@type=\"%s\"]", "setting", "domain_large_scale", "latitude", "max");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->latitude_max = xmlXPathCastStringToNumber(val);
  else
    data->conf->latitude_max = 60.0;
  (void) fprintf(stdout, "%s: Large-scale domain latitude max = %lf\n", __FILE__, data->conf->latitude_max);
  if (val != NULL)
    (void) xmlFree(val);    

  /**** OUTPUT CONFIGURATION ****/

  /** path **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "path");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->output_path = strdup((char *) val);
  else {
    (void) fprintf(stderr, "%s: Missing or invalid output path setting. Aborting.\n", __FILE__);
    return -1;
  }
  (void) fprintf(stdout, "%s: output path = %s\n", __FILE__, data->conf->output_path);
  if (val != NULL)
    (void) xmlFree(val);    

  /** month_begin **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "month_begin");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->output_month_begin = xmlXPathCastStringToNumber(val);
  else {
    (void) fprintf(stderr, "%s: Missing or invalid output month_begin setting. Aborting.\n", __FILE__);
    return -1;
  }
  (void) fprintf(stdout, "%s: output month_begin = %d\n", __FILE__, data->conf->output_month_begin);
  if (val != NULL)
    (void) xmlFree(val);    

  /**** OBSERVATION DATABASE CONFIGURATION ****/

  data->conf->obs_var = (var_struct *) malloc(sizeof(var_struct));
  if (data->conf->obs_var == NULL) alloc_error(__FILE__, __LINE__);
  data->conf->obs_var->proj = (proj_struct *) malloc(sizeof(proj_struct));
  if (data->conf->obs_var->proj == NULL) alloc_error(__FILE__, __LINE__);

  /** number_of_variables **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "observations", "number_of_variables");
  val = xml_get_setting(conf, path);
  if (val != NULL) {
    data->conf->obs_var->nobs_var = (int) xmlXPathCastStringToNumber(val);
    (void) xmlFree(val);
    (void) fprintf(stdout, "%s: observations: number_of_variables = %d\n", __FILE__, data->conf->obs_var->nobs_var);

    /** Allocate memory for variable informations **/

    data->conf->obs_var->acronym = (char **) malloc(data->conf->obs_var->nobs_var * sizeof(char *));
    if (data->conf->obs_var->acronym == NULL) alloc_error(__FILE__, __LINE__);
    data->conf->obs_var->netcdfname = (char **) malloc(data->conf->obs_var->nobs_var * sizeof(char *));
    if (data->conf->obs_var->netcdfname == NULL) alloc_error(__FILE__, __LINE__);
    data->conf->obs_var->name = (char **) malloc(data->conf->obs_var->nobs_var * sizeof(char *));
    if (data->conf->obs_var->name == NULL) alloc_error(__FILE__, __LINE__);
    data->conf->obs_var->factor = (double *) malloc(data->conf->obs_var->nobs_var * sizeof(double));
    if (data->conf->obs_var->factor == NULL) alloc_error(__FILE__, __LINE__);
    data->conf->obs_var->delta = (double *) malloc(data->conf->obs_var->nobs_var * sizeof(double));
    if (data->conf->obs_var->delta == NULL) alloc_error(__FILE__, __LINE__);

    /* Loop over observation variables */
    for (i=0; i<data->conf->obs_var->nobs_var; i++) {

      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s[@id=\"%d\"]/@%s", "setting", "observations", "variables", "name", i+1, "acronym");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->obs_var->acronym[i] = strdup((char *) val);
        (void) xmlFree(val);
      }
      else {
        (void) fprintf(stderr, "%s: Missing or invalid observation variable acronym setting. Aborting.\n", __FILE__);
        return -1;
      }

      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s[@id=\"%d\"]/@%s", "setting", "observations", "variables", "name", i+1, "netcdfname");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->obs_var->netcdfname[i] = strdup((char *) val);
        (void) xmlFree(val);
      }
      else {
        (void) fprintf(stderr, "%s: Missing or invalid observation variable netcdfname setting. Aborting.\n", __FILE__);
        return -1;
      }

      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s[@id=\"%d\"]", "setting", "observations", "variables", "name", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->obs_var->name[i] = strdup((char *) val);
        (void) xmlFree(val);
      }
      else {
        (void) fprintf(stderr, "%s: Missing or invalid observation variable name setting. Aborting.\n", __FILE__);
        return -1;
      }

      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s[@id=\"%d\"]/@%s", "setting", "observations", "variables", "name", i+1, "factor");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->obs_var->factor[i] = (double) xmlXPathCastStringToNumber(val);
        (void) xmlFree(val);
      }
      else {
        (void) fprintf(stderr, "%s: Missing or invalid observation variable factor setting. Aborting.\n", __FILE__);
        return -1;
      }

      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s[@id=\"%d\"]/@%s", "setting", "observations", "variables", "name", i+1, "delta");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->obs_var->delta[i] = (double) xmlXPathCastStringToNumber(val);
        (void) xmlFree(val);
      }
      else {
        (void) fprintf(stderr, "%s: Missing or invalid observation variable delta setting. Aborting.\n", __FILE__);
        return -1;
      }

      (void) printf("%s: Variable id=%d name=\"%s\" netcdfname=%s acronym=%s factor=%f delta=%f\n", __FILE__, i+1, data->conf->obs_var->name[i], data->conf->obs_var->netcdfname[i], data->conf->obs_var->acronym[i], data->conf->obs_var->factor[i], data->conf->obs_var->delta[i]);
    }
  }
  else {
    (void) fprintf(stderr, "%s: Invalid number_of_variables value %s in configuration file. Aborting.\n", __FILE__, val);
    return -1;
  }

  /** Data frequency **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "observations", "frequency");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->obs_var->frequency = strdup((char *) val);
  else {
    (void) fprintf(stderr, "%s: Missing or invalid observations data frequency setting. Aborting.\n", __FILE__);
    return -1;
  }
  if (val != NULL)
    (void) xmlFree(val);    

  /** template **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "observations", "template");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->obs_var->template = strdup((char *) val);
  else {
    (void) fprintf(stderr, "%s: Missing or invalid output template setting. Aborting.\n", __FILE__);
    return -1;
  }
  (void) fprintf(stdout, "%s: output template = %s\n", __FILE__, data->conf->obs_var->template);
  if (val != NULL)
    (void) xmlFree(val);    

  /** Number of digits for year in data filename **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "observations", "year_digits");
  val = xml_get_setting(conf, path);
  if (val != NULL) {
    data->conf->obs_var->year_digits = (int) xmlXPathCastStringToNumber(val);
    (void) xmlFree(val);
    if (data->conf->obs_var->year_digits != 2 && data->conf->obs_var->year_digits != 4) {
      (void) fprintf(stderr, "%s: Invalid observations data year_digits setting %d. Only values of 2 or 4 are valid. Aborting.\n",
                     __FILE__, data->conf->obs_var->year_digits);
      return -1;
    }
  }
  else {
    (void) fprintf(stderr, "%s: Missing or invalid observations data year_digits setting. Aborting.\n", __FILE__);
    return -1;
  }

  /** Data path **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "observations", "path");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->obs_var->path = strdup((char *) val);
  else {
    (void) fprintf(stderr, "%s: Missing or invalid observations data path setting. Aborting.\n", __FILE__);
    return -1;
  }
  if (val != NULL)
    (void) xmlFree(val);    

  /** month_begin **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "observations", "month_begin");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->obs_var->month_begin = xmlXPathCastStringToNumber(val);
  else {
    (void) fprintf(stderr, "%s: Missing or invalid observations data month_begin setting. Aborting.\n", __FILE__);
    return -1;
  }
  if (val != NULL)
    (void) xmlFree(val);    

  /** dimx_name **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "observations", "dimx_name");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->obs_var->dimxname = strdup((char *) val);
  else
    data->conf->obs_var->dimxname = strdup("x");  
  (void) fprintf(stdout, "%s: Observations dimx_name = %s\n", __FILE__, data->conf->obs_var->dimxname);
  if (val != NULL)
    (void) xmlFree(val);    

  /** dimy_name **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "observations", "dimy_name");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->obs_var->dimyname = strdup((char *) val);
  else
    data->conf->obs_var->dimyname = strdup("y");  
  (void) fprintf(stdout, "%s: Observations dimy_name = %s\n", __FILE__, data->conf->obs_var->dimyname);
  if (val != NULL)
    (void) xmlFree(val);    

  /** coords **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "observations", "dim_coordinates");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->obs_var->dimcoords = strdup((char *) val);
  else
    data->conf->obs_var->dimcoords = strdup("1D");
  (void) fprintf(stdout, "%s: Observations coords = %s\n", __FILE__, data->conf->obs_var->dimcoords);
  if (val != NULL)
    (void) xmlFree(val);    

  /** longitude_name **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "observations", "longitude_name");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->obs_var->lonname = strdup((char *) val);
  else
    data->conf->obs_var->lonname = strdup("lon");  
  (void) fprintf(stdout, "%s: Observations longitude_name = %s\n", __FILE__, data->conf->obs_var->lonname);
  if (val != NULL)
    (void) xmlFree(val);    

  /** latitude_name **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "observations", "latitude_name");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->obs_var->latname = strdup((char *) val);
  else
    data->conf->obs_var->latname = strdup("lat");
  (void) fprintf(stdout, "%s: Observations latitude_name = %s\n", __FILE__, data->conf->obs_var->latname);
  if (val != NULL)
    (void) xmlFree(val);    

  /** time_name **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "observations",  "time_name");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->obs_var->timename = strdup((char *) val);
  else
    data->conf->obs_var->timename = strdup("time");
  (void) fprintf(stdout, "%s: Observations time_name = %s\n", __FILE__, data->conf->obs_var->timename);
  if (val != NULL)
    (void) xmlFree(val);    

  /** coords **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "observations", "coordinates");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->obs_var->proj->coords = strdup((char *) val);
  else
    data->conf->obs_var->proj->coords = strdup("2D");
  (void) fprintf(stdout, "%s: Observations coords = %s\n", __FILE__, data->conf->obs_var->proj->coords);
  if (val != NULL)
    (void) xmlFree(val);    

  /**** CONTROL-RUN PERIOD CONFIGURATION ****/

  /** year_begin **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s", "setting", "period_ctrl", "period", "year_begin");
  val = xml_get_setting(conf, path);
  if (val != NULL) {
    data->conf->period_ctrl->year_begin = xmlXPathCastStringToNumber(val);
    (void) fprintf(stdout, "%s: period_ctrl year_begin = %d\n", __FILE__, data->conf->period_ctrl->year_begin);
    (void) xmlFree(val);
  }
  /** month_begin **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s", "setting", "period_ctrl", "period", "month_begin");
  val = xml_get_setting(conf, path);
  if (val != NULL) {
    data->conf->period_ctrl->month_begin = xmlXPathCastStringToNumber(val);
    (void) fprintf(stdout, "%s: period_ctrl month_begin = %d\n", __FILE__, data->conf->period_ctrl->month_begin);
    (void) xmlFree(val);
  }
  /** day_begin **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s", "setting", "period_ctrl", "period", "day_begin");
  val = xml_get_setting(conf, path);
  if (val != NULL) {
    data->conf->period_ctrl->day_begin = xmlXPathCastStringToNumber(val);
    (void) fprintf(stdout, "%s: period_ctrl day_begin = %d\n", __FILE__, data->conf->period_ctrl->day_begin);
    (void) xmlFree(val);
  }
  /** year_end **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s", "setting", "period_ctrl", "period", "year_end");
  val = xml_get_setting(conf, path);
  if (val != NULL) {
    data->conf->period_ctrl->year_end = xmlXPathCastStringToNumber(val);
    (void) fprintf(stdout, "%s: period_ctrl year_end = %d\n", __FILE__, data->conf->period_ctrl->year_end);
    (void) xmlFree(val);
  }
  /** month_end **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s", "setting", "period_ctrl", "period", "month_end");
  val = xml_get_setting(conf, path);
  if (val != NULL) {
    data->conf->period_ctrl->month_end = xmlXPathCastStringToNumber(val);
    (void) fprintf(stdout, "%s: period_ctrl month_end = %d\n", __FILE__, data->conf->period_ctrl->month_end);
    (void) xmlFree(val);
  }
  /** day_end **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s", "setting", "period_ctrl", "period", "day_end");
  val = xml_get_setting(conf, path);
  if (val != NULL) {
    data->conf->period_ctrl->day_end = xmlXPathCastStringToNumber(val);
    (void) fprintf(stdout, "%s: period_ctrl day_end = %d\n", __FILE__, data->conf->period_ctrl->day_end);
    (void) xmlFree(val);
  }
  /** downscale **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "period_ctrl", "downscale");
  val = xml_get_setting(conf, path);
  if (val != NULL) {
    data->conf->period_ctrl->downscale = xmlXPathCastStringToNumber(val);
    (void) fprintf(stdout, "%s: period_ctrl downscale = %d\n", __FILE__, data->conf->period_ctrl->downscale);
    (void) xmlFree(val);
  }

  /**** PERIODS CONFIGURATION ****/

  /** number_of_periods **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "number_of_periods");
  val = xml_get_setting(conf, path);
  if (val != NULL) {
    data->conf->nperiods = (int) xmlXPathCastStringToNumber(val);
    (void) xmlFree(val);
    (void) fprintf(stdout, "%s: number_of_periods = %d\n", __FILE__, data->conf->nperiods);
    data->conf->period = (period_struct *) malloc(data->conf->nperiods * sizeof(period_struct));
    if (data->conf->period == NULL) alloc_error(__FILE__, __LINE__);
    /* Loop over periods */
    for (i=0; i<data->conf->nperiods; i++) {
      /** year_begin **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s[@id=\"%d\"]", "setting", "periods", "period", "year_begin", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->period[i].year_begin = xmlXPathCastStringToNumber(val);
        (void) xmlFree(val);
        (void) fprintf(stdout, "%s: period #%d year_begin = %d\n", __FILE__, i+1, data->conf->period[i].year_begin);
      }
      /** month_begin **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s[@id=\"%d\"]", "setting", "periods", "period", "month_begin", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->period[i].month_begin = xmlXPathCastStringToNumber(val);
        (void) xmlFree(val);
        (void) fprintf(stdout, "%s: period #%d month_begin = %d\n", __FILE__, i+1, data->conf->period[i].month_begin);
      }
      /** day_begin **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s[@id=\"%d\"]", "setting", "periods", "period", "day_begin", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->period[i].day_begin = xmlXPathCastStringToNumber(val);
        (void) xmlFree(val);
        (void) fprintf(stdout, "%s: period #%d day_begin = %d\n", __FILE__, i+1, data->conf->period[i].day_begin);
      }
      /** year_end **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s[@id=\"%d\"]", "setting", "periods", "period", "year_end", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->period[i].year_end = xmlXPathCastStringToNumber(val);
        (void) xmlFree(val);
        (void) fprintf(stdout, "%s: period #%d year_end = %d\n", __FILE__, i+1, data->conf->period[i].year_end);
      }
      /** month_end **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s[@id=\"%d\"]", "setting", "periods", "period", "month_end", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->period[i].month_end = xmlXPathCastStringToNumber(val);
        (void) xmlFree(val);
        (void) fprintf(stdout, "%s: period #%d month_end = %d\n", __FILE__, i+1, data->conf->period[i].month_end);
      }
      /** day_end **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s[@id=\"%d\"]", "setting", "periods", "period", "day_end", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->period[i].day_end = xmlXPathCastStringToNumber(val);
        (void) xmlFree(val);
        (void) fprintf(stdout, "%s: period #%d day_end = %d\n", __FILE__, i+1, data->conf->period[i].day_end);
      }
    }
  }
  else
    data->conf->nperiods = 0;

  /**** LARGE-SCALE FIELDS CONFIGURATION ****/

  /** number_of_large_scale_fields **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "number_of_large_scale_fields");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->field[0].n_ls = (int) strtol((char *) val, (char **)NULL, 10);
  else
    data->field[0].n_ls = 0;
  if (val != NULL)
    (void) xmlFree(val);    

  /** number_of_large_scale_control_fields **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "number_of_large_scale_control_fields");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->field[1].n_ls = (int) strtol((char *) val, (char **)NULL, 10);
  else {
    (void) fprintf(stderr, "%s: Missing or invalid number_of_large_scale_control_fields setting. Aborting.\n", __FILE__);
    return -1;
  }
  if (val != NULL)
    (void) xmlFree(val);    

  /** number_of_secondary_large_scale_fields **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "number_of_secondary_large_scale_fields");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->field[2].n_ls = (int) strtol((char *) val, (char **)NULL, 10);
  else
    data->field[2].n_ls = 0;
  if (val != NULL)
    (void) xmlFree(val);    

  /** number_of_secondary_large_scale_control_fields **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "number_of_secondary_large_scale_control_fields");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->field[3].n_ls = (int) strtol((char *) val, (char **)NULL, 10);
  else {
    (void) fprintf(stderr, "%s: Missing or invalid number_of_secondary_large_scale_control_fields setting. Aborting.\n", __FILE__);
    return -1;
  }
  if (val != NULL)
    (void) xmlFree(val);    

  /* Loop over field categories */
  for (i=0; i<NCAT; i++) {

    /* Only process if at least one field defined */
    if (data->field[i].n_ls > 0) {

      if (data->field[i].n_ls > 1) {
        (void) fprintf(stderr, "%s: WARNING: only 1 large-scale field supported. Going back to one field and ignoring others in the configuration file!!!\n", __FILE__);
        data->field[i].n_ls = 1;
      }

      /* Allocate appropriate memory for data structures for each large-scale field */
      data->field[i].data = (field_data_struct *) malloc(data->field[i].n_ls * sizeof(field_data_struct));
      if (data->field[i].data == NULL) alloc_error(__FILE__, __LINE__);            
      data->field[i].proj = (proj_struct *) malloc(data->field[i].n_ls * sizeof(proj_struct));
      if (data->field[i].proj == NULL) alloc_error(__FILE__, __LINE__);
      
      for (j=0; j<data->field[i].n_ls; j++) {
        data->field[i].data[j].info = (info_field_struct *) malloc(sizeof(info_field_struct));
        if (data->field[i].data[j].info == NULL) alloc_error(__FILE__, __LINE__);
        data->field[i].data[j].clim_info = (clim_info_struct *) malloc(sizeof(clim_info_struct));
        if (data->field[i].data[j].clim_info == NULL) alloc_error(__FILE__, __LINE__);
        data->field[i].data[j].eof_info = (eof_info_struct *) malloc(sizeof(eof_info_struct));
        if (data->field[i].data[j].eof_info == NULL) alloc_error(__FILE__, __LINE__);
        data->field[i].data[j].eof_info->info = (info_field_struct *) malloc(sizeof(info_field_struct));
        if (data->field[i].data[j].eof_info->info == NULL) alloc_error(__FILE__, __LINE__);
        data->field[i].data[j].eof_data = (eof_data_struct *) malloc(sizeof(eof_data_struct));
        if (data->field[i].data[j].eof_data == NULL) alloc_error(__FILE__, __LINE__);
        data->field[i].data[j].down = (downscale_struct *) malloc(sizeof(downscale_struct));
        if (data->field[i].data[j].down == NULL) alloc_error(__FILE__, __LINE__);
        
        data->field[i].data[j].field_ls = NULL;
        data->field[i].data[j].field_eof_ls = NULL;
        data->field[i].data[j].eof_data->eof_ls = NULL;
        data->field[i].data[j].eof_data->sing_ls = NULL;
        data->field[i].data[j].down->mean_dist = NULL;
        data->field[i].data[j].down->var_dist = NULL;
      }
    }
  }

  /* Loop over field categories */
  for (cat=0; cat<NCAT; cat++) {

    /* Set strings */
    if (cat == 0) {
      catstr = strdup("large_scale_fields");
      catstrt = strdup("Large-scale fields");
    }
    else if (cat == 1) {
      catstr = strdup("large_scale_control_fields");
      catstrt = strdup("Large-scale control fields");
    }
    else if (cat == 2) {
      catstr = strdup("secondary_large_scale_fields");
      catstrt = strdup("Large-scale secondary fields");
    }
    else if (cat == 3) {
      catstr = strdup("secondary_large_scale_control_fields");
      catstrt = strdup("Large-scale secondary control fields");
    }
    else {
      catstr = strdup("large_scale_fields");
      catstrt = strdup("Large-scale fields");
    }

    /* Process only if at least one large-scale field defined */
    if (data->field[cat].n_ls > 0) {

      (void) fprintf(stdout, "%s: number_of_%s = %d\n", __FILE__, catstr, data->field[cat].n_ls);

      /* Loop over large-scale fields */
      /* Set filename and variable name strings */
      for (i=0; i<data->field[cat].n_ls; i++) {
      
        (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "name", i+1);
        val = xml_get_setting(conf, path);
        if (val != NULL) {
          data->field[cat].data[i].nomvar_ls = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));          
          if (data->field[cat].data[i].nomvar_ls == NULL) alloc_error(__FILE__, __LINE__);
          (void) strcpy( data->field[cat].data[i].nomvar_ls, (char *) val);
          (void) xmlFree(val);
        }
        else {
          (void) fprintf(stderr, "%s: Missing name setting %s. Aborting.\n", __FILE__, catstrt);
          return -1;
        }

        (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "filename", i+1);
        val = xml_get_setting(conf, path);
        if (val != NULL) {
          data->field[cat].data[i].filename_ls = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
          if (data->field[cat].data[i].filename_ls == NULL) alloc_error(__FILE__, __LINE__);
          (void) strcpy(data->field[cat].data[i].filename_ls, (char *) val);
          (void) xmlFree(val);
          (void) fprintf(stdout, "%s: %s #%d: name = %s filename = %s\n",
                         __FILE__, catstrt, i, data->field[cat].data[i].nomvar_ls, data->field[cat].data[i].filename_ls);
        }
        else {
          (void) fprintf(stderr, "%s: Missing filename setting %s. Aborting.\n", __FILE__, catstrt);
          return -1;
        }

        /* Fallback projection type */
        (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "projection", i+1);
        val = xml_get_setting(conf, path);
        if (val != NULL) {
          data->field[cat].proj[i].name = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
          if (data->field[cat].proj[i].name == NULL) alloc_error(__FILE__, __LINE__);
          (void) strcpy(data->field[cat].proj[i].name, (char *) val);
          (void) xmlFree(val);
          (void) fprintf(stdout, "%s: %s #%d: name = %s projection = %s\n",
                         __FILE__, catstrt, i, data->field[cat].data[i].nomvar_ls, data->field[cat].proj[i].name);
        }
        else
          data->field[cat].proj[i].name = strdup("unknown");

        /* Fallback coordinate system dimensions */
        (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "coordinates", i+1);
        val = xml_get_setting(conf, path);
        if (val != NULL) {
          data->field[cat].proj[i].coords = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
          if (data->field[cat].proj[i].coords == NULL) alloc_error(__FILE__, __LINE__);
          (void) strcpy(data->field[cat].proj[i].coords, (char *) val);
          (void) xmlFree(val);
          (void) fprintf(stdout, "%s: %s #%d: name = %s coordinates = %s\n",
                         __FILE__, catstrt, i, data->field[cat].data[i].nomvar_ls, data->field[cat].proj[i].coords);
        }
        else
          data->field[cat].proj[i].coords = strdup("2D");


        /** Climatology values **/

        /** clim_remove **/
        (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "clim_remove", i+1);
        val = xml_get_setting(conf, path);
        if (val != NULL)
          data->field[cat].data[i].clim_info->clim_remove = (int) xmlXPathCastStringToNumber(val);
        else
          data->field[cat].data[i].clim_info->clim_remove = 0;
        (void) fprintf(stdout, "%s: clim_remove = %d\n", __FILE__, data->field[cat].data[i].clim_info->clim_remove);
        if (val != NULL)
          (void) xmlFree(val);    

        /** clim_provided **/
        (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "clim_provided", i+1);
        val = xml_get_setting(conf, path);
        if (val != NULL) {
          if ( !xmlStrcmp(val, (xmlChar *) "1") )
            data->field[cat].data[i].clim_info->clim_provided = 1;
          else
            data->field[cat].data[i].clim_info->clim_provided = 0;
          (void) fprintf(stdout, "%s: clim_provided #%d = %d\n", __FILE__, i+1, data->field[cat].data[i].clim_info->clim_provided);

          /* If climatology is provided, additional parameters are needed */
          if (data->field[cat].data[i].clim_info->clim_provided == 1) {

            /** clim_openfilename **/
            (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "clim_openfilename",i+1);
            val = xml_get_setting(conf, path);
            if (val != NULL) {
              data->field[cat].data[i].clim_info->clim_filein_ls = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
              if (data->field[cat].data[i].clim_info->clim_filein_ls == NULL) alloc_error(__FILE__, __LINE__);
              (void) strcpy(data->field[cat].data[i].clim_info->clim_filein_ls, (char *) val);
              (void) fprintf(stdout, "%s:  Climatology input filename #%d = %s\n", __FILE__, i+1,
                             data->field[cat].data[i].clim_info->clim_filein_ls);
            }
            else {
              (void) fprintf(stderr, "%s: Missing clim_openfilename setting %s. Aborting.\n", __FILE__, catstrt);
              return -1;
            }
          }
          (void) xmlFree(val);
        }
        else
          data->field[cat].data[i].clim_info->clim_provided = 0;

        /** clim_save **/
        (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "clim_save", i+1);
        val = xml_get_setting(conf, path);
        if (val != NULL) {
          if ( !xmlStrcmp(val, (xmlChar *) "1") )
            data->field[cat].data[i].clim_info->clim_save = 1;
          else
            data->field[cat].data[i].clim_info->clim_save = 0;
          (void) fprintf(stdout, "%s: clim_save #%d = %d\n", __FILE__, i+1, data->field[cat].data[i].clim_info->clim_save);
          (void) xmlFree(val);

          /* If we want to save climatology in output file */
          if (data->field[cat].data[i].clim_info->clim_save == 1) {

            (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "clim_savefilename",i+1);
            val = xml_get_setting(conf, path);
            if (val != NULL) {
              data->field[cat].data[i].clim_info->clim_fileout_ls = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
              if (data->field[cat].data[i].clim_info->clim_fileout_ls == NULL) alloc_error(__FILE__, __LINE__);
              (void) strcpy(data->field[cat].data[i].clim_info->clim_fileout_ls, (char *) val);
              (void) fprintf(stdout, "%s:  Climatology output filename #%d = %s\n", __FILE__, i+1,
                             data->field[cat].data[i].clim_info->clim_fileout_ls);
              (void) xmlFree(val);
            }
            else {
              (void) fprintf(stderr, "%s: Missing clim_savefilename setting %s. Aborting.\n", __FILE__, catstrt);
              return -1;
            }
          }

          /* Climatology variable name */
          if (data->field[cat].data[i].clim_info->clim_save == 1 || data->field[cat].data[i].clim_info->clim_provided == 1) {
            (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "clim_name", i+1);
            val = xml_get_setting(conf, path);
            if (val != NULL) {
              data->field[cat].data[i].clim_info->clim_nomvar_ls = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
              if (data->field[cat].data[i].clim_info->clim_nomvar_ls == NULL) alloc_error(__FILE__, __LINE__);
              (void) strcpy(data->field[cat].data[i].clim_info->clim_nomvar_ls, (char *) val);
              (void) fprintf(stdout, "%s:  Climatology variable name #%d = %s\n", __FILE__, i+1,
                             data->field[cat].data[i].clim_info->clim_nomvar_ls);
              (void) xmlFree(val);
            }
            else {
              (void) fprintf(stderr, "%s: Missing clim_name setting %s. Aborting.\n", __FILE__, catstrt);
              return -1;
            }
          }
        }
        else
          data->field[cat].data[i].clim_info->clim_save = 0;

        /** EOF and Singular values **/
        /** number_of_eofs **/
        (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr,
                       "number_of_eofs", i+1);
        val = xml_get_setting(conf, path);
        if (val != NULL) {
          data->field[cat].data[i].eof_info->neof_ls = (int) xmlXPathCastStringToNumber(val);
          (void) fprintf(stdout, "%s: number_of_eofs = %d\n", __FILE__, data->field[cat].data[i].eof_info->neof_ls);
          (void) xmlFree(val);
        }
        
        /** eof_project **/
        (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "eof_project", i+1);
        val = xml_get_setting(conf, path);
        if (val != NULL)
          data->field[cat].data[i].eof_info->eof_project = (int) xmlXPathCastStringToNumber(val);
        else
          data->field[cat].data[i].eof_info->eof_project = 0;
        (void) fprintf(stdout, "%s: eof_project = %d\n", __FILE__, data->field[cat].data[i].eof_info->eof_project);
        if (val != NULL)
          (void) xmlFree(val);    
      
        /** eof_provided **/
        (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "eof_provided", i+1);
        val = xml_get_setting(conf, path);
        if (val != NULL) {
          if ( !xmlStrcmp(val, (xmlChar *) "1") )
            data->field[cat].data[i].eof_info->eof_provided = 1;
          else
            data->field[cat].data[i].eof_info->eof_provided = 0;
          (void) fprintf(stdout, "%s: eof_provided #%d = %d\n", __FILE__, i+1, data->field[cat].data[i].eof_info->eof_provided);
          (void) xmlFree(val);

          /* If EOFs are provided, additional parameters are needed */
          if (data->field[cat].data[i].eof_info->eof_provided == 1) {

            /** eof_coordinates **/
            (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "eof_coordinates", i+1);
            val = xml_get_setting(conf, path);
            if (val != NULL) {
              data->field[cat].data[i].eof_info->eof_coords = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
              if (data->field[cat].data[i].eof_info->eof_coords == NULL) alloc_error(__FILE__, __LINE__);
              (void) strcpy(data->field[cat].data[i].eof_info->eof_coords, (char *) val);
              (void) fprintf(stdout, "%s: %s #%d: name = %s eof_coordinates = %s\n",
                             __FILE__, catstrt, i, data->field[cat].data[i].nomvar_ls, data->field[cat].data[i].eof_info->eof_coords);
              (void) xmlFree(val);
            }
            else
              data->field[cat].data[i].eof_info->eof_coords = strdup("2D");

            /** eof_openfilename **/
            (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "eof_openfilename", i+1);
            val = xml_get_setting(conf, path);
            if (val != NULL) {
              data->field[cat].data[i].eof_info->eof_filein_ls = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
              if (data->field[cat].data[i].eof_info->eof_filein_ls == NULL) alloc_error(__FILE__, __LINE__);
              (void) strcpy(data->field[cat].data[i].eof_info->eof_filein_ls, (char *) val);
              (void) fprintf(stdout, "%s: EOF/Singular values input filename #%d = %s\n", __FILE__, i+1,
                             data->field[cat].data[i].eof_info->eof_filein_ls);
              (void) xmlFree(val);
            }
            else {
              (void) fprintf(stderr, "%s: Missing eof_openfilename setting. Aborting.\n", __FILE__);
              return -1;
            }
          }
        }
        else
          data->field[cat].data[i].eof_info->eof_provided = 0;

        /** eof_scale **/
        (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "eof_scale", i+1);
        val = xml_get_setting(conf, path);
        if (val != NULL)
          data->field[cat].data[i].eof_info->eof_scale = xmlXPathCastStringToNumber(val);
        else
          data->field[cat].data[i].eof_info->eof_scale = 1.0;
        (void) fprintf(stdout, "%s: units scaling = %lf\n", __FILE__, data->field[cat].data[i].eof_info->eof_scale);
        if (val != NULL)
          (void) xmlFree(val);    

        /** eof_save **/
        (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "eof_save", i+1);
        val = xml_get_setting(conf, path);
        if (val != NULL) {
          if ( !xmlStrcmp(val, (xmlChar *) "1") )
            data->field[cat].data[i].eof_info->eof_save = 1;
          else
            data->field[cat].data[i].eof_info->eof_save = 0;
          (void) fprintf(stdout, "%s: eof_save #%d = %d\n", __FILE__, i+1, data->field[cat].data[i].eof_info->eof_save);
          (void) xmlFree(val);

          /** If we want to save EOF, must have output filename in parameter */
          if (data->field[cat].data[i].eof_info->eof_save == 1) {

            (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "eof_savefilename", i+1);
            val = xml_get_setting(conf, path);
            if (val != NULL) {
              data->field[cat].data[i].eof_info->eof_fileout_ls = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
              if (data->field[cat].data[i].eof_info->eof_fileout_ls == NULL) alloc_error(__FILE__, __LINE__);
              (void) strcpy(data->field[cat].data[i].eof_info->eof_fileout_ls, (char *) val);
              (void) fprintf(stdout, "%s: EOF/Singular values output filename #%d = %s\n", __FILE__, i+1,
                             data->field[cat].data[i].eof_info->eof_fileout_ls);
              (void) xmlFree(val);
            }
            else {
              (void) fprintf(stderr, "%s: Missing eof_savefilename setting. Aborting.\n", __FILE__);
              return -1;
            }
          }

          /* If EOF is provided or saved, must have variable names */
          if (data->field[cat].data[i].eof_info->eof_save == 1 || data->field[cat].data[i].eof_info->eof_provided == 1) {
            /** eof_name **/
            (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "eof_name", i+1);
            val = xml_get_setting(conf, path);
            if (val != NULL) {
              data->field[cat].data[i].eof_data->eof_nomvar_ls = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
              if (data->field[cat].data[i].eof_data->eof_nomvar_ls == NULL) alloc_error(__FILE__, __LINE__);
              (void) strcpy(data->field[cat].data[i].eof_data->eof_nomvar_ls, (char *) val);
              (void) fprintf(stdout, "%s: EOF variable name #%d = %s\n", __FILE__, i+1, data->field[cat].data[i].eof_data->eof_nomvar_ls);
              (void) xmlFree(val);
            }
            else {
              (void) fprintf(stderr, "%s: Missing eof_name setting. Aborting.\n", __FILE__);
              return -1;
            }
            /** sing_name **/
            (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "sing_name", i+1);
            val = xml_get_setting(conf, path);
            if (val != NULL) {
              data->field[cat].data[i].eof_data->sing_nomvar_ls = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
              if (data->field[cat].data[i].eof_data->sing_nomvar_ls == NULL) alloc_error(__FILE__, __LINE__);
              (void) strcpy(data->field[cat].data[i].eof_data->sing_nomvar_ls, (char *) val);
              (void) fprintf(stdout, "%s: Singular values variable name #%d = %s\n", __FILE__, i+1,
                             data->field[cat].data[i].eof_data->sing_nomvar_ls);
              (void) xmlFree(val);
            }
            else {
              (void) fprintf(stderr, "%s: Missing sing_name setting. Aborting.\n", __FILE__);
              return -1;
            }
          }
        }
        else
          data->field[cat].data[i].eof_info->eof_save = 0;
      }
    }
    (void) free(catstr);
    (void) free(catstrt);
  }


  /**** SEASONS CONFIGURATION ****/

  /** number_of_seasons **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "number_of_seasons");
  val = xml_get_setting(conf, path);
  if (val != NULL) {
    data->conf->nseasons = (int) xmlXPathCastStringToNumber(val);
    (void) fprintf(stdout, "%s: number_of_seasons = %d\n", __FILE__, data->conf->nseasons);
    (void) xmlFree(val);

    /** Allocate memory for season-dependent variables **/

    data->conf->season = (season_struct *) malloc(data->conf->nseasons * sizeof(season_struct));
    if (data->conf->season == NULL) alloc_error(__FILE__, __LINE__);

    data->learning->data = (learning_data_struct *) malloc(data->conf->nseasons * sizeof(learning_data_struct));
    if (data->learning->data == NULL) alloc_error(__FILE__, __LINE__);

    /* Loop over field categories */
    for (cat=0; cat<NCAT; cat++) {

      data->field[cat].precip_index = (double **) malloc(data->conf->nseasons * sizeof(double *));
      if (data->field[cat].precip_index == NULL) alloc_error(__FILE__, __LINE__);
      data->field[cat].analog_days = (analog_day_struct *) malloc(data->conf->nseasons * sizeof(analog_day_struct));
      if (data->field[cat].analog_days == NULL) alloc_error(__FILE__, __LINE__);

      /* Loop over large-scale fields */
      for (i=0; i<data->field[cat].n_ls; i++) {
        if (cat == 0 || cat == 1) {
          /* Large-scale fields */
          data->field[cat].data[i].down->mean_dist = (double **) malloc(data->conf->nseasons * sizeof(double *));
          if (data->field[cat].data[i].down->mean_dist == NULL) alloc_error(__FILE__, __LINE__);
          data->field[cat].data[i].down->var_dist = (double **) malloc(data->conf->nseasons * sizeof(double *));
          if (data->field[cat].data[i].down->var_dist == NULL) alloc_error(__FILE__, __LINE__);
          data->field[cat].data[i].down->dist = (double **) malloc(data->conf->nseasons * sizeof(double *));
          if (data->field[cat].data[i].down->dist == NULL) alloc_error(__FILE__, __LINE__);
          data->field[cat].data[i].down->days_class_clusters = (int **) malloc(data->conf->nseasons * sizeof(int *));
          if (data->field[cat].data[i].down->days_class_clusters == NULL) alloc_error(__FILE__, __LINE__);
          data->field[cat].data[i].down->var_pc_norm = (double *) malloc(data->field[cat].data[i].eof_info->neof_ls * sizeof(double));
          if (data->field[cat].data[i].down->var_pc_norm == NULL) alloc_error(__FILE__, __LINE__);
        }
        else {
          /* Secondary large-scale fields */
          data->field[cat].data[i].down->smean_norm = (double **) malloc(data->conf->nseasons * sizeof(double *));
          if (data->field[cat].data[i].down->smean_norm == NULL) alloc_error(__FILE__, __LINE__);
          data->field[cat].data[i].down->mean = (double *) malloc(data->conf->nseasons * sizeof(double));
          if (data->field[cat].data[i].down->mean == NULL) alloc_error(__FILE__, __LINE__);
          data->field[cat].data[i].down->var = (double *) malloc(data->conf->nseasons * sizeof(double));
          if (data->field[cat].data[i].down->var == NULL) alloc_error(__FILE__, __LINE__);
          data->field[cat].data[i].down->delta = (double **) malloc(data->conf->nseasons * sizeof(double *));
          if (data->field[cat].data[i].down->delta == NULL) alloc_error(__FILE__, __LINE__);
        }
      }
    }

    /* Loop over seasons: season-dependent parameters */
    for (i=0; i<data->conf->nseasons; i++) {

      data->learning->data[i].time_s = (time_struct *) malloc(sizeof(time_struct));
      if (data->learning->data[i].time_s == NULL) alloc_error(__FILE__, __LINE__);

      /** number_of_clusters **/
      (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "number_of_clusters", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->season[i].nclusters = xmlXPathCastStringToNumber(val);
        (void) fprintf(stdout, "%s: season #%d number_of_clusters = %d\n", __FILE__, i+1, data->conf->season[i].nclusters);
        (void) xmlFree(val);
      }
      else
        data->conf->season[i].nclusters = -1;

      /** number_of_regression_vars **/
      (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "number_of_regression_vars", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->season[i].nreg = xmlXPathCastStringToNumber(val);
        (void) fprintf(stdout, "%s: season #%d number_of_regression_vars = %d\n", __FILE__, i+1, data->conf->season[i].nreg);
        (void) xmlFree(val);
      }
      else
        data->conf->season[i].nreg = -1;

      /** number_of_days_search **/
      (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "number_of_days_search", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->season[i].ndays = xmlXPathCastStringToNumber(val);
        (void) fprintf(stdout, "%s: season #%d number_of_days_search = %d\n", __FILE__, i+1, data->conf->season[i].ndays);
        (void) xmlFree(val);
      }
      else
        data->conf->season[i].ndays = 15;

      /** number_of_days_choices **/
      (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "number_of_days_choices", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->season[i].ndayschoices = xmlXPathCastStringToNumber(val);
        (void) fprintf(stdout, "%s: season #%d number_of_days_choices = %d\n", __FILE__, i+1, data->conf->season[i].ndayschoices);
        (void) xmlFree(val);
      }
      else
        if (i == 0 || i == 1)
          data->conf->season[i].ndayschoices = 15;
        else
          data->conf->season[i].ndayschoices = 10;

      /** days_shuffle **/
      (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "days_shuffle", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->season[i].shuffle = xmlXPathCastStringToNumber(val);
        (void) fprintf(stdout, "%s: season #%d days_shuffle = %d\n", __FILE__, i+1, data->conf->season[i].shuffle);
        (void) xmlFree(val);
      }
      else
        if (i == 0 || i == 1)
          data->conf->season[i].shuffle = 1;
        else
          data->conf->season[i].shuffle = 0;

      /** secondary_field_choice **/
      (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "secondary_field_choice", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->season[i].shuffle = xmlXPathCastStringToNumber(val);
        (void) fprintf(stdout, "%s: season #%d secondary_field_choice = %d\n", __FILE__, i+1, data->conf->season[i].secondary_choice);
        (void) xmlFree(val);
      }
      else
        if (i == 0 || i == 1)
          data->conf->season[i].secondary_choice = 0;
        else
          data->conf->season[i].secondary_choice = 1;

      /** secondary_field_main_choice **/
      (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "secondary_field_main_choice", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->season[i].shuffle = xmlXPathCastStringToNumber(val);
        (void) fprintf(stdout, "%s: season #%d secondary_field_main_choice = %d\n", __FILE__, i+1, data->conf->season[i].secondary_main_choice);
        (void) xmlFree(val);
      }
      else
        if (data->conf->season[i].secondary_choice == 0)
          data->conf->season[i].secondary_main_choice = 1;
        else
          data->conf->season[i].secondary_main_choice = 0;

      /** season **/
      (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]/@nmonths", "setting", "season", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->season[i].nmonths = xmlXPathCastStringToNumber(val);
        (void) fprintf(stdout, "%s: season #%d number_of_months = %d\n", __FILE__, i+1, data->conf->season[i].nmonths);
        data->conf->season[i].month = (int *) malloc(data->conf->season[i].nmonths * sizeof(int));
        if (data->conf->season[i].month == NULL) alloc_error(__FILE__, __LINE__);
        (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "season", i+1);
        (void) xmlFree(val);
        val = xml_get_setting(conf, path);
        if (val != NULL) {
          token = NULL;
          token = strtok_r((char *) val, " ", &saveptr);
          for (j=0; j<data->conf->season[i].nmonths; j++) {
            if (token != NULL) {
              (void) sscanf(token, "%d", &(data->conf->season[i].month[j]));
              (void) fprintf(stdout, "%s: season #%d month=%d\n", __FILE__, i+1, data->conf->season[i].month[j]);
              token = strtok_r(NULL, " ", &saveptr);    
            }
          }
          (void) xmlFree(val);
        }
      }
      else
        data->conf->season[i].nmonths = 0;
    }
  }
  else {
    (void) fprintf(stderr, "%s: Invalid number_of_seasons value %s in configuration file. Aborting.\n", __FILE__, val);
    return -1;
  }

  /**** LEARNING CONFIGURATION ****/

  /* Whole learning period */
  data->learning->time_s = (time_struct *) malloc(sizeof(time_struct));
  if (data->learning->time_s == NULL) alloc_error(__FILE__, __LINE__);

  /** learning_provided **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "learning_provided");
  val = xml_get_setting(conf, path);
  if (val != NULL) 
    data->learning->learning_provided = (int) strtol((char *) val, (char **)NULL, 10);
  else
    data->learning->learning_provided = -1;
  if (data->learning->learning_provided != 0 && data->learning->learning_provided != 1) {
    (void) fprintf(stderr, "%s: Invalid or missing learning_provided value %s in configuration file. Aborting.\n", __FILE__, val);
    return -1;
  }
  (void) fprintf(stdout, "%s: learning_provided=%d\n", __FILE__, data->learning->learning_provided);
  if (val != NULL) 
    (void) xmlFree(val);

  /* If learning data is provided, additional parameters are needed */
  if (data->learning->learning_provided == 1) {

    /** filename_weight **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "filename_weight");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->filename_weight = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->filename_weight == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->filename_weight, (char *) val);
      (void) fprintf(stdout, "%s: Learning filename_weight = %s\n", __FILE__, data->learning->filename_weight);
      (void) xmlFree(val);
    }
    else {
      (void) fprintf(stderr, "%s: Missing learning filename_weight setting. Aborting.\n", __FILE__);
      (void) xmlFree(val);
      return -1;
    }

    /** filename_learn **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "filename_learn");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->filename_learn = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->filename_learn == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->filename_learn, (char *) val);
      (void) fprintf(stdout, "%s: Learning filename_learn = %s\n", __FILE__, data->learning->filename_learn);
      (void) xmlFree(val);
    }
    else {
      (void) fprintf(stderr, "%s: Missing learning filename_learn setting. Aborting.\n", __FILE__);
      (void) xmlFree(val);
      return -1;
    }

    /** filename_clust_learn **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "filename_clust_learn");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->filename_clust_learn = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->filename_clust_learn == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->filename_clust_learn, (char *) val);
      (void) fprintf(stdout, "%s: Learning filename_clust_learn = %s\n", __FILE__, data->learning->filename_clust_learn);
      (void) xmlFree(val);
    }
    else {
      (void) fprintf(stderr, "%s: Missing learning filename_clust_learn setting. Aborting.\n", __FILE__);
      (void) xmlFree(val);
      return -1;
    }

    /** nomvar_time **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "nomvar_time");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->nomvar_time = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->nomvar_time == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->nomvar_time, (char *) val);
      (void) xmlFree(val);
    }
    else
      data->learning->nomvar_time = strdup("time");
    (void) fprintf(stdout, "%s: Learning nomvar_time = %s\n", __FILE__, data->learning->nomvar_time);

    /** nomvar_weight **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "nomvar_weight");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->nomvar_weight = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->nomvar_weight == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->nomvar_weight, (char *) val);
      (void) xmlFree(val);
    }
    else
      data->learning->nomvar_weight = strdup("poid");
    (void) fprintf(stdout, "%s: Learning nomvar_weight = %s\n", __FILE__, data->learning->nomvar_weight);

    /** nomvar_class_clusters **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "nomvar_class_clusters");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->nomvar_class_clusters = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->nomvar_class_clusters == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->nomvar_class_clusters, (char *) val);
      (void) xmlFree(val);
    }
    else
      data->learning->nomvar_class_clusters = strdup("clust_learn");
    (void) fprintf(stdout, "%s: Learning nomvar_class_clusters = %s\n", __FILE__, data->learning->nomvar_class_clusters);

    /** nomvar_precip_reg **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "nomvar_precip_reg");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->nomvar_precip_reg = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->nomvar_precip_reg == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->nomvar_precip_reg, (char *) val);
      (void) xmlFree(val);
    }
    else
      data->learning->nomvar_precip_reg = strdup("reg");
    (void) fprintf(stdout, "%s: Learning nomvar_precip_reg = %s\n", __FILE__, data->learning->nomvar_precip_reg);

    /** nomvar_precip_reg_cst **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "nomvar_precip_reg_cst");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->nomvar_precip_reg_cst = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->nomvar_precip_reg_cst == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->nomvar_precip_reg_cst, (char *) val);
      (void) xmlFree(val);
    }
    else
      data->learning->nomvar_precip_reg_cst = strdup("cst");
    (void) fprintf(stdout, "%s: Learning nomvar_precip_reg_cst = %s\n", __FILE__, data->learning->nomvar_precip_reg_cst);

    /** nomvar_precip_index **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "nomvar_precip_index");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->nomvar_precip_index = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->nomvar_precip_index == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->nomvar_precip_index, (char *) val);
      (void) xmlFree(val);
    }
    else
      data->learning->nomvar_precip_index = strdup("rrd");
    (void) fprintf(stdout, "%s: Learning nomvar_precip_index = %s\n", __FILE__, data->learning->nomvar_precip_index);

    /** nomvar_sup_index **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "nomvar_sup_index");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->nomvar_sup_index = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->nomvar_sup_index == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->nomvar_sup_index, (char *) val);
      (void) xmlFree(val);
    }
    else
      data->learning->nomvar_sup_index = strdup("ta");
    (void) fprintf(stdout, "%s: Learning nomvar_sup_index = %s\n", __FILE__, data->learning->nomvar_sup_index);

    /** nomvar_sup_index_mean **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "nomvar_sup_index_mean");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->nomvar_sup_index_mean = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->nomvar_sup_index_mean == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->nomvar_sup_index_mean, (char *) val);
      (void) xmlFree(val);
    }
    else
      data->learning->nomvar_sup_index_mean = strdup("tancp_mean");
    (void) fprintf(stdout, "%s: Learning nomvar_sup_index_mean = %s\n", __FILE__, data->learning->nomvar_sup_index_mean);

    /** nomvar_sup_index_var **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "nomvar_sup_index_var");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->nomvar_sup_index_var = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->nomvar_sup_index_var == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->nomvar_sup_index_var, (char *) val);
      (void) xmlFree(val);
    }
    else
      data->learning->nomvar_sup_index_var = strdup("tancp_var");
    (void) fprintf(stdout, "%s: Learning nomvar_sup_index_var = %s\n", __FILE__, data->learning->nomvar_sup_index_var);

    /** nomvar_pc_normalized_var **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "nomvar_pc_normalized_var");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->nomvar_pc_normalized_var = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->nomvar_pc_normalized_var == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->nomvar_pc_normalized_var, (char *) val);
      (void) xmlFree(val);
    }
    else
      data->learning->nomvar_pc_normalized_var = strdup("eca_pc_learn");
    (void) fprintf(stdout, "%s: Learning nomvar_pc_normalized_var = %s\n", __FILE__, data->learning->nomvar_pc_normalized_var);

  }

  /**** REGRESSION CONFIGURATION ****/
  /** filename **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "regression", "filename");
  val = xml_get_setting(conf, path);
  if (val != NULL) {
    data->reg->filename = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
    if (data->reg->filename == NULL) alloc_error(__FILE__, __LINE__);
    (void) strcpy(data->reg->filename, (char *) val);
    (void) fprintf(stdout, "%s: Regression points filename = %s\n", __FILE__, data->reg->filename);
    (void) xmlFree(val);
  }
  else {
    (void) fprintf(stderr, "%s: Missing regression points filename setting. Aborting.\n", __FILE__);
    (void) xmlFree(val);
    return -1;
  }
  /** lonname **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "regression", "lonname");
  val = xml_get_setting(conf, path);
  if (val != NULL) {
    data->reg->lonname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
    if (data->reg->lonname == NULL) alloc_error(__FILE__, __LINE__);
    (void) strcpy(data->reg->lonname, (char *) val);
    (void) fprintf(stdout, "%s: Regression points lonname = %s\n", __FILE__, data->reg->lonname);
    (void) xmlFree(val);
  }
  else {
    (void) fprintf(stderr, "%s: Missing regression points lonname setting. Aborting.\n", __FILE__);
    (void) xmlFree(val);
    return -1;
  }
  /** latname **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "regression", "latname");
  val = xml_get_setting(conf, path);
  if (val != NULL) {
    data->reg->latname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
    if (data->reg->latname == NULL) alloc_error(__FILE__, __LINE__);
    (void) strcpy(data->reg->latname, (char *) val);
    (void) fprintf(stdout, "%s: Regression points latname = %s\n", __FILE__, data->reg->latname);
    (void) xmlFree(val);
  }
  else {
    (void) fprintf(stderr, "%s: Missing regression points latname setting. Aborting.\n", __FILE__);
    (void) xmlFree(val);
    return -1;
  }
  /** ptsname **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "regression", "ptsname");
  val = xml_get_setting(conf, path);
  if (val != NULL) {
    data->reg->ptsname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
    if (data->reg->ptsname == NULL) alloc_error(__FILE__, __LINE__);
    (void) strcpy(data->reg->ptsname, (char *) val);
    (void) fprintf(stdout, "%s: Regression points ptsname = %s\n", __FILE__, data->reg->ptsname);
    (void) xmlFree(val);
  }
  else {
    (void) fprintf(stderr, "%s: Missing regression points ptsname setting. Aborting.\n", __FILE__);
    (void) xmlFree(val);
    return -1;
  }

  /* Free memory */
  (void) xml_free_config(conf);
  (void) free(path);

  /* Success status */
  return 0;
}
