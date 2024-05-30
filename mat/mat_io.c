#include "mat_io.h"
#include "../err_helper.h"
#include "mat.h"
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uli _repeat_bytes(unsigned char c_in) { return (((uli)-1) / 0xff) * c_in; }
uli find_similar_char(uli word, unsigned char c_in) {
  uli rby = _repeat_bytes(c_in);
  uli xored = word ^ rby;
  uli mask = _repeat_bytes(0x7f);
  return ~(((xored & mask) + mask) | mask | xored);
}
uli shift_bits(uli word, uintptr_t s) {
  if (__BYTE_ORDER == __LITTLE_ENDIAN) {
    return word >> (CHAR_BIT * (s % sizeof(uli)));
  } else {
    return word << (CHAR_BIT * (s % sizeof(uli)));
  }
}

// double pointer because giving single pointer is just sending the address of
// pointer and changing the address won't change the original pointer address as
// copied address is sent. Whereas giving double pointer, which is pointer to
// the pointer of matrix. The we can change the pointer value of matrix by
// dereferencing
int read_csv(char *filepath, char *target_name, Matrix **csv_mat, Matrix **y,
             int extra_one) {
  int retval = 0;

  FILE *fp = fopen(filepath, "r");
  if (fp == NULL) {
    LINE_FILE_PRINT(2);
    fprintf(stderr, "%s: can't open %s\n", "CSV_READER", filepath);
    return 2;
  }

  char sml_buffer[SMALL_BUFFER_SIZE];
  size_t bytes_read;

  size_t file_idx = 0;
  int got_new_line = 0;
  int row = 0, col = 0, target_col = -1;
  while ((bytes_read = fread(sml_buffer, 1, SMALL_BUFFER_SIZE, fp)) > 0) {
    size_t start = 0;
    for (size_t i = 0; i < bytes_read; i++) {
      if (sml_buffer[i] == '\n') {
        sml_buffer[i] = '\0';
        if ((target_col == -1) &&
            (strcmp(&sml_buffer[start], target_name) == 0)) {
          target_col = col;
        }
        sml_buffer[i] = '\n';

        file_idx += i;
        got_new_line = 1;
        // for last column
        col++;
        break;
      } else if (sml_buffer[i] == ',') {
        sml_buffer[i] = '\0';
        if ((target_col == -1) &&
            (strcmp(&sml_buffer[start], target_name) == 0)) {
          target_col = col;
        }
        sml_buffer[i] = ',';
        start = i + 1;
        col++;
      }
    }
    if (got_new_line == 1) {
      break;
    }

    if (sml_buffer[bytes_read - 1] != ',') {
      file_idx += start;
      if (fseek(fp, file_idx, SEEK_SET)) {
        LINE_FILE_PRINT(1);
        fprintf(stderr, "Can't Seek at %zu from start\n", file_idx);
        retval = 7;
        goto cleanup;
      }
    } else {
      file_idx += bytes_read;
    }
  }
  if (ferror(fp)) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "Error reading the file\n");
    retval = 8;
    goto cleanup;
  }
  if (target_col == -1) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "Target Name not present in Header\n");
    retval = 9;
    goto cleanup;
  }
  // to move to next char after '\n'
  file_idx++;

  if (fseek(fp, file_idx, SEEK_SET)) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "Can't Seek at %zu from start\n", file_idx);
    fclose(fp);
    return 7;
  }

  int check = 0;
  char buffer[BUFFER_SIZE];
  while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
    char *string = &buffer[0];
    uli *word_ptr = PTR_ALIGN_DOWN((void *)string, sizeof(uli));
    // converting address into integer for operation
    uintptr_t s = (uintptr_t)string;

    uintptr_t last_byte = ((s + bytes_read - 1) > UINTPTR_MAX)
                              ? UINTPTR_MAX
                              : (s + bytes_read - 1);
    uli *last_word_ptr = PTR_ALIGN_DOWN((void *)last_byte, sizeof(uli));
    // got last_word_ptr to use in loop as to only check for word which are
    // in middle and handling last word differently as like first word, this
    // word may contain memory not part of buffer.

    uli word = *word_ptr;

    uli res = find_similar_char(word, '\n'); // '\n' is 0x0a in hex
    res = shift_bits(res, s);
    row += __builtin_popcountll(res);

    if (word_ptr == last_word_ptr) {
      // thus goes to end line of parent loop
      continue;
    }

    word = *++word_ptr;
    while (word_ptr != last_word_ptr) {
      row += __builtin_popcountll(find_similar_char(word, '\n'));
      word = *++word_ptr;
    }

    // now handling last word
    res = find_similar_char(word, '\n');
    if (__BYTE_ORDER == __LITTLE_ENDIAN) {
      res = res << (CHAR_BIT * (sizeof(uli) - (last_byte % sizeof(uli)) - 1));
    } else {
      res = res >> (CHAR_BIT * (sizeof(uli) - (last_byte % sizeof(uli)) - 1));
    }
    row += __builtin_popcountll(res);

    check = buffer[bytes_read - 1];
  }
  if (ferror(fp)) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "Error reading the file\n");
    retval = 8;
    goto cleanup;
  }
  row += (check != '\n');

  if (fseek(fp, file_idx, SEEK_SET)) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "Can't Seek at %zu from start\n", file_idx);
    fclose(fp);
    return 7;
  }

  *csv_mat = Matrix_create(row, col - 1 + extra_one);
  if (*csv_mat == NULL) {
    LINE_FILE_PRINT(2);
    fprintf(stderr, "Couldn't allocate memory for csv_mat\n");
    retval = 2;
    goto cleanup;
  }
  *y = Matrix_create(1, row);
  if (*y == NULL) {
    LINE_FILE_PRINT(2);
    fprintf(stderr, "Couldn't allocate memory for y\n");
    retval = 2;
    goto cleanup;
  }
  int read_columns = 0;
  int csv_indx = -1;
  int y_indx = -1;
  double num = 0;

  while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
    size_t char_start = 0;
    for (size_t i = 0; i < bytes_read; i++) {
      if ((buffer[i] == ',') || (buffer[i] == '\n')) {
        char temp = buffer[i];
        buffer[i] = '\0';
        // for "" case it gives 0.0
        // "" case when ,, or ,\n thus missing value
        // case. Below handling this when not
        // encountered number. Saving those missing values
        // as NAN
        num = atof(&buffer[char_start]);

        if (num == 0.0) {
          // let's check for "" or "\r" as these two are
          // possible for missing value
          // '\r' cause i got this when i tried editing
          // csv with neovim
          if ((char_start == i) ||
              ((buffer[char_start] == '\r') && (i == (char_start + 1)))) {
            num = NAN;
          } else {
            // now we need the format of
            // (0 to n zeros)(either a dot or not)(if
            // dot then 0 to m zeros)
            // any other string then it is invalid

            VALIDATE_ZERO_STRING(char_start, i, temp);
          }
        }

        buffer[i] = temp;
        if (read_columns == target_col) {
          y_indx++;
          (*y)->data[y_indx] = num;
        } else {
          csv_indx++;
          (*csv_mat)->data[csv_indx] = num;
        }
        read_columns++;

        char_start = i + 1;

        if (buffer[i] == '\n') {
          if (read_columns != col) {
            LINE_FILE_PRINT(1);
            fprintf(stderr, "Incorrect file format\n");
            retval = 3;
            goto cleanup;
          }
          // adding that extra one
          if (extra_one == 1) {
            csv_indx++;
            (*csv_mat)->data[csv_indx] = 1;
          }
          read_columns = 0;
        }
      }
    }

    if (bytes_read == BUFFER_SIZE) {
      if ((buffer[bytes_read - 1] != '\n') || (buffer[bytes_read - 1] != ',')) {
        file_idx += char_start;
        if (fseek(fp, file_idx, SEEK_SET)) {
          LINE_FILE_PRINT(1);
          fprintf(stderr, "Can't Seek at %zu from start\n", file_idx);
          retval = 7;
          goto cleanup;
        }
      } else {
        file_idx += bytes_read;
      }
    } else if (buffer[bytes_read - 1] != '\n') {
      // now only possiblity is either a number
      // or any other unexpected char
      buffer[bytes_read] = '\0';
      num = atof(&buffer[char_start]);

      if (num == 0.0) {
        if ((char_start == (bytes_read - 1) && buffer[char_start] == '\r') ||
            (char_start == bytes_read)) {
          num = NAN;
        } else {
          VALIDATE_ZERO_STRING(char_start, bytes_read, '\0');
        }
      }

      if (read_columns == target_col) {
        y_indx++;
        (*y)->data[y_indx] = num;
      } else {
        csv_indx++;
        (*csv_mat)->data[csv_indx] = num;
      }
      read_columns++;

      if (read_columns != col) {
        LINE_FILE_PRINT(1);
        fprintf(stderr, "Incorrect file format\n");
        retval = 4;
        goto cleanup;
      }

      // adding that extra one
      if (extra_one == 1) {
        csv_indx++;
        (*csv_mat)->data[csv_indx] = 1;
      }
    }
  }

