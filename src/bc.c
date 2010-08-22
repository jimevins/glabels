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
#include "bc-zint.h"
#include "bc-iec16022.h"
#include "bc-iec18004.h"

#include "debug.h"


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

static void gl_barcode_add_shape        (glBarcode      *bc,
                                         glBarcodeShape *shape);

static void gl_barcode_shape_free       (glBarcodeShape *shape);


/*****************************************************************************/
/* Allocate new empty glBarcode structure.                                   */
/*****************************************************************************/
glBarcode *
gl_barcode_new (void)
{
        return g_new0 (glBarcode, 1);
}


/*****************************************************************************/
/* Free previously created barcode.                                          */
/*****************************************************************************/
void
gl_barcode_free (glBarcode **gbc)
{
        GList *p;

        if (*gbc != NULL)
        {

                for (p = (*gbc)->shapes; p != NULL; p = p->next)
                {
                        gl_barcode_shape_free ((glBarcodeShape *)p->data);
                        p->data = NULL;
                }
                g_list_free ((*gbc)->shapes);
                (*gbc)->shapes = NULL;

                g_free (*gbc);
                *gbc = NULL;
        }
}


/*****************************************************************************/
/* Add a line.                                                               */
/*****************************************************************************/
void
gl_barcode_add_line (glBarcode      *bc,
                     gdouble         x,
                     gdouble         y,
                     gdouble         length,
                     gdouble         width)
{
        glBarcodeShapeLine *line_shape = g_new0 (glBarcodeShapeLine, 1);
        line_shape->type = GL_BARCODE_SHAPE_LINE;

        line_shape->x      = x;
        line_shape->y      = y;
        line_shape->length = length;
        line_shape->width  = width;

        gl_barcode_add_shape (bc, (glBarcodeShape *)line_shape);
}


/*****************************************************************************/
/* Add box.                                                                  */
/*****************************************************************************/
void
gl_barcode_add_box (glBarcode      *bc,
                    gdouble         x,
                    gdouble         y,
                    gdouble         width,
                    gdouble         height)
{
        glBarcodeShapeBox *box_shape = g_new0 (glBarcodeShapeBox, 1);
        box_shape->type = GL_BARCODE_SHAPE_BOX;

        box_shape->x      = x;
        box_shape->y      = y;
        box_shape->width  = width;
        box_shape->height = height;

        gl_barcode_add_shape (bc, (glBarcodeShape *)box_shape);
}


/*****************************************************************************/
/* Add character.                                                            */
/*****************************************************************************/
void
gl_barcode_add_char (glBarcode      *bc,
                     gdouble         x,
                     gdouble         y,
                     gdouble         fsize,
                     gchar           c)
{
        glBarcodeShapeChar *char_shape = g_new0 (glBarcodeShapeChar, 1);
        char_shape->type = GL_BARCODE_SHAPE_CHAR;

        char_shape->x      = x;
        char_shape->y      = y;
        char_shape->fsize  = fsize;
        char_shape->c      = c;

        gl_barcode_add_shape (bc, (glBarcodeShape *)char_shape);
}


/*****************************************************************************/
/* Add string.                                                               */
/*****************************************************************************/
void
gl_barcode_add_string (glBarcode      *bc,
                       gdouble         x,
                       gdouble         y,
                       gdouble         fsize,
                       gchar          *string,
                       gsize           length)
{
        glBarcodeShapeString *string_shape = g_new0 (glBarcodeShapeString, 1);
        string_shape->type = GL_BARCODE_SHAPE_STRING;

        string_shape->x      = x;
        string_shape->y      = y;
        string_shape->fsize  = fsize;
        string_shape->string = g_strndup(string, length);

        gl_barcode_add_shape (bc, (glBarcodeShape *)string_shape);
}

/*****************************************************************************/
/* Add ring.                                                                 */
/*****************************************************************************/
void
gl_barcode_add_ring (glBarcode      *bc,
                     gdouble         x,
                     gdouble         y,
                     gdouble         radius,
                     gdouble         line_width)
{
        glBarcodeShapeRing *ring_shape = g_new0 (glBarcodeShapeRing, 1);
        ring_shape->type = GL_BARCODE_SHAPE_RING;

        ring_shape->x          = x;
        ring_shape->y          = y;
        ring_shape->radius     = radius;
        ring_shape->line_width = line_width;

        gl_barcode_add_shape (bc, (glBarcodeShape *)ring_shape);
}

/*****************************************************************************/
/* Add hexagon.                                                              */
/*****************************************************************************/
void
gl_barcode_add_hexagon (glBarcode      *bc,
                        gdouble         x,
                        gdouble         y)
{
        glBarcodeShapeHexagon *hexagon_shape = g_new0 (glBarcodeShapeHexagon, 1);
        hexagon_shape->type = GL_BARCODE_SHAPE_HEXAGON;

        hexagon_shape->x      = x;
        hexagon_shape->y      = y;

        gl_barcode_add_shape (bc, (glBarcodeShape *)hexagon_shape);
}


/*****************************************************************************/
/* Add shape to barcode.                                                     */
/*****************************************************************************/
static void
gl_barcode_add_shape (glBarcode      *bc,
                      glBarcodeShape *shape)
{
        g_return_if_fail (bc);
        g_return_if_fail (shape);

        bc->shapes = g_list_prepend (bc->shapes, shape);
}


/*****************************************************************************/
/* Free a shape primitive.                                                   */
/*****************************************************************************/
static void
gl_barcode_shape_free (glBarcodeShape *shape)
{
        switch (shape->type)
        {

        case GL_BARCODE_SHAPE_STRING:
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
