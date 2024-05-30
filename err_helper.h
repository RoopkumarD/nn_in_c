#ifndef ERR_HELPER

#define LINE_FILE_PRINT(offset)                                                \
  (fprintf(stderr, "%s: Line %d\n", __FILE__, __LINE__ - offset))

#endif // !ERR_HELPER
