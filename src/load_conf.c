/* ***************************************************** */
/* Read and set variables from XML configuration file.   */
/* load_conf.c                                           */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file load_conf.c
    \brief Read and set variables from XML configuration file.
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


#include <libs/xml_utils/xml_utils.h>
#include <dsclim.h>

/** Read and set variables from XML configuration file. */
int
load_conf(data_struct *data, char *fileconf) {
  /**
     @param[in]  data      MASTER data structure.
     @param[in]  fileconf  XML input filename
     
     \return           Status.
  */
  FILE *infile; /* Input file pointer */
  long int numbytes; /* Size of entire file */

  xmlConfig_t *conf; /* Pointer to XML Config */
  char setting_name[1000]; /* Setting name in XML file */
  xmlChar *val; /* Value in XML file */
  int i; /* Loop counter */
  int j; /* Loop counter */
  int cat; /* Loop counter for field category */
  int istat; /* Diagnostic status */
  char *path = NULL; /* XPath */

  char *token; /* Token for string decoding */
  char *saveptr; /* Pointer to save buffer data for thread-safe strtok use */
  char *catstr; /* Category string */
  char *catstrt; /* Category string */

  (void) fprintf(stdout, "%s: *** Current Configuration ***\n\n", __FILE__);

  (void) strcpy(setting_name, "setting");

  /* Allocate main memory data structure */
  data->conf = (conf_struct *) malloc(sizeof(conf_struct));
  if (data->conf == NULL) alloc_error(__FILE__, __LINE__);

  /** Read entire file into memory for archive in the output file later **/

  /* Open config file */
  infile = fopen(fileconf, "r");
  if (infile == NULL) {
    (void) fprintf(stderr, "%s: Cannot open %s configuration file for reading. Aborting.\n", __FILE__, fileconf);
    (void) free(data->conf);
    return -1;
  }
  
  /* Get the number of bytes */
  istat = fseek(infile, 0L, SEEK_END);
  if (istat < 0) {
    (void) fprintf(stderr, "%s: Cannot seek to end of %s configuration file. Aborting.\n", __FILE__, fileconf);
    (void) free(data->conf);
    return -1;
  }
  numbytes = ftell(infile);
  if (numbytes < 0) {
    (void) fprintf(stderr, "%s: Cannot get file pointer position of %s configuration file. Aborting.\n", __FILE__, fileconf);
    (void) free(data->conf);
    return -1;
  }
  
  /* Reset the file position indicator to the beginning of the file */
  istat = fseek(infile, 0L, SEEK_SET);	
  if (istat < 0) {
    (void) fprintf(stderr, "%s: Cannot seek to beginning of %s configuration file. Aborting.\n", __FILE__, fileconf);
    (void) free(data->conf);
    return -1;
  }
  
  /* Allocate memory */
  data->conf->config = (char *) calloc(numbytes+1, sizeof(char));
  if (data->conf->config == NULL) alloc_error(__FILE__, __LINE__);
  
  /* Copy all the text into the buffer */
  istat = fread(data->conf->config, sizeof(char), numbytes, infile);
  if (istat < 0) {
    (void) fprintf(stderr, "%s: Cannot read %s configuration file. Aborting.\n", __FILE__, fileconf);
    (void) free(data->conf);
    (void) free(data->conf->config);
    return -1;
  }
  /* Add null character at the end of the string */
  data->conf->config[numbytes] = '\0';
  
  /* Close file */
  istat = fclose(infile);
  if (istat < 0) {
    (void) fprintf(stderr, "%s: Cannot close properly %s configuration file. Aborting.\n", __FILE__, fileconf);
    (void) free(data->conf);
    (void) free(data->conf->config);
    return -1;
  }

#if DEBUG > 7
  printf("The file called test.dat contains this text\n\n%s", data->conf->config);
#endif
  
  /* Load XML configuration file into memory */
  conf = xml_load_config(fileconf);
  if (conf == NULL) {
    (void) free(data->conf);
    (void) free(data->conf->config);
    (void) xmlCleanupParser();
    return -1;
  }

  /* Allocate memory in main data structures */
  data->conf->period_ctrl = (period_struct *) malloc(sizeof(period_struct));
  if (data->conf->period_ctrl == NULL) alloc_error(__FILE__, __LINE__);
  data->info = (info_struct *) malloc(sizeof(info_struct));
  if (data->info == NULL) alloc_error(__FILE__, __LINE__);
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
    data->field[i].time_s = (time_vect_struct *) malloc(sizeof(time_vect_struct));
    if (data->field[i].time_s == NULL) alloc_error(__FILE__, __LINE__);    

    data->field[i].lat_ls = NULL;
    data->field[i].lon_ls = NULL;

    data->field[i].lat_eof_ls = NULL;
    data->field[i].lon_eof_ls = NULL;
  }

  /*** Get needed settings ***/
  /* Set default value if not in configuration file */
  path = (char *) malloc(MAXPATH * sizeof(char));
  if (path == NULL) alloc_error(__FILE__, __LINE__);

  /** debug **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "debug");
  val = xml_get_setting(conf, path);
  if ( !xmlStrcmp(val, (xmlChar *) "On") )
    data->conf->debug = TRUE;
  else
    data->conf->debug = FALSE;
  (void) fprintf(stdout, "%s: debug = %d\n", __FILE__, data->conf->debug);
  if (val != NULL)
    (void) xmlFree(val);

  /** format: NetCDF-4 or NetCDF-3 for output files **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "format");
  val = xml_get_setting(conf, path);
  if (val != NULL) {
    data->conf->format = (int) xmlXPathCastStringToNumber(val);
    if (data->conf->format != 3 && data->conf->format != 4)
      data->conf->format = 3;
    (void) xmlFree(val);
  }
  else
    data->conf->format = 3;
  if (data->conf->format == 3)
    (void) fprintf(stdout, "%s: NetCDF-3 Classic output format.\n", __FILE__);
  else
    (void) fprintf(stdout, "%s: NetCDF-4 New HDF5-based format with Classic-type output support.\n", __FILE__);

  /** compression for NetCDF-4 **/
  if (data->conf->format == 4) {
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "compression");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      if ( !xmlStrcmp(val, (xmlChar *) "On") ) {
        data->conf->compression = TRUE;
        (void) fprintf(stdout, "%s: Compression ACTIVE for NetCDF-4 format\n", __FILE__);
      }
      else {
        data->conf->compression = FALSE;
        (void) fprintf(stdout, "%s: Compression DISABLED for NetCDF-4 format\n", __FILE__);
      }
      (void) xmlFree(val);
    }
  }
  else
    data->conf->compression = FALSE;

  /** compression level for NetCDF-4 **/
  if (data->conf->compression == TRUE) {
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "compression_level");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->conf->compression_level = (int) xmlXPathCastStringToNumber(val);
      if (data->conf->compression_level < 0) {
        data->conf->compression_level = 1;
        (void) fprintf(stdout,
                       "%s: WARNING: NetCDF-4 Compression Level invalid value (must be between 1 and 9 inclusively). Forced to %d.\n",
                       __FILE__, data->conf->compression_level);
      }
      else if (data->conf->compression_level > 9) {
        data->conf->compression_level = 9;
        (void) fprintf(stdout,
                       "%s: WARNING: NetCDF-4 Compression Level invalid value (must be between 1 and 9 inclusively). Forced to %d.\n",
                       __FILE__, data->conf->compression_level);
      }
      (void) xmlFree(val);
    }
    else {
      data->conf->compression_level = 1;
      (void) fprintf(stdout,
                     "%s: WARNING: NetCDF-4 Compression Level not set! (must be between 1 and 9 inclusively). Forced to default value of %d.\n",
                     __FILE__, data->conf->compression_level);
    }
    (void) fprintf(stdout, "%s: NetCDF-4 Compression Level = %d.\n", __FILE__, data->conf->compression_level);
  }
  else
    data->conf->compression_level = 0;

  /** Fix incorrect time in input climate model file, and use 01/01/YEARBEGIN as first day, and assume daily data since it is required. */
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "fixtime");
  val = xml_get_setting(conf, path);
  if (val != NULL) {
    if ( !xmlStrcmp(val, (xmlChar *) "On") ) {
      data->conf->fixtime = TRUE;
      (void) fprintf(stdout, "%s: WARNING: Will fix time coordinate start date using start date in configuration file!\n", __FILE__);
    }
    else {
      data->conf->fixtime = FALSE;
      (void) fprintf(stdout, "%s: Will NOT fix time coordinate start date.\n", __FILE__);
    }
    (void) xmlFree(val);
  }

  if (data->conf->fixtime == TRUE) {
    /** year_begin_ctrl **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "year_begin_ctrl");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->conf->year_begin_ctrl = xmlXPathCastStringToNumber(val);
      (void) fprintf(stdout, "%s: year_begin_ctrl = %d\n", __FILE__, data->conf->year_begin_ctrl);
      (void) xmlFree(val);
    }
    else
      data->conf->year_begin_ctrl = -1;
    /** year_begin_other **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "year_begin_other");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->conf->year_begin_other = xmlXPathCastStringToNumber(val);
      (void) fprintf(stdout, "%s: year_begin_other = %d\n", __FILE__, data->conf->year_begin_other);
      (void) xmlFree(val);
    }
    else
      data->conf->year_begin_other = -1;
    if (data->conf->year_begin_ctrl == -1 || data->conf->year_begin_other == -1) {
      (void) fprintf(stderr, "%s: ERROR: must specify year_begin_ctrl and year_begin_other when using the fixtime setting! Aborting.\n", __FILE__);
      return -1;
    }
  }

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

  /** npartitions **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "number_of_partitions");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->npartitions = xmlXPathCastStringToNumber(val);
  else
    data->conf->npartitions = 30;
  (void) fprintf(stdout, "%s: Number of partitions = %d\n", __FILE__, data->conf->npartitions);
  if (val != NULL)
    (void) xmlFree(val);    

  /** nclassifications **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "number_of_classifications");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->nclassifications = xmlXPathCastStringToNumber(val);
  else
    data->conf->nclassifications = 1000;
  (void) fprintf(stdout, "%s: Number of classifications = %d\n", __FILE__, data->conf->nclassifications);
  if (val != NULL)
    (void) xmlFree(val);    

  /** use_downscaled_year **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "use_downscaled_year");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->use_downscaled_year = xmlXPathCastStringToNumber(val);
  else
    data->conf->use_downscaled_year = 1;
  (void) fprintf(stdout, "%s: Use_downscaled_year = %d\n", __FILE__, data->conf->use_downscaled_year);
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

  /** dimx_name_eof **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "dimx_name_eof");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->dimxname_eof = strdup((char *) val);
  else
    data->conf->dimxname_eof = strdup("lon");  
  (void) fprintf(stdout, "%s: dimx_name_eof = %s\n", __FILE__, data->conf->dimxname_eof);
  if (val != NULL)
    (void) xmlFree(val);

  /** dimy_name_eof **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "dimy_name_eof");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->dimyname_eof = strdup((char *) val);
  else
    data->conf->dimyname_eof = strdup("lat");
  (void) fprintf(stdout, "%s: dimy_name_eof = %s\n", __FILE__, data->conf->dimyname_eof);
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

  /**** LARGE-SCALE FIELDS (CLASSIFICATION) DOMAIN CONFIGURATION ****/

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

  /**** SECONDARY LARGE-SCALE FIELDS DOMAIN CONFIGURATION ****/

  /** longitude min **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@type=\"%s\"]", "setting", "domain_secondary_large_scale", "longitude", "min");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->secondary_longitude_min = xmlXPathCastStringToNumber(val);
  else
    data->conf->secondary_longitude_min = -15.0;
  (void) fprintf(stdout, "%s: Large-scale domain longitude min = %lf\n", __FILE__, data->conf->secondary_longitude_min);
  if (val != NULL)
    (void) xmlFree(val);    

  /** longitude max **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@type=\"%s\"]", "setting", "domain_secondary_large_scale", "longitude", "max");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->secondary_longitude_max = xmlXPathCastStringToNumber(val);
  else
    data->conf->secondary_longitude_max = 20.0;
  (void) fprintf(stdout, "%s: Large-scale domain longitude max = %lf\n", __FILE__, data->conf->secondary_longitude_max);
  if (val != NULL)
    (void) xmlFree(val);    

  /** latitude min **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@type=\"%s\"]", "setting", "domain_secondary_large_scale", "latitude", "min");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->secondary_latitude_min = xmlXPathCastStringToNumber(val);
  else
    data->conf->secondary_latitude_min = 35.0;
  (void) fprintf(stdout, "%s: Large-scale domain latitude min = %lf\n", __FILE__, data->conf->secondary_latitude_min);
  if (val != NULL)
    (void) xmlFree(val);    

  /** latitude max **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@type=\"%s\"]", "setting", "domain_secondary_large_scale", "latitude", "max");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->secondary_latitude_max = xmlXPathCastStringToNumber(val);
  else
    data->conf->secondary_latitude_max = 60.0;
  (void) fprintf(stdout, "%s: Large-scale domain latitude max = %lf\n", __FILE__, data->conf->secondary_latitude_max);
  if (val != NULL)
    (void) xmlFree(val);    

  /**** SECONDARY-LARGE SCALE FIELDS MASK CONFIGURATION ****/
  data->secondary_mask = (mask_struct *) malloc(sizeof(mask_struct));
  if (data->secondary_mask == NULL) alloc_error(__FILE__, __LINE__);
  /** use_mask **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "domain_secondary_large_scale_mask", "use_mask");
  val = xml_get_setting(conf, path);
  if (val != NULL) 
    data->secondary_mask->use_mask = (int) strtol((char *) val, (char **)NULL, 10);
  else
    data->secondary_mask->use_mask = FALSE;
  if (data->secondary_mask->use_mask != FALSE && data->secondary_mask->use_mask != TRUE) {
    (void) fprintf(stderr, "%s: Invalid or missing secondary_mask use_mask value %s in configuration file. Aborting.\n", __FILE__, val);
    return -1;
  }
  (void) fprintf(stdout, "%s: secondary_mask use_mask=%d\n", __FILE__, data->secondary_mask->use_mask);
  if (val != NULL) 
    (void) xmlFree(val);

  if (data->secondary_mask->use_mask == TRUE) {
    /** filename **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "domain_secondary_large_scale_mask", "filename");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->secondary_mask->filename = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->secondary_mask->filename == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->secondary_mask->filename, (char *) val);
      (void) fprintf(stdout, "%s: Secondary large-scale fields mask filename = %s\n", __FILE__, data->secondary_mask->filename);
      (void) xmlFree(val);
      
      /** maskname **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "domain_secondary_large_scale_mask", "mask_name");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->secondary_mask->maskname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
        if (data->secondary_mask->maskname == NULL) alloc_error(__FILE__, __LINE__);
        (void) strcpy(data->secondary_mask->maskname, (char *) val);
        (void) fprintf(stdout, "%s: Secondary large-scale fields mask name = %s\n", __FILE__, data->secondary_mask->maskname);
        (void) xmlFree(val);
      }
      else {
        data->secondary_mask->maskname = strdup("mask");
        (void) fprintf(stderr, "%s: Default secondary large-scale fields mask name = %s\n", __FILE__,
                       data->secondary_mask->maskname);
        (void) xmlFree(val);
      }
      /** lonname **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "domain_secondary_large_scale_mask", "longitude_name");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->secondary_mask->lonname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
        if (data->secondary_mask->lonname == NULL) alloc_error(__FILE__, __LINE__);
        (void) strcpy(data->secondary_mask->lonname, (char *) val);
        (void) fprintf(stdout, "%s: Secondary large-scale fields mask longitude_name = %s\n", __FILE__, data->secondary_mask->lonname);
        (void) xmlFree(val);
      }
      else {
        data->secondary_mask->lonname = strdup("lon");
        (void) fprintf(stderr, "%s: Default secondary large-scale fields mask longitude_name = %s\n", __FILE__,
                       data->secondary_mask->lonname);
        (void) xmlFree(val);
      }
      /** latname **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "domain_secondary_large_scale_mask", "latitude_name");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->secondary_mask->latname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
        if (data->secondary_mask->latname == NULL) alloc_error(__FILE__, __LINE__);
        (void) strcpy(data->secondary_mask->latname, (char *) val);
        (void) fprintf(stdout, "%s: Secondary large-scale fields mask latitude_name = %s\n", __FILE__, data->secondary_mask->latname);
        (void) xmlFree(val);
      }
      else {
        data->secondary_mask->latname = strdup("lat");
        (void) fprintf(stderr, "%s: Default secondary large-scale fields mask latitude_name = %s\n", __FILE__,
                       data->secondary_mask->latname);
        (void) xmlFree(val);
      }
      /** coords **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "domain_secondary_large_scale_mask", "coordinates");
      val = xml_get_setting(conf, path);
      if (val != NULL)
        data->secondary_mask->coords = strdup((char *) val);
      else
        data->secondary_mask->coords = strdup("2D");
      (void) fprintf(stdout, "%s: Secondary large-scale fields mask coords = %s\n", __FILE__, data->secondary_mask->coords);
      if (val != NULL)
        (void) xmlFree(val);    
      /** dimxname **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "domain_secondary_large_scale_mask", "dimx_name");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->secondary_mask->dimxname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
        if (data->secondary_mask->dimxname == NULL) alloc_error(__FILE__, __LINE__);
        (void) strcpy(data->secondary_mask->dimxname, (char *) val);
        (void) fprintf(stdout, "%s: Secondary large-scale fields mask dimx_name = %s\n", __FILE__, data->secondary_mask->dimxname);
        (void) xmlFree(val);
      }
      else {
        data->secondary_mask->dimxname = strdup("dimx");
        (void) fprintf(stderr, "%s: Default secondary large-scale fields mask dimx_name = %s\n", __FILE__,
                       data->secondary_mask->dimxname);
        (void) xmlFree(val);
      }
      /** dimyname **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "domain_secondary_large_scale_mask", "dimy_name");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->secondary_mask->dimyname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
        if (data->secondary_mask->dimyname == NULL) alloc_error(__FILE__, __LINE__);
        (void) strcpy(data->secondary_mask->dimyname, (char *) val);
        (void) fprintf(stdout, "%s: Secondary large-scale fields mask dimy_name = %s\n", __FILE__, data->secondary_mask->dimyname);
        (void) xmlFree(val);
      }
      else {
        data->secondary_mask->dimyname = strdup("dimy");
        (void) fprintf(stderr, "%s: Default secondary large-scale fields mask dimy_name = %s\n", __FILE__,
                       data->secondary_mask->dimyname);
        (void) xmlFree(val);
      }
      /** dimcoords **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "domain_secondary_large_scale_mask", "dim_coordinates");
      val = xml_get_setting(conf, path);
      if (val != NULL)
        data->secondary_mask->dimcoords = strdup((char *) val);
      else
        data->secondary_mask->dimcoords = strdup("2D");
      (void) fprintf(stdout, "%s: Secondary large-scale fields mask dim_coords = %s\n", __FILE__, data->secondary_mask->dimcoords);
      if (val != NULL)
        (void) xmlFree(val);    
      /** projection **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "domain_secondary_large_scale_mask", "projection");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->secondary_mask->proj = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
        if (data->secondary_mask->proj == NULL) alloc_error(__FILE__, __LINE__);
        (void) strcpy(data->secondary_mask->proj, (char *) val);
        (void) xmlFree(val);
      }
      else
        data->secondary_mask->proj = strdup("Latitude_Longitude");
      (void) fprintf(stdout, "%s: Secondary large-scale fields mask projection = %s\n",
                     __FILE__, data->secondary_mask->proj);
    }
    else {
      (void) fprintf(stderr, "%s: No secondary large-scale fields mask. Desactivating the use of the mask.\n", __FILE__);
      data->secondary_mask->filename = NULL;
      data->secondary_mask->use_mask = FALSE;
      (void) xmlFree(val);
    }
  }
  
  /**** LEARNING MASK DOMAIN CONFIGURATION ****/
  
  /** longitude min **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@type=\"%s\"]", "setting", "domain_learning_mask", "longitude", "min");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->learning_mask_longitude_min = xmlXPathCastStringToNumber(val);
  else
    data->conf->learning_mask_longitude_min = -999.0;
  (void) fprintf(stdout, "%s: Learning mask domain longitude min = %lf\n", __FILE__, data->conf->learning_mask_longitude_min);
  if (val != NULL)
    (void) xmlFree(val);    

  /** longitude max **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@type=\"%s\"]", "setting", "domain_learning_mask", "longitude", "max");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->learning_mask_longitude_max = xmlXPathCastStringToNumber(val);
  else
    data->conf->learning_mask_longitude_max = -999.0;
  (void) fprintf(stdout, "%s: Learning mask domain longitude max = %lf\n", __FILE__, data->conf->learning_mask_longitude_max);
  if (val != NULL)
    (void) xmlFree(val);    

  /** latitude min **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@type=\"%s\"]", "setting", "domain_learning_mask", "latitude", "min");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->learning_mask_latitude_min = xmlXPathCastStringToNumber(val);
  else
    data->conf->learning_mask_latitude_min = -999.0;
  (void) fprintf(stdout, "%s: Learning mask domain latitude min = %lf\n", __FILE__, data->conf->learning_mask_latitude_min);
  if (val != NULL)
    (void) xmlFree(val);    

  /** latitude max **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@type=\"%s\"]", "setting", "domain_learning_mask", "latitude", "max");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->learning_mask_latitude_max = xmlXPathCastStringToNumber(val);
  else
    data->conf->learning_mask_latitude_max = -999.0;
  (void) fprintf(stdout, "%s: Learning mask domain latitude max = %lf\n", __FILE__, data->conf->learning_mask_latitude_max);
  if (val != NULL)
    (void) xmlFree(val);    

  /**** LEARNING MASKFILE CONFIGURATION ****/
  data->conf->learning_maskfile = (mask_struct *) malloc(sizeof(mask_struct));
  if (data->conf->learning_maskfile == NULL) alloc_error(__FILE__, __LINE__);
  /** use_mask **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "domain_learning_maskfile", "use_mask");
  val = xml_get_setting(conf, path);
  if (val != NULL) 
    data->conf->learning_maskfile->use_mask = (int) strtol((char *) val, (char **)NULL, 10);
  else
    data->conf->learning_maskfile->use_mask = FALSE;
  if (data->conf->learning_maskfile->use_mask != FALSE && data->conf->learning_maskfile->use_mask != TRUE) {
    (void) fprintf(stderr, "%s: Invalid or missing domain_learning_maskfile use_mask value %s in configuration file. Aborting.\n", __FILE__, val);
    return -1;
  }
  (void) fprintf(stdout, "%s: domain_learning_maskfile use_mask=%d\n", __FILE__, data->conf->learning_maskfile->use_mask);
  if (val != NULL) 
    (void) xmlFree(val);

  if (data->conf->learning_maskfile->use_mask == TRUE) {
    /** filename **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "domain_learning_maskfile", "filename");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->conf->learning_maskfile->filename = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->conf->learning_maskfile->filename == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->conf->learning_maskfile->filename, (char *) val);
      (void) fprintf(stdout, "%s: Learning domain maskfile filename = %s\n", __FILE__, data->conf->learning_maskfile->filename);
      (void) xmlFree(val);
      
      /** maskname **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "domain_learning_maskfile", "mask_name");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->learning_maskfile->maskname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
        if (data->conf->learning_maskfile->maskname == NULL) alloc_error(__FILE__, __LINE__);
        (void) strcpy(data->conf->learning_maskfile->maskname, (char *) val);
        (void) fprintf(stdout, "%s: Learning domain maskfile name = %s\n", __FILE__, data->conf->learning_maskfile->maskname);
        (void) xmlFree(val);
      }
      else {
        data->conf->learning_maskfile->maskname = strdup("mask");
        (void) fprintf(stderr, "%s: Default learning domain maskfile name = %s\n", __FILE__,
                       data->conf->learning_maskfile->maskname);
        (void) xmlFree(val);
      }
      /** lonname **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "domain_learning_maskfile", "longitude_name");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->learning_maskfile->lonname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
        if (data->conf->learning_maskfile->lonname == NULL) alloc_error(__FILE__, __LINE__);
        (void) strcpy(data->conf->learning_maskfile->lonname, (char *) val);
        (void) fprintf(stdout, "%s: Learning domain maskfile longitude_name = %s\n", __FILE__, data->conf->learning_maskfile->lonname);
        (void) xmlFree(val);
      }
      else {
        data->conf->learning_maskfile->lonname = strdup("lon");
        (void) fprintf(stderr, "%s: Default learning domain maskfile longitude_name = %s\n", __FILE__,
                       data->conf->learning_maskfile->lonname);
        (void) xmlFree(val);
      }
      /** latname **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "domain_learning_maskfile", "latitude_name");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->learning_maskfile->latname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
        if (data->conf->learning_maskfile->latname == NULL) alloc_error(__FILE__, __LINE__);
        (void) strcpy(data->conf->learning_maskfile->latname, (char *) val);
        (void) fprintf(stdout, "%s: Learning domain maskfile latitude_name = %s\n", __FILE__, data->conf->learning_maskfile->latname);
        (void) xmlFree(val);
      }
      else {
        data->conf->learning_maskfile->latname = strdup("lat");
        (void) fprintf(stderr, "%s: Default learning domain maskfile latitude_name = %s\n", __FILE__,
                       data->conf->learning_maskfile->latname);
        (void) xmlFree(val);
      }
      /** coords **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "domain_learning_maskfile", "coordinates");
      val = xml_get_setting(conf, path);
      if (val != NULL)
        data->conf->learning_maskfile->coords = strdup((char *) val);
      else
        data->conf->learning_maskfile->coords = strdup("2D");
      (void) fprintf(stdout, "%s: Learning domain maskfile coords = %s\n", __FILE__, data->conf->learning_maskfile->coords);
      if (val != NULL)
        (void) xmlFree(val);    
      /** dimxname **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "domain_learning_maskfile", "dimx_name");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->learning_maskfile->dimxname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
        if (data->conf->learning_maskfile->dimxname == NULL) alloc_error(__FILE__, __LINE__);
        (void) strcpy(data->conf->learning_maskfile->dimxname, (char *) val);
        (void) fprintf(stdout, "%s: Learning domain maskfile dimx_name = %s\n", __FILE__, data->conf->learning_maskfile->dimxname);
        (void) xmlFree(val);
      }
      else {
        data->conf->learning_maskfile->dimxname = strdup("dimx");
        (void) fprintf(stderr, "%s: Default learning domain maskfile dimx_name = %s\n", __FILE__,
                       data->conf->learning_maskfile->dimxname);
        (void) xmlFree(val);
      }
      /** dimyname **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "domain_learning_maskfile", "dimy_name");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->learning_maskfile->dimyname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
        if (data->conf->learning_maskfile->dimyname == NULL) alloc_error(__FILE__, __LINE__);
        (void) strcpy(data->conf->learning_maskfile->dimyname, (char *) val);
        (void) fprintf(stdout, "%s: Learning domain maskfile dimy_name = %s\n", __FILE__, data->conf->learning_maskfile->dimyname);
        (void) xmlFree(val);
      }
      else {
        data->conf->learning_maskfile->dimyname = strdup("dimy");
        (void) fprintf(stderr, "%s: Default learning domain maskfile dimy_name = %s\n", __FILE__,
                       data->conf->learning_maskfile->dimyname);
        (void) xmlFree(val);
      }
      /** dimcoords **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "domain_learning_maskfile", "dim_coordinates");
      val = xml_get_setting(conf, path);
      if (val != NULL)
        data->conf->learning_maskfile->dimcoords = strdup((char *) val);
      else
        data->conf->learning_maskfile->dimcoords = strdup("2D");
      (void) fprintf(stdout, "%s: Learning domain maskfile dim_coords = %s\n", __FILE__, data->conf->learning_maskfile->dimcoords);
      if (val != NULL)
        (void) xmlFree(val);    
      /** projection **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "domain_learning_maskfile", "projection");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->learning_maskfile->proj = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
        if (data->conf->learning_maskfile->proj == NULL) alloc_error(__FILE__, __LINE__);
        (void) strcpy(data->conf->learning_maskfile->proj, (char *) val);
        (void) xmlFree(val);
      }
      else
        data->conf->learning_maskfile->proj = strdup("Latitude_Longitude");
      (void) fprintf(stdout, "%s: Learning domain maskfile projection = %s\n",
                     __FILE__, data->conf->learning_maskfile->proj);
    }
    else {
      (void) fprintf(stderr, "%s: No learning domain maskfile. Desactivating the use of the mask.\n", __FILE__);
      data->conf->learning_maskfile->filename = NULL;
      data->conf->learning_maskfile->use_mask = FALSE;
      (void) xmlFree(val);
    }
  }

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

  /** title **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "title");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->title = strdup((char *) val);
  else {
    data->info->title = strdup("Downscaling data from Cerfacs");
  }
  (void) fprintf(stdout, "%s: output metadata title = %s\n", __FILE__, data->info->title);
  if (val != NULL)
    (void) xmlFree(val);    

  /** title_french **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "title_french");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->title_french = strdup((char *) val);
  else {
    data->info->title_french = strdup("Donnees de desagregation produites par le Cerfacs");
  }
  (void) fprintf(stdout, "%s: output metadata title_french = %s\n", __FILE__, data->info->title_french);
  if (val != NULL)
    (void) xmlFree(val);    
  
  /** summary **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "summary");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->summary = strdup((char *) val);
  else {
    data->info->summary = strdup("Downscaling data from Cerfacs");
  }
  (void) fprintf(stdout, "%s: output metadata summary = %s\n", __FILE__, data->info->summary);
  if (val != NULL)
    (void) xmlFree(val);    

  /** summary_french **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "summary_french");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->summary_french = strdup((char *) val);
  else {
    data->info->summary_french = strdup("Donnees de desagregation produites par le Cerfacs");
  }
  (void) fprintf(stdout, "%s: output metadata summary_french = %s\n", __FILE__, data->info->summary_french);
  if (val != NULL)
    (void) xmlFree(val);    

  /** description **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "description");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->description = strdup((char *) val);
  else {
    data->info->description = strdup("Downscaling data from Cerfacs");
  }
  (void) fprintf(stdout, "%s: output metadata description = %s\n", __FILE__, data->info->description);
  if (val != NULL)
    (void) xmlFree(val);    

  /** keywords **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "keywords");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->keywords = strdup((char *) val);
  else {
    data->info->keywords = strdup("climat,scenarios,desagregation,downscaling,Cerfacs");
  }
  (void) fprintf(stdout, "%s: output metadata keywords = %s\n", __FILE__, data->info->keywords);
  if (val != NULL)
    (void) xmlFree(val);    

  /** processor **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "processor");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->processor = strdup((char *) val);
  else {
    data->info->processor = strdup("C programming language");
  }
  (void) fprintf(stdout, "%s: output metadata processor = %s\n", __FILE__, data->info->processor);
  if (val != NULL)
    (void) xmlFree(val);    

  /** institution **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "institution");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->institution = strdup((char *) val);
  else {
    data->info->institution = strdup("Cerfacs");
  }
  (void) fprintf(stdout, "%s: output metadata institution = %s\n", __FILE__, data->info->institution);
  if (val != NULL)
    (void) xmlFree(val);    

  /** creator_email **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "creator_email");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->creator_email = strdup((char *) val);
  else {
    data->info->creator_email = strdup("globc@cerfacs.fr");
  }
  (void) fprintf(stdout, "%s: output metadata creator_email = %s\n", __FILE__, data->info->creator_email);
  if (val != NULL)
    (void) xmlFree(val);    

  /** creator_url **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "creator_url");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->creator_url = strdup((char *) val);
  else {
    data->info->creator_url = strdup("http://www.cerfacs.fr/globc/");
  }
  (void) fprintf(stdout, "%s: output metadata creator_url = %s\n", __FILE__, data->info->creator_url);
  if (val != NULL)
    (void) xmlFree(val);    

  /** creator_name **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "creator_name");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->creator_name = strdup((char *) val);
  else {
    data->info->creator_name = strdup("Global Change Team");
  }
  (void) fprintf(stdout, "%s: output metadata creator_name = %s\n", __FILE__, data->info->creator_name);
  if (val != NULL)
    (void) xmlFree(val);    

  /** version **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "version");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->version = strdup((char *) val);
  else {
    data->info->version = strdup("1.0");
  }
  (void) fprintf(stdout, "%s: output metadata version = %s\n", __FILE__, data->info->version);
  if (val != NULL)
    (void) xmlFree(val);    

  /** scenario **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "scenario");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->scenario = strdup((char *) val);
  else {
    data->info->scenario = strdup("SRESA1B");
  }
  (void) fprintf(stdout, "%s: output metadata scenario = %s\n", __FILE__, data->info->scenario);
  if (val != NULL)
    (void) xmlFree(val);    

  /** scenario_co2 **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "scenario_co2");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->scenario_co2 = strdup((char *) val);
  else {
    data->info->scenario_co2 = strdup("A1B");
  }
  (void) fprintf(stdout, "%s: output metadata scenario_co2 = %s\n", __FILE__, data->info->scenario_co2);
  if (val != NULL)
    (void) xmlFree(val);    

  /** model **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "model");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->model = strdup((char *) val);
  else {
    data->info->model = strdup("ARPEGE grille etiree");
  }
  (void) fprintf(stdout, "%s: output metadata model = %s\n", __FILE__, data->info->model);
  if (val != NULL)
    (void) xmlFree(val);    

  /** institution_model **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "institution_model");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->institution_model = strdup((char *) val);
  else {
    data->info->institution_model = strdup("Meteo-France CNRM/GMGEC");
  }
  (void) fprintf(stdout, "%s: output metadata institution_model = %s\n", __FILE__, data->info->institution_model);
  if (val != NULL)
    (void) xmlFree(val);    

  /** country **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "country");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->country = strdup((char *) val);
  else {
    data->info->country = strdup("France");
  }
  (void) fprintf(stdout, "%s: output metadata country = %s\n", __FILE__, data->info->country);
  if (val != NULL)
    (void) xmlFree(val);    

  /** member **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "member");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->member = strdup((char *) val);
  else {
    data->info->member = strdup("1");
  }
  (void) fprintf(stdout, "%s: output metadata member = %s\n", __FILE__, data->info->member);
  if (val != NULL)
    (void) xmlFree(val);    

  /** downscaling_forcing **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "downscaling_forcing");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->downscaling_forcing = strdup((char *) val);
  else {
    data->info->downscaling_forcing = strdup("SAFRAN 1970-2005");
  }
  (void) fprintf(stdout, "%s: output metadata downscaling_forcing = %s\n", __FILE__, data->info->downscaling_forcing);
  if (val != NULL)
    (void) xmlFree(val);    

  /** timestep **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "timestep");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->timestep = strdup((char *) val);
  else {
    data->info->timestep = strdup("daily");
  }
  if ( !strcmp(data->info->timestep, "daily") || !strcmp(data->info->timestep, "hourly"))
    (void) fprintf(stdout, "%s: output metadata timestep = %s\n", __FILE__, data->info->timestep);
  else {
    (void) fprintf(stderr, "%s: Invalid output timestep! Values accepted are either \"hourly\" or \"daily\"! Aborting.\n", __FILE__);
  }
  if (val != NULL)
    (void) xmlFree(val);    

  /** contact_email **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "contact_email");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->contact_email = strdup((char *) val);
  else {
    data->info->contact_email = strdup("christian.page@cerfacs.fr");
  }
  (void) fprintf(stdout, "%s: output metadata contact_email = %s\n", __FILE__, data->info->contact_email);
  if (val != NULL)
    (void) xmlFree(val);    

  /** contact_name **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "contact_name");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->contact_name = strdup((char *) val);
  else {
    data->info->contact_name = strdup("Christian PAGE");
  }
  (void) fprintf(stdout, "%s: output metadata contact_name = %s\n", __FILE__, data->info->contact_name);
  if (val != NULL)
    (void) xmlFree(val);    

  /** other_contact_email **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "other_contact_email");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->other_contact_email = strdup((char *) val);
  else {
    data->info->other_contact_email = strdup("laurent.terray@cerfacs.fr");
  }
  (void) fprintf(stdout, "%s: output metadata other_contact_email = %s\n", __FILE__, data->info->other_contact_email);
  if (val != NULL)
    (void) xmlFree(val);    

  /** other_contact_name **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "output", "other_contact_name");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->info->other_contact_name = strdup((char *) val);
  else {
    data->info->other_contact_name = strdup("Laurent TERRAY");
  }
  (void) fprintf(stdout, "%s: output metadata other_contact_name = %s\n", __FILE__, data->info->other_contact_name);
  if (val != NULL)
    (void) xmlFree(val);    

  /**** OBSERVATION DATABASE CONFIGURATION ****/

  data->conf->obs_var = (var_struct *) malloc(sizeof(var_struct));
  if (data->conf->obs_var == NULL) alloc_error(__FILE__, __LINE__);
  data->conf->obs_var->proj = (proj_struct *) malloc(sizeof(proj_struct));
  if (data->conf->obs_var->proj == NULL) alloc_error(__FILE__, __LINE__);
  data->conf->obs_var->proj->name = NULL;

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
    data->conf->obs_var->post = (char **) malloc(data->conf->obs_var->nobs_var * sizeof(char *));
    if (data->conf->obs_var->post == NULL) alloc_error(__FILE__, __LINE__);
    data->conf->obs_var->output = (char **) malloc(data->conf->obs_var->nobs_var * sizeof(char *));
    if (data->conf->obs_var->output == NULL) alloc_error(__FILE__, __LINE__);
    data->conf->obs_var->units = (char **) malloc(data->conf->obs_var->nobs_var * sizeof(char *));
    if (data->conf->obs_var->units == NULL) alloc_error(__FILE__, __LINE__);
    data->conf->obs_var->height = (char **) malloc(data->conf->obs_var->nobs_var * sizeof(char *));
    if (data->conf->obs_var->height == NULL) alloc_error(__FILE__, __LINE__);

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

      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s[@id=\"%d\"]/@%s", "setting", "observations", "variables", "name", i+1, "postprocess");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->obs_var->post[i] = strdup((char *) val);
        (void) xmlFree(val);
      }
      else {
        data->conf->obs_var->post[i] = strdup("no");
      }

      if ( strcmp(data->conf->obs_var->post[i], "yes") && strcmp(data->conf->obs_var->post[i], "no") ) {
        (void) fprintf(stderr, "%s: Invalid observation variable postprocess setting (valid values are \"yes\" or \"no\"). Aborting.\n", __FILE__);
        return -1;
      }
      if (i == 0 && !strcmp(data->conf->obs_var->post[i], "yes")) {
        (void) fprintf(stderr, "%s: Invalid observation variable postprocess setting. A variable having a postprocess attribute of \"yes\" must not be the first one in the list. Aborting.\n", __FILE__);
        return -1;
      }

      /* Output */
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s[@id=\"%d\"]/@%s", "setting", "observations", "variables", "name", i+1, "output");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->obs_var->output[i] = strdup((char *) val);
        (void) xmlFree(val);
      }
      else {
        data->conf->obs_var->output[i] = strdup("yes");
      }

      if ( strcmp(data->conf->obs_var->output[i], "yes") && strcmp(data->conf->obs_var->output[i], "no") ) {
        (void) fprintf(stderr, "%s: Invalid observation variable output setting (valid values are \"yes\" or \"no\"). Aborting.\n", __FILE__);
        return -1;
      }

      /* Try to retrieve units and height. */
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s[@id=\"%d\"]/@%s", "setting", "observations", "variables", "name", i+1, "units");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->obs_var->units[i] = strdup((char *) val);
        (void) xmlFree(val);
      }
      else {
        data->conf->obs_var->units[i] = strdup("unknown");
      }
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s[@id=\"%d\"]/@%s", "setting", "observations", "variables", "name", i+1, "height");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->obs_var->height[i] = strdup((char *) val);
        (void) xmlFree(val);
      }
      else {
        data->conf->obs_var->height[i] = strdup("unknown");
      }
    
      (void) printf("%s: Variable id=%d name=\"%s\" netcdfname=%s acronym=%s factor=%f delta=%f postprocess=%s output=%s\n", __FILE__, i+1, data->conf->obs_var->name[i], data->conf->obs_var->netcdfname[i], data->conf->obs_var->acronym[i], data->conf->obs_var->factor[i], data->conf->obs_var->delta[i], data->conf->obs_var->post[i], data->conf->obs_var->output[i]);
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
  if ( strcmp(data->conf->obs_var->frequency, "daily") && strcmp(data->conf->obs_var->frequency, "hourly")) {
    (void) fprintf(stderr, "%s: Missing or invalid observations data frequency setting. Aborting.\n", __FILE__);
    return -1;
  }
  if ( !strcmp(data->info->timestep, "hourly") && !strcmp(data->conf->obs_var->frequency, "daily") ) {
    (void) fprintf(stderr, "%s: Invalid observations data frequency setting \"daily\" while output timestep is set to \"hourly\"! Aborting.\n", __FILE__);
    return -1;
  }

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

  /** dimx_name **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "observations", "dimx_name");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->obs_var->dimxname = strdup((char *) val);
  else
    data->conf->obs_var->dimxname = strdup("lon");  
  (void) fprintf(stdout, "%s: Observations dimx_name = %s\n", __FILE__, data->conf->obs_var->dimxname);
  if (val != NULL)
    (void) xmlFree(val);    

  /** dimy_name **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "observations", "dimy_name");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->obs_var->dimyname = strdup((char *) val);
  else
    data->conf->obs_var->dimyname = strdup("lat");
  (void) fprintf(stdout, "%s: Observations dimy_name = %s\n", __FILE__, data->conf->obs_var->dimyname);
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

  /** Altitude NetCDF filename **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "observations", "altitude");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->obs_var->altitude = strdup((char *) val);
  else {
    (void) fprintf(stderr, "%s: Missing observations altitude filename. Will not be able to calculate Relative Humidity if specified.\n", __FILE__);
    data->conf->obs_var->altitude = strdup("");
  }
  if (val != NULL)
    (void) xmlFree(val);    

  /** altitude_name NetCDF variable name **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "observations",  "altitude_name");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->conf->obs_var->altitudename = strdup((char *) val);
  else
    data->conf->obs_var->altitudename = strdup("Altitude");
  (void) fprintf(stdout, "%s: Observations altitude_name = %s\n", __FILE__, data->conf->obs_var->altitudename);
  if (val != NULL)
    (void) xmlFree(val);    

  /**** LEARNING CONFIGURATION ****/

  /* Whole learning period */
  data->learning->time_s = (time_vect_struct *) malloc(sizeof(time_vect_struct));
  if (data->learning->time_s == NULL) alloc_error(__FILE__, __LINE__);

  /** learning_provided **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "learning_provided");
  val = xml_get_setting(conf, path);
  if (val != NULL) 
    data->learning->learning_provided = (int) strtol((char *) val, (char **)NULL, 10);
  else
    data->learning->learning_provided = -1;
  if (data->learning->learning_provided != FALSE && data->learning->learning_provided != TRUE) {
    (void) fprintf(stderr, "%s: Invalid or missing learning_provided value %s in configuration file. Aborting.\n", __FILE__, val);
    return -1;
  }
  (void) fprintf(stdout, "%s: learning_provided=%d\n", __FILE__, data->learning->learning_provided);
  if (val != NULL) 
    (void) xmlFree(val);

  /** learning_save **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "learning_save");
  val = xml_get_setting(conf, path);
  if (val != NULL) 
    data->learning->learning_save = (int) strtol((char *) val, (char **)NULL, 10);
  else
    data->learning->learning_save = FALSE;
  if (data->learning->learning_save != FALSE && data->learning->learning_save != TRUE) {
    (void) fprintf(stderr, "%s: Invalid learning_save value %s in configuration file. Aborting.\n", __FILE__, val);
    return -1;
  }
  (void) fprintf(stdout, "%s: learning_save=%d\n", __FILE__, data->learning->learning_save);
  if (val != NULL) 
    (void) xmlFree(val);

  /** number of EOFs one parameter **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "number_of_eofs");
  val = xml_get_setting(conf, path);
  if (val != NULL) {
    data->learning->rea_neof = xmlXPathCastStringToNumber(val);
    data->learning->obs_neof = xmlXPathCastStringToNumber(val);
    (void) fprintf(stdout, "%s: Number of EOF for learning period for reanalysis data = %d\n", __FILE__, data->learning->rea_neof);
    (void) fprintf(stdout, "%s: Number of EOF for learning period for observation data = %d\n", __FILE__, data->learning->obs_neof);
    (void) xmlFree(val);    
  }
  else {
    /** number of EOFs observation data **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "number_of_obs_eofs");
    val = xml_get_setting(conf, path);
    if (val != NULL)
      data->learning->obs_neof = xmlXPathCastStringToNumber(val);
    else
      data->learning->obs_neof = 10;
    (void) fprintf(stdout, "%s: Number of EOF for learning period for observation data = %d\n", __FILE__, data->learning->obs_neof);
    if (val != NULL)
      (void) xmlFree(val);    
    
    /** number of EOFs reanalysis data **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "number_of_rea_eofs");
    val = xml_get_setting(conf, path);
    if (val != NULL)
      data->learning->rea_neof = xmlXPathCastStringToNumber(val);
    else
      data->learning->rea_neof = 10;
    (void) fprintf(stdout, "%s: Number of EOF for learning period for reanalysis data = %d\n", __FILE__, data->learning->rea_neof);
    if (val != NULL)
      (void) xmlFree(val);    
  }

  /* If learning data is saved, additional parameters are needed */
  if (data->learning->learning_save == TRUE) {

    /** filename_save_weight **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "filename_save_weight");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->filename_save_weight = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->filename_save_weight == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->filename_save_weight, (char *) val);
      (void) fprintf(stdout, "%s: Learning filename_save_weight = %s\n", __FILE__, data->learning->filename_save_weight);
      (void) xmlFree(val);
    }
    else {
      (void) fprintf(stderr, "%s: Missing learning filename_save_weight setting. Aborting.\n", __FILE__);
      (void) xmlFree(val);
      return -1;
    }

    /** filename_save_learn **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "filename_save_learn");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->filename_save_learn = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->filename_save_learn == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->filename_save_learn, (char *) val);
      (void) fprintf(stdout, "%s: Learning filename_save_learn = %s\n", __FILE__, data->learning->filename_save_learn);
      (void) xmlFree(val);
    }
    else {
      (void) fprintf(stderr, "%s: Missing learning filename_save_learn setting. Aborting.\n", __FILE__);
      (void) xmlFree(val);
      return -1;
    }

    /** filename_save_clust_learn **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "filename_save_clust_learn");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->filename_save_clust_learn = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->filename_save_clust_learn == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->filename_save_clust_learn, (char *) val);
      (void) fprintf(stdout, "%s: Learning filename_save_clust_learn = %s\n", __FILE__, data->learning->filename_save_clust_learn);
      (void) xmlFree(val);
    }
    else {
      (void) fprintf(stderr, "%s: Missing learning filename_save_clust_learn setting. Aborting.\n", __FILE__);
      (void) xmlFree(val);
      return -1;
    }
  }

  /* If learning data is provided, additional parameters are needed */
  if (data->learning->learning_provided == TRUE) {

    /** filename_open_weight **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "filename_open_weight");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->filename_open_weight = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->filename_open_weight == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->filename_open_weight, (char *) val);
      (void) fprintf(stdout, "%s: Learning filename_open_weight = %s\n", __FILE__, data->learning->filename_open_weight);
      (void) xmlFree(val);
    }
    else {
      (void) fprintf(stderr, "%s: Missing learning filename_open_weight setting. Aborting.\n", __FILE__);
      (void) xmlFree(val);
      return -1;
    }

    /** filename_open_learn **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "filename_open_learn");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->filename_open_learn = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->filename_open_learn == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->filename_open_learn, (char *) val);
      (void) fprintf(stdout, "%s: Learning filename_open_learn = %s\n", __FILE__, data->learning->filename_open_learn);
      (void) xmlFree(val);
    }
    else {
      (void) fprintf(stderr, "%s: Missing learning filename_open_learn setting. Aborting.\n", __FILE__);
      (void) xmlFree(val);
      return -1;
    }

    /** filename_open_clust_learn **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "filename_open_clust_learn");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->filename_open_clust_learn = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->filename_open_clust_learn == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->filename_open_clust_learn, (char *) val);
      (void) fprintf(stdout, "%s: Learning filename_open_clust_learn = %s\n", __FILE__, data->learning->filename_open_clust_learn);
      (void) xmlFree(val);
    }
    else {
      (void) fprintf(stderr, "%s: Missing learning filename_open_clust_learn setting. Aborting.\n", __FILE__);
      (void) xmlFree(val);
      return -1;
    }
  }
  else {

    data->learning->obs = (learning_eof_struct *) malloc(sizeof(learning_eof_struct));
    if (data->learning->obs == NULL) alloc_error(__FILE__, __LINE__);
    data->learning->rea = (learning_eof_struct *) malloc(sizeof(learning_eof_struct));
    if (data->learning->rea == NULL) alloc_error(__FILE__, __LINE__);
    
    data->learning->obs->time_s = (time_vect_struct *) malloc(sizeof(time_vect_struct));
    if (data->learning->obs->time_s == NULL) alloc_error(__FILE__, __LINE__);
    data->learning->rea->time_s = (time_vect_struct *) malloc(sizeof(time_vect_struct));
    if (data->learning->rea->time_s == NULL) alloc_error(__FILE__, __LINE__);

    /** filename_obs_eof **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "filename_obs_eof");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->obs->filename_eof = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->obs->filename_eof == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->obs->filename_eof, (char *) val);
      (void) fprintf(stdout, "%s: Learning filename_obs_eof = %s\n", __FILE__, data->learning->obs->filename_eof);
      (void) xmlFree(val);
    }
    else {
      (void) fprintf(stderr, "%s: Missing learning filename_obs_eof setting. Aborting.\n", __FILE__);
      (void) xmlFree(val);
      return -1;
    }

    /** filename_rea_eof **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "filename_rea_eof");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->rea->filename_eof = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->rea->filename_eof == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->rea->filename_eof, (char *) val);
      (void) fprintf(stdout, "%s: Learning filename_rea_eof = %s\n", __FILE__, data->learning->rea->filename_eof);
      (void) xmlFree(val);
    }
    else {
      (void) fprintf(stderr, "%s: Missing learning filename_rea_eof setting. Aborting.\n", __FILE__);
      (void) xmlFree(val);
      return -1;
    }

    /** filename_rea_sup **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "filename_rea_sup");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->filename_rea_sup = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->filename_rea_sup == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->filename_rea_sup, (char *) val);
      (void) fprintf(stdout, "%s: Learning filename_rea_sup = %s\n", __FILE__, data->learning->filename_rea_sup);
      (void) xmlFree(val);
    }
    else {
      (void) fprintf(stderr, "%s: Missing learning filename_rea_sup setting. Aborting.\n", __FILE__);
      (void) xmlFree(val);
      return -1;
    }

    /** nomvar_obs_eof **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "nomvar_obs_eof");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->obs->nomvar_eof = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->obs->nomvar_eof == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->obs->nomvar_eof, (char *) val);
      (void) xmlFree(val);
    }
    else {
      data->learning->obs->nomvar_eof = strdup("pre_pc");
    }
    (void) fprintf(stdout, "%s: Learning nomvar_eof = %s\n", __FILE__, data->learning->obs->nomvar_eof);
    
    /** nomvar_rea_eof **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "nomvar_rea_eof");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->rea->nomvar_eof = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->rea->nomvar_eof == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->rea->nomvar_eof, (char *) val);
      (void) xmlFree(val);
    }
    else {
      data->learning->rea->nomvar_eof = strdup("psl_pc");
    }
    (void) fprintf(stdout, "%s: Learning nomvar_eof = %s\n", __FILE__, data->learning->obs->nomvar_eof);
    
    /** nomvar_obs_sing **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "nomvar_obs_sing");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->obs->nomvar_sing = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->obs->nomvar_sing == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->obs->nomvar_sing, (char *) val);
      (void) xmlFree(val);
    }
    else
      data->learning->obs->nomvar_sing = strdup("pre_sing");
    (void) fprintf(stdout, "%s: Learning nomvar_obs_sing = %s\n", __FILE__, data->learning->obs->nomvar_sing);

    /** nomvar_rea_sing **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "nomvar_rea_sing");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->rea->nomvar_sing = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->rea->nomvar_sing == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->rea->nomvar_sing, (char *) val);
      (void) xmlFree(val);
    }
    else
      data->learning->rea->nomvar_sing = strdup("pre_sing");
    (void) fprintf(stdout, "%s: Learning nomvar_rea_sing = %s\n", __FILE__, data->learning->rea->nomvar_sing);

    /** nomvar_rea_sup **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "nomvar_rea_sup");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->nomvar_rea_sup = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->nomvar_rea_sup == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->nomvar_rea_sup, (char *) val);
      (void) xmlFree(val);
    }
    else {
      data->learning->nomvar_rea_sup = strdup("tas");
    }
    (void) fprintf(stdout, "%s: Learning nomvar_sup = %s\n", __FILE__, data->learning->nomvar_rea_sup);

    /** rea_coords **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "rea_coords");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->rea_coords = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->rea_coords == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->rea_coords, (char *) val);
      (void) xmlFree(val);
    }
    else {
      data->learning->rea_coords = strdup("1D");
    }
    (void) fprintf(stdout, "%s: Learning rea_coords = %s\n", __FILE__, data->learning->rea_coords);

    /** rea_gridname **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "rea_gridname");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->rea_gridname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->rea_gridname == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->rea_gridname, (char *) val);
      (void) xmlFree(val);
    }
    else {
      data->learning->rea_gridname = strdup("Latitude_Longitude");
    }
    (void) fprintf(stdout, "%s: Learning rea_gridname = %s\n", __FILE__, data->learning->rea_gridname);

    /** rea_dimxname **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "rea_dimx_name");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->rea_dimxname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->rea_dimxname == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->rea_dimxname, (char *) val);
      (void) xmlFree(val);
    }
    else {
      data->learning->rea_dimxname = strdup("lon");
    }
    (void) fprintf(stdout, "%s: Learning rea_dimx_name = %s\n", __FILE__, data->learning->rea_dimxname);

    /** rea_dimyname **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "rea_dimy_name");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->rea_dimyname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->rea_dimyname == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->rea_dimyname, (char *) val);
      (void) xmlFree(val);
    }
    else {
      data->learning->rea_dimyname = strdup("lat");
    }
    (void) fprintf(stdout, "%s: Learning rea_latitude_name = %s\n", __FILE__, data->learning->rea_dimyname);

    /** rea_lonname **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "rea_longitude_name");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->rea_lonname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->rea_lonname == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->rea_lonname, (char *) val);
      (void) xmlFree(val);
    }
    else {
      data->learning->rea_lonname = strdup("lon");
    }
    (void) fprintf(stdout, "%s: Learning rea_longitude_name = %s\n", __FILE__, data->learning->rea_lonname);

    /** rea_latname **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "rea_latitude_name");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->rea_latname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->rea_latname == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->rea_latname, (char *) val);
      (void) xmlFree(val);
    }
    else {
      data->learning->rea_latname = strdup("lat");
    }
    (void) fprintf(stdout, "%s: Learning rea_latitude_name = %s\n", __FILE__, data->learning->rea_latname);

    /** rea_timename **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "rea_time_name");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->rea_timename = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->rea_timename == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->rea_timename, (char *) val);
      (void) xmlFree(val);
    }
    else {
      data->learning->rea_timename = strdup("time");
    }
    (void) fprintf(stdout, "%s: Learning rea_time_name = %s\n", __FILE__, data->learning->rea_timename);

    /** obs_dimxname **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "obs_dimx_name");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->obs_dimxname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->obs_dimxname == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->obs_dimxname, (char *) val);
      (void) xmlFree(val);
    }
    else {
      data->learning->obs_dimxname = strdup("lon");
    }
    (void) fprintf(stdout, "%s: Learning obs_dimx_name = %s\n", __FILE__, data->learning->obs_dimxname);

    /** obs_dimyname **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "obs_dimy_name");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->obs_dimyname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->obs_dimyname == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->obs_dimyname, (char *) val);
      (void) xmlFree(val);
    }
    else {
      data->learning->obs_dimyname = strdup("lat");
    }
    (void) fprintf(stdout, "%s: Learning obs_dimy_name = %s\n", __FILE__, data->learning->obs_dimyname);

    /** obs_lonname **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "obs_longitude_name");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->obs_lonname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->obs_lonname == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->obs_lonname, (char *) val);
      (void) xmlFree(val);
    }
    else {
      data->learning->obs_lonname = strdup("lon");
    }
    (void) fprintf(stdout, "%s: Learning obs_longitude_name = %s\n", __FILE__, data->learning->obs_lonname);

    /** obs_latname **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "obs_latitude_name");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->obs_latname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->obs_latname == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->obs_latname, (char *) val);
      (void) xmlFree(val);
    }
    else {
      data->learning->obs_latname = strdup("lat");
    }
    (void) fprintf(stdout, "%s: Learning obs_latitude_name = %s\n", __FILE__, data->learning->obs_latname);

    /** obs_timename **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "obs_time_name");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->obs_timename = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->obs_timename == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->obs_timename, (char *) val);
      (void) xmlFree(val);
    }
    else {
      data->learning->obs_timename = strdup("time");
    }
    (void) fprintf(stdout, "%s: Learning obs_time_name = %s\n", __FILE__, data->learning->obs_timename);

    /** obs_eofname **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "obs_eof_name");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->learning->obs_eofname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->learning->obs_eofname == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->learning->obs_eofname, (char *) val);
      (void) xmlFree(val);
    }
    else {
      data->learning->obs_eofname = strdup("eof");
    }
    (void) fprintf(stdout, "%s: Learning obs_eof_name = %s\n", __FILE__, data->learning->obs_eofname);
  }

  /** sup_lonname **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "sup_lonname");
  val = xml_get_setting(conf, path);
  if (val != NULL) {
    data->learning->sup_lonname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
    if (data->learning->sup_lonname == NULL) alloc_error(__FILE__, __LINE__);
    (void) strcpy(data->learning->sup_lonname, (char *) val);
    (void) xmlFree(val);
  }
  else
    data->learning->sup_lonname = strdup("lon");
  (void) fprintf(stdout, "%s: Learning sup_lonname = %s\n", __FILE__, data->learning->sup_lonname);

  /** sup_latname **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "sup_latname");
  val = xml_get_setting(conf, path);
  if (val != NULL) {
    data->learning->sup_latname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
    if (data->learning->sup_latname == NULL) alloc_error(__FILE__, __LINE__);
    (void) strcpy(data->learning->sup_latname, (char *) val);
    (void) xmlFree(val);
  }
  else
    data->learning->sup_latname = strdup("lat");
  (void) fprintf(stdout, "%s: Learning sup_latname = %s\n", __FILE__, data->learning->sup_latname);

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
  
  /** nomvar_precip_index_obs **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "nomvar_precip_index_obs");
  val = xml_get_setting(conf, path);
  if (val != NULL) {
    data->learning->nomvar_precip_index_obs = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
    if (data->learning->nomvar_precip_index_obs == NULL) alloc_error(__FILE__, __LINE__);
    (void) strcpy(data->learning->nomvar_precip_index_obs, (char *) val);
    (void) xmlFree(val);
  }
  else
    data->learning->nomvar_precip_index_obs = strdup("rro");
  (void) fprintf(stdout, "%s: Learning nomvar_precip_index_obs = %s\n", __FILE__, data->learning->nomvar_precip_index_obs);
  
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

  /** nomvar_sup_val **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "learning", "nomvar_sup_val");
  val = xml_get_setting(conf, path);
  if (val != NULL) {
    data->learning->nomvar_sup_val = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
    if (data->learning->nomvar_sup_val == NULL) alloc_error(__FILE__, __LINE__);
    (void) strcpy(data->learning->nomvar_sup_val, (char *) val);
    (void) xmlFree(val);
  }
  else
    data->learning->nomvar_sup_val = strdup("tad");
  (void) fprintf(stdout, "%s: Learning nomvar_sup_val = %s\n", __FILE__, data->learning->nomvar_sup_val);

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

    /** dimxname **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "regression", "dimx_name");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->reg->dimxname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->reg->dimxname == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->reg->dimxname, (char *) val);
      (void) fprintf(stdout, "%s: Regression points dimx_name = %s\n", __FILE__, data->reg->dimxname);
      (void) xmlFree(val);
    }
    else {
      data->reg->dimxname = strdup("lon");
      (void) fprintf(stderr, "%s: Default regression points dimx_name setting = %s.\n", __FILE__, data->reg->dimxname);
      (void) xmlFree(val);
    }
    /** dimyname **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "regression", "dimy_name");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->reg->dimyname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->reg->dimyname == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->reg->dimyname, (char *) val);
      (void) fprintf(stdout, "%s: Regression points dimy_name = %s\n", __FILE__, data->reg->dimyname);
      (void) xmlFree(val);
    }
    else {
      data->reg->dimyname = strdup("dimy");
      (void) fprintf(stderr, "%s: Default regression points dimy_name setting = %s.\n", __FILE__, data->reg->dimyname);
      (void) xmlFree(val);
    }
    /** lonname **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "regression", "longitude_name");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->reg->lonname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->reg->lonname == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->reg->lonname, (char *) val);
      (void) fprintf(stdout, "%s: Regression points longitude_name = %s\n", __FILE__, data->reg->lonname);
      (void) xmlFree(val);
    }
    else {
      data->reg->lonname = strdup("lon");
      (void) fprintf(stderr, "%s: Default regression points longitude_name setting = %s.\n", __FILE__, data->reg->lonname);
      (void) xmlFree(val);
    }
    /** latname **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "regression", "latitude_name");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->reg->latname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->reg->latname == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->reg->latname, (char *) val);
      (void) fprintf(stdout, "%s: Regression points latitude_name = %s\n", __FILE__, data->reg->latname);
      (void) xmlFree(val);
    }
    else {
      data->reg->latname = strdup("lat");
      (void) fprintf(stderr, "%s: Default regression points latitude_name setting = %s.\n", __FILE__, data->reg->latname);
      (void) xmlFree(val);
    }
    /** ptsname **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "regression", "pts_name");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->reg->ptsname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->reg->ptsname == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->reg->ptsname, (char *) val);
      (void) fprintf(stdout, "%s: Regression points pts_name = %s\n", __FILE__, data->reg->ptsname);
      (void) xmlFree(val);
    }
    else {
      data->reg->ptsname = strdup("pts");
      (void) fprintf(stderr, "%s: Default regression points pts_name setting = %s.\n", __FILE__, data->reg->ptsname);
      (void) xmlFree(val);
    }
    /** dist **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "regression", "distance");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->reg->dist = xmlXPathCastStringToNumber(val);
      (void) xmlFree(val);
      (void) fprintf(stdout, "%s: Regression distance in meters for spatial mean = %lf\n", __FILE__, data->reg->dist);
    }
    else {
      data->reg->dist = 40000.0;
      (void) fprintf(stdout, "%s: Regression distance in meters for spatial mean = %lf.\n", __FILE__, data->reg->dist);
    }
    /** regression_save **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "regression", "regression_save");
    val = xml_get_setting(conf, path);
    if (val != NULL) 
      data->reg->reg_save = (int) strtol((char *) val, (char **)NULL, 10);
    else
      data->reg->reg_save = FALSE;
    if (data->reg->reg_save != FALSE && data->reg->reg_save != TRUE) {
      (void) fprintf(stderr, "%s: Invalid regression_save value %s in configuration file. Aborting.\n", __FILE__, val);
      return -1;
    }
    (void) fprintf(stdout, "%s: regression_save=%d\n", __FILE__, data->reg->reg_save);
    if (val != NULL) 
      (void) xmlFree(val);
  }
  else {
    (void) fprintf(stderr, "%s: No regression points. Cannot perform learning or downscale. Can just output data given analog days.\n",
                   __FILE__);
    data->reg->filename = NULL;
    (void) xmlFree(val);
  }

  /* If regression data is saved, additional parameters are needed */
  if (data->reg->reg_save == TRUE) {
    /** filename_save_ctrl_reg **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "regression", "filename_save_ctrl_reg");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->reg->filename_save_ctrl_reg = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->reg->filename_save_ctrl_reg == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->reg->filename_save_ctrl_reg, (char *) val);
      (void) fprintf(stdout, "%s: Regression filename_save_ctrl_reg = %s\n", __FILE__, data->reg->filename_save_ctrl_reg);
      (void) xmlFree(val);
    }
    else {
      (void) fprintf(stderr, "%s: Missing regression filename_save_ctrl_reg setting. Aborting.\n", __FILE__);
      (void) xmlFree(val);
      return -1;
    }
    /** filename_save_other_reg **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "regression", "filename_save_other_reg");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->reg->filename_save_other_reg = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->reg->filename_save_other_reg == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->reg->filename_save_other_reg, (char *) val);
      (void) fprintf(stdout, "%s: Regression filename_save_other_reg = %s\n", __FILE__, data->reg->filename_save_other_reg);
      (void) xmlFree(val);
    }
    else {
      (void) fprintf(stderr, "%s: Missing regression filename_save_other_reg setting. Aborting.\n", __FILE__);
      (void) xmlFree(val);
      return -1;
    }

    /** timename **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "regression", "time_name");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->reg->timename = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->reg->timename == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->reg->timename, (char *) val);
      (void) xmlFree(val);
    }
    else
      data->reg->timename = strdup("time");
    (void) fprintf(stdout, "%s: Regression time dimension name = %s\n", __FILE__, data->reg->timename);
  }

  /**** LARGE-SCALE FIELDS CONFIGURATION ****/

  /** number_of_large_scale_fields **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "number_of_large_scale_fields");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->field[0].n_ls = (int) strtol((char *) val, (char **)NULL, 10);
  else {
    (void) fprintf(stderr, "%s: Missing or invalid number_of_large_scale_fields setting. Aborting.\n", __FILE__);
    return -1;
  }
  if (val != NULL)
    (void) xmlFree(val);    
  //  if (data->field[0].n_ls == 0) {
  //  (void) fprintf(stderr, "%s: number_of_large_scale_fields cannot be 0. Aborting.\n", __FILE__);
  //  return -1;
  // }

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
  if (data->field[1].n_ls == 0) {
    (void) fprintf(stderr, "%s: number_of_large_scale_control_fields cannot be 0. Aborting.\n", __FILE__);
    return -1;
  }

  /** number_of_secondary_large_scale_fields **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "number_of_secondary_large_scale_fields");
  val = xml_get_setting(conf, path);
  if (val != NULL)
    data->field[2].n_ls = (int) strtol((char *) val, (char **)NULL, 10);
  else
    data->field[2].n_ls = 0;
  if (val != NULL)
    (void) xmlFree(val);    
  if (data->field[2].n_ls == 0) {
    (void) fprintf(stderr, "%s: number_of_secondary_large_scale_fields cannot be 0. Aborting.\n", __FILE__);
    return -1;
  }

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
  if (data->field[3].n_ls == 0) {
    (void) fprintf(stderr, "%s: number_of_secondary_large_scale_control_fields cannot be 0. Aborting.\n", __FILE__);
    return -1;
  }

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

        data->field[i].proj[j].grid_mapping_name = NULL;
        data->field[i].proj[j].name = NULL;
        data->field[i].proj[j].coords = NULL;
        
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

        (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "dimy_name", i+1);
        val = xml_get_setting(conf, path);
        if (val != NULL) {
          data->field[cat].data[i].dimyname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));          
          if (data->field[cat].data[i].dimyname == NULL) alloc_error(__FILE__, __LINE__);
          (void) strcpy(data->field[cat].data[i].dimyname, (char *) val);
          (void) xmlFree(val);
        }
        else
          data->field[cat].data[i].dimyname = strdup("lat");

        (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "dimx_name", i+1);
        val = xml_get_setting(conf, path);
        if (val != NULL) {
          data->field[cat].data[i].dimxname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));          
          if (data->field[cat].data[i].dimxname == NULL) alloc_error(__FILE__, __LINE__);
          (void) strcpy(data->field[cat].data[i].dimxname, (char *) val);
          (void) xmlFree(val);
        }
        else
          data->field[cat].data[i].dimxname = strdup("lon");

        (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "latitude_name", i+1);
        val = xml_get_setting(conf, path);
        if (val != NULL) {
          data->field[cat].data[i].latname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));          
          if (data->field[cat].data[i].latname == NULL) alloc_error(__FILE__, __LINE__);
          (void) strcpy(data->field[cat].data[i].latname, (char *) val);
          (void) xmlFree(val);
        }
        else
          data->field[cat].data[i].latname = strdup("lat");

        (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "longitude_name", i+1);
        val = xml_get_setting(conf, path);
        if (val != NULL) {
          data->field[cat].data[i].lonname = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));          
          if (data->field[cat].data[i].lonname == NULL) alloc_error(__FILE__, __LINE__);
          (void) strcpy(data->field[cat].data[i].lonname, (char *) val);
          (void) xmlFree(val);
        }
        else
          data->field[cat].data[i].lonname = strdup("lon");

        (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "time_name", i+1);
        val = xml_get_setting(conf, path);
        if (val != NULL) {
          data->field[cat].data[i].timename = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));          
          if (data->field[cat].data[i].timename == NULL) alloc_error(__FILE__, __LINE__);
          (void) strcpy(data->field[cat].data[i].timename, (char *) val);
          (void) xmlFree(val);
        }
        else
          data->field[cat].data[i].timename = strdup("time");

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
          data->field[cat].data[i].clim_info->clim_remove = FALSE;
        (void) fprintf(stdout, "%s: clim_remove = %d\n", __FILE__, data->field[cat].data[i].clim_info->clim_remove);
        if (val != NULL)
          (void) xmlFree(val);    

        /** clim_provided **/
        (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "clim_provided", i+1);
        val = xml_get_setting(conf, path);
        if (val != NULL) {
          if ( !xmlStrcmp(val, (xmlChar *) "1") )
            data->field[cat].data[i].clim_info->clim_provided = TRUE;
          else
            data->field[cat].data[i].clim_info->clim_provided = FALSE;
          (void) fprintf(stdout, "%s: clim_provided #%d = %d\n", __FILE__, i+1, data->field[cat].data[i].clim_info->clim_provided);
          (void) xmlFree(val);

          /* If climatology is provided, additional parameters are needed */
          if (data->field[cat].data[i].clim_info->clim_provided == TRUE) {

            /** clim_openfilename **/
            (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "clim_openfilename",i+1);
            val = xml_get_setting(conf, path);
            if (val != NULL) {
              data->field[cat].data[i].clim_info->clim_filein_ls = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
              if (data->field[cat].data[i].clim_info->clim_filein_ls == NULL) alloc_error(__FILE__, __LINE__);
              (void) strcpy(data->field[cat].data[i].clim_info->clim_filein_ls, (char *) val);
              (void) fprintf(stdout, "%s:  Climatology input filename #%d = %s\n", __FILE__, i+1,
                             data->field[cat].data[i].clim_info->clim_filein_ls);
              (void) xmlFree(val);
            }
            else {
              (void) fprintf(stderr, "%s: Missing clim_openfilename setting %s. Aborting.\n", __FILE__, catstrt);
              return -1;
            }
          }
        }
        else
          data->field[cat].data[i].clim_info->clim_provided = FALSE;

        /** clim_save **/
        (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "clim_save", i+1);
        val = xml_get_setting(conf, path);
        if (val != NULL) {
          if ( !xmlStrcmp(val, (xmlChar *) "1") )
            data->field[cat].data[i].clim_info->clim_save = TRUE;
          else
            data->field[cat].data[i].clim_info->clim_save = FALSE;
          (void) fprintf(stdout, "%s: clim_save #%d = %d\n", __FILE__, i+1, data->field[cat].data[i].clim_info->clim_save);
          (void) xmlFree(val);

          /* If we want to save climatology in output file */
          if (data->field[cat].data[i].clim_info->clim_save == TRUE) {

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
          if (data->field[cat].data[i].clim_info->clim_save == TRUE || data->field[cat].data[i].clim_info->clim_provided == TRUE) {
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
          data->field[cat].data[i].clim_info->clim_save = FALSE;

        /** EOF and Singular values **/
        /** eof_project **/
        (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr, "eof_project", i+1);
        val = xml_get_setting(conf, path);
        if (val != NULL)
          data->field[cat].data[i].eof_info->eof_project = (int) xmlXPathCastStringToNumber(val);
        else
          data->field[cat].data[i].eof_info->eof_project = FALSE;
        (void) fprintf(stdout, "%s: eof_project = %d\n", __FILE__, data->field[cat].data[i].eof_info->eof_project);
        if (val != NULL)
          (void) xmlFree(val);    
      
        if (data->field[cat].data[i].eof_info->eof_project == TRUE) {
          /** number_of_eofs **/
          (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s[@id=\"%d\"]", "setting", catstr,
                         "number_of_eofs", i+1);
          val = xml_get_setting(conf, path);
          if (val != NULL) {
            data->field[cat].data[i].eof_info->neof_ls = (int) xmlXPathCastStringToNumber(val);
            if (data->field[cat].data[i].eof_info->neof_ls != data->learning->rea_neof) {
              (void) fprintf(stderr,
                             "%s: Fatal error in configuration. The number of eof for field #%d of category %d is %d and the corresponding learning number of eof is %d. They should be equal!! Aborting.\n",
                             __FILE__, i, cat, data->field[cat].data[i].eof_info->neof_ls, data->learning->rea_neof);
              return -1;
            }
            (void) fprintf(stdout, "%s: number_of_eofs = %d\n", __FILE__, data->field[cat].data[i].eof_info->neof_ls);
            (void) xmlFree(val);
          }
          
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
    }
    (void) free(catstr);
    (void) free(catstrt);
  }


  /**** CONTROL-RUN PERIOD CONFIGURATION ****/

  /** downscale **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "period_ctrl", "downscale");
  val = xml_get_setting(conf, path);
  if (val != NULL) {
    data->conf->period_ctrl->downscale = xmlXPathCastStringToNumber(val);
    (void) fprintf(stdout, "%s: period_ctrl downscale = %d\n", __FILE__, data->conf->period_ctrl->downscale);
    (void) xmlFree(val);
  }
  else
    data->conf->period_ctrl->downscale = TRUE;
  if (data->conf->period_ctrl->downscale == TRUE) {
    /** year_begin **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s", "setting", "period_ctrl", "period", "year_begin");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->conf->period_ctrl->year_begin = xmlXPathCastStringToNumber(val);
      (void) fprintf(stdout, "%s: period_ctrl year_begin = %d\n", __FILE__, data->conf->period_ctrl->year_begin);
      (void) xmlFree(val);
    }
    else
      data->conf->period_ctrl->year_begin = -1;
    /** month_begin **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s", "setting", "period_ctrl", "period", "month_begin");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->conf->period_ctrl->month_begin = xmlXPathCastStringToNumber(val);
      (void) fprintf(stdout, "%s: period_ctrl month_begin = %d\n", __FILE__, data->conf->period_ctrl->month_begin);
      (void) xmlFree(val);
    }
    else
      data->conf->period_ctrl->month_begin = -1;
    /** day_begin **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s", "setting", "period_ctrl", "period", "day_begin");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->conf->period_ctrl->day_begin = xmlXPathCastStringToNumber(val);
      (void) fprintf(stdout, "%s: period_ctrl day_begin = %d\n", __FILE__, data->conf->period_ctrl->day_begin);
      (void) xmlFree(val);
    }
    else
      data->conf->period_ctrl->day_begin = -1;
    /** year_end **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s", "setting", "period_ctrl", "period", "year_end");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->conf->period_ctrl->year_end = xmlXPathCastStringToNumber(val);
      (void) fprintf(stdout, "%s: period_ctrl year_end = %d\n", __FILE__, data->conf->period_ctrl->year_end);
      (void) xmlFree(val);
    }
    else
      data->conf->period_ctrl->year_end = -1;
    /** month_end **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s", "setting", "period_ctrl", "period", "month_end");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->conf->period_ctrl->month_end = xmlXPathCastStringToNumber(val);
      (void) fprintf(stdout, "%s: period_ctrl month_end = %d\n", __FILE__, data->conf->period_ctrl->month_end);
      (void) xmlFree(val);
    }
    else
      data->conf->period_ctrl->month_end = -1;
    /** day_end **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s", "setting", "period_ctrl", "period", "day_end");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->conf->period_ctrl->day_end = xmlXPathCastStringToNumber(val);
      (void) fprintf(stdout, "%s: period_ctrl day_end = %d\n", __FILE__, data->conf->period_ctrl->day_end);
      (void) xmlFree(val);
    }
    else
      data->conf->period_ctrl->day_end = -1;
  }
  else {
    data->conf->period_ctrl->year_begin = -1;
    data->conf->period_ctrl->month_begin = -1;
    data->conf->period_ctrl->day_begin = -1;
    data->conf->period_ctrl->year_end = -1;
    data->conf->period_ctrl->month_end = -1;
    data->conf->period_ctrl->day_end = -1;
  }


  /**** PERIODS CONFIGURATION FOR NON-CONTROL ****/

  if (data->field[0].n_ls > 0) {
    data->conf->nperiods = 1;
    data->conf->period = (period_struct *) malloc(sizeof(period_struct));
    if (data->conf->period == NULL) alloc_error(__FILE__, __LINE__);
  }
  else
    data->conf->nperiods = 0;

  /* Loop over periods */
  for (i=0; i<data->conf->nperiods; i++) {
    /** downscale **/
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s", "setting", "period", "downscale");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->conf->period[i].downscale = xmlXPathCastStringToNumber(val);
      (void) fprintf(stdout, "%s: period downscale = %d\n", __FILE__, data->conf->period[i].downscale);
      (void) xmlFree(val);
    }
    else
      data->conf->period[i].downscale = TRUE;
    if (data->conf->period[i].downscale == TRUE) {
      /** year_begin **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s", "setting", "period", "period", "year_begin");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->period[i].year_begin = xmlXPathCastStringToNumber(val);
        (void) xmlFree(val);
        (void) fprintf(stdout, "%s: period #%d year_begin = %d\n", __FILE__, i+1, data->conf->period[i].year_begin);
      }
      else
        data->conf->period[i].year_begin = -1;
      /** month_begin **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s", "setting", "period", "period", "month_begin");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->period[i].month_begin = xmlXPathCastStringToNumber(val);
        (void) xmlFree(val);
        (void) fprintf(stdout, "%s: period #%d month_begin = %d\n", __FILE__, i+1, data->conf->period[i].month_begin);
      }
      else
        data->conf->period[i].month_begin = -1;
      /** day_begin **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s", "setting", "period", "period", "day_begin");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->period[i].day_begin = xmlXPathCastStringToNumber(val);
        (void) xmlFree(val);
        (void) fprintf(stdout, "%s: period #%d day_begin = %d\n", __FILE__, i+1, data->conf->period[i].day_begin);
      }
    else
      data->conf->period[i].day_begin = -1;
      /** year_end **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s", "setting", "period", "period", "year_end");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->period[i].year_end = xmlXPathCastStringToNumber(val);
        (void) xmlFree(val);
        (void) fprintf(stdout, "%s: period #%d year_end = %d\n", __FILE__, i+1, data->conf->period[i].year_end);
      }
      else
        data->conf->period[i].year_end = -1;
      /** month_end **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s", "setting", "period", "period", "month_end");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->period[i].month_end = xmlXPathCastStringToNumber(val);
        (void) xmlFree(val);
        (void) fprintf(stdout, "%s: period #%d month_end = %d\n", __FILE__, i+1, data->conf->period[i].month_end);
      }
      else
        data->conf->period[i].month_end = -1;
      /** day_end **/
      (void) sprintf(path, "/configuration/%s[@name=\"%s\"]/%s/%s", "setting", "period", "period", "day_end");
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->period[i].day_end = xmlXPathCastStringToNumber(val);
        (void) xmlFree(val);
        (void) fprintf(stdout, "%s: period #%d day_end = %d\n", __FILE__, i+1, data->conf->period[i].day_end);
      }
      else
        data->conf->period[i].day_end = -1;
    }
    else {
      data->conf->period[i].year_begin = -1;
      data->conf->period[i].month_begin = -1;
      data->conf->period[i].day_begin = -1;
      data->conf->period[i].year_end = -1;
      data->conf->period[i].month_end = -1;
      data->conf->period[i].day_end = -1;
    }
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
          data->field[cat].data[i].down->sup_val_norm = (double **) malloc(data->conf->nseasons * sizeof(double));
          if (data->field[cat].data[i].down->sup_val_norm == NULL) alloc_error(__FILE__, __LINE__);
          /* Only needed for secondary large-scale control field */
          if (cat == 3) {
            data->field[cat].data[i].down->smean_2d = (double **) malloc(data->conf->nseasons * sizeof(double));
            if (data->field[cat].data[i].down->smean_2d == NULL) alloc_error(__FILE__, __LINE__);
            data->field[cat].data[i].down->svar_2d = (double **) malloc(data->conf->nseasons * sizeof(double));
            if (data->field[cat].data[i].down->svar_2d == NULL) alloc_error(__FILE__, __LINE__);
          }
        }
      }
    }

    /* Loop over seasons: season-dependent parameters */
    for (i=0; i<data->conf->nseasons; i++) {

      data->learning->data[i].time_s = (time_vect_struct *) malloc(sizeof(time_vect_struct));
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

      if ( ! ((data->conf->season[i].nreg == data->conf->season[i].nclusters) ||
              ( data->conf->season[i].nreg == data->conf->season[i].nclusters+1 ) ) ) {
        (void) fprintf(stderr, "%s: For season=%d, invalid correspondence between number_of_clusters=%d and number_of_regression_vars=%d. number_of_regression_vars should be equal to number_of_clusters or number_of_clusters+1 (temperature as supplemental regression variable). Aborting.\n",
                       __FILE__, i, data->conf->season[i].nclusters, data->conf->season[i].nreg);
        return -1;
      }

      /** number_of_days_search **/
      (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "number_of_days_search", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->season[i].ndays = xmlXPathCastStringToNumber(val);
        (void) fprintf(stdout, "%s: season #%d number_of_days_search = %d\n", __FILE__, i+1, data->conf->season[i].ndays);
        (void) xmlFree(val);
      }
      else
        data->conf->season[i].ndays = 10;

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
          data->conf->season[i].ndayschoices = 16;
        else
          data->conf->season[i].ndayschoices = 11;

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
          data->conf->season[i].shuffle = TRUE;
        else
          data->conf->season[i].shuffle = FALSE;

      /** secondary_field_choice **/
      (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "secondary_field_choice", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->season[i].secondary_choice = xmlXPathCastStringToNumber(val);
        (void) fprintf(stdout, "%s: season #%d secondary_field_choice = %d\n", __FILE__, i+1, data->conf->season[i].secondary_choice);
        (void) xmlFree(val);
      }
      else
        if (i == 0 || i == 1)
          data->conf->season[i].secondary_choice = FALSE;
        else
          data->conf->season[i].secondary_choice = TRUE;

      /** secondary_field_main_choice **/
      (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "secondary_field_main_choice", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->season[i].secondary_main_choice = xmlXPathCastStringToNumber(val);
        (void) fprintf(stdout, "%s: season #%d secondary_field_main_choice = %d\n", __FILE__, i+1, data->conf->season[i].secondary_main_choice);
        (void) xmlFree(val);
      }
      else
        if (data->conf->season[i].secondary_choice == FALSE)
          data->conf->season[i].secondary_main_choice = TRUE;
        else
          data->conf->season[i].secondary_main_choice = FALSE;

      /** secondary_covariance **/
      (void) sprintf(path, "/configuration/%s/%s[@id=\"%d\"]", "setting", "secondary_covariance", i+1);
      val = xml_get_setting(conf, path);
      if (val != NULL) {
        data->conf->season[i].secondary_cov = xmlXPathCastStringToNumber(val);
        (void) fprintf(stdout, "%s: season #%d secondary_covariance = %d\n", __FILE__, i+1, data->conf->season[i].secondary_cov);
        (void) xmlFree(val);
      }
      else
        data->conf->season[i].secondary_cov = FALSE;

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

  /**** ANALOG DATA CONFIGURATION ****/

  /** output_only **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "output_only");
  val = xml_get_setting(conf, path);
  if (val != NULL) 
    data->conf->output_only = (int) strtol((char *) val, (char **)NULL, 10);
  else
    data->conf->output_only = FALSE;
  if (data->conf->output_only != FALSE && data->conf->output_only != TRUE) {
    (void) fprintf(stderr, "%s: Invalid or missing analog data output_only value %s in configuration file. Aborting.\n", __FILE__, val);
    return -1;
  }
  (void) fprintf(stdout, "%s: analog data output_only=%d\n", __FILE__, data->conf->output_only);
  if (val != NULL) 
    (void) xmlFree(val);
  if (data->conf->output_only == TRUE) {
    if (data->learning->learning_provided == FALSE) {
      (void) fprintf(stderr, "%s: WARNING: Desactivating learning process because option for output only has been set!\n", __FILE__);
      data->learning->learning_provided = TRUE;
    }
    if (data->learning->learning_save == TRUE) {
      (void) fprintf(stderr, "%s: WARNING: Desactivating learning save process because option for output only has been set!\n", __FILE__);
      data->learning->learning_save = FALSE;
    }
    if (data->conf->learning_maskfile->use_mask == TRUE) {
      (void) fprintf(stderr, "%s: WARNING: Desactivating use_mask for learning because option for output only has been set!\n", __FILE__);
      data->conf->learning_maskfile->use_mask = FALSE;
    }
    if (data->reg->reg_save == TRUE) {
      (void) fprintf(stderr, "%s: WARNING: Desactivating regression save process because option for output only has been set!\n", __FILE__);
      data->reg->reg_save = FALSE;
    }
    if (data->secondary_mask->use_mask == TRUE) {
      (void) fprintf(stderr, "%s: WARNING: Desactivating use_mask for secondary large-scale fields because option for output only has been set!\n", __FILE__);
      data->secondary_mask->use_mask = FALSE;
    }
  }

  /** analog_save **/
  (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "analog_save");
  val = xml_get_setting(conf, path);
  if (val != NULL) 
    data->conf->analog_save = (int) strtol((char *) val, (char **)NULL, 10);
  else
    data->conf->analog_save = FALSE;
  if (data->conf->analog_save != FALSE && data->conf->analog_save != TRUE) {
    (void) fprintf(stderr, "%s: Invalid or missing analog data analog_save value %s in configuration file. Aborting.\n", __FILE__, val);
    return -1;
  }
  (void) fprintf(stdout, "%s: analog data analog_save=%d\n", __FILE__, data->conf->analog_save);
  if (val != NULL) 
    (void) xmlFree(val);

  /** analog_file_ctrl **/
  if ( (data->conf->analog_save == TRUE || data->conf->output_only == TRUE) && data->conf->period_ctrl->downscale == TRUE) {
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "analog_file_ctrl");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->conf->analog_file_ctrl = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->conf->analog_file_ctrl == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->conf->analog_file_ctrl, (char *) val);
      (void) fprintf(stdout, "%s: analog_file_ctrl = %s\n", __FILE__, data->conf->analog_file_ctrl);
      (void) xmlFree(val);
    }
    else {
      (void) fprintf(stderr, "%s: Missing analog_file_ctrl setting. Aborting.\n", __FILE__);
      (void) xmlFree(val);
      return -1;
    }
  }

  /** analog_file_other **/
  if (data->conf->analog_save == TRUE || data->conf->output_only == TRUE) {
    (void) sprintf(path, "/configuration/%s[@name=\"%s\"]", "setting", "analog_file_other");
    val = xml_get_setting(conf, path);
    if (val != NULL) {
      data->conf->analog_file_other = (char *) malloc((xmlStrlen(val)+1) * sizeof(char));
      if (data->conf->analog_file_other == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(data->conf->analog_file_other, (char *) val);
      (void) fprintf(stdout, "%s: analog_file_other = %s\n", __FILE__, data->conf->analog_file_other);
      (void) xmlFree(val);
    }
    else {
      (void) fprintf(stderr, "%s: Missing analog_file_other setting. Aborting.\n", __FILE__);
      (void) xmlFree(val);
      return -1;
    }
  }


  /* Free memory */
  (void) xml_free_config(conf);
  (void) xmlCleanupParser();
  (void) free(path);

  /* Success status */
  return 0;
}
