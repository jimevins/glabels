/*
 *  bc-zint.c
 *  Copyright (C) 2001-2009  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of gLabels.
 *
 *  gLabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gLabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gLabels.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#ifdef HAVE_LIBZINT

#include "bc-zint.h"

#include <glib.h>
#include <ctype.h>
#include <string.h>
#include <zint.h> /* Zint */

#include "debug.h"


/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/
#define DEFAULT_W 144
#define DEFAULT_H  72


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/
static glBarcode *render_zint     (struct zint_symbol *symbol, gboolean text_flag);



/*****************************************************************************/
/* Generate intermediate representation of barcode.                          */
/*****************************************************************************/
glBarcode *
gl_barcode_zint_new (const gchar          *id,
			   gboolean        text_flag,
			   gboolean        checksum_flag,
			   gdouble         w,
			   gdouble         h,
			   const gchar    *digits)
{
	glBarcode           *gbc;
	struct zint_symbol  *symbol;
	gint                type;
	gint                result;
	gchar               errtxt[100];

	symbol = ZBarcode_Create();

	/* Auto set to default size */
	if ( (w == 0) && (h == 0) )
	{
		w = DEFAULT_W;
		h = DEFAULT_H;
	}

	/* Assign type flag.  Pre-filter by length for subtypes. */
	if (g_ascii_strcasecmp (id, "GS1-128") == 0) {
		symbol->symbology = BARCODE_EAN128;
	} else if (g_ascii_strcasecmp (id, "GTIN-13") == 0) {
		symbol->symbology = BARCODE_EANX;
	} else if (g_ascii_strcasecmp (id, "GTIN-14") == 0) {
		symbol->symbology = BARCODE_EAN14;
	} else {
		g_message( "Illegal barcode id %s", id );
		ZBarcode_Delete (symbol);
		return NULL;
	}

	/* Checksum not supported yet!! 
	if (!checksum_flag) {
		flags |= BARCODE_NO_CHECKSUM;
	}
	*/

	result = ZBarcode_Encode(symbol, (unsigned char *)digits, 0);
	if (result) {
		gl_debug (DEBUG_BARCODE, "Zint Error: %s", symbol->errtxt);
		strcpy(errtxt, symbol->errtxt);
		ZBarcode_Delete (symbol);
		// Invalid! Print out a Code128 barcode instead with message
		symbol = ZBarcode_Create();
		symbol->symbology = BARCODE_CODE128;
		result = ZBarcode_Encode(symbol, (unsigned char *)errtxt, 0);
		if (result) {
			gl_debug (DEBUG_BARCODE, "Zint Error: %s", symbol->errtxt);
			ZBarcode_Delete (symbol);
		  return NULL;
		}
	}

	/* Scale calculated after height, always maintain aspect ratio */
	symbol->scale = (w / symbol->width);
	symbol->height = h / symbol->scale; // height always in standard size

  /*
	 * With the size and scale set, send a request to Zint renderer
	 */
	if (!ZBarcode_Render(symbol, (unsigned int) !text_flag, (float) w, (float) h)) {
		ZBarcode_Delete(symbol);
		gl_debug(DEBUG_BARCODE, "Zint Rendering Error: %s", symbol->errtxt);
		return NULL;
	}

	/* Convert Sums provided by zint encode */
	gbc = render_zint(symbol, text_flag);

	ZBarcode_Delete(symbol);

	return gbc;
}


/*--------------------------------------------------------------------------
 * PRIVATE. Render to glBarcode the provided Zint Rendered Plot
 *
 * Takes the special zint_render structure and converts to glBarcode.
 *
 *--------------------------------------------------------------------------*/
static glBarcode *render_zint(struct zint_symbol *symbol, gboolean text_flag) {

	glBarcode     *gbc;
	glBarcodeLine *line;
	glBarcodeChar *bchar;

	struct zint_render        *render;
	struct zint_render_line   *zline;
	struct zint_render_string *zstring;

  gchar *p;
  gint length;
  gdouble xoffset, width, char_width;

	render = symbol->rendered;
	gbc = g_new0(glBarcode, 1);

	/*
	 * Go through each line, alter the x position for centering, and add.
	 * Zint already applies the scaling factor.
	 */
	zline = render->lines;
  while (zline) {
	  line = g_new0 (glBarcodeLine, 1);

		line->width = (gdouble) zline->width;
		line->length = (gdouble) zline->length;
		/* glBarcodeLine centers based on width, counter-act!!! */
		line->x = (gdouble) (zline->x + (zline->width / 2.0));
		line->y = (gdouble) zline->y;

		gbc->lines = g_list_append (gbc->lines, line);

		// g_message ("Zint Adding Line at: %f x %f dim: %f x %f", line->x, line->y, line->width, line->length);
		zline = zline->next;
	}

	/*
	 * Repeat loop for characters
	 */
	zstring = render->strings;
	if (text_flag) {
		while (zstring) {

      // Guess the first position based on width or number of characters
      length = strlen(zstring->text);
      width = zstring->width;
      if (width > 0) {
        char_width = width / length;
      } else {
        // No width, set our own by removing 30% of height
        char_width = zstring->fsize - (zstring->fsize * 0.3);
        width = char_width * length;
      }
      xoffset = zstring->x - (width / 2.0);

      for (p = (gchar *) zstring->text; *p != 0; p++) {

        bchar = g_new0 (glBarcodeChar, 1);
        bchar->x = xoffset;
        bchar->y = (gdouble) zstring->y;
        bchar->fsize = (gdouble) zstring->fsize;
        bchar->c = (gchar) *p;

        gbc->chars = g_list_append (gbc->chars, bchar);

        xoffset += char_width;
      }

			zstring = zstring->next;
		}
	}

	/*
	 * Finally add complete sizes
	 */
  //gbc->width = (gdouble) render->width;
	//gbc->height = (gdouble) render->height;

	
	return gbc;
}

#endif /* HAVE_LIBZINT */

/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
