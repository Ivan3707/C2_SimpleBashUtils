#include "s21_cat.h"

static void print_line_number(unsigned long line_number) {
  printf("%6lu\t", line_number);
}

void print_nonprinting(int c) {
  unsigned char ch = (unsigned char)c;

  if (ch < 32 && ch != '\t' && ch != '\n') {
    putchar('^');
    putchar('@' + ch);
  } else if (ch == 127) {
    printf("^?");
  } else if (ch >= 128) {
    printf("M-");
    print_nonprinting(ch - 128);
  } else {
    putchar(ch);
  }
}

void parse_options(int argc, char* argv[], CatOptions* options) {
  static struct option long_options[] = {
      {"number-nonblank", no_argument, NULL, 'b'},
      {"number", no_argument, NULL, 'n'},
      {"squeeze-blank", no_argument, NULL, 's'},
      {NULL, 0, NULL, 0}};

  int opt;

  while ((opt = getopt_long(argc, argv, "beEnstTv", long_options, NULL)) !=
         -1) {
    switch (opt) {
      case 'b':
        options->number_nonblank = 1;
        break;
      case 'e':
        options->show_ends = 1;
        options->show_nonprinting = 1;
        break;
      case 'E':
        options->show_ends = 1;
        break;
      case 'n':
        options->number = 1;
        break;
      case 's':
        options->squeeze_blank = 1;
        break;
      case 't':
        options->show_tabs = 1;
        options->show_nonprinting = 1;
        break;
      case 'T':
        options->show_tabs = 1;
        break;
      case 'v':
        options->show_nonprinting = 1;
        break;
      default:
        break;
    }
  }

  if (options->number_nonblank) {
    options->number = 0;
  }
}

void process_stream(FILE* file, const CatOptions* options) {
  int c;
  int at_line_start = 1;
  int line_is_blank = 1;
  int previous_line_blank = 0;
  unsigned long line_number = 1;

  while ((c = fgetc(file)) != EOF) {
    if (at_line_start) {
      line_is_blank = (c == '\n');

      if (c == '\n' && options->squeeze_blank && previous_line_blank) {
        continue;
      }

      if ((options->number && !options->number_nonblank) ||
          (options->number_nonblank && !line_is_blank)) {
        print_line_number(line_number);
        line_number++;
      }

      at_line_start = 0;
    }

    if (c == '\n') {
      if (options->show_ends) {
        putchar('$');
      }

      putchar('\n');

      previous_line_blank = line_is_blank;
      at_line_start = 1;
      continue;
    }

    if (c == '\t') {
      if (options->show_tabs) {
        printf("^I");
      } else {
        putchar('\t');
      }
    } else if (options->show_nonprinting) {
      print_nonprinting(c);
    } else {
      putchar(c);
    }
  }
}

void process_file(const char* filename, const CatOptions* options) {
  FILE* file = fopen(filename, "rb");

  if (file == NULL) {
    fprintf(stderr, "s21_cat: %s: No such file or directory\n", filename);
    return;
  }
  process_stream(file, options);
  fclose(file);
}

int main(int argc, char* argv[]) {
  CatOptions options = {0};

  parse_options(argc, argv, &options);

  if (optind == argc) {
    process_stream(stdin, &options);
  } else {
    for (int i = optind; i < argc; i++) {
      process_file(argv[i], &options);
    }
  }

  return 0;
}