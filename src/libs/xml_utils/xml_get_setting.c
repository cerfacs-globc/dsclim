/* ***************************************************** */
/* Get XML setting element value.                        */
/* xml_get_setting.c                                     */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file xml_get_setting.c
    \brief Get XML setting element value.
*/

#include <xml_utils.h>

/** Get XML setting element value. */
xmlChar *xml_get_setting(xmlConfig_t *conf, char *path) {
  /**
     @param[in]  conf    XML information for DOM and XPath
     @param[in]  path    path for XPath

     \return     XML string value
   */

  xmlNodePtr nodeptr;

  /* Get DOM node pointer using XPath */
  nodeptr = xml_get_node_by_xpath(conf, path);
  
  if (nodeptr != NULL) {
    return xmlNodeGetContent(nodeptr);
  }
  else
    return NULL;
}
