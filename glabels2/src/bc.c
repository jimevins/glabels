/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  bc.c:  GLabels barcode module
 *
 *  Copyright (C) 2001-2003  Jim Evins <evins@snaught.com>.
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

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

typedef struct {
	gchar            *name;
	glBarcodeNewFunc  new;
	gboolean          can_text;
	gboolean          text_optional;
	gboolean          can_checksum;
	gboolean          checksum_optional;
	gchar            *default_digits;
} Backend;


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

Backend backends[GL_BARCODE_N_STYLES] = {

	{ "POSTNET", gl_barcode_postnet_new,
	  FALSE, FALSE, TRUE, FALSE, "000000000"},

	{ "EAN", gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, FALSE, "000000000000 00000"},

	{ "UPC", gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, FALSE, "00000000000 00000"},

	{ "ISBN", gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, TRUE, "0-00000-000-0 00000"},

	{ "Code39", gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, TRUE, "0000000000"},

	{ "Code128", gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, TRUE, "0000000000"},

	{ "Code128C", gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, TRUE, "0000000000"},

	{ "Code128B", gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, TRUE, "0000000000"},

	{ "I25", gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, TRUE, "0000000000"},

	{ "CBR", gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, TRUE, "0000000000"},

	{ "MSI", gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, TRUE, "0000000000"},

	{ "PLS", gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, TRUE, "0000000000"},

};

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/


/*****************************************************************************/
/* Call appropriate barcode backend to create barcode in intermediate format.*/
/*****************************************************************************/
glBarcode *
gl_barcode_new (glBarcodeStyle  style,
		gboolean        text_flag,
		gboolean        checksum_flag,
		gdouble         w,
		gdouble         h,
		gchar          *digits)
{
	glBarcode *gbc;

	g_return_val_if_fail ((style>=0) && (style<GL_BARCODE_N_STYLES), NULL);
	g_return_val_if_fail (digits!=NULL, NULL);

	gbc = backends[style].new (style,
				   text_flag,
				   checksum_flag,
				   w,
				   h,
				   digits);

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
/* Get a list of names for valid barcode styles.                             */
/*****************************************************************************/
GList *
gl_barcode_get_styles_list  (void)
{
	glBarcodeStyle  style;
	GList          *list = NULL;

	for (style=0; style <GL_BARCODE_N_STYLES; style++) {
		list = g_list_append (list, g_strdup (backends[style].name));
	}

	return list;
}

/*****************************************************************************/
/* Free up a previously allocated list of style names.                       */
/*****************************************************************************/
void
gl_barcode_free_styles_list (GList *styles_list)
{
	GList *p;

	for (p=styles_list; p != NULL; p=p->next) {
		g_free (p->data);
		p->data = NULL;
	}

	g_list_free (styles_list);
}


/*****************************************************************************/
/* Return an appropriate set of digits for the given barcode style.          */
/*****************************************************************************/
gchar *
gl_barcode_default_digits (glBarcodeStyle style)
{
	g_return_val_if_fail ((style>=0) && (style<GL_BARCODE_N_STYLES), "0");

	return g_strdup (backends[style].default_digits);
}

/*****************************************************************************/
/* Query text capabilities.                                                  */
/*****************************************************************************/
gboolean
gl_barcode_can_text (glBarcodeStyle  style)
{
	g_return_val_if_fail ((style>=0) && (style<GL_BARCODE_N_STYLES), FALSE);

	return backends[style].can_text;
}

gboolean
gl_barcode_text_optional (glBarcodeStyle  style)
{
	g_return_val_if_fail ((style>=0) && (style<GL_BARCODE_N_STYLES), FALSE);

	return backends[style].text_optional;
}

/*****************************************************************************/
/* Query checksum capabilities.                                              */
/*****************************************************************************/
gboolean
gl_barcode_can_csum (glBarcodeStyle  style)
{
	g_return_val_if_fail ((style>=0) && (style<GL_BARCODE_N_STYLES), FALSE);

	return backends[style].can_checksum;
}

gboolean
gl_barcode_csum_optional (glBarcodeStyle  style)
{
	g_return_val_if_fail ((style>=0) && (style<GL_BARCODE_N_STYLES), FALSE);

	return backends[style].checksum_optional;
}

/*****************************************************************************/
/* Convert style to text.                                                    */
/*****************************************************************************/
const gchar *
gl_barcode_style_to_text (glBarcodeStyle style)
{
	g_return_val_if_fail ((style>=0) && (style<GL_BARCODE_N_STYLES), NULL);

	return backends[style].name;
}

/*****************************************************************************/
/* Convert text to style.                                                    */
/*****************************************************************************/
glBarcodeStyle
gl_barcode_text_to_style (const gchar *text)
{

	glBarcodeStyle  style;

	for (style=0; style <GL_BARCODE_N_STYLES; style++) {
		if (g_strcasecmp (text, backends[style].name) == 0) {
			return style;
		}
	}

	g_warning( "Unknown barcode style text \"%s\"", text );
	return GL_BARCODE_STYLE_POSTNET;
}
