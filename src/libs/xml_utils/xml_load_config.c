/* ***************************************************** */
/* Read an XML file into memory.                         */
/* xml_load_config.c                                     */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file xml_load_config.c
    \brief Read an XML file into memory.
*/

#include <xml_utils.h>

/** Read an XML file into memory. */
xmlConfig_t *xml_load_config(char *filename) {
  /**
     @param[in]  filename    Input XML filename
     
     \return     XML information about DOM and XPath
  */

  xmlConfig_t *conf = NULL;

  conf = (void *) malloc(sizeof(xmlConfig_t));
  if (conf == NULL) alloc_error(__FILE__, __LINE__);
 
  /* Copy filename */
  conf->file = strdup(filename);
  if (conf->file == NULL) alloc_error(__FILE__, __LINE__);

  /* Create DOM tree from XML file */
  (void) xmlKeepBlanksDefault(0);
  conf->doc = xmlParseFile(conf->file);
  if (conf->doc == NULL) {
    (void) xml_free_config(conf);
    return NULL;
  }

  /* Get root */
  conf->root = xmlDocGetRootElement(conf->doc);
  if (conf->root != NULL && xmlStrcasecmp(conf->root->name, (unsigned const char *) "configuration")) {
    (void) xml_free_config(conf);
    return NULL;
  }

  /* Initialize XPath environment */
  (void) xmlXPathInit();

  /* Create XPath Context */
  conf->ctxt = xmlXPathNewContext(conf->doc);
  if (conf->ctxt == NULL) {
    (void) xml_free_config(conf);
    return NULL;
  }

  return conf;
}
