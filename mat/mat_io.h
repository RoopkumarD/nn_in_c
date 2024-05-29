#ifndef MAT_IO
#define MAT_IO 1

#include "mat.h"

#define ALIGN_DOWN(base, size) ((base) & -((__typeof__(base))(size)))

#define PTR_ALIGN_DOWN(base, size)                                             \
  ((__typeof__(base))ALIGN_DOWN((uintptr_t)(base), (size)))

// validating if string given in char_start is valid 0.00 or 0 string
#define VALIDATE_ZERO_STRING(char_start, i, temp)                              \
  int invalid_string = 0;                                                      \
  int got_dot = 0;                                                             \
  int end_posn = (buffer[i - 1] == '\r') ? (i - 1) : i;                        \
  for (int zi = char_start; zi < end_posn; zi++) {                             \
    if (buffer[zi] != '0' && buffer[zi] != '.') {                              \
      invalid_string = 1;                                                      \
      break;                                                                   \
    }                                                                          \
    if (got_dot == 1 && buffer[zi] != '0') {                                   \
      invalid_string = 1;                                                      \
      break;                                                                   \
    } else if (got_dot == 0) {                                                 \
      if (buffer[zi] == '.') {                                                 \
        got_dot = 1;                                                           \
        continue;                                                              \
      } else if (buffer[zi] != '0') {                                          \
        invalid_string = 1;                                                    \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
  }                                                                            \
  if (invalid_string == 1) {                                                   \
    int pps = ((char_start - 10) < 0) ? 0 : (char_start - 10);                 \
    int ppe = ((i + 10) > (bytes_read)) ? (bytes_read) : (i + 10);             \
    buffer[i] = temp;                                                          \
    for (int m = pps; m < ppe; m++) {                                          \
      if ((m >= char_start) && (m < i)) {                                      \
        fprintf(stderr, "\033[1;33m%c\033[0m", buffer[m]);                     \
        continue;                                                              \
      }                                                                        \
      fprintf(stderr, "%c", buffer[m]);                                        \
    }                                                                          \
    fprintf(stderr, "\n");                                                     \
    fprintf(stderr, "char_start: %zu, i: %zu\n", char_start, i);               \
    buffer[i] = '\0';                                                          \
    fprintf(stderr, "Couldn't convert: %s\n", &buffer[char_start]);            \
    fprintf(stderr, "--------------\n");                                       \
    retval = -10;                                                              \
    goto cleanup;                                                              \
  }

#define BUFFER_SIZE 1024
#define SMALL_BUFFER_SIZE 200

typedef unsigned long int uli;

int read_csv(char *filepath, char *target_name, Matrix **csv_mat, Matrix **y,
             int extra_one);
int matrix_dump_csv(Matrix *mat, char *filename);
int store_mat_bin(char *filename, Matrix *mat);
int read_mat_bin(char *filename, Matrix **mat);

#endif /* ifndef MAT_IO */
