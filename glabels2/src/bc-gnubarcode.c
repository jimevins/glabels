/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  bc-gnubarcode.c:  front-end to GNU-barcode-library module
 *
 *  Copyright (C) 2001-2003  Jim Evins <evins@snaught.com>.
 *
 *  Some of this code is borrowed from the postscript renderer (ps.c)
 *  from the GNU barcode library:
 *
 *     Copyright (C) 1999 Alessaandro Rubini (rubini@gnu.org)
 *     Copyright (C) 1999 Prosa Srl. (prosa@prosa.it)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <config.h>

#include <ctype.h>
#include <string.h>

#include "bc.h"
#include "bc-gnubarcode.h"
#include "barcode.h"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/
#define SHRINK_AMOUNT 0.15	/* shrink bars to account for ink spreading */
#define FONT_SCALE    0.95	/* Shrink fonts just a hair */

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/
static glBarcode *render_pass1     (struct Barcode_Item *bci,
				    gint                 flags);

static gboolean   is_length_valid  (const gchar         *digits,
				    gint                 n1,
				    gint                 n2);

static gboolean   is_length1_valid (const gchar         *digits,
				    gint                 n1,
				    gint                 n2);

static gboolean   is_length2_valid (const gchar         *digits,
				    gint                 n1,
				    gint                 n2);


/*****************************************************************************/
/* Generate intermediate representation of barcode.                          */
/*****************************************************************************/
glBarcode *
gl_barcode_gnubarcode_new (const gchar    *id,
			   gboolean        text_flag,
			   gboolean        checksum_flag,
			   gdouble         w,
			   gdouble         h,
			   const gchar    *digits)
{
	glBarcode           *gbc;
	struct Barcode_Item *bci;
	gint                 flags;

	/* Assign type flag.  Pre-filter by length for subtypes. */
	if (g_strcasecmp (id, "EAN") == 0) {
		flags = BARCODE_EAN;
	} else if (g_strcasecmp (id, "EAN-8") == 0) {
		if (!is_length_valid (digits, 7, 8)) {
			return NULL;
		}
		flags = BARCODE_EAN;
	} else if (g_strcasecmp (id, "EAN-8+2") == 0) {
		if (!is_length1_valid (digits, 7, 8) || !is_length2_valid (digits, 2, 2)) {
			return NULL;
		}
		flags = BARCODE_EAN;
	} else if (g_strcasecmp (id, "EAN-8+5") == 0) {
		if (!is_length1_valid (digits, 7, 8) || !is_length2_valid (digits, 5, 5)) {
			return NULL;
		}
		flags = BARCODE_EAN;
	} else if (g_strcasecmp (id, "EAN-13") == 0) {
		if (!is_length_valid (digits, 12, 13)) {
			return NULL;
		}
		flags = BARCODE_EAN;
	} else if (g_strcasecmp (id, "EAN-13+2") == 0) {
		if (!is_length1_valid (digits, 12,13) || !is_length2_valid (digits, 2,2)) {
			return NULL;
		}
		flags = BARCODE_EAN;
	} else if (g_strcasecmp (id, "EAN-13+5") == 0) {
		if (!is_length1_valid (digits, 12,13) || !is_length2_valid (digits, 5,5)) {
			return NULL;
		}
		flags = BARCODE_EAN;
	} else if (g_strcasecmp (id, "UPC") == 0) {
		flags = BARCODE_UPC;
	} else if (g_strcasecmp (id, "UPC-A") == 0) {
		if (!is_length_valid (digits, 11, 12)) {
			return NULL;
		}
		flags = BARCODE_UPC;
	} else if (g_strcasecmp (id, "UPC-A+2") == 0) {
		if (!is_length1_valid (digits, 11,12) || !is_length2_valid (digits, 2,2)) {
			return NULL;
		}
		flags = BARCODE_UPC;
	} else if (g_strcasecmp (id, "UPC-A+5") == 0) {
		if (!is_length1_valid (digits, 11,12) || !is_length2_valid (digits, 5,5)) {
			return NULL;
		}
		flags = BARCODE_UPC;
	} else if (g_strcasecmp (id, "UPC-E") == 0) {
		if (!is_length_valid (digits, 6, 8)) {
			return NULL;
		}
		flags = BARCODE_UPC;
	} else if (g_strcasecmp (id, "UPC-E+2") == 0) {
		if (!is_length1_valid (digits, 6, 8) || !is_length2_valid (digits, 2,2)) {
			return NULL;
		}
		flags = BARCODE_UPC;
	} else if (g_strcasecmp (id, "UPC-E+5") == 0) {
		if (!is_length1_valid (digits, 6, 8) || !is_length2_valid (digits, 5,5)) {
			return NULL;
		}
		flags = BARCODE_UPC;
	} else if (g_strcasecmp (id, "ISBN") == 0) {
		if (!is_length_valid (digits, 9, 10)) {
			return NULL;
		}
		flags = BARCODE_ISBN;
	} else if (g_strcasecmp (id, "ISBN+5") == 0) {
		if (!is_length1_valid (digits, 9, 10) || !is_length2_valid (digits, 5,5)) {
			return NULL;
		}
		flags = BARCODE_ISBN;
	} else if (g_strcasecmp (id, "Code39") == 0) {
		flags = BARCODE_39;
	} else if (g_strcasecmp (id, "Code128") == 0) {
		flags = BARCODE_128;
	} else if (g_strcasecmp (id, "Code128C") == 0) {
		flags = BARCODE_128C;
	} else if (g_strcasecmp (id, "Code128B") == 0) {
		flags = BARCODE_128B;
	} else if (g_strcasecmp (id, "I25") == 0) {
		flags = BARCODE_I25;
	} else if (g_strcasecmp (id, "CBR") == 0) {
		flags = BARCODE_CBR;
	} else if (g_strcasecmp (id, "MSI") == 0) {
		flags = BARCODE_MSI;
	} else if (g_strcasecmp (id, "PLS") == 0) {
		flags = BARCODE_PLS;
	} else {
		g_warning( "Illegal barcode id %s", id );
		flags = BARCODE_ANY;
	}


	bci = Barcode_Create ((char *)digits);

	/* First encode using GNU Barcode library */
	if (!text_flag) {
		flags |= BARCODE_NO_ASCII;
	}
	if (!checksum_flag) {
		flags |= BARCODE_NO_CHECKSUM;
	}

	bci->scalef = 0.0;
	bci->width  = w;
	bci->height = h;

	Barcode_Encode (bci, flags);
	if (!bci->partial || !bci->textinfo) {
		g_warning ("Barcode Data Invalid");
		Barcode_Delete (bci);
		return NULL;
	}

	/* now render with our custom back-end,
	   to create appropriate intermdediate format */
	gbc = render_pass1 (bci, flags);

	Barcode_Delete (bci);
	return gbc;
}

