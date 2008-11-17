int comparf(const void *a, const void *b)
{
  /* Compare two float values to sort descending */

  float *n1 = (float *) a;
  float *n2 = (float *) b;

  if (*n1 < *n2)
    return 1;
  else if (*n1 > *n2)
    return -1;
  else
    return 0;
}
