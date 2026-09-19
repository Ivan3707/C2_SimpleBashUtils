#ifndef S21_GREP_H
#define S21_GREP_H

#define PCRE2_CODE_UNIT_WIDTH 8

#include <pcre2.h>
#include <stdio.h>

typedef struct {
  int ignore_case;
  int invert_match;
  int count_only;
  int files_only;
  int line_number;
  int no_filename;
  int suppress_errors;
  int only_matching;
} GrepOptions;

typedef struct {
  char **patterns;
  int count;
} PatternList;

void add_pattern(PatternList *patterns, const char *pattern);

int add_patterns_from_file(PatternList *patterns, const char *filename,
                           const GrepOptions *options);

int parse_options(int argc, char *argv[], GrepOptions *options,
                  PatternList *patterns);

int read_line(FILE *file, char **buffer, size_t *capacity);

int compile_patterns(const PatternList *patterns, const GrepOptions *options,
                     pcre2_code ***regexes);

int line_matches(const char *line, pcre2_code **regexes, int regex_count,
                 const GrepOptions *options);

int print_only_matches(const char *line, pcre2_code **regexes, int regex_count,
                       const GrepOptions *options, const char *filename,
                       unsigned long line_number, int multiple_files);

int process_file(const char *filename, pcre2_code **regexes, int regex_count,
                 const GrepOptions *options, int multiple_files);

void free_patterns(PatternList *patterns);

void free_regexes(pcre2_code **regexes, int count);

#endif