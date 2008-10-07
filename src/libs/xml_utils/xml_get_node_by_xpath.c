/* ***************************************************** */
/* Get XML setting element node.                         */
/* xml_get_node_by_xpath.c                               */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file xml_get_node_by_xpath.c
    \brief Get XML setting element node.
*/

#include <xml_utils.h>

xmlNodePtr xml_get_node_by_xpath(xmlConfig_t *conf, char *path) {
  
  xmlXPathObjectPtr xpathRes;
  xmlNodePtr nodeptr = NULL;
 
  /* Evaluate XPath expression */
  xpathRes = xmlXPathEvalExpression(path, conf->ctxt);

  if (xpathRes)
    if (xpathRes->nodesetval) {

      if (xpathRes->type == XPATH_NODESET && xpathRes->nodesetval->nodeNr == 1)
        nodeptr = xpathRes->nodesetval->nodeTab[0];      
      (void) xmlXPathFreeObject(xpathRes);

    }
  
  return nodeptr;
}
