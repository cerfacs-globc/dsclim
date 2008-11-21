/* ***************************************************** */
/* Free memory for an xmlConfig_t structure.             */
/* xml_free_config.c                                     */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file xml_free_config.c
    \brief Free memory for an xmlConfig_t structure.
*/

#include <xml_utils.h>

/** Free memory for an xmlConfig_t structure. */
void xml_free_config(xmlConfig_t *conf) {
  /**
     @param[in]  conf    XML information for DOM and XPath
   */

  (void) xmlXPathFreeContext(conf->ctxt);
  (void) xmlFreeDoc(conf->doc);
  (void) free(conf->file);
  (void) free(conf);

}
