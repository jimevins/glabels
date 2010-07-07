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
	if (g_ascii_strcasecmp (id, "AUSP") == 0) { symbol->symbology = BARCODE_AUSPOST; }
	if (g_ascii_strcasecmp (id, "AUSRP") == 0) { symbol->symbology = BARCODE_AUSREPLY; }
	if (g_ascii_strcasecmp (id, "AUSRT") == 0) { symbol->symbology = BARCODE_AUSROUTE; }
	if (g_ascii_strcasecmp (id, "AUSRD") == 0) { symbol->symbology = BARCODE_AUSREDIRECT; }
	if (g_ascii_strcasecmp (id, "AZTEC") == 0) { symbol->symbology = BARCODE_AZTEC; }
	if (g_ascii_strcasecmp (id, "AZRUN") == 0) { symbol->symbology = BARCODE_AZRUNE; }
	if (g_ascii_strcasecmp (id, "CBR") == 0) { symbol->symbology = BARCODE_CODABAR; }
	if (g_ascii_strcasecmp (id, "Code1") == 0) { symbol->symbology = BARCODE_CODEONE; }
	if (g_ascii_strcasecmp (id, "Code11") == 0) { symbol->symbology = BARCODE_CODE11; }
	if (g_ascii_strcasecmp (id, "C16K") == 0) { symbol->symbology = BARCODE_CODE16K; }
	if (g_ascii_strcasecmp (id, "C25M") == 0) { symbol->symbology = BARCODE_C25MATRIX; }
	if (g_ascii_strcasecmp (id, "C25I") == 0) { symbol->symbology = BARCODE_C25IATA; }
	if (g_ascii_strcasecmp (id, "C25DL") == 0) { symbol->symbology = BARCODE_C25LOGIC; }
	if (g_ascii_strcasecmp (id, "Code32") == 0) { symbol->symbology = BARCODE_CODE32; }
	if (g_ascii_strcasecmp (id, "Code39") == 0) { symbol->symbology = BARCODE_CODE39; }
	if (g_ascii_strcasecmp (id, "Code39E") == 0) { symbol->symbology = BARCODE_EXCODE39; }
	if (g_ascii_strcasecmp (id, "Code49") == 0) { symbol->symbology = BARCODE_CODE49; }
	if (g_ascii_strcasecmp (id, "Code93") == 0) { symbol->symbology = BARCODE_CODE93; }
	if (g_ascii_strcasecmp (id, "Code128") == 0) { symbol->symbology = BARCODE_CODE128; }
	if (g_ascii_strcasecmp (id, "Code128B") == 0) { symbol->symbology = BARCODE_CODE128B; }
	if (g_ascii_strcasecmp (id, "DAFT") == 0) { symbol->symbology = BARCODE_DAFT; }
	if (g_ascii_strcasecmp (id, "DMTX") == 0) { symbol->symbology = BARCODE_DATAMATRIX; }
	if (g_ascii_strcasecmp (id, "DPL") == 0) { symbol->symbology = BARCODE_DPLEIT; }
	if (g_ascii_strcasecmp (id, "DPI") == 0) { symbol->symbology = BARCODE_DPIDENT; }
	if (g_ascii_strcasecmp (id, "KIX") == 0) { symbol->symbology = BARCODE_KIX; }
	if (g_ascii_strcasecmp (id, "EAN") == 0) { symbol->symbology = BARCODE_EANX; }
	if (g_ascii_strcasecmp (id, "HIBC128") == 0) { symbol->symbology = BARCODE_HIBC_128; }
	if (g_ascii_strcasecmp (id, "HIBC39") == 0) { symbol->symbology = BARCODE_HIBC_39; }
	if (g_ascii_strcasecmp (id, "HIBCDM") == 0) { symbol->symbology = BARCODE_HIBC_DM; }
	if (g_ascii_strcasecmp (id, "HIBCQR") == 0) { symbol->symbology = BARCODE_HIBC_QR; }
	if (g_ascii_strcasecmp (id, "HIBCPDF") == 0) { symbol->symbology = BARCODE_HIBC_MICPDF; }
	if (g_ascii_strcasecmp (id, "HIBCMPDF") == 0) { symbol->symbology = BARCODE_HIBC_AZTEC; }
	if (g_ascii_strcasecmp (id, "HIBCAZ") == 0) { symbol->symbology = BARCODE_C25INTER; }
	if (g_ascii_strcasecmp (id, "I25") == 0) { symbol->symbology = BARCODE_C25INTER; }
	if (g_ascii_strcasecmp (id, "ISBN") == 0) { symbol->symbology = BARCODE_ISBNX; }
	if (g_ascii_strcasecmp (id, "ITF14") == 0) { symbol->symbology = BARCODE_ITF14; }
	if (g_ascii_strcasecmp (id, "GMTX") == 0) { symbol->symbology = BARCODE_GRIDMATRIX; }
	if (g_ascii_strcasecmp (id, "GS1-128") == 0) { symbol->symbology = BARCODE_EAN128; }
	if (g_ascii_strcasecmp (id, "LOGM") == 0) { symbol->symbology = BARCODE_LOGMARS; }
	if (g_ascii_strcasecmp (id, "RSS14") == 0) { symbol->symbology = BARCODE_RSS14; }
	if (g_ascii_strcasecmp (id, "RSSLTD") == 0) { symbol->symbology = BARCODE_RSS_LTD; }
	if (g_ascii_strcasecmp (id, "RSSEXP") == 0) { symbol->symbology = BARCODE_RSS_EXP; }
	if (g_ascii_strcasecmp (id, "RSSS") == 0) { symbol->symbology = BARCODE_RSS14STACK; }
	if (g_ascii_strcasecmp (id, "RSSSO") == 0) { symbol->symbology = BARCODE_RSS14STACK_OMNI; }
	if (g_ascii_strcasecmp (id, "RSSSE") == 0) { symbol->symbology = BARCODE_RSS_EXPSTACK; }
	if (g_ascii_strcasecmp (id, "PHARMA") == 0) { symbol->symbology = BARCODE_PHARMA; }
	if (g_ascii_strcasecmp (id, "PHARMA2") == 0) { symbol->symbology = BARCODE_PHARMA_TWO; }
	if (g_ascii_strcasecmp (id, "PZN") == 0) { symbol->symbology = BARCODE_PZN; }
	if (g_ascii_strcasecmp (id, "TELE") == 0) { symbol->symbology = BARCODE_TELEPEN; }
	if (g_ascii_strcasecmp (id, "TELEX") == 0) { symbol->symbology = BARCODE_TELEPEN_NUM; }
	if (g_ascii_strcasecmp (id, "JAPAN") == 0) { symbol->symbology = BARCODE_JAPANPOST; }
	if (g_ascii_strcasecmp (id, "KOREA") == 0) { symbol->symbology = BARCODE_KOREAPOST; }
	if (g_ascii_strcasecmp (id, "MPDF") == 0) { symbol->symbology = BARCODE_MICROPDF417; }
	if (g_ascii_strcasecmp (id, "MSI") == 0) { symbol->symbology = BARCODE_MSI_PLESSEY; }
	if (g_ascii_strcasecmp (id, "MQR") == 0) { symbol->symbology = BARCODE_MICROQR; }
	if (g_ascii_strcasecmp (id, "NVE") == 0) { symbol->symbology = BARCODE_NVE18; }
	if (g_ascii_strcasecmp (id, "PLAN") == 0) { symbol->symbology = BARCODE_PLANET; }
	if (g_ascii_strcasecmp (id, "POSTNET") == 0) { symbol->symbology = BARCODE_POSTNET; }
	if (g_ascii_strcasecmp (id, "PDF") == 0) { symbol->symbology = BARCODE_PDF417; }
	if (g_ascii_strcasecmp (id, "PDFT") == 0) { symbol->symbology = BARCODE_PDF417TRUNC; }
	if (g_ascii_strcasecmp (id, "QR") == 0) { symbol->symbology = BARCODE_QRCODE; }
	if (g_ascii_strcasecmp (id, "RM4") == 0) { symbol->symbology = BARCODE_RM4SCC; }
	if (g_ascii_strcasecmp (id, "UPC-A") == 0) { symbol->symbology = BARCODE_UPCA; }
	if (g_ascii_strcasecmp (id, "UPC-E") == 0) { symbol->symbology = BARCODE_UPCE; }
	if (g_ascii_strcasecmp (id, "USPS") == 0) { symbol->symbology = BARCODE_ONECODE; }
	if (g_ascii_strcasecmp (id, "PLS") == 0) { symbol->symbology = BARCODE_PLESSEY; }


	/* Checksum not supported yet!! 
	if (!checksum_flag) {
		flags |= BARCODE_NO_CHECKSUM;
	}
	*/

	result = ZBarcode_Encode(symbol, (unsigned char *)digits, 0);
	if (result) {
		gl_debug (DEBUG_BARCODE, "Zint Error: %s", symbol->errtxt);
		ZBarcode_Delete (symbol);
		return NULL;
	}
	
	if(!text_flag) {
		symbol->show_hrt = 0;
	}

	if (!ZBarcode_Render(symbol, (float) w, (float) h)) {
		g_message("Zint Rendering Error: %s", symbol->errtxt);
		ZBarcode_Delete(symbol);
		return NULL;
	}

	/* Convert Sums provided by zint encode */
	gbc = render_zint(symbol, text_flag);

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
	
	int i;
	double string_offset, x;

        glBarcode           *gbc;
        glBarcodeShapeLine  *line;
        glBarcodeShapeAlpha *bchar;
	
	struct zint_render      *render;
	struct zint_render_line *zline;
	struct zint_render_string *zstring;

	render = symbol->rendered;
	gbc = g_new0(glBarcode, 1);
	
	
	for ( zline = render->lines; zline != NULL; zline = zline->next ) {
		line = gl_barcode_shape_line_new ();

		line->width = (double) zline->width;
		line->length = (double) zline->length;
		/* glBarcodeLine centers based on width, counter-act!!! */
		line->x = (double) (zline->x + (zline->width / 2.0));
		line->y = (double) zline->y;

		gl_barcode_add_shape (gbc, (glBarcodeShape *)line);
	}

	/*
	 * Repeat loop for characters
	 */
	if(text_flag) {
		for ( zstring = render->strings; zstring != NULL; zstring = zstring->next ) {
			string_offset = (double) zstring->x - (((6.0 / 9.0) * zstring->length * zstring->fsize) / 2);
			for(i = 0; i < zstring->length; i++) {
        x = 0.0;
        // Poor man's kerning
        if (zstring->text[i] == '(') { x = 0.18; }
				bchar = gl_barcode_shape_alpha_new();
				bchar->x = (double) string_offset + ((((6.0 / 9.0) * i) + x) * zstring->fsize);
				bchar->y = (double) zstring->y;
				bchar->fsize = (double) zstring->fsize;
				bchar->c = (char) zstring->text[i];
				gl_barcode_add_shape (gbc, (glBarcodeShape *)bchar);
			}
		}
	}

	/*
	 * Finally add complete sizes
	 */
	gbc->width = (gdouble) render->width;
	gbc->height = (gdouble) render->height;

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
