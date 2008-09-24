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

char *xml_get_setting(xmlConfig_t *conf, char *path) {

  xmlNodePtr nodeptr;

  nodeptr = xml_get_node_by_xpath(conf, path);
  
  if (nodeptr != NULL) {
    return xmlNodeGetContent(nodeptr);
  }
  else
    return NULL;
}
