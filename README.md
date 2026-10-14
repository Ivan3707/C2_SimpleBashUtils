# C2_SimpleBashUtils

## Information

### History of cat 

> cat was part of the early versions of Unix, e.g., Version 1, and replaced pr, a PDP-7 and Multics utility for copying a single file to the screen.

### Usage of cat

Cat is one of the most frequently used commands on Unix-like operating systems. It has three related functions with regard to text files: displaying them, combining copies of them and creating new ones.

`cat [OPTION] [FILE]...`

### cat options

| No. | Options | Description |
| ------ | ------ | ------ |
| 1 | -b (GNU: --number-nonblank) | numbers only non-empty lines |
| 2 | -e (implies -v), -E (GNU only: without -v) | displays `$` at line ends (displays `^M$` on Windows `\r\n` lines) |
| 3 | -n (GNU: --number) | number all output lines |
| 4 | -s (GNU: --squeeze-blank) | squeeze multiple adjacent blank lines |
| 5 | -t implies -v (GNU: -T the same, but without implying -v) | but also display tabs as ^I  |

### History of grep 

> Thompson wrote the first version in PDP-11 assembly language to help Lee E. McMahon analyse the text of the Federalist Papers to determine the authorship of each paper. The text editor ed (also written by Thompson) had support for regular expressions, but could not be used on such a large amount of text, so Thompson extracted this code into a standalone tool. He chose the name because in ed, the command g/re/p would print all lines matching a given pattern. 
grep was first included in version 4 of Unix. Noting that it is "generally cited as the prototypical software tool", McIlroy credited grep with "irrevocably entrenching" Thompson's tool philosophy in Unix.

### Usage of grep 

`grep [OPTION]... PATTERNS [FILE]...`

### grep options

| No. | Options | Description |
| ------ | ------ | ------ |
| 1 | -e | pattern |
| 2 | -i | Ignore uppercase vs. lowercase.  |
| 3 | -v | Invert match. |
| 4 | -c | Output count of matching lines only. |
| 5 | -l | Output matching files only.  |
| 6 | -n | Precede each matching line with a line number. |
| 7 | -h | Output matching lines without preceding them by file names. |
| 8 | -s | Suppress error messages about nonexistent or unreadable files. |
| 9 | -f file | Take regexes from a file. |
| 10 | -o | Output the matched parts of a matching line. |


## Chapter III

How to work with the project:

- Before starting, clone the project from GitLab into a repository with the same name.
- The programs must be developed in C language of C11 standard using gcc compiler.
- The program code of the cat and grep must be located on the develop branch in the src/cat/ and src/grep/ folders, respectively.
- Do not use outdated and legacy language constructions and library functions. Pay attention to the legacy and obsolete marks in the official documentation on the language and the libraries used. Use the POSIX.1-2017 standard.
- When writing code it is necessary to follow the Google style for C++ ([link](https://google.github.io/styleguide/cppguide.html)).
- The programs must be executable files with command line arguments.
- The programs must be built with Makefile with appropriate targets: s21_cat, s21_grep.
- If third-party libraries are used, there must be build scripts in makefile to connect/load them.
- Integration tests must cover all flag variants and input values, based on a comparison with the behavior of real Bash utilities.
- The programs must be developed according to the principles of structured programming.
- Code duplication must be avoided, common modules must be reused between the utilities. Common modules can be moved to a separate folder src/common.
- You can use standard and non-standard C libraries, or you can use your own developed libraries from other projects
- The statement of the message in the case of an error does not matter.
- Input via stdin is not required to be supported.

### Testing specifics

Your implementation will be tested using alpine version 3.20, where cat and grep utilities are presented as busybox versions. Note that busybox behavior may differ from GNU implementations.

## Part 1. Working with the cat utility

You need to develop a cat utility:
- Support of all flags (including GNU versions) specified [above](#cat-options).
- Support both calls with and without flags.
- The source, header, and build files must be placed in the src/cat/ directory.
- The resulting executable file must be placed in the directory src/cat/ and named s21_cat.

## Part 2. Working with grep utility

You need to develop the grep utility:
- Support of the following flags: `-e`, `-i`, `-v`, `-c`, `-l`, `-n`.
- Support both calls with and without flags.
- Only pcre2 or regex libraries can be used for regular expressions.
- The source, header and make files must be placed in the src/grep/ directory.
- The resulting executable file must be placed in the directory src/grep/ and named s21_grep.

## Part 3. Implementation of some grep utility flags

Bonus assignment for extra points. You need to develop the grep utility:
- Support of all flags, including: `-h`, `-s`, `-f`, `-o`.
- Only pcre2 or regex libraries can be used for regular expressions.
- The source, header and make files must be placed in the src/grep/ directory.
- The resulting executable file must be placed in the directory src/grep/ and named s21_grep.

## Part 4. Implementation of grep utility flag combinations

Bonus assignment for extra points. You need to develop the grep utility:
- Support of all flags, including their _pair_ combinations (e.g. `-iv`, `-in`).
- Only pcre2 or regex libraries can be used for regular expressions.
- The source, header and make files must be placed in the src/grep/ directory.
- The resulting executable file must be placed in the directory src/grep/ and named s21_grep.

💡 [Tap here](https://new.oprosso.net/p/4cb31ec3f47a4596bc758ea1861fb624) **to leave your feedback on the project**. It's anonymous and will help our team make your educational experience better. We recommend completing the survey immediately after the project.
