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
gint module_is_set(struct zint_symbol *symbol, gint y_coord, gint x_coord);



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
	gint                 type;
	gint		     result;

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

	// g_message ("Zint Requested Dimensions: %f x %f", w, h);

	result = ZBarcode_Encode(symbol, (unsigned char *)digits, 0);
	if (result) {
		ZBarcode_Delete (symbol);
		g_message ("Zint Error: %s", symbol->errtxt);
		return NULL;
	}

	/* Scale calculated after height, always maintain aspect ratio */
	// symbol->height = (h > 0.0 ? (gint)h : 50);
	symbol->scale = (w / symbol->width);
	symbol->height = h / symbol->scale; // height always in standard size


	/* Convert Sums provided by zint encode */
	gbc = render_zint(symbol, text_flag);

	// g_message ("Zint Barcode Dimensions: %f x %f", gbc->width, gbc->height);

	ZBarcode_Delete(symbol);

	return gbc;
}


/*--------------------------------------------------------------------------
 * PRIVATE. Render to glBarcode the provided Zint symbol.
 *
 * Based on the SVG output from Zint library, handles lots of otherwise
 * internal  Zint code to convert directly to glBarcode representation.
 *
 *--------------------------------------------------------------------------*/
static glBarcode *render_zint(struct zint_symbol *symbol, gboolean text_flag) {

	glBarcode     *gbc;
	glBarcodeLine *line;
	glBarcodeChar *bchar;
	
	gint i, r, block_width, latch, this_row;
	gfloat textpos, large_bar_height, preset_height, row_height, row_posn = 0.0;
	gint error_number = 0;
	gint textoffset, textheight, xoffset, yoffset, textdone, main_width;
	gchar textpart[10], addon[6];
	gint large_bar_count, comp_offset;
	gfloat addon_text_posn;
	gfloat default_text_posn;
	gfloat scaler = symbol->scale;
	gchar *p;

	gbc = g_new0(glBarcode, 1);

	row_height = 0;
	textdone = 0;
	main_width = symbol->width;
	strcpy(addon, "");
	comp_offset = 0;
	addon_text_posn = 0.0;

	if (symbol->height < 15) {
		symbol->height = 15;
	}
	// symbol->height = 50;

	if(text_flag && strlen(symbol->text) != 0) {
		textheight = 9.0;
		textoffset = 2.0;
	} else {
		textheight = textoffset = 0.0;
	}
	// Update height for texts
	symbol->height -= textheight + textoffset;

	large_bar_count = 0;
	preset_height = 0.0;
	for(i = 0; i < symbol->rows; i++) {
		preset_height += symbol->row_height[i];
		if(symbol->row_height[i] == 0) {
			large_bar_count++;
		}
	}
	large_bar_height = (symbol->height - preset_height) / large_bar_count;

	if (large_bar_count == 0) {
		symbol->height = preset_height;
	}
	
	while(!(module_is_set(symbol, symbol->rows - 1, comp_offset))) {
		comp_offset++;
	}

	xoffset = symbol->border_width + symbol->whitespace_width;
	yoffset = symbol->border_width;

	gbc->width = (gdouble) (symbol->width + xoffset + xoffset) * scaler;
	gbc->height = (gdouble) (symbol->height + textheight + textoffset + yoffset + yoffset) * scaler;

	default_text_posn = (symbol->height + textoffset + symbol->border_width) * scaler;

	if(symbol->symbology != BARCODE_MAXICODE) {
		/* everything else uses rectangles (or squares) */
		/* Works from the bottom of the symbol up */
		int addon_latch = 0;
		
		for(r = 0; r < symbol->rows; r++) {
			this_row = r;
			if(symbol->row_height[this_row] == 0) {
				row_height = large_bar_height;
			} else {
				row_height = symbol->row_height[this_row];
			}
			row_posn = 0;
			for(i = 0; i < r; i++) {
				if(symbol->row_height[i] == 0) {
					row_posn += large_bar_height;
				} else {
					row_posn += symbol->row_height[i];
				}
			}
			row_posn += yoffset;
			
			i = 0;
			if(module_is_set(symbol, this_row, 0)) {
				latch = 1;
			} else {
				latch = 0;
			}

			do {
				block_width = 0;
				do {
					block_width++;
				} while (module_is_set(symbol, this_row, i + block_width) == module_is_set(symbol, this_row, i));
				if((addon_latch == 0) && (r == (symbol->rows - 1)) && (i > main_width)) {
					addon_text_posn = (row_posn + 8.0) * scaler;
					addon_latch = 1;
				} 
				if(latch == 1) {
					/* a bar */
                                        line = g_new0 (glBarcodeLine, 1);

					line->width = block_width * scaler;
					/* glBarcodeLine centers based on width, counter-act!!! */
					line->x = ((i + xoffset) + (block_width / 2.0)) * scaler;
				
					if(addon_latch == 0) {
						line->y = row_posn * scaler;
						line->length = row_height * scaler;
					} else {
						line->y = (row_posn + 10.0) * scaler;
						line->length = (row_height - 5.0) * scaler;
					}
					latch = 0;
					// g_message ("Zint Adding Line at: %f x %f dim: %f x %f", line->x, line->y, line->width, line->length);
					gbc->lines = g_list_append (gbc->lines, line);
				} else {
					/* a space */
					latch = 1;
				}
				i += block_width;
				
			} while (i < symbol->width);
		}
	}
	/* That's done the actual data area, everything else is human-friendly */





	/* Add the text */
	xoffset -= comp_offset;

	if (text_flag) {
		// caculate start xoffset to center text
		xoffset = symbol->width / 2.0;
		xoffset -= (strlen(symbol->text) / 2) * 5.0;

		for (p = symbol->text; *p != 0; p++) {
			if (p != (gchar *)symbol->text && *p == '(') xoffset += 3.0;
			bchar = g_new0 (glBarcodeChar, 1);
			bchar->x = (textpos + xoffset) * scaler;
			bchar->y = default_text_posn;
			bchar->fsize = 8.0 * scaler;
			bchar->c = (gchar) *p;
			gbc->chars = g_list_append (gbc->chars, bchar);
			// Poor mans kerning
			if (*p == '(') {
				xoffset += 3.0;
			} else if (*p == ')') {
				xoffset += 3.0;
			} else {
				xoffset += 5.0;
			}
		}
	}

	return gbc;
}

/*
 * Stolen from Zint common.c
 */
gint module_is_set(struct zint_symbol *symbol, gint y_coord, gint x_coord)
{
	return (symbol->encoded_data[y_coord][x_coord / 7] & (1 << (x_coord % 7))) ? 1 : 0;
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