/*--------------------------------------------------------------------------
 * PRIVATE.  Render to glBarcode intermediate representation of barcode.
 *
 *  Some of this code is borrowed from the postscript renderer (ps.c)
 *  from the GNU barcode library:
 *
 *     Copyright (C) 1999 Alessaandro Rubini (rubini@gnu.org)
 *     Copyright (C) 1999 Prosa Srl. (prosa@prosa.it)
 *
 *--------------------------------------------------------------------------*/
static glBarcode *
render_pass1 (struct Barcode_Item *bci,
	      gint                 flags)
{
	gint           validbits = BARCODE_NO_ASCII;
	glBarcode     *gbc;
	glBarcodeLine *line;
	glBarcodeChar *bchar;
	gdouble        scalef = 1.0;
	gdouble        x;
	gint           i, j, barlen;
	gdouble        f1, f2;
	gint           mode = '-'; /* text below bars */
	gdouble        x0, y0, yr;
	guchar        *p, c;

	if (bci->width > (2*bci->margin)) {
		bci->width -= 2*bci->margin;
	}
	if (bci->height > (2*bci->margin)) {
		bci->height -= 2*bci->margin;
	}

	/* If any flag is clear in "flags", inherit it from "bci->flags" */
	if (!(flags & BARCODE_NO_ASCII)) {
		flags |= bci->flags & BARCODE_NO_ASCII;
	}
	flags = bci->flags = (flags & validbits) | (bci->flags & ~validbits);

	/* First calculate barlen */
	barlen = bci->partial[0] - '0';
	for (p = bci->partial + 1; *p != 0; p++) {
		if (isdigit (*p)) {
			barlen += *p - '0';
		} else {
			if ((*p != '+') && (*p != '-')) {
				barlen += *p - 'a' + 1;
			}
		}
	}

	/* The scale factor depends on bar length */
	if (!bci->scalef) {
		if (!bci->width) bci->width = barlen; /* default */
		scalef = bci->scalef = (double)bci->width / (double)barlen;
		if (scalef < 0.5) scalef = 0.5;
	}

	/* The width defaults to "just enough" */
	bci->width = barlen * scalef + 1;

	/* But it can be too small, in this case enlarge and center the area */
	if (bci->width < barlen * scalef) {
		int wid = barlen * scalef + 1;
		bci->xoff -= (wid - bci->width)/2 ;
		bci->width = wid;
		/* Can't extend too far on the left */
		if (bci->xoff < 0) {
			bci->width += -bci->xoff;
			bci->xoff = 0;
		}
	}

	/* The height defaults to 80 points (rescaled) */
	if (!bci->height)
		bci->height = 80 * scalef;

	/* If too small (5 + text), reduce the scale factor and center */
	i = 5 + 10 * ((bci->flags & BARCODE_NO_ASCII)==0);
	if (bci->height < i * scalef ) {
#if 0
		double scaleg = ((double)bci->height) / i;
		int wid = bci->width * scaleg / scalef;
		bci->xoff += (bci->width - wid)/2;
		bci->width = wid;
		scalef = scaleg;
#else
		bci->height = i * scalef;
#endif
	}

	gbc = g_new0 (glBarcode, 1);

	/* Now traverse the code string and create a list of lines */
	x = bci->margin + (bci->partial[0] - '0') * scalef;
	for (p = bci->partial + 1, i = 1; *p != 0; p++, i++) {
		/* special cases: '+' and '-' */
		if (*p == '+' || *p == '-') {
			mode = *p;	/* don't count it */
			i++;
			continue;
		}
		/* j is the width of this bar/space */
		if (isdigit (*p))
			j = *p - '0';
		else
			j = *p - 'a' + 1;
		if (i % 2) {	/* bar */
			x0 = x + (j * scalef) / 2;
			y0 = bci->margin;
			yr = bci->height;
			if (!(bci->flags & BARCODE_NO_ASCII)) {	/* leave space for text */
				if (mode == '-') {
					/* text below bars: 10 or 5 points */
					yr -= (isdigit (*p) ? 10 : 5) * scalef;
				} else {	/* '+' */
					/* above bars: 10 or 0 from bottom,
					   and 10 from top */
					y0 += 10 * scalef;
					yr -= (isdigit (*p) ? 20 : 10) * scalef;
				}
			}
			line = g_new0 (glBarcodeLine, 1);
			line->x = x0;
			line->y = y0;
			line->length = yr;
			line->width = (j * scalef) - SHRINK_AMOUNT;
			gbc->lines = g_list_append (gbc->lines, line);
		}
		x += j * scalef;

	}

	/* Now the text */
	mode = '-';		/* reinstantiate default */
	if (!(bci->flags & BARCODE_NO_ASCII)) {
		for (p = bci->textinfo; p; p = strchr (p, ' ')) {
			while (*p == ' ')
				p++;
			if (!*p)
				break;
			if (*p == '+' || *p == '-') {
				mode = *p;
				continue;
			}
			if (sscanf (p, "%lf:%lf:%c", &f1, &f2, &c) != 3) {
				g_warning ("impossible data: %s", p);
				continue;
			}
			bchar = g_new0 (glBarcodeChar, 1);
			bchar->x = f1 * scalef + bci->margin;
			if (mode == '-') {
				bchar->y =
				    bci->margin + bci->height - 8 * scalef;
			} else {
				bchar->y = bci->margin;
			}
			bchar->fsize = f2 * FONT_SCALE * scalef;
			bchar->c = c;
			gbc->chars = g_list_append (gbc->chars, bchar);
		}
	}

	/* Fill in other info */
	gbc->height = bci->height + 2.0 * bci->margin;
	gbc->width = bci->width + 2.0 * bci->margin;

#if 0
	g_print ("w=%f, h=%f\n", gbc->width, gbc->height);
#endif

	return gbc;
}

