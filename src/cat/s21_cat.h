#ifndef S21_CAT_H
#define S21_CAT_H

#include <getopt.h>
#include <stdio.h>

typedef struct {
  int number_nonblank;
  int show_ends;
  int number;
  int squeeze_blank;
  int show_tabs;
  int show_nonprinting;
} CatOptions;

void parse_options(int argc, char *argv[], CatOptions *options);
void process_file(const char *filename, const CatOptions *options);
void process_stream(FILE *file, const CatOptions *options);
void print_nonprinting(int c);

#endif  