/* ***************************************************** */
/* Show XML setting element value.                       */
/* xml_print_setting.c                                   */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file xml_print_setting.c
    \brief Show XML setting element value.
*/

#include <xml_utils.h>

/** Show XML setting element value. */
void xml_print_setting(xmlConfig_t *conf, char *path) {
  /**
     @param[in]  conf    XML information for DOM and XPath
     @param[in]  path    path for XPath
   */

  xmlChar *val; /* Value */

  /* Get value using XPath */
  val = xml_get_setting(conf, path);
 
  (void) printf("Value of '%s' : %s\n", path, (char *) val != NULL ? (char *) val : "(setting unknown)");
}