/*--------------------------------------------------------------------------*/
/* Validate specific length of string (for subtypes).                       */
/*--------------------------------------------------------------------------*/
static gboolean
is_length_valid (const gchar *digits,
		 gint         n1,
		 gint         n2)
{
	gchar *p;
	gint   i;

	if (!digits) {
		return FALSE;
	}

	for (p = (gchar *)digits, i=0; *p != 0; p++) {
		if (g_ascii_isdigit (*p)) {
			i++;
		}
	}

	return (i >= n1) && (i <= n2);
}

/*--------------------------------------------------------------------------*/
/* Validate specific length of string (for subtypes).                       */
/*--------------------------------------------------------------------------*/
static gboolean
is_length1_valid (const gchar *digits,
		  gint         n1,
		  gint         n2)
{
	gchar *p;
	gint   i;

	if (!digits) {
		return FALSE;
	}

	for (p = (gchar *)digits, i=0; !g_ascii_isspace (*p) && *p != 0; p++) {
		if (g_ascii_isdigit (*p)) {
			i++;
		}
	}

	return (i >= n1) && (i <= n2);
}

/*--------------------------------------------------------------------------*/
/* Validate specific length of second string (for subtypes).                */
/*--------------------------------------------------------------------------*/
static gboolean
is_length2_valid (const gchar *digits,
		  gint         n1,
		  gint         n2)
{
	gchar *p;
	gint   i;

	if (!digits) {
		return FALSE;
	}

	for (p = (gchar *)digits; !g_ascii_isspace (*p) && (*p != 0); p++) {
		/* Skip over 1st string */
	}

	for (i=0; *p != 0; p++) {
		if (g_ascii_isdigit (*p)) {
			i++;
		}
	}

	return (i >= n1) && (i <= n2);
}

