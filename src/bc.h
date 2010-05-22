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

typedef enum {
        GL_BARCODE_SHAPE_LINE,
        GL_BARCODE_SHAPE_ALPHA,
} glBarcodeShapeType;

typedef struct {

        /* Begin Common Fields */
        glBarcodeShapeType  type;
        gdouble             x;
        gdouble             y;
        /* End Common Fields */

} glBarcodeShapeAny;

/*
 * glBarcodeShapeLine:
 *
 * @ =  origin (x,y) from top left corner of barcode
 *
 *              +--@--+
 *              |     |
 *              |     |
 *              |     |
 *              |     | length
 *              |     |
 *              |     |
 *              |     |
 *              +-----+
 *               width
 */
typedef struct {

        /* Begin Common Fields */
        glBarcodeShapeType  type; /* Always GL_BARCODE_SHAPE_LINE. */
        gdouble             x;
        gdouble             y;
        /* End Common Fields */

        gdouble             length;
        gdouble             width;

} glBarcodeShapeLine;

/*
 * glBarcodeShapeAlpha:
 *
 * @ =  origin (x,y) from top left corner of barcode
 *
 *              ____ ------------
 *             /    \           ^
 *            /  /\  \          |
 *           /  /__\  \         |
 *          /  ______  \        | ~fsize
 *         /  /      \  \       |
 *        /__/        \__\      |
 *                              v
 *       @ ----------------------
 */
typedef struct {

        /* Begin Common Fields */
        glBarcodeShapeType  type; /* Always GL_BARCODE_SHAPE_ALPHA. */
        gdouble             x;
        gdouble             y;
        /* End Common Fields */

        gdouble             fsize;
        gchar               c;

} glBarcodeShapeAlpha;

typedef union {

        glBarcodeShapeType    type;
        glBarcodeShapeAny     any;

        glBarcodeShapeLine    line;
        glBarcodeShapeAlpha   alpha;

} glBarcodeShape;

typedef struct {
	gdouble width, height;
	GList *shapes;		/* List of glBarcodeShape */
} glBarcode;

typedef glBarcode *(*glBarcodeNewFunc) (const gchar    *id,
					gboolean        text_flag,
					gboolean        checksum_flag,
					gdouble         w,
					gdouble         h,
					const gchar    *digits);


#define GL_BARCODE_FONT_FAMILY      "Sans"
#define GL_BARCODE_FONT_WEIGHT      PANGO_WEIGHT_NORMAL


glBarcodeShapeLine  *gl_barcode_shape_line_new  (void);
glBarcodeShapeAlpha *gl_barcode_shape_alpha_new (void);

glBarcode       *gl_barcode_new              (const gchar    *id,
					      gboolean        text_flag,
					      gboolean        checksum_flag,
					      gdouble         w,
					      gdouble         h,
					      const gchar    *digits);

void             gl_barcode_free             (glBarcode     **bc);

void             gl_barcode_add_shape        (glBarcode      *bc,
                                              glBarcodeShape *shape);

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
