/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  bc.c:  GLabels barcode module
 *
 *  Copyright (C) 2001-2002  Jim Evins <evins@snaught.com>.
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

#include "bc.h"
#include "bc-postnet.h"
#include "bc-gnubarcode.h"

#include "debug.h"

/*****************************************************************************/
/* Call appropriate barcode backend to create barcode in intermediate format.*/
/*****************************************************************************/
glBarcode *
gl_barcode_new (glBarcodeStyle  style,
		gboolean        text_flag,
		gdouble         scale,
		gchar          *digits)
{
	glBarcode *gbc;

	switch (style) {

	case GL_BARCODE_STYLE_POSTNET:
		/* Use the POSTNET backend module */
		gbc = gl_barcode_postnet_new (digits);
		break;

	default:
		/* Use the GNU barcode library backend */
		gbc = gl_barcode_gnubarcode_new (style,
						 text_flag,
						 scale,
						 digits);
		break;

	}
	return gbc;
}

/*****************************************************************************/
/* Free previously created barcode.                                          */
/*****************************************************************************/
void
gl_barcode_free (glBarcode **gbc)
{
	GList *p;

	if (*gbc != NULL) {

		for (p = (*gbc)->lines; p != NULL; p = p->next) {
			g_free (p->data);
			p->data = NULL;
		}
		g_list_free ((*gbc)->lines);
		(*gbc)->lines = NULL;

		for (p = (*gbc)->chars; p != NULL; p = p->next) {
			g_free (p->data);
			p->data = NULL;
		}
		g_list_free ((*gbc)->chars);
		(*gbc)->chars = NULL;

		g_free (*gbc);
		*gbc = NULL;
	}
}

/*****************************************************************************/
/* Return an appropriate set of digits for the given barcode style.          */
/*****************************************************************************/
gchar *
gl_barcode_default_digits (glBarcodeStyle style)
{
	switch (style) {

	case GL_BARCODE_STYLE_POSTNET:
		return g_strdup ("000000000");
	case GL_BARCODE_STYLE_EAN:
		return g_strdup ("000000000000 00000");
	case GL_BARCODE_STYLE_UPC:
		return g_strdup ("00000000000 00000");
	case GL_BARCODE_STYLE_ISBN:
		return g_strdup ("0-00000-000-0 00000");
	case GL_BARCODE_STYLE_39:
	case GL_BARCODE_STYLE_128:
	case GL_BARCODE_STYLE_128C:
	case GL_BARCODE_STYLE_128B:
	case GL_BARCODE_STYLE_I25:
	case GL_BARCODE_STYLE_CBR:
	case GL_BARCODE_STYLE_MSI:
	case GL_BARCODE_STYLE_PLS:
		return g_strdup ("0000000000");
	default:
		return g_strdup ("0");
	}

}

/*****************************************************************************/
/* Convert style to text.                                                    */
/*****************************************************************************/
const gchar *
gl_barcode_style_to_text (glBarcodeStyle style)
{
	switch (style) {
	case GL_BARCODE_STYLE_POSTNET:
		return "POSTNET";
	case GL_BARCODE_STYLE_EAN:
		return "EAN";
	case GL_BARCODE_STYLE_UPC:
		return "UPC";
	case GL_BARCODE_STYLE_ISBN:
		return "ISBN";
	case GL_BARCODE_STYLE_39:
		return "Code39";
	case GL_BARCODE_STYLE_128:
		return "Code128";
	case GL_BARCODE_STYLE_128C:
		return "Code128C";
	case GL_BARCODE_STYLE_128B:
		return "Code128B";
	case GL_BARCODE_STYLE_I25:
		return "I25";
	case GL_BARCODE_STYLE_CBR:
		return "CBR";
	case GL_BARCODE_STYLE_MSI:
		return "MSI";
	case GL_BARCODE_STYLE_PLS:
		return "PLS";
	default:
		g_warning( "Illegal barcode style %d", style );
		return "?";
	}
}

/*****************************************************************************/
/* Convert text to style.                                                    */
/*****************************************************************************/
glBarcodeStyle
gl_barcode_text_to_style (const gchar *text)
{

	if (g_strcasecmp (text, "POSTNET") == 0) {
		return GL_BARCODE_STYLE_POSTNET;
	}
	if (g_strcasecmp (text, "EAN") == 0) {
		return GL_BARCODE_STYLE_EAN;
	}
	if (g_strcasecmp (text, "UPC") == 0) {
		return GL_BARCODE_STYLE_UPC;
	}
	if (g_strcasecmp (text, "ISBN") == 0) {
		return GL_BARCODE_STYLE_ISBN;
	}
	if (g_strcasecmp (text, "Code39") == 0) {
		return GL_BARCODE_STYLE_39;
	}
	if (g_strcasecmp (text, "Code128") == 0) {
		return GL_BARCODE_STYLE_128;
	}
	if (g_strcasecmp (text, "Code128C") == 0) {
		return GL_BARCODE_STYLE_128C;
	}
	if (g_strcasecmp (text, "Code128B") == 0) {
		return GL_BARCODE_STYLE_128B;
	}
	if (g_strcasecmp (text, "I25") == 0) {
		return GL_BARCODE_STYLE_I25;
	}
	if (g_strcasecmp (text, "CBR") == 0) {
		return GL_BARCODE_STYLE_CBR;
	}
	if (g_strcasecmp (text, "MSI") == 0) {
		return GL_BARCODE_STYLE_MSI;
	}
	if (g_strcasecmp (text, "PLS") == 0) {
		return GL_BARCODE_STYLE_PLS;
	} else {
		g_warning( "Unknown barcode style text \"%s\"", text );
		return GL_BARCODE_STYLE_POSTNET;
	}

}
