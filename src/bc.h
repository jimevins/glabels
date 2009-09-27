/*
 *  bc.h
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

#ifndef __BC_H__
#define __BC_H__

#include <glib.h>
#include <pango/pango.h>

G_BEGIN_DECLS

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

typedef glBarcode *(*glBarcodeNewFunc) (const gchar    *id,
					gboolean        text_flag,
					gboolean        checksum_flag,
					gdouble         w,
					gdouble         h,
					const gchar    *digits);


#define GL_BARCODE_FONT_FAMILY      "Sans"
#define GL_BARCODE_FONT_WEIGHT      PANGO_WEIGHT_NORMAL


glBarcode       *gl_barcode_new              (const gchar    *id,
					      gboolean        text_flag,
					      gboolean        checksum_flag,
					      gdouble         w,
					      gdouble         h,
					      const gchar    *digits);

void             gl_barcode_free             (glBarcode     **bc);

GList           *gl_barcode_get_styles_list  (void);
void             gl_barcode_free_styles_list (GList          *styles_list);

gchar           *gl_barcode_default_digits   (const gchar    *id,
					      guint            n);

gboolean         gl_barcode_can_text         (const gchar    *id);
gboolean         gl_barcode_text_optional    (const gchar    *id);

gboolean         gl_barcode_can_csum         (const gchar    *id);
gboolean         gl_barcode_csum_optional    (const gchar    *id);

gboolean         gl_barcode_can_freeform     (const gchar    *id);
guint            gl_barcode_get_prefered_n   (const gchar    *id);

const gchar     *gl_barcode_id_to_name       (const gchar    *id);
const gchar     *gl_barcode_name_to_id       (const gchar    *name);

G_END_DECLS

#endif /* __BC_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
