/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  bc.h:  GLabels barcode module header file
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
#ifndef __BC_H__
#define __BC_H__

#include <glib.h>

G_BEGIN_DECLS

typedef enum {
	GL_BARCODE_STYLE_POSTNET,
	GL_BARCODE_STYLE_EAN,
	GL_BARCODE_STYLE_UPC,
	GL_BARCODE_STYLE_ISBN,
	GL_BARCODE_STYLE_39,
	GL_BARCODE_STYLE_128,
	GL_BARCODE_STYLE_128C,
	GL_BARCODE_STYLE_128B,
	GL_BARCODE_STYLE_I25,
	GL_BARCODE_STYLE_CBR,
	GL_BARCODE_STYLE_MSI,
	GL_BARCODE_STYLE_PLS,

	GL_BARCODE_N_STYLES
} glBarcodeStyle;

typedef struct {
	gdouble x, y, length, width;
} glBarcodeLine;

typedef struct {
	gdouble x, y, fsize;
	gchar c;
} glBarcodeChar;

typedef struct {
	gdouble width, height;
	GList *lines;		/* List of glBarcodeLine */
	GList *chars;		/* List of glBarcodeChar */
} glBarcode;

typedef glBarcode *(*glBarcodeNewFunc) (glBarcodeStyle  style,
					gboolean        text_flag,
					gboolean        checksum_flag,
					gdouble         w,
					gdouble         h,
					gchar          *digits);


#define GL_BARCODE_FONT_FAMILY      "Helvetica"
#define GL_BARCODE_FONT_WEIGHT      GNOME_FONT_BOOK


glBarcode       *gl_barcode_new              (glBarcodeStyle  style,
					      gboolean        text_flag,
					      gboolean        checksum_flag,
					      gdouble         w,
					      gdouble         h,
					      gchar          *digits);

void             gl_barcode_free             (glBarcode     **bc);

GList           *gl_barcode_get_styles_list  (void);
void             gl_barcode_free_styles_list (GList          *styles_list);

gchar           *gl_barcode_default_digits   (glBarcodeStyle  style);

gboolean         gl_barcode_can_text         (glBarcodeStyle  style);
gboolean         gl_barcode_text_optional    (glBarcodeStyle  style);

gboolean         gl_barcode_can_csum         (glBarcodeStyle  style);
gboolean         gl_barcode_csum_optional    (glBarcodeStyle  style);

const gchar     *gl_barcode_style_to_text    (glBarcodeStyle  style);
glBarcodeStyle   gl_barcode_text_to_style    (const gchar    *text);

G_END_DECLS

#endif /* __BC_H__ */