cleanup:
  fclose(fp);

  return retval;
}

int matrix_dump_csv(Matrix *mat, char *filename) {
  if (mat == NULL) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "Mat address is NULL\n");
    return 1;
  }

  FILE *fp = fopen(filename, "w");
  if (fp == NULL) {
    LINE_FILE_PRINT(2);
    fprintf(stderr, "Can't open %s\n", filename);
    return 2;
  }

  SET_MATRIX_DIMENSIONS(mat);

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      if (j < cols - 1) {
        if (isnan(mat->data[i * mi + j * nj])) {
          fprintf(fp, ",");
        } else {
          fprintf(fp, "%.*f,", DBL_DIG, mat->data[i * mi + j * nj]);
        }
      } else {
        // if NAN don't print anything to file
        if (isnan(mat->data[i * mi + j * nj]) == 0) {
          fprintf(fp, "%.*f", DBL_DIG, mat->data[i * mi + j * nj]);
        }
      }
    }
    fprintf(fp, "\n");
  }

  if (ferror(fp)) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "Err writing file: %s", filename);
    fclose(fp);
    return 3;
  }

  fclose(fp);

  return 0;
}

int store_mat_bin(char *filename, Matrix *mat) {
  int retval = 0;

  // there are two information i have to store, one is
  // rows, cols and transpose
  // other is whole data array
  FILE *fp = fopen(filename, "wb");
  if (fp == NULL) {
    LINE_FILE_PRINT(2);
    fprintf(stderr, "Couldn't Open the file\n");
    retval = 1;
    goto cleanup;
  }

  // structure of binary file
  // rkad limiteds -> heading which is char *
  // then rows, cols, transpose at once as all of them are integers
  // then storing the whole data floats => don't have to worry about
  // total num as we can get from above rows and cols

  char *HEADER = "RKAD LIMITED - MATRIX";
  int length = strlen(HEADER);

  if (fwrite(HEADER, sizeof(char), length, fp) != length) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "Failed to write header\n");
    retval = 2;
    goto cleanup;
  }

  // this is because in struct, the mat is address to first field
  // of the struct. Thus if those field are not allocated in heap
  // then all of them contain values
  // so mat -> address of mat->rows, mat+4(as int) -> address of mat->cols
  // and lastly mat+8(as after cols) -> address of mat->transpose
  // if not able to understand, then create a simple matrix and
  // print out all the addresses
  if (fwrite(mat, sizeof(int), 3, fp) != 3) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "Failed to write rows, cols and transpose\n");
    retval = 2;
    goto cleanup;
  }

  int total_elems = mat->rows * mat->cols;

  if (fwrite(mat->data, sizeof(double), total_elems, fp) != total_elems) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "Failed to write data arr\n");
    retval = 2;
    goto cleanup;
  }

