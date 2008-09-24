#include <utils.h>

/** Allocate memory using mmap for a long int array. */
void alloc_mmap_longint(long int **map, int *fd, size_t *byte_size, char *filename, size_t page_size, int size) {
  /**
     @param[in,out]  map        Pointer to a long int array.
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
    (void) perror("alloc_mmap_longint: ERROR: Error opening file for writing");
    (void) kill(getpid(), 5);
  }
  
  total_size = size * sizeof(long int);
  *byte_size = total_size / page_size * page_size + page_size;

  /* Stretch the file size to the size of the (mmapped) array */
  result = ftruncate(*fd, (off_t) *byte_size);
  if (result != 0) {
    (void) close(*fd);
    (void) perror("alloc_mmap_longint: ERROR: Error calling ftruncate() to 'stretch' the file");
    (void) kill(getpid(), 5);
  }
  
  /* Now the file is ready to be mmapped. */
  *map = (long int *) mmap(NULL, *byte_size, ( PROT_READ | PROT_WRITE ), MAP_SHARED, *fd, 0);
  if (*map == (long int *) MAP_FAILED) {
    (void) close(*fd);
    (void) perror("alloc_mmap_longint: ERROR: Error mmapping the file");
    (void) kill(getpid(), 5);
  }
}
