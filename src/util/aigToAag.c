
#include "aigToAag.h"

int
aigToAag (int argc, char** argv)
{
  const char *src, *dst, *src_name, *dst_name, *error;
  int verbose, ascii, strip, res;
  stream reader, writer;
  aiger_mode mode;
  memory memory;
  aiger *aiger;
  unsigned i;

  res = verbose = ascii = strip = 0;
  src_name = dst_name = src = dst = 0;

  for (i = 0; i < argc; i++)
    {
      if (!strcmp (argv[i], "-h"))
	{
	  fprintf (stderr, USAGE);
	  exit (0);
	}
      else if (!strcmp (argv[i], "-v"))
	verbose = 1;
      else if (!strcmp (argv[i], "-s"))
	strip = 1;
      else if (!strcmp (argv[i], "-a"))
	ascii = 1;
      else if (argv[i][0] == '-' && argv[i][1])
	die ("invalid command line option '%s'", argv[i]);
      else if (!src_name)
	{
	  if (!strcmp (argv[i], "-"))
	    {
	      src = 0;
	      src_name = "<stdin>";
	    }
	  else
	    src = src_name = argv[i];
	}
      else if (!dst_name)
	{
	  if (!strcmp (argv[i], "-"))
	    {
	      dst = 0;
	      dst_name = "<stdout>";
	    }
	  else
	    dst = dst_name = argv[i];
	}
      else
	die ("more than two files specified");
    }

  if (dst && ascii)
    die ("'dst' file and '-a' specified");

  if (!dst && !ascii && isatty (1))
    ascii = 1;

  if (src && dst && !strcmp (src, dst))
    die ("identical 'src' and 'dst' file");

  memory.max = memory.bytes = 0;
  aiger = aiger_init_mem (&memory,
			  (aiger_malloc) aigtoaig_malloc,
			  (aiger_free) aigtoaig_free);
  if (src)
    {
      error = aiger_open_and_read_from_file (aiger, src);
      if (error)
	{
	READ_ERROR:
	  fprintf (stderr, "*** [aigtoaig] %s\n", error);
	  res = 1;
	}
      else
	{
	  reader.bytes = size_of_file (src);

	  if (verbose)
	    {
	      fprintf (stderr,
		       "[aigtoaig] read from '%s' (%.0f bytes)\n",
		       src, (double) reader.bytes);
	      fflush (stderr);
	    }
	}
    }
  else
    {
      reader.file = stdin;
      reader.bytes = 0;

      error = aiger_read_generic (aiger, &reader, (aiger_get) aigtoaig_get);

      if (error)
	goto READ_ERROR;

      if (verbose)
	{
	  fprintf (stderr,
		   "[aigtoaig] read from '<stdin>' (%.0f bytes)\n",
		   (double) reader.bytes);
	  fflush (stderr);
	}
    }

  if (!res)
    {
      if (strip)
	{
	  i = aiger_strip_symbols_and_comments (aiger);

	  if (verbose)
	    {
	      fprintf (stderr, "[aigtoaig] stripped %u symbols\n", i);
	      fflush (stderr);
	    }
	}

      if (dst)
	{
	  if (aiger_open_and_write_to_file (aiger, dst))
	    {
	      writer.bytes = size_of_file (dst);

	      if (verbose)
		{
		  fprintf (stderr,
			   "[aigtoaig] wrote to '%s' (%.0f bytes)\n",
			   dst, (double) writer.bytes);
		  fflush (stderr);
		}
	    }
	  else
	    {
	      unlink (dst);
	    WRITE_ERROR:
	      fprintf (stderr, "*** [aigtoai]: write error\n");
	      res = 1;
	    }
	}
      else
	{
	  writer.file = stdout;
	  writer.bytes = 0;

	  if (ascii)
	    mode = aiger_ascii_mode;
	  else
	    mode = aiger_binary_mode;

	  if (!aiger_write_generic (aiger, mode,
				    &writer, (aiger_put) aigtoaig_put))
	    goto WRITE_ERROR;

	  if (verbose)
	    {
	      fprintf (stderr,
		       "[aigtoaig] wrote to '<stdout>' (%.0f bytes)\n",
		       (double) writer.bytes);
	      fflush (stderr);
	    }
	}
    }

  aiger_reset (aiger);

  if (!res && verbose)
    {
      if (reader.bytes > writer.bytes)
	fprintf (stderr, "[aigtoaig] deflated to %.1f%%\n",
		 PERCENT (writer.bytes, reader.bytes));
      else
	fprintf (stderr, "[aigtoaig] inflated to %.1f%%\n",
		 PERCENT (writer.bytes, reader.bytes));

      fprintf (stderr,
	       "[aigtoaig] allocated %.0f bytes maximum\n", memory.max);

      fflush (stderr);
    }

  return res;
}