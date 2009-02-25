/* ***************************************************** */
/* Get XML setting element node.                         */
/* xml_get_node_by_xpath.c                               */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file xml_get_node_by_xpath.c
    \brief Get XML setting element node.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2009)

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

#include <xml_utils.h>

/** Get XML setting element node. */
xmlNodePtr
xml_get_node_by_xpath(xmlConfig_t *conf, char *path) {
  /**
     @param[in]  conf    XML information for DOM and XPath
     @param[in]  path    path for XPath

     \return     DOM Node pointer
   */
  
  xmlXPathObjectPtr xpathRes;
  xmlNodePtr nodeptr = NULL;
 
  /* Evaluate XPath expression */
  xpathRes = xmlXPathEvalExpression((unsigned char *) path, conf->ctxt);

  if (xpathRes)
    if (xpathRes->nodesetval) {

      if (xpathRes->type == XPATH_NODESET && xpathRes->nodesetval->nodeNr == 1)
        nodeptr = xpathRes->nodesetval->nodeTab[0];      
      (void) xmlXPathFreeObject(xpathRes);

    }
  
  return nodeptr;
}
