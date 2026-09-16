#include "s21_grep.h"

#include <getopt.h>
#include <stdlib.h>
#include <string.h>

void add_pattern(PatternList *patterns, const char *pattern) {
  char **new_patterns =
      realloc(patterns->patterns, (patterns->count + 1) * sizeof(char *));

  if (new_patterns == NULL) {
    fprintf(stderr, "s21_grep: memory allocation error\n");
    exit(EXIT_FAILURE);
  }

  patterns->patterns = new_patterns;

  patterns->patterns[patterns->count] = malloc(strlen(pattern) + 1);

  if (patterns->patterns[patterns->count] == NULL) {
    fprintf(stderr, "s21_grep: memory allocation error\n");
    exit(EXIT_FAILURE);
  }

  strcpy(patterns->patterns[patterns->count], pattern);

  patterns->count++;
}

void parse_options(int argc, char *argv[], GrepOptions *options,
                   PatternList *patterns) {
  int opt;

  while ((opt = getopt(argc, argv, "e:ivcln")) != -1) {
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

      default:
        break;
    }
  }

  if (patterns->count == 0 && optind < argc) {
    add_pattern(patterns, argv[optind]);
    optind++;
  }
}

int read_line(FILE *file, char **buffer, size_t *capacity) {
  size_t length = 0;
  int c;

  if (*buffer == NULL) {
    *capacity = 128;
    *buffer = malloc(*capacity);

    if (*buffer == NULL) {
      return -1;
    }
  }

  while ((c = fgetc(file)) != EOF) {
    if (length + 1 >= *capacity) {
      size_t new_capacity = *capacity * 2;
      char *new_buffer = realloc(*buffer, new_capacity);

      if (new_buffer == NULL) {
        return -1;
      }

      *buffer = new_buffer;
      *capacity = new_capacity;
    }

    (*buffer)[length++] = (char)c;

    if (c == '\n') {
      break;
    }
  }

  if (c == EOF && length == 0) {
    return 0;
  }

  (*buffer)[length] = '\0';

  return 1;
}

int compile_patterns(const PatternList *patterns, const GrepOptions *options,
                     pcre2_code ***regexes) {
  uint32_t compile_options = 0;

  if (options->ignore_case) {
    compile_options |= PCRE2_CASELESS;
  }

  *regexes = calloc((size_t)patterns->count, sizeof(pcre2_code *));

  if (*regexes == NULL) {
    fprintf(stderr, "s21_grep: memory allocation error\n");
    return 0;
  }

  for (int i = 0; i < patterns->count; i++) {
    int error_code = 0;
    PCRE2_SIZE error_offset = 0;

    (*regexes)[i] =
        pcre2_compile((PCRE2_SPTR)patterns->patterns[i], PCRE2_ZERO_TERMINATED,
                      compile_options, &error_code, &error_offset, NULL);

    if ((*regexes)[i] == NULL) {
      PCRE2_UCHAR error_message[256];

      pcre2_get_error_message(error_code, error_message, sizeof(error_message));

      fprintf(stderr, "s21_grep: %s\n", (char *)error_message);

      for (int j = 0; j < i; j++) {
        pcre2_code_free((*regexes)[j]);
      }

      free(*regexes);
      *regexes = NULL;

      return 0;
    }
  }

  return 1;
}

int line_matches(const char *line, pcre2_code **regexes, int regex_count,
                 const GrepOptions *options) {
  int matched = 0;

  for (int i = 0; i < regex_count; i++) {
    pcre2_match_data *match_data =
        pcre2_match_data_create_from_pattern(regexes[i], NULL);

    if (match_data == NULL) {
      continue;
    }

    int result = pcre2_match(regexes[i], (PCRE2_SPTR)line, strlen(line), 0, 0,
                             match_data, NULL);

    pcre2_match_data_free(match_data);

    if (result >= 0) {
      matched = 1;
      break;
    }
  }

  if (options->invert_match) {
    matched = !matched;
  }

  return matched;
}

int process_file(const char *filename, pcre2_code **regexes, int regex_count,
                 const GrepOptions *options, int multiple_files) {
  FILE *file = fopen(filename, "r");

  if (file == NULL) {
    fprintf(stderr, "s21_grep: %s: No such file or directory\n", filename);
    return 2;
  }

  char *line = NULL;
  size_t capacity = 0;

  unsigned long line_number = 0;
  unsigned long match_count = 0;
  int found_match = 0;

  int read_result;

  while ((read_result = read_line(file, &line, &capacity)) == 1) {
    line_number++;

    if (!line_matches(line, regexes, regex_count, options)) {
      continue;
    }

    found_match = 1;
    match_count++;

    if (options->files_only) {
      continue;
    }

    if (options->count_only) {
      continue;
    }

    if (multiple_files) {
      printf("%s:", filename);
    }

    if (options->line_number) {
      printf("%lu:", line_number);
    }

    fputs(line, stdout);
  }

  if (read_result == -1) {
    fprintf(stderr, "s21_grep: memory allocation error\n");
    free(line);
    fclose(file);
    return 2;
  }

  if (options->files_only) {
    if (found_match) {
      printf("%s\n", filename);
    }
  } else if (options->count_only) {
    if (multiple_files) {
      printf("%s:%lu\n", filename, match_count);
    } else {
      printf("%lu\n", match_count);
    }
  }

  free(line);
  fclose(file);

  return found_match ? 0 : 1;
}

void free_patterns(PatternList *patterns) {
  for (int i = 0; i < patterns->count; i++) {
    free(patterns->patterns[i]);
  }

  free(patterns->patterns);

  patterns->patterns = NULL;
  patterns->count = 0;
}

void free_regexes(pcre2_code **regexes, int count) {
  if (regexes == NULL) {
    return;
  }

  for (int i = 0; i < count; i++) {
    if (regexes[i] != NULL) {
      pcre2_code_free(regexes[i]);
    }
  }

  free(regexes);
}

int main(int argc, char *argv[]) {
  GrepOptions options = {0};
  PatternList patterns = {0};

  parse_options(argc, argv, &options, &patterns);

  if (patterns.count == 0 || optind >= argc) {
    free_patterns(&patterns);
    return EXIT_FAILURE;
  }

  pcre2_code **regexes = NULL;

  if (!compile_patterns(&patterns, &options, &regexes)) {
    free_patterns(&patterns);
    return EXIT_FAILURE;
  }

  int file_count = argc - optind;
  int multiple_files = file_count > 1;

  int exit_code = 1;

  for (int i = optind; i < argc; i++) {
    int result = process_file(argv[i], regexes, patterns.count, &options,
                              multiple_files);

    if (result == 0) {
      exit_code = 0;
    } else if (result == 2 && exit_code != 0) {
      exit_code = 2;
    }
  }

  free_regexes(regexes, patterns.count);
  free_patterns(&patterns);

  return exit_code;
}