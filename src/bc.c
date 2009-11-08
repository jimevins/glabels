/*
 *  bc.c
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

#include "bc.h"

#include <glib.h>
#include <glib/gi18n.h>

#include "bc-postnet.h"
#include "bc-gnubarcode.h"
#include "bc-iec16022.h"
#include "bc-iec18004.h"

#include "debug.h"


/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/


/*========================================================*/
/* Private types.                                         */
/*========================================================*/

typedef struct {
	gchar            *id;
	gchar            *name;
	glBarcodeNewFunc  new;
	gboolean          can_text;
	gboolean          text_optional;
	gboolean          can_checksum;
	gboolean          checksum_optional;
	gchar            *default_digits;
	gboolean          can_freeform;
	guint             prefered_n;
} Backend;


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static const Backend backends[] = {

	{ "POSTNET", N_("POSTNET (any)"), gl_barcode_postnet_new,
	  FALSE, FALSE, TRUE, FALSE, "12345-6789-12", FALSE, 11},

	{ "POSTNET-5", N_("POSTNET-5 (ZIP only)"), gl_barcode_postnet_new,
	  FALSE, FALSE, TRUE, FALSE, "12345", FALSE, 5},

	{ "POSTNET-9", N_("POSTNET-9 (ZIP+4)"), gl_barcode_postnet_new,
	  FALSE, FALSE, TRUE, FALSE, "12345-6789", FALSE, 9},

	{ "POSTNET-11", N_("POSTNET-11 (DPBC)"), gl_barcode_postnet_new,
	  FALSE, FALSE, TRUE, FALSE, "12345-6789-12", FALSE, 11},

#ifdef HAVE_LIBBARCODE

	{ "CEPNET", N_("CEPNET"), gl_barcode_postnet_new,
	  FALSE, FALSE, TRUE, FALSE, "12345-678", FALSE, 8},

	{ "EAN", N_("EAN (any)"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, FALSE, "000000000000 00000", FALSE, 17},

	{ "EAN-8", N_("EAN-8"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, FALSE, "0000000", FALSE, 7},

	{ "EAN-8+2", N_("EAN-8 +2"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, FALSE, "0000000 00", FALSE, 9},

	{ "EAN-8+5", N_("EAN-8 +5"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, FALSE, "0000000 00000", FALSE, 12},

	{ "EAN-13", N_("EAN-13"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, FALSE, "000000000000", FALSE, 12},

	{ "EAN-13+2", N_("EAN-13 +2"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, FALSE, "000000000000 00", FALSE, 14},

	{ "EAN-13+5", N_("EAN-13 +5"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, FALSE, "000000000000 00000", FALSE, 17},

	{ "UPC", N_("UPC (UPC-A or UPC-E)"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, FALSE, "00000000000 00000", FALSE, 16},

	{ "UPC-A", N_("UPC-A"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, FALSE, "00000000000", FALSE, 11},

	{ "UPC-A+2", N_("UPC-A +2"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, FALSE, "00000000000 00", FALSE, 13},

	{ "UPC-A+5", N_("UPC-A +5"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, FALSE, "00000000000 00000", FALSE, 16},

	{ "UPC-E", N_("UPC-E"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, FALSE, "000000", FALSE, 6},

	{ "UPC-E+2", N_("UPC-E +2"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, FALSE, "000000 00", FALSE, 8},

	{ "UPC-E+5", N_("UPC-E +5"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, FALSE, "000000 00000", FALSE, 11},

	{ "ISBN", N_("ISBN"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, TRUE, "0-00000-000-0", FALSE, 10},

	{ "ISBN+5", N_("ISBN +5"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, TRUE, "0-00000-000-0 00000", FALSE, 15},

	{ "Code39", N_("Code 39"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, TRUE, "0000000000", TRUE, 10},

	{ "Code128", N_("Code 128"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, TRUE, "0000000000", TRUE, 10},

	{ "Code128C", N_("Code 128C"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},

	{ "Code128B", N_("Code 128B"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, TRUE, "0000000000", TRUE, 10},

	{ "I25", N_("Interleaved 2 of 5"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, TRUE, "0000000000", TRUE, 10},

	{ "CBR", N_("Codabar"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, TRUE, "0000000000", TRUE, 10},

	{ "MSI", N_("MSI"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, TRUE, "0000000000", TRUE, 10},

	{ "PLS", N_("Plessey"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, TRUE, "0000000000", TRUE, 10},

	{ "Code93", N_("Code 93"), gl_barcode_gnubarcode_new,
	  TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},

#endif /* HAVE_LIBBARCODE */

#ifdef HAVE_LIBIEC16022

	{ "IEC16022", N_("IEC16022 (DataMatrix)"), gl_barcode_iec16022_new,
	  FALSE, FALSE, TRUE, FALSE, "12345678", TRUE, 8},

#endif /* HAVE_LIBIEC16022 */

#ifdef HAVE_LIBQRENCODE

	{ "IEC18004", N_("IEC18004 (QRCode)"), gl_barcode_iec18004_new,
	  FALSE, FALSE, TRUE, FALSE, "12345678", TRUE, 8},

#endif /* HAVE_LIBQRENCODE */

	{ NULL, NULL, NULL, FALSE, FALSE, FALSE, FALSE, NULL, FALSE, 0}

};


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/


/*---------------------------------------------------------------------------*/
/* Convert id to index into above table.                                     */
/*---------------------------------------------------------------------------*/
static gint
id_to_index (const gchar *id)
{
	gint i;

	if (id == 0) {
		return 0; /* NULL request default. I.e., the first element. */
	}

	for (i=0; backends[i].id != NULL; i++) {
		if (g_ascii_strcasecmp (id, backends[i].id) == 0) {
			return i;
		}
	}

	g_message( "Unknown barcode id \"%s\"", id );
	return 0;
}


/*---------------------------------------------------------------------------*/
/* Convert name to index into above table.                                   */
/*---------------------------------------------------------------------------*/
static gint
name_to_index (const gchar *name)
{
	gint i;

	g_return_val_if_fail (name!=NULL, 0);

	for (i=0; backends[i].id != NULL; i++) {
		if (strcmp (name, gettext (backends[i].name)) == 0) {
			return i;
		}
	}

	g_message( "Unknown barcode name \"%s\"", name );
	return 0;
}


/*****************************************************************************/
/* Call appropriate barcode backend to create barcode in intermediate format.*/
/*****************************************************************************/
glBarcode *
gl_barcode_new (const gchar    *id,
		gboolean        text_flag,
		gboolean        checksum_flag,
		gdouble         w,
		gdouble         h,
		const gchar    *digits)
{
	glBarcode *gbc;
	gint       i;

	g_return_val_if_fail (digits!=NULL, NULL);

	i = id_to_index (id);
	gbc = backends[i].new (backends[i].id,
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
	gint   i;
	GList *list = NULL;

	for (i=0; backends[i].id != NULL; i++) {
		list = g_list_append (list, g_strdup (gettext (backends[i].name)));
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
gl_barcode_default_digits (const gchar *id,
			   guint        n)
{
	int i;

	i = id_to_index (id);

	if (backends[i].can_freeform) {

		return g_strnfill (MAX (n,1), '0');

	} else {

		return g_strdup (backends[i].default_digits);

	}
}


/*****************************************************************************/
/* Query text capabilities.                                                  */
/*****************************************************************************/
gboolean
gl_barcode_can_text (const gchar *id)
{
	return backends[id_to_index (id)].can_text;
}


gboolean
gl_barcode_text_optional (const gchar *id)
{
	return backends[id_to_index (id)].text_optional;
}


/*****************************************************************************/
/* Query checksum capabilities.                                              */
/*****************************************************************************/
gboolean
gl_barcode_can_csum (const gchar *id)
{
	return backends[id_to_index (id)].can_checksum;
}


gboolean
gl_barcode_csum_optional (const gchar *id)
{
	return backends[id_to_index (id)].checksum_optional;
}


/*****************************************************************************/
/* Query if freeform input is allowed.                                       */
/*****************************************************************************/
gboolean
gl_barcode_can_freeform     (const gchar    *id)
{
	return backends[id_to_index (id)].can_freeform;
}


/*****************************************************************************/
/* Query prefered number of digits of input.                                 */
/*****************************************************************************/
guint
gl_barcode_get_prefered_n (const gchar    *id)
{
	return backends[id_to_index (id)].prefered_n;
}


/*****************************************************************************/
/* Convert style to text.                                                    */
/*****************************************************************************/
const gchar *
gl_barcode_id_to_name (const gchar *id)
{
	return gettext (backends[id_to_index (id)].name);
}


/*****************************************************************************/
/* Convert name to style.                                                    */
/*****************************************************************************/
const gchar *
gl_barcode_name_to_id (const gchar *name)
{
	g_return_val_if_fail (name!=NULL, backends[0].id);

	return backends[name_to_index (name)].id;
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
