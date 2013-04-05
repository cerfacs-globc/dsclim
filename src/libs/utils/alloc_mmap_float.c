/* ***************************************************** */
/* Allocate memory using mmap for a float array.         */
/* alloc_mmap_float.c                                    */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file alloc_mmap_float.c
    \brief Allocate memory using mmap for a float array.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2013)

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





#include <utils.h>

/** Allocate memory using mmap for a float array. */
void
alloc_mmap_float(float **map, int *fd, size_t *byte_size, char *filename, size_t page_size, int size) {
  /**
     @param[in,out]  map        Pointer to a floating-point array.
     @param[in]      fd         File unit previously opened with open().
     @param[in]      byte_size  Number of bytes allocated.
     @param[in]      filename   Filename to use to store allocated mmap virtual memory.
     @param[in]      page_size  The paging size of the operating system in bytes.
     @param[in]      size       Number of elements in map array.
  */
  
  int result; /* Return status of functions */
  size_t total_size; /* Total size in bytes, not taking into account the page size */

  /* Open a file for writing.
   *  - Creating the file if it doesn't exist.
   *  - Truncating it to 0 size if it already exists. (not really needed)
   *
   * Note: "O_WRONLY" mode is not sufficient when mmaping.
   */
  *fd = open(filename, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
  if (*fd == -1) {
    (void) perror("alloc_mmap_float: ERROR: Error opening file for writing");
    (void) kill(getpid(), 5);
  }
  
  total_size = size * sizeof(float);
  *byte_size = total_size / page_size * page_size + page_size;

  /* Stretch the file size to the size of the (mmapped) array */
  result = ftruncate(*fd, (off_t) *byte_size);
  if (result != 0) {
    (void) close(*fd);
    (void) perror("alloc_mmap_float: ERROR: Error calling ftruncate() to 'stretch' the file");
    (void) kill(getpid(), 5);
  }
  
  /* Now the file is ready to be mmapped to the array. */
  *map = (float *) mmap(NULL, *byte_size, ( PROT_READ | PROT_WRITE ), MAP_SHARED, *fd, 0);
  if (*map == (float *) MAP_FAILED) {
    (void) close(*fd);
    (void) perror("alloc_mmap_float: ERROR: Error mmapping the file");
    (void) kill(getpid(), 5);
  }
}
