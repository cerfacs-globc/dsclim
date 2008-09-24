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

  (void) fprintf(stdout, "*** Current Configuration ***\n\n");

  (void) strcpy(setting_name, "setting");
  
  /* Load XML configuration file into memory */
  conf = xml_load_config(fileconf);
  if (conf == NULL) return -1;

  data->conf = (conf_struct *) malloc(sizeof(conf_struct));
  if (data->conf == NULL) alloc_error(__FILE__, __LINE__);
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
  (void) fprintf(stdout, "debug = %d\n", data->conf->debug);

  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "clim_filter_width");
  val = xml_get_setting(conf, path);
  if ( !xmlStrcmp(val, "1") )
    data->conf->clim_filter_width = 1;
  else
    data->conf->clim_filter_width = 0;
  (void) fprintf(stdout, "clim_filter_width = %d\n", data->conf->clim_filter_width);

  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "clim_filter_type");
  val = xml_get_setting(conf, path);
  if ( !xmlStrcmp(val, "hanning") )
    data->conf->clim_filter_type = strdup("hanning");
  else {
    (void) fprintf(stderr, "%s: Invalid clim_filter_type value %s in configuration file. Aborting.", __FILE__, val);
    (void) abort();
  }
  (void) fprintf(stdout, "clim_filter_type = %s\n", data->conf->clim_filter_type);

  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "clim_provided");
  val = xml_get_setting(conf, path);
  if ( !xmlStrcmp(val, "1") )
    data->conf->clim_provided = 1;
  else
    data->conf->clim_provided = 0;
  (void) fprintf(stdout, "clim_provided = %d\n", data->conf->clim_provided);

  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "base_time_units");
  val = xml_get_setting(conf, path);
  data->conf->time_units = strdup(val);
  (void) fprintf(stdout, "base_time_units = %s\n", data->conf->time_units);

  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "base_calendar_type");
  val = xml_get_setting(conf, path);
  data->conf->cal_type = strdup(val);
  (void) fprintf(stdout, "base_calendar_type = %s\n", data->conf->cal_type);

  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "longitude_name");
  val = xml_get_setting(conf, path);
  data->conf->lonname = strdup(val);
  (void) fprintf(stdout, "longitude_name = %s\n", data->conf->lonname);

  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "latitude_name");
  val = xml_get_setting(conf, path);
  data->conf->latname = strdup(val);
  (void) fprintf(stdout, "latitude_name = %s\n", data->conf->latname);

  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "time_name");
  val = xml_get_setting(conf, path);
  data->conf->timename = strdup(val);
  (void) fprintf(stdout, "time_name = %s\n", data->conf->timename);

  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "number_of_large_scale_fields");
  val = xml_get_setting(conf, path);
  data->field->n_ls = (int) strtol(val, (char **)NULL, 10);
  data->field->fname_ls = (char **) malloc(data->field->n_ls * sizeof(char *));
  if (data->field->fname_ls == NULL) alloc_error(__FILE__, __LINE__);
  data->field->field_ls = (double **) malloc(data->field->n_ls * sizeof(double *));
  if (data->field->field_ls == NULL) alloc_error(__FILE__, __LINE__);

  if (val > 0) {
    (void) fprintf(stdout, "number_of_large_scale_fields = %d\n", data->field->n_ls);
    for (i=0; i<data->field->n_ls; i++) {
      
      (void) sprintf(path, "/configuration/%s/%s/[@id=\"%d\"]", "setting", "name", i);
      val = xml_get_setting(conf, path);
      data->field->fname_ls[i] = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->field->fname_ls[i] == NULL) alloc_error(__FILE__, __LINE__);
      data->field->fname_ls[i] = BAD_CAST xmlStrdup(val);

      (void) sprintf(path, "/configuration/%s/%s/[@id=\"%d\"]", "setting", "filename", i);
      val = xml_get_setting(conf, path);
      data->field->filename_ls[i] = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->field->filename_ls[i] == NULL) alloc_error(__FILE__, __LINE__);
      data->field->filename_ls[i] = BAD_CAST xmlStrdup(val);
      (void) fprintf(stdout, "Large-scale field #%d: name=%s filename=%s",
                     i, data->field->fname_ls[i], data->field->filename_ls[i]);

      if (data->conf->clim_provided == 1) {
        (void) sprintf(path, "/configuration/%s/%s/[@id=\"%d\"]", "setting", "clim_filename", i);
        val = xml_get_setting(conf, path);
        data->field->clim_filename_ls[i] = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
        if (data->field->clim_filename_ls[i] == NULL) alloc_error(__FILE__, __LINE__);
        data->field->clim_filename_ls[i] = BAD_CAST xmlStrdup(val);
        (void) fprintf(stdout, " Climatology filename=%s", data->field->clim_filename_ls[i]);
      }
      (void) fprintf(stdout, "\n");
      
    }
  }
  else {
    (void) fprintf(stderr, "%s: Invalid number_of_large_scale_fields value %s in configuration file. Aborting.", __FILE__, val);
    (void) abort();
  }
  

  /* Free memory */
  (void) xml_free_config(conf);
  (void) free(path);

  return 0;
}
