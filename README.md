# pst
Postscript displays of binary trees

This code was originally written by Dr. Paul Utgoff, Professor of Computer
Science at the University of Massachusetts at Amherst.  I later modified it for
my purposes. This code was provided to me without a license or restrictions,
but I cannot claim to own own most of it and cannot provide a license.

The PST program constructs a postscript file that contains a plot of a binary
tree.  The program will place the plot on as many pages as needed, including
directions for how to cut and paste the pages.  One can specify the font size
to control the overall size of the plot.

To run PST:
```
> pst [-f{font name}] [-s{font size}] file
```

The `file' argument must be the name of a tree file.  See the sample files to
see how a tree file should be formatted.  The output is a postscript file with
the same name as the original but with a '.ps' extension.

Options:

	-f  Use the specified font.  See the included fonts directory 
            for the possibilities.  The default is Helvetica-Narrow.

	-s  Use the specified font size.  The default is 6.0 (point);

