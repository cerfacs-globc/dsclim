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

void xml_print_setting(xmlConfig_t *conf, char *path) {

  const char *val;

  val = xml_get_setting(conf, path);
 
  (void) printf("Value of '%s' : %s\n", path, val != NULL ? val : "(setting unknown)");
}
