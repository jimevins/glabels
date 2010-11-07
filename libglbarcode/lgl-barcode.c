/*
 *  lgl-barcode.c
 *  Copyright (C) 2001-2010  Jim Evins <evins@snaught.com>.
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

#include "lgl-barcode.h"


/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/


/*========================================================*/
/* Private types.                                         */
/*========================================================*/


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void lgl_barcode_add_shape        (lglBarcode      *bc,
                                          lglBarcodeShape *shape);

static void lgl_barcode_shape_free       (lglBarcodeShape *shape);


/*****************************************************************************/
/**
 * lgl_barcode_new:
 *
 * Allocate a new #lglBarcode structure.
 *
 * This function allocates a new #lglBarcode structure.
 * 
 * <note><para>
 *       This function is intended to be used internally by barcode implementations.
 *       Typically an end-user would use lgl_barcode_create() instead.
 * </para></note>
 * 
 *
 * Returns: A newly allocated #lglBarcode structure.  Use lgl_barcode_free() to
 *          free it.
 *
 */
lglBarcode *
lgl_barcode_new (void)
{
        return g_new0 (lglBarcode, 1);
}


/*****************************************************************************/
/**
 * lgl_barcode_free:
 * @bc: The #lglBarcode structure to free
 *
 * Free a previously allocated #lglBarcode structure.
 *
 */
void
lgl_barcode_free (lglBarcode *bc)
{
        GList *p;

        if (bc != NULL)
        {

                for (p = bc->shapes; p != NULL; p = p->next)
                {
                        lgl_barcode_shape_free ((lglBarcodeShape *)p->data);
                }
                g_list_free (bc->shapes);

                g_free (bc);

        }
}


/*****************************************************************************/
/**
 * lgl_barcode_add_line:
 * @bc:     An #lglBarcode structure
 * @x:      x coordinate of top of line
 * @y:      y coordinate of top of line
 * @length: Length of line
 * @width:  Width of line
 *
 * Add a vertical line to barcode.  Coordinates are relative to top left corner
 * of barcode.  All units are in points ( 1 point = 1/72 inch ).
 *
 * <note><para>
 *        This function is intended to be used internally by barcode implementations.
 * </para></note>
 *
 */
void
lgl_barcode_add_line (lglBarcode      *bc,
                      gdouble          x,
                      gdouble          y,
                      gdouble          length,
                      gdouble          width)
{
        lglBarcodeShapeLine *line_shape = g_new0 (lglBarcodeShapeLine, 1);
        line_shape->type = LGL_BARCODE_SHAPE_LINE;

        line_shape->x      = x;
        line_shape->y      = y;
        line_shape->length = length;
        line_shape->width  = width;

        lgl_barcode_add_shape (bc, (lglBarcodeShape *)line_shape);
}


/*****************************************************************************/
/**
 * lgl_barcode_add_box:
 * @bc:     An #lglBarcode structure
 * @x:      x coordinate of top left corner of box
 * @y:      y coordinate of top left corner of box
 * @width:  Width of box
 * @height: Height of box
 *
 * Add a box to barcode.  Coordinates are relative to top left corner
 * of barcode.  All units are in points ( 1 point = 1/72 inch ).
 *
 * <note><para>
 *        This function is intended to be used internally by barcode implementations.
 * </para></note>
 *
 */
void
lgl_barcode_add_box (lglBarcode      *bc,
                     gdouble          x,
                     gdouble          y,
                     gdouble          width,
                     gdouble          height)
{
        lglBarcodeShapeBox *box_shape = g_new0 (lglBarcodeShapeBox, 1);
        box_shape->type = LGL_BARCODE_SHAPE_BOX;

        box_shape->x      = x;
        box_shape->y      = y;
        box_shape->width  = width;
        box_shape->height = height;

        lgl_barcode_add_shape (bc, (lglBarcodeShape *)box_shape);
}


/*****************************************************************************/
/**
 * lgl_barcode_add_char:
 * @bc:     An #lglBarcode structure
 * @x:      x coordinate of left baseline of character
 * @y:      y coordinate of left baseline of character
 * @fsize:  Font size
 * @c:      Character to add
 *
 * Add an ASCII character to barcode.  Coordinates are relative to top left corner
 * of barcode.  All units are in points ( 1 point = 1/72 inch ).
 *
 * <note><para>
 *        This function is intended to be used internally by barcode implementations.
 * </para></note>
 *
 */
void
lgl_barcode_add_char (lglBarcode      *bc,
                      gdouble          x,
                      gdouble          y,
                      gdouble          fsize,
                      gchar            c)
{
        lglBarcodeShapeChar *char_shape = g_new0 (lglBarcodeShapeChar, 1);
        char_shape->type = LGL_BARCODE_SHAPE_CHAR;

        char_shape->x      = x;
        char_shape->y      = y;
        char_shape->fsize  = fsize;
        char_shape->c      = c;

        lgl_barcode_add_shape (bc, (lglBarcodeShape *)char_shape);
}


/*****************************************************************************/
/**
 * lgl_barcode_add_string:
 * @bc:     An #lglBarcode structure
 * @x:      x coordinate of horizontal center of baseline of string
 * @y:      y coordinate of horizontal center of baseline of string
 * @fsize:  Font size
 * @string: String to add
 * @length: Number of bytes in string
 *
 * Add a character string to barcode.  Coordinates are relative to top left corner
 * of barcode.  All units are in points ( 1 point = 1/72 inch ).
 *
 * <note><para>
 *        This function is intended to be used internally by barcode implementations.
 * </para></note>
 *
 */
void
lgl_barcode_add_string (lglBarcode      *bc,
                        gdouble          x,
                        gdouble          y,
                        gdouble          fsize,
                        gchar           *string,
                        gsize            length)
{
        lglBarcodeShapeString *string_shape = g_new0 (lglBarcodeShapeString, 1);
        string_shape->type = LGL_BARCODE_SHAPE_STRING;

        string_shape->x      = x;
        string_shape->y      = y;
        string_shape->fsize  = fsize;
        string_shape->string = g_strndup(string, length);

        lgl_barcode_add_shape (bc, (lglBarcodeShape *)string_shape);
}

/*****************************************************************************/
/**
 * lgl_barcode_add_ring:
 * @bc:         An #lglBarcode structure
 * @x:          x coordinate of center of circle
 * @y:          y coordinate of center of circle
 * @radius:     Radius of ring (center of line)
 * @line_width: Width of line
 *
 * Add a ring to barcode.  Coordinates are relative to top left corner
 * of barcode.  All units are in points ( 1 point = 1/72 inch ).
 *
 * <note><para>
 *        This function is intended to be used internally by barcode implementations.
 * </para></note>
 *
 */
void
lgl_barcode_add_ring (lglBarcode      *bc,
                      gdouble          x,
                      gdouble          y,
                      gdouble          radius,
                      gdouble          line_width)
{
        lglBarcodeShapeRing *ring_shape = g_new0 (lglBarcodeShapeRing, 1);
        ring_shape->type = LGL_BARCODE_SHAPE_RING;

        ring_shape->x          = x;
        ring_shape->y          = y;
        ring_shape->radius     = radius;
        ring_shape->line_width = line_width;

        lgl_barcode_add_shape (bc, (lglBarcodeShape *)ring_shape);
}

/*****************************************************************************/
/**
 * lgl_barcode_add_hexagon:
 * @bc:         An #lglBarcode structure
 * @x:          x coordinate of top point of hexagon
 * @y:          y coordinate of top point of hexagon
 * @height:     Height of hexagon
 *
 * Add a regular hexagon (oriented with vertexes at top and bottom) to barcode.
 * Coordinates are relative to top left corner of barcode.  All units are in
 * points ( 1 point = 1/72 inch ).
 *
 * <note><para>
 *        This function is intended to be used internally by barcode implementations.
 * </para></note>
 *
 */
void
lgl_barcode_add_hexagon (lglBarcode      *bc,
                         gdouble          x,
                         gdouble          y,
                         gdouble          height)
{
        lglBarcodeShapeHexagon *hexagon_shape = g_new0 (lglBarcodeShapeHexagon, 1);
        hexagon_shape->type = LGL_BARCODE_SHAPE_HEXAGON;

        hexagon_shape->x      = x;
        hexagon_shape->y      = y;
        hexagon_shape->height = height;

        lgl_barcode_add_shape (bc, (lglBarcodeShape *)hexagon_shape);
}


/*****************************************************************************/
/* Add shape to barcode.                                                     */
/*****************************************************************************/
static void
lgl_barcode_add_shape (lglBarcode      *bc,
                       lglBarcodeShape *shape)
{
        g_return_if_fail (bc);
        g_return_if_fail (shape);

        bc->shapes = g_list_prepend (bc->shapes, shape);
}


/*****************************************************************************/
/* Free a shape primitive.                                                   */
/*****************************************************************************/
static void
lgl_barcode_shape_free (lglBarcodeShape *shape)
{
        switch (shape->type)
        {

        case LGL_BARCODE_SHAPE_STRING:
                g_free (shape->string.string);
                break;

        default:
                break;
        }

        g_free (shape);
}




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
