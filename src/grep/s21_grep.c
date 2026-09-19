#include "s21_grep.h"

#include <getopt.h>
#include <stdlib.h>
#include <string.h>

void add_pattern(PatternList *patterns, const char *pattern) {
  char **tmp = realloc(patterns->patterns,
                       (patterns->count + 1) * sizeof(char *));

  if (tmp == NULL) {
    fprintf(stderr, "s21_grep: memory allocation error\n");
    exit(EXIT_FAILURE);
  }

  patterns->patterns = tmp;
  patterns->patterns[patterns->count] = malloc(strlen(pattern) + 1);

  if (patterns->patterns[patterns->count] == NULL) {
    fprintf(stderr, "s21_grep: memory allocation error\n");
    exit(EXIT_FAILURE);
  }

  strcpy(patterns->patterns[patterns->count], pattern);
  patterns->count++;
}

int add_patterns_from_file(PatternList *patterns, const char *filename,
                           const GrepOptions *options) {
  FILE *file = fopen(filename, "r");
  char *line = NULL;
  size_t capacity = 0;
  int result = 1;
  int read_result = 0;

  if (file == NULL) {
    if (!options->suppress_errors) {
      fprintf(stderr, "s21_grep: %s: No such file or directory\n", filename);
    }
    result = 0;
  } else {
    while ((read_result = read_line(file, &line, &capacity)) == 1) {
      size_t len = strlen(line);

      if (len > 0 && line[len - 1] == '\n') {
        line[len - 1] = '\0';
      }

      add_pattern(patterns, line);
    }

    if (read_result == -1) {
      fprintf(stderr, "s21_grep: memory allocation error\n");
      result = 0;
    }

    free(line);
    fclose(file);
  }

  return result;
}

int parse_options(int argc, char *argv[], GrepOptions *options,
                  PatternList *patterns) {
  int opt;
  int result = 1;

  while ((opt = getopt(argc, argv, "e:ivclnhsf:o")) != -1) {
    switch (opt) {
      case 'e':
        add_pattern(patterns, optarg);
        break;
      case 'i':
        options->ignore_case = 1;
        break;
      case 'v':
        options->invert_match = 1;
        break;
      case 'c':
        options->count_only = 1;
        break;
      case 'l':
        options->files_only = 1;
        break;
      case 'n':
        options->line_number = 1;
        break;
      case 'h':
        options->no_filename = 1;
        break;
      case 's':
        options->suppress_errors = 1;
        break;
      case 'f':
        if (!add_patterns_from_file(patterns, optarg, options)) {
          result = 0;
        }
        break;
      case 'o':
        options->only_matching = 1;
        break;
      default:
        result = 0;
        break;
    }
  }

  if (result && patterns->count == 0 && optind < argc) {
    add_pattern(patterns, argv[optind]);
    optind++;
  }

  return result;
}

int read_line(FILE *file, char **buffer, size_t *capacity) {
  size_t length = 0;
  int c = 0;
  int result = 1;

  if (*buffer == NULL) {
    *capacity = 128;
    *buffer = malloc(*capacity);

    if (*buffer == NULL) {
      result = -1;
    }
  }

  while (result == 1 && (c = fgetc(file)) != EOF) {
    if (length + 1 >= *capacity) {
      size_t new_capacity = *capacity * 2;
      char *tmp = realloc(*buffer, new_capacity);

      if (tmp == NULL) {
        result = -1;
      } else {
        *buffer = tmp;
        *capacity = new_capacity;
      }
    }

    if (result == 1) {
      (*buffer)[length++] = (char)c;
    }

    if (result == 1 && c == '\n') {
      break;
    }
  }

  if (result == 1 && c == EOF && length == 0) {
    result = 0;
  }

  if (result == 1) {
    (*buffer)[length] = '\0';
  }

  return result;
}

int compile_patterns(const PatternList *patterns, const GrepOptions *options,
                     pcre2_code ***regexes) {
  uint32_t flags = 0;
  int result = 1;
  int i = 0;

  if (options->ignore_case) {
    flags |= PCRE2_CASELESS;
  }

  *regexes = calloc((size_t)patterns->count, sizeof(pcre2_code *));

  if (*regexes == NULL) {
    fprintf(stderr, "s21_grep: memory allocation error\n");
    result = 0;
  }

  while (result && i < patterns->count) {
    int error_code = 0;
    PCRE2_SIZE error_offset = 0;

    (*regexes)[i] = pcre2_compile(
        (PCRE2_SPTR)patterns->patterns[i], PCRE2_ZERO_TERMINATED, flags,
        &error_code, &error_offset, NULL);

    if ((*regexes)[i] == NULL) {
      PCRE2_UCHAR message[256];

      pcre2_get_error_message(error_code, message, sizeof(message));
      fprintf(stderr, "s21_grep: %s\n", (char *)message);
      result = 0;
    }

    i++;
  }

  if (!result) {
    free_regexes(*regexes, patterns->count);
    *regexes = NULL;
  }

  return result;
}

int line_matches(const char *line, pcre2_code **regexes, int regex_count,
                 const GrepOptions *options) {
  int matched = 0;
  int i = 0;

  while (i < regex_count && !matched) {
    pcre2_match_data *data =
        pcre2_match_data_create_from_pattern(regexes[i], NULL);

    if (data != NULL) {
      int result = pcre2_match(regexes[i], (PCRE2_SPTR)line, strlen(line), 0,
                               0, data, NULL);

      if (result >= 0) {
        matched = 1;
      }

      pcre2_match_data_free(data);
    }

    i++;
  }

  if (options->invert_match) {
    matched = !matched;
  }

  return matched;
}