cleanup:
  fclose(fp);

  return retval;
}

int read_mat_bin(char *filename, Matrix **mat) {
  int retval = 0;

  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    LINE_FILE_PRINT(2);
    fprintf(stderr, "Couldn't Open the file\n");
    retval = 1;
    goto cleanup;
  }

  // let's first check the header
  char HEADER[22];
  HEADER[21] = '\0';
  if (fread(&HEADER, sizeof(char), 21, fp) != 21) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "Failed to read header\n");
    retval = 2;
    goto cleanup;
  }

  if (strcmp(HEADER, "RKAD LIMITED - MATRIX") != 0) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "HEADER is not matched\n");
    retval = 3;
    goto cleanup;
  }

  // if header matched then it is our binary file
  *mat = (Matrix *)malloc(1 * sizeof(Matrix));
  if (*mat == NULL) {
    LINE_FILE_PRINT(2);
    fprintf(stderr, "Couldn't allocated memory for mat\n");
    retval = 4;
    goto cleanup;
  }

  if (fread(*mat, sizeof(int), 3, fp) != 3) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "Failed to read rows, cols and transpose\n");
    retval = 2;
    goto cleanup;
  }

  int total_elems = (*mat)->rows * (*mat)->cols;

  double *data = malloc(total_elems * sizeof(double));
  if (data == NULL) {
    LINE_FILE_PRINT(2);
    fprintf(stderr, "Couldn't allocated memory for data\n");
    retval = 4;
    goto cleanup;
  }

  if (fread(data, sizeof(double), total_elems, fp) != total_elems) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "Failed to read data arr\n");
    retval = 2;
    goto cleanup;
  }

  (*mat)->data = data;

cleanup:
  fclose(fp);

  return retval;
}
