/*
 *  lgl-barcode-render-to-cairo.c
 *  Copyright (C) 2010  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of libglbarcode.
 *
 *  libglbarcode is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libglbarcode is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libglbarcode.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include "lgl-barcode-render-to-cairo.h"

#include <pango/pangocairo.h>
#include <math.h>


/*===========================================*/
/* Private macros and constants.             */
/*===========================================*/

#define FONT_SCALE (72.0/96.0)
#define BARCODE_FONT_FAMILY      "Sans"
#define BARCODE_FONT_WEIGHT      PANGO_WEIGHT_NORMAL


/*===========================================*/
/* Private types                             */
/*===========================================*/


/*===========================================*/
/* Private globals                           */
/*===========================================*/


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/


/****************************************************************************/
/**
 * lgl_barcode_render_to_cairo:
 * @bc:     An #lglBarcode structure
 * @cr:     A #cairo_t context
 *
 * Render barcode to cairo context.  Context should be prepared with desired
 * translation and appropriate scale.  Context should be translated such that
 * the origin is at the desired location of the upper left hand corner of the
 * barcode bounding box.  Context should be scaled such that all dimensions
 * are in points ( 1 point = 1/72 inch ) and that positive y coordinates
 * go down the surface.
 */
void
lgl_barcode_render_to_cairo (const lglBarcode  *bc,
                             cairo_t           *cr)
{
        GList                  *p;

        lglBarcodeShape        *shape;
        lglBarcodeShapeLine    *line;
        lglBarcodeShapeBox     *box;
        lglBarcodeShapeChar    *bchar;
        lglBarcodeShapeString  *bstring;
        lglBarcodeShapeRing    *ring;
        lglBarcodeShapeHexagon *hexagon;

        PangoLayout            *layout;
        PangoFontDescription   *desc;
        gchar                  *cstring;
        gdouble                 x_offset, y_offset;
        gint                    iw, ih;
        gdouble                 layout_width;


        for (p = bc->shapes; p != NULL; p = p->next) {

                shape = (lglBarcodeShape *)p->data;

                switch (shape->type)
                {

                case LGL_BARCODE_SHAPE_LINE:
                        line = (lglBarcodeShapeLine *) shape;

                        cairo_move_to (cr, line->x, line->y);
                        cairo_line_to (cr, line->x, line->y + line->length);
                        cairo_set_line_width (cr, line->width);
                        cairo_stroke (cr);

                        break;

                case LGL_BARCODE_SHAPE_BOX:
                        box = (lglBarcodeShapeBox *) shape;

                        cairo_rectangle (cr, box->x, box->y, box->width, box->height);
                        cairo_fill (cr);

                        break;

                case LGL_BARCODE_SHAPE_CHAR:
                        bchar = (lglBarcodeShapeChar *) shape;

                        layout = pango_cairo_create_layout (cr);

                        desc = pango_font_description_new ();
                        pango_font_description_set_family (desc, BARCODE_FONT_FAMILY);
                        pango_font_description_set_size   (desc, bchar->fsize * PANGO_SCALE * FONT_SCALE);
                        pango_layout_set_font_description (layout, desc);
                        pango_font_description_free       (desc);

                        cstring = g_strdup_printf ("%c", bchar->c);
                        pango_layout_set_text (layout, cstring, -1);
                        g_free (cstring);

                        y_offset = 0.2 * bchar->fsize;

                        cairo_move_to (cr, bchar->x, bchar->y-y_offset);
                        pango_cairo_show_layout (cr, layout);

                        g_object_unref (layout);

                        break;

                case LGL_BARCODE_SHAPE_STRING:
                        bstring = (lglBarcodeShapeString *) shape;

                        layout = pango_cairo_create_layout (cr);

                        desc = pango_font_description_new ();
                        pango_font_description_set_family (desc, BARCODE_FONT_FAMILY);
                        pango_font_description_set_size   (desc, bstring->fsize * PANGO_SCALE * FONT_SCALE);
                        pango_layout_set_font_description (layout, desc);
                        pango_font_description_free       (desc);

                        pango_layout_set_text (layout, bstring->string, -1);

                        pango_layout_get_size (layout, &iw, &ih);
                        layout_width = (gdouble)iw / (gdouble)PANGO_SCALE;

                        x_offset = layout_width / 2.0;
                        y_offset = 0.2 * bstring->fsize;

                        cairo_move_to (cr, (bstring->x - x_offset), (bstring->y - y_offset));
                        pango_cairo_show_layout (cr, layout);

                        g_object_unref (layout);

                        break;

                case LGL_BARCODE_SHAPE_RING:
                        ring = (lglBarcodeShapeRing *) shape;

                        cairo_arc (cr, ring->x, ring->y, ring->radius, 0.0, 2 * M_PI);
                        cairo_set_line_width (cr, ring->line_width);
                        cairo_stroke (cr);

                        break;

                case LGL_BARCODE_SHAPE_HEXAGON:
                        hexagon = (lglBarcodeShapeHexagon *) shape;

                        cairo_move_to (cr, hexagon->x, hexagon->y);
                        cairo_line_to (cr, hexagon->x + 0.433*hexagon->height, hexagon->y + 0.25*hexagon->height);
                        cairo_line_to (cr, hexagon->x + 0.433*hexagon->height, hexagon->y + 0.75*hexagon->height);
                        cairo_line_to (cr, hexagon->x,                         hexagon->y +      hexagon->height);
                        cairo_line_to (cr, hexagon->x - 0.433*hexagon->height, hexagon->y + 0.75*hexagon->height);
                        cairo_line_to (cr, hexagon->x - 0.433*hexagon->height, hexagon->y + 0.25*hexagon->height);
                        cairo_close_path (cr);
                        cairo_fill (cr);

                        break;

                default:
                        g_assert_not_reached ();
                        break;

                }

        }

}


/****************************************************************************/
/**
 * lgl_barcode_render_to_cairo_path:
 * @bc:     An #lglBarcode structure
 * @cr:     A #cairo_t context
 *
 * Render barcode to cairo context, but only create a path to be filled or
 * tested against.  Context should be prepared with desired
 * translation and appropriate scale.  Context should be translated such that
 * the origin is at the desired location of the upper left hand corner of the
 * barcode bounding box.  Context should be scaled such that all dimensions
 * are in points ( 1 point = 1/72 inch ) and that positive y coordinates
 * go down the surface.
 */
void
lgl_barcode_render_to_cairo_path (const lglBarcode  *bc,
                                  cairo_t           *cr)
{
        GList                  *p;

        lglBarcodeShape        *shape;
        lglBarcodeShapeLine    *line;
        lglBarcodeShapeBox     *box;
        lglBarcodeShapeChar    *bchar;
        lglBarcodeShapeString  *bstring;
        lglBarcodeShapeRing    *ring;
        lglBarcodeShapeHexagon *hexagon;

        PangoLayout            *layout;
        PangoFontDescription   *desc;
        gchar                  *cstring;
        gdouble                 x_offset, y_offset;
        gint                    iw, ih;
        gdouble                 layout_width;


        for (p = bc->shapes; p != NULL; p = p->next) {

                shape = (lglBarcodeShape *)p->data;

                switch (shape->type)
                {

                case LGL_BARCODE_SHAPE_LINE:
                        line = (lglBarcodeShapeLine *) shape;

                        cairo_rectangle (cr, line->x - line->width/2, line->y, line->width, line->length);

                        break;

                case LGL_BARCODE_SHAPE_BOX:
                        box = (lglBarcodeShapeBox *) shape;

                        cairo_rectangle (cr, box->x, box->y, box->width, box->height);

                        break;

                case LGL_BARCODE_SHAPE_CHAR:
                        bchar = (lglBarcodeShapeChar *) shape;

                        layout = pango_cairo_create_layout (cr);

                        desc = pango_font_description_new ();
                        pango_font_description_set_family (desc, BARCODE_FONT_FAMILY);
                        pango_font_description_set_size   (desc, bchar->fsize * PANGO_SCALE * FONT_SCALE);
                        pango_layout_set_font_description (layout, desc);
                        pango_font_description_free       (desc);

                        cstring = g_strdup_printf ("%c", bchar->c);
                        pango_layout_set_text (layout, cstring, -1);
                        g_free (cstring);

                        y_offset = 0.2 * bchar->fsize;

                        cairo_move_to (cr, bchar->x, bchar->y-y_offset);
                        pango_cairo_layout_path (cr, layout);

                        g_object_unref (layout);

                        break;

                case LGL_BARCODE_SHAPE_STRING:
                        bstring = (lglBarcodeShapeString *) shape;

                        layout = pango_cairo_create_layout (cr);

                        desc = pango_font_description_new ();
                        pango_font_description_set_family (desc, BARCODE_FONT_FAMILY);
                        pango_font_description_set_size   (desc, bstring->fsize * PANGO_SCALE * FONT_SCALE);
                        pango_layout_set_font_description (layout, desc);
                        pango_font_description_free       (desc);

                        pango_layout_set_text (layout, bstring->string, -1);

                        pango_layout_get_size (layout, &iw, &ih);
                        layout_width = (gdouble)iw / (gdouble)PANGO_SCALE;

                        x_offset = layout_width / 2.0;
                        y_offset = 0.2 * bstring->fsize;

                        cairo_move_to (cr, (bstring->x - x_offset), (bstring->y - y_offset));
                        pango_cairo_layout_path (cr, layout);

                        g_object_unref (layout);

                        break;

                case LGL_BARCODE_SHAPE_RING:
                        ring = (lglBarcodeShapeRing *) shape;

                        cairo_new_sub_path (cr);
                        cairo_arc (cr, ring->x, ring->y, ring->radius + ring->line_width/2, 0.0, 2 * M_PI);
                        cairo_close_path (cr);
                        cairo_new_sub_path (cr);
                        cairo_arc (cr, ring->x, ring->y, ring->radius - ring->line_width/2, 0.0, 2 * M_PI);
                        cairo_close_path (cr);
                        break;

                case LGL_BARCODE_SHAPE_HEXAGON:
                        hexagon = (lglBarcodeShapeHexagon *) shape;

                        cairo_move_to (cr, hexagon->x, hexagon->y);
                        cairo_line_to (cr, hexagon->x + 0.433*hexagon->height, hexagon->y + 0.25*hexagon->height);
                        cairo_line_to (cr, hexagon->x + 0.433*hexagon->height, hexagon->y + 0.75*hexagon->height);
                        cairo_line_to (cr, hexagon->x,                         hexagon->y +      hexagon->height);
                        cairo_line_to (cr, hexagon->x - 0.433*hexagon->height, hexagon->y + 0.75*hexagon->height);
                        cairo_line_to (cr, hexagon->x - 0.433*hexagon->height, hexagon->y + 0.25*hexagon->height);
                        cairo_close_path (cr);
                        break;

                default:
                        g_assert_not_reached ();
                        break;

                }

        }

}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
