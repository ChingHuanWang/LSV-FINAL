/***************************************************************************
Copyright (c) 2006-2007, Armin Biere, Johannes Kepler University.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
***************************************************************************/
#ifndef AIG_TO_AIG_H
#define AIG_TO_AIG_H

#include "aiger.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>

#define PERCENT(a,b) ((b) ? (100.0 * (a))/ (double)(b) : 0.0)

typedef struct stream stream;
typedef struct memory memory;

struct stream
{
  double bytes;
  FILE *file;
};

struct memory
{
  double bytes;
  double max;
};

static void *
aigtoaig_malloc (memory * m, size_t bytes)
{
  m->bytes += bytes;
  assert (m->bytes);
  if (m->bytes > m->max)
    m->max = m->bytes;
  return malloc (bytes);
}

static void
aigtoaig_free (memory * m, void *ptr, size_t bytes)
{
  assert (m->bytes >= bytes);
  m->bytes -= bytes;
  free (ptr);
}

static int
aigtoaig_put (char ch, stream * stream)
{
  int res;

  res = putc ((unsigned char) ch, stream->file);
  if (res != EOF)
    stream->bytes++;

  return res;
}

static int
aigtoaig_get (stream * stream)
{
  int res;

  res = getc (stream->file);
  if (res != EOF)
    stream->bytes++;

  return res;
}

static double
size_of_file (const char *file_name)
{
  struct stat buf;
  buf.st_size = 0;
  stat (file_name, &buf);
  return buf.st_size;
}

static void
die (const char *fmt, ...)
{
  va_list ap;
  fputs ("*** [aigtoaig] ", stderr);
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);
  fputc ('\n', stderr);
  exit (1);
}

#define USAGE \
"usage: aigtoaig [-h][-v][-s][-a][src [dst]]\n" \
"\n" \
"This is an utility to translate files in AIGER format.\n" \
"\n" \
"  -h     print this command line option summary\n" \
"  -v     verbose output on 'stderr'\n" \
"  -a     output in ASCII AIGER '.aag' format\n" \
"  -s     strip symbols and comments of the output file\n" \
"  src    input file or '-' for 'stdin'\n" \
"  dst    output file or '-' for 'stdout'\n" \
"\n" \
"The input format is given by the header in the input file, while\n" \
"the output format is determined by the name of the output file.\n" \
"If the name of the output file has a '.aag' or '.aag.gz' suffix or '-a'\n" \
"is used then the output is written in ASCII format, otherwise in\n" \
"in binary format.  Input files and output files can be compressed\n" \
"by GZIP if they are not 'stdin' or 'stdout' respectively.  The name of\n" \
"a compressed file needs to have a '.gz' suffix.\n"

int
aigToAag (int argc, char** argv);


#endif