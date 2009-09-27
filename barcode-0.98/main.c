/*
 * main.c - a commandline frontend for the barcode library
 *
 * Copyright (c) 1999 Michele Comitini (mcm@glisco.it)
 * Copyright (c) 1999 Alessandro Rubini (rubini@gnu.org)
 * Copyright (c) 1999 Prosa Srl. (prosa@prosa.it)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */
	
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "cmdline.h"
#include "barcode.h"

#ifndef NO_LIBPAPER
#include <paper.h>
#endif

/*
 * Most of this file deals with command line options, by exploiting
 * the cmdline.[ch] engine to offer defaults via environment variables
 * and handling functions for complex options.
 *
 * In order to offer a friendly interface (for those who feel the
 * cmdline *is* friendly, like me), we have to convert names to enums...
 */

struct {
    char *name;
    int type;
} encode_tab[] = {
    {"ean",      BARCODE_EAN},
    {"ean13",    BARCODE_EAN},
    {"ean-13",   BARCODE_EAN},
    {"ean8",     BARCODE_EAN},
    {"ean-8",    BARCODE_EAN},
    {"upc",      BARCODE_UPC},
    {"upc-a",    BARCODE_UPC},
    {"upc-e",    BARCODE_UPC},
    {"isbn",     BARCODE_ISBN},
    {"39",       BARCODE_39},
    {"code39",   BARCODE_39},
    {"128c",     BARCODE_128C},
    {"code128c", BARCODE_128C},
    {"128b",     BARCODE_128B},
    {"code128b", BARCODE_128B},
    {"128",      BARCODE_128},
    {"code128",  BARCODE_128},
    {"128raw",   BARCODE_128RAW},
    {"i25",      BARCODE_I25},
    {"interleaved 2 of 5", BARCODE_I25},
    {"cbr",      BARCODE_CBR},
    {"codabar",  BARCODE_CBR},
    {"msi",     BARCODE_MSI},
    {"pls",      BARCODE_PLS},
    {"plessey",  BARCODE_PLS},
    {"code93",   BARCODE_93},
    {"93",       BARCODE_93},

    {NULL, 0}
};

/*
 * Get encoding type from string rapresentation.
 * Returns -1 on error.
 */
#ifndef HAVE_STRCASECMP /* some libs (windows, for example) have stricmp */
#  define strcasecmp stricmp
#endif

int encode_id(char *encode_name)
{
    int i;
    for (i = 0;  encode_tab[i].name; i++)
	if (!strcasecmp(encode_tab[i].name, encode_name))
	    return encode_tab[i].type;
    return -1;
}

int list_encodes(FILE *f) /* used in the help message */
{
    int prev = -1;
    int i;

    fprintf(f, "Known encodings are (synonyms appear on the same line):");
    for (i = 0;  encode_tab[i].name; i++) {
	if (encode_tab[i].type != prev)
	    fprintf(f, "\n\t");
	else
	    fprintf(f, ", ");
	fprintf(f, "\"%s\"", encode_tab[i].name);
	prev = encode_tab[i].type;
    }
    fprintf(f, "\n");
    return 0;
}


/*
 * Variables to hold cmdline arguments (or defaults)
 */

char *ifilename, *ofilename;
int encoding_type;                    /* filled by get_encoding() */
int code_width, code_height;          /* "-g" for standalone codes */
int lines, columns;                   /* "-t" for tables */
int xmargin0, ymargin0;               /* both for "-g" and "-t" */
int xmargin1, ymargin1;               /* same, but right and top */
int ximargin, yimargin;               /* "-m": internal margins */
int eps, pcl, ps, noascii, nochecksum; /* boolean flags */
int page_wid, page_hei;               /* page size in points */
char *page_name;                      /* name of the media */
double unit = 1.0;                    /* unit specification */

char *prgname;  /* used to print error msgs, initialized to argv[0] by main */

/*
 * Functions to handle command line arguments
 */

struct encode_item {
    char *string;
    struct encode_item *next;
} *list_head, *list_tail;

/* each "-b" option adds a string to the input pool allocating its space */
int get_input_string(void *arg)
{
    struct encode_item *item = malloc(sizeof(*item));
    if (!item) {
	fprintf(stderr, "%s: malloc: %s\n", prgname, strerror(errno));
	return -2;
    }
    item->string = strdup(arg);
    if (!list_head) {
	list_head = list_tail = item;
    } else {
	list_tail->next = item;
	list_tail = item;
    }
    item->next = NULL;
    return 0;
}

/* and this function extracts strings from the pool */
unsigned char *retrieve_input_string(FILE *ifile)
{
    char *string;
    static char fileline[128];

    struct encode_item *item = list_head;
    if (list_tail) { /* this means at least one "-b" was specified */
	if (!item)
	    return NULL; /* the list is empty */
	string = item->string;
	list_head = item->next;
	free(item);
	return string;
    }

    /* else,  read from the file */
    if (!fgets(fileline, 128, ifile))
	return NULL;
    if (fileline[strlen(fileline)-1]=='\n')
	fileline[strlen(fileline)-1]= '\0';
    return strdup(fileline);
}

/* accept a unit specification */
int get_unit(void *arg)
{
    static struct {
	char *str;
	double unit;
    } *ptr, unittab[] = {
	{"pt",  1.0},
	{"in",  72.0},
	{"cm",  72.0/2.54},
	{"mm",  72.0/25.4},
	{NULL, 0.0}
    };

    for (ptr = unittab; ptr->str && strcmp((char *)arg, ptr->str); ptr++)
	;
    unit = ptr->unit;
    if (ptr->str) return 0;

    fprintf(stderr, "%s: incorrect unit \"%s\" (use one of",
	    prgname, (char *)arg);
    for (ptr = unittab; ptr->str; ptr++)
	fprintf(stderr, " \"%s\"", ptr->str);
    fprintf(stderr, ")\n");
    return -2;
}

/* convert an encoding name to an encoding integer code */
int get_encoding(void *arg)
{
    encoding_type = encode_id((char *)arg);
    if (encoding_type >=0) return 0;
    fprintf(stderr, "%s: wrong encoding \"%s\"\n", prgname,
	    (char *)arg);
    return -2; /* error, no help */
}

/* convert a geometry specification */
int get_geometry(void *arg)
{
    double w = 0.0, h = 0.0;
    double x = 0.0, y = 0.0;
    int n;

    if (((char *)arg)[0]=='+') {
	n = sscanf((char *)arg, "+%lf+%lf%s", &x, &y, (char *)arg);
    } else {
	n = sscanf((char *)arg, "%lfx%lf+%lf+%lf%s", &w, &h, &x, &y,
		   (char *)arg);
    }
    if (n!=4 && n!=2) {
	fprintf(stderr, "%s: wrong geometry \"%s\"\n", prgname, (char *)arg);
	return -2;
    }
    /* convert to points */
    code_width  = w * unit;
    code_height = h * unit;
    xmargin0 = x * unit;
    ymargin0 = y * unit;
    return 0;
}

/* convert a geometry specification */
int get_table(void *arg)
{
    double x0 = 0.0, y0 = 0.0, x1 = 0.0, y1 = 0.0;
    int n;

    n = sscanf((char *)arg, "%dx%d+%lf+%lf-%lf-%lf",
	       &columns, &lines, &x0, &y0, &x1, &y1);

    if (n==1 || n==3) { /* error: 2, 4, 5, 6 are fine */
	fprintf(stderr, "%s: wrong table specification \"%s\"\n", prgname,
		(char *)arg);
	return -2;
    }
    if (n < 6) y1 = y0; /* symmetric by default */
    if (n < 5) x1 = x0;

    /* convert and return */
    xmargin0 = x0 * unit;
    ymargin0 = y0 * unit;
    xmargin1 = x1 * unit;
    ymargin1 = y1 * unit;
    return 0;
}

/* convert an internal margin specification */
int get_margin(void *arg)
{
    char separator;
    double x,y;
    int n;

    /* accept one number or two, separated by any char */
    n = sscanf((char *)arg, "%lf%c%lf", &x, &separator, &y);

    if (n==1) {
	n=3; y = x;
    }
    if (n==3) {
	ximargin = x * unit;
	yimargin = y * unit;
	return 0;
    }
    fprintf(stderr, "%s: wrong margin specification \"%s\"\n", prgname,
	    (char *)arg);
	return -2;
    return 0;
}

/* convert a page geometry specification */
int get_page_geometry(void *arg)
{
    int n;
    double dpw, dph; /* page width, height in mm or inches */
    static char tmpstr[20];
    page_name = arg; /* if undecipherable, we won't run the program :) */
    /*
     * try to decode a "mm" string (eg. "210mmx297mm" or "210x297mm")
     */
    n = sscanf((char *)arg, "%lfmmx%lf", &dpw, &dph);
    if (n != 2 && strlen(arg)<20) {
	n =  sscanf((char *)arg, "%lfx%lf%s", &dpw, &dph, tmpstr);
	if (n == 3 && !strcmp(tmpstr, "mm")) {
	    /* Ok, convert to points: 1in is 25.4mm, 1in is also 72p */
	    page_wid = (int)(dpw / 25.4 * 72.0 + 0.5);
	    page_hei = (int)(dph / 25.4 * 72.0 + 0.5);
	    return 0;
	}
    }

    /*
     * try to decode an "in" string (eg. "8.5inx11in" or "8.5x11in")
     */
    n = sscanf((char *)arg, "%lfinx%lf", &dpw, &dph);
    if (n != 2 && strlen(arg)<20) {
	n =  sscanf((char *)arg, "%lfx%lf%s", &dpw, &dph, tmpstr);
	if (n == 3 && !strcmp(tmpstr, "in")) {
	    page_wid = (int)(dpw * 72.0 + 0.5); /* round to points */
	    page_hei = (int)(dph * 72.0 + 0.5);
	    return 0;
	}
    }

    /*
     * try to decode a numeric specification
     */
    n = sscanf((char *)arg, "%lfx%lf", &dpw, &dph);
    if (n == 2) {
	page_wid = dpw * unit;
	page_hei = dph * unit;
	if (unit != 1.0) { /* rebuild the page name */
	    page_name = malloc(32); /* big, to avoid snprintf, missing on HP */
	    if (page_name)
		sprintf(page_name, "%dx%d\n", page_wid, page_hei);
	}
	return 0;
    }

#ifndef NO_LIBPAPER
    /*
     * try to use libpaper, since it is available
     */
    {
    const struct paper* paptr;

    paperinit();
    paptr = paperinfo(arg);
    if (!paptr) { /* unknown name */
	paperdone();
	return -1;
    }
    page_wid = (int)(paperpswidth(paptr) + 0.5);
    page_hei = (int)(paperpsheight(paptr) + 0.5);
    paperdone();
    return 0;
    }
#endif
    /* If we got here, the argument is undecipherable: fail */
    fprintf(stderr, "%s: wrong page size specification \"%s\"\n", prgname,
	    (char *)arg);
    return -2;
}

/*
 * The table of possible arguments
 */
struct commandline option_table[] = {
    {'i', CMDLINE_S, &ifilename, NULL, NULL, NULL,
                   "input file (strings to encode), default is stdin"},
    {'o', CMDLINE_S, &ofilename, NULL, NULL, NULL,
                    "output file, default is stdout"},
    {'b', CMDLINE_S, NULL, get_input_string, NULL, NULL,
                   "string to encode (use input file if missing)"},
    {'e', CMDLINE_S, NULL, get_encoding, "BARCODE_ENCODING", NULL,
                   "encoding type (default is best fit for first string)"},
    {'u', CMDLINE_S, NULL, get_unit, "BARCODE_UNIT", NULL,
                    "unit (\"mm\", \"in\", ...) used to decode -g, -t, -p"},
    {'g', CMDLINE_S, NULL, get_geometry, "BARCODE_GEOMETRY", NULL,
                    "geometry on the page: [<wid>x<hei>][+<margin>+<margin>]"},
    {'t', CMDLINE_S, NULL, get_table, "BARCODE_TABLE", NULL,
                    "table geometry: <cols>x<lines>[+<margin>+<margin>]"},
    {'m', CMDLINE_S, NULL, get_margin, "BARCODE_MARGIN", "10",
                    "internal margin for each item in a table: <xm>[,<ym>]"},
    {'n', CMDLINE_NONE, &noascii, NULL, NULL, NULL,
                    "\"numeric\": avoid printing text along with the bars"},
    {'c', CMDLINE_NONE, &nochecksum, NULL, NULL, NULL,
                    "no Checksum character, if the chosen encoding allows it"},
    {'E', CMDLINE_NONE, &eps, NULL, NULL, NULL,
                    "print one code as eps file (default: multi-page ps)"},
    {'P', CMDLINE_NONE, &pcl, NULL, NULL, NULL,
                    "create PCL output instead of postscript"},
    {'p', CMDLINE_S, NULL, get_page_geometry, NULL, NULL,
                    "page size (refer to the man page)"},
    {0,}
};

#ifdef NO_STRERROR
/*
 * A strerror replacement (thanks to Thad Floryan <thad@thadlabs.com>)
 */
char *strerror(int error)
{
    static char msg[16];
    if (error >= 0 && error < sys_nerr)
	return sys_errlist[error];
    sprintf(msg, "Error %d", error);
    return msg;
}
#endif
	 
/*
 * The main function
 */
int main(int argc, char **argv)
{
    struct Barcode_Item * bc;
    FILE *ifile = stdin;
    FILE *ofile = stdout;
    char *line;
    int flags=0; /* for the library */
    int page, retval;

    prgname = argv[0];

    /* First of all, accept "--help" and "-h" as a special case */
    if (argc == 2 && (!strcmp(argv[1],"--help") || !strcmp(argv[1],"-h"))) {
	commandline_errormsg(stderr, option_table, argv[0], "Options:\n");
	fprintf(stderr,"\n");
	list_encodes(stderr);
	exit(1);
    }
    /* Also, accept "--version" as a special case */
    if (argc == 2 && (!strcmp(argv[1],"--version"))) {
	printf("barcode frontend (GNU barcode) " BARCODE_VERSION "\n");
	exit(0);
    }

    /* Otherwise, parse the commandline */
    retval = commandline(option_table, argc, argv, "Use: %s [options]\n");
    if (retval) {
	if (retval == -1) /* help printed, complete it */
	    list_encodes(stderr);
	else /* no help printed, suggest it */
	    fprintf(stderr, "%s: try \"%s --help\"\n", prgname, prgname);
	exit(1);
    }

    /* If no paper size has been specified, use the default, if any */
    if (!page_name) {
	page_wid = 595; page_hei = 842;
	page_name = "A4"; /* I live in Europe :) */
#ifndef NO_LIBPAPER
	get_page_geometry(systempapername()); /* or the system default */
#endif
    }

    /* FIXME: print warnings for incompatible options */

    /* open the input stream if specified */
    if (ifilename)
	ifile = fopen(ifilename,"r");
    if (!ifile) {
	fprintf(stderr, "%s: %s: %s\n", argv[0], ifilename,
		strerror(errno));
	exit(1);
    }

    /* open the output stream if specified */
    if (ofilename)
	ofile = fopen(ofilename,"w");
    if (!ofile) {
	fprintf(stderr, "%s: %s: %s\n", argv[0], ofilename,
		strerror(errno));
	exit(1);
    }

    if (encoding_type < 0) { /* unknown type specified */
	fprintf(stderr,"%s: Unknown endoding. Try \"%s --help\"\n",
 		argv[0], argv[0]);
	exit(1);
    }
    flags |= encoding_type;  
    if (pcl) {
	flags |= BARCODE_OUT_PCL;
    } else {
	ps = !eps; /* a shortcut */
	if (eps)
	    flags |= BARCODE_OUT_EPS; /* print headers too */
	else
	    flags |= BARCODE_OUT_PS | BARCODE_OUT_NOHEADERS;
    }
    if (noascii)
	flags |= BARCODE_NO_ASCII;
    if (nochecksum)
	flags |= BARCODE_NO_CHECKSUM;

    /* the table is not available in eps mode */
    if (eps && (lines>1 || columns>1)) {
	fprintf(stderr, "%s: can't print tables in EPS format\n",argv[0]);
	exit(1);
    }

    if (ps) { /* The header is independent of single/table mode */
	/* Headers. Don't let the library do it, we may need multi-page */
	fprintf(ofile, "%%!PS-Adobe-2.0\n");
	/* It would be nice to know the bounding box. Leave it alone */
	fprintf(ofile, "%%%%Creator: \"barcode\", "
		"libbarcode sample frontend\n");
	if (page_name)
	    fprintf(ofile, "%%%%DocumentPaperSizes: %s\n", page_name);
	fprintf(ofile, "%%%%EndComments\n");
	fprintf(ofile, "%%%%EndProlog\n\n");
    }

    /*
     * Here we are, ready to work. Handle the one-per-page case first,
     * as it is shorter.
     */
    if (!lines && !columns) {
	page = 0;
	while ( (line = retrieve_input_string(ifile)) ) {
	    page++;
	    if (ps) {
		fprintf(ofile, "%%%%Page: %i %i\n\n",page,page);
	    }
	    if (Barcode_Encode_and_Print(line, ofile, code_width, code_height,
					 xmargin0, ymargin0, flags) < 0) {
		fprintf(stderr, "%s: can't encode \"%s\"\n", argv[0], line);
	    }
	    if (eps) break; /* if output is eps, do it once only */
	    if (ps) fprintf(ofile, "showpage\n");
	    if (pcl) fprintf(ofile, "\f");
	}
	/* no more lines, print footers */
	if (ps) {
	    fprintf(ofile, "%%%%Trailer\n\n");
	}
    } else {

	/* table mode, the header has been already printed */
	
	int xstep = (page_wid - xmargin0 - xmargin1)/columns;
	int ystep = (page_hei - ymargin0 - ymargin1)/lines;
	int x = columns, y = -1; /* position in the table, start off-page */

	if (!ximargin) ximargin = BARCODE_DEFAULT_MARGIN;
	if (!yimargin) yimargin = BARCODE_DEFAULT_MARGIN;
	/* Assign default size unless -g did it (Joachim Reichelt) */
	if ( !code_width && !code_height) {
	    code_width = xstep - 2*ximargin;
	    code_height = ystep - 2*yimargin;
	}

	page=0;
	while ( (line = retrieve_input_string(ifile)) ) {
	    x++;  /* fit x and y */
	    if (x >= columns) {
		x=0; y--;
		if (y<0) {
		    y = lines-1; page++;
		    /* flush page */
		    if (ps && page > 1) fprintf(ofile, "showpage\n");
		    if (pcl && page > 1) fprintf(ofile, "\f");
		    /* new page */
		    if (ps) fprintf(ofile, "%%%%Page: %i %i\n\n",page,page);
		}
	    }

	    /*
	     * Create a barcode item. This allows to set the margin to 0, as
	     * we have [xy]imargin to use. But don't use Encode_and_Print(),
	     * unroll it here instead
	     */
	    bc = Barcode_Create(line);
	    if (!bc) {
		fprintf(stderr, "%s: Barcode_Create(): %s\n", argv[0],
			strerror(errno));
		exit(1);
	    }
	    bc->margin = 0;
	    if ( (Barcode_Position(bc, code_width, code_height,
				   xmargin0 + ximargin + x * xstep,
				   ymargin0 + yimargin + y * ystep, 0.0) < 0)
		 || (Barcode_Encode(bc, flags) < 0)
		 || (Barcode_Print(bc, ofile, flags) < 0) ) {
		fprintf(stderr, "%s: can't encode \"%s\": %s\n", argv[0],
			line, strerror(bc->error));
	    }
	    Barcode_Delete(bc);
	}
	if (ps) fprintf(ofile, "showpage\n\n%%%%Trailer\n\n");
	if (pcl) fprintf(ofile, "\f");
    }
    return 0;
}