int find_next_match(const char *line, pcre2_code **regexes, int regex_count,
                    PCRE2_SIZE offset, PCRE2_SIZE *start, PCRE2_SIZE *end) {
  int found = 0;
  int i = 0;

  while (i < regex_count) {
    pcre2_match_data *data =
        pcre2_match_data_create_from_pattern(regexes[i], NULL);

    if (data != NULL) {
      int result = pcre2_match(regexes[i], (PCRE2_SPTR)line, strlen(line),
                               offset, 0, data, NULL);

      if (result >= 0) {
        PCRE2_SIZE *vector = pcre2_get_ovector_pointer(data);

        if (!found || vector[0] < *start) {
          *start = vector[0];
          *end = vector[1];
          found = 1;
        }
      }

      pcre2_match_data_free(data);
    }

    i++;
  }

  return found;
}

int print_only_matches(const char *line, pcre2_code **regexes, int regex_count,
                       const GrepOptions *options, const char *filename,
                       unsigned long line_number, int multiple_files) {
  size_t length = strlen(line);
  PCRE2_SIZE offset = 0;
  int printed = 0;

  if (length > 0 && line[length - 1] == '\n') {
    length--;
  }

  while (offset <= length) {
    PCRE2_SIZE start = length;
    PCRE2_SIZE end = length;

    if (!find_next_match(line, regexes, regex_count, offset, &start, &end)) {
      offset = length + 1;
    } else {
      if (end > start) {
        if (multiple_files && !options->no_filename) {
          printf("%s:", filename);
        }

        if (options->line_number) {
          printf("%lu:", line_number);
        }

        fwrite(line + start, 1, end - start, stdout);
        putchar('\n');
        printed = 1;
      }

      if (end > offset) {
        offset = end;
      } else {
        offset++;
      }
    }
  }

  return printed;
}

void print_line(const char *line, const char *filename,
                const GrepOptions *options, unsigned long line_number,
                int multiple_files) {
  if (multiple_files && !options->no_filename) {
    printf("%s:", filename);
  }

  if (options->line_number) {
    printf("%lu:", line_number);
  }

  fputs(line, stdout);
}

void print_count(const char *filename, const GrepOptions *options,
                 unsigned long count, int multiple_files) {
  if (multiple_files && !options->no_filename) {
    printf("%s:%lu\n", filename, count);
  } else {
    printf("%lu\n", count);
  }
}

int process_file(const char *filename, pcre2_code **regexes, int regex_count,
                 const GrepOptions *options, int multiple_files) {
  FILE *file = fopen(filename, "r");
  char *line = NULL;
  size_t capacity = 0;
  unsigned long line_number = 0;
  unsigned long match_count = 0;
  int found = 0;
  int result = 0;
  int read_result = 0;

  if (file == NULL) {
    if (!options->suppress_errors) {
      fprintf(stderr, "s21_grep: %s: No such file or directory\n", filename);
    }
    result = 2;
  } else {
    while ((read_result = read_line(file, &line, &capacity)) == 1) {
      line_number++;

      if (line_matches(line, regexes, regex_count, options)) {
        found = 1;
        match_count++;

        if (!options->files_only && !options->count_only) {
          if (options->only_matching && !options->invert_match) {
            print_only_matches(line, regexes, regex_count, options, filename,
                               line_number, multiple_files);
          } else {
            print_line(line, filename, options, line_number, multiple_files);
          }
        }
      }
    }

    if (read_result == -1) {
      fprintf(stderr, "s21_grep: memory allocation error\n");
      result = 2;
    } else if (options->files_only && found) {
      printf("%s\n", filename);
    } else if (options->count_only) {
      print_count(filename, options, match_count, multiple_files);
    } else if (found) {
      result = 0;
    }

    if (result == 0 && !found) {
      result = 1;
    }

    free(line);
    fclose(file);
  }

  return result;
}

void free_patterns(PatternList *patterns) {
  int i = 0;

  while (i < patterns->count) {
    free(patterns->patterns[i]);
    i++;
  }

  free(patterns->patterns);
  patterns->patterns = NULL;
  patterns->count = 0;
}

void free_regexes(pcre2_code **regexes, int count) {
  int i = 0;

  if (regexes != NULL) {
    while (i < count) {
      if (regexes[i] != NULL) {
        pcre2_code_free(regexes[i]);
      }

      i++;
    }

    free(regexes);
  }
}

int main(int argc, char *argv[]) {
  GrepOptions options = {0};
  PatternList patterns = {0};
  pcre2_code **regexes = NULL;
  int exit_code = EXIT_FAILURE;

  if (parse_options(argc, argv, &options, &patterns) &&
      patterns.count > 0 && optind < argc) {
    if (compile_patterns(&patterns, &options, &regexes)) {
      int files = argc - optind;
      int multiple_files = files > 1;
      int i = optind;

      exit_code = 1;

      while (i < argc) {
        int result = process_file(argv[i], regexes, patterns.count, &options,
                                  multiple_files);

        if (result == 0) {
          exit_code = 0;
        } else if (result == 2 && exit_code != 0) {
          exit_code = 2;
        }

        i++;
      }
    }
  }

  free_regexes(regexes, patterns.count);
  free_patterns(&patterns);

  return exit_code;
}