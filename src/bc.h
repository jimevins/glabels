/*
 *  bc.h
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

#ifndef __BC_H__
#define __BC_H__

#include <glib.h>
#include <pango/pango.h>

G_BEGIN_DECLS


#define GL_BARCODE_FONT_FAMILY      "Sans"
#define GL_BARCODE_FONT_WEIGHT      PANGO_WEIGHT_NORMAL


/********************************/
/* Barcode Intermediate Format. */
/********************************/

typedef struct {

        gdouble  width;
        gdouble  height;

        GList   *shapes;    /* List of glBarcodeShape drawing primitives */

} glBarcode;


glBarcode       *gl_barcode_new              (void);

void             gl_barcode_free             (glBarcode     **bc);

void             gl_barcode_add_line         (glBarcode      *bc,
                                              gdouble         x,
                                              gdouble         y,
                                              gdouble         length,
                                              gdouble         width);

void             gl_barcode_add_box          (glBarcode      *bc,
                                              gdouble         x,
                                              gdouble         y,
                                              gdouble         width,
                                              gdouble         height);

void             gl_barcode_add_char         (glBarcode      *bc,
                                              gdouble         x,
                                              gdouble         y,
                                              gdouble         fsize,
                                              gchar           c);

void             gl_barcode_add_string       (glBarcode      *bc,
                                              gdouble         x,
                                              gdouble         y,
                                              gdouble         fsize,
                                              gchar          *string,
                                              gsize           length);

void             gl_barcode_add_ring         (glBarcode      *bc,
                                              gdouble         x,
                                              gdouble         y,
                                              gdouble         radius,
                                              gdouble         line_width);

void             gl_barcode_add_hexagon      (glBarcode      *bc,
                                              gdouble         x,
                                              gdouble         y);

/*******************************/
/* Barcode Drawing Primitives. */
/*******************************/

typedef enum {
        GL_BARCODE_SHAPE_LINE,
        GL_BARCODE_SHAPE_BOX,
        GL_BARCODE_SHAPE_CHAR,
        GL_BARCODE_SHAPE_STRING,
        GL_BARCODE_SHAPE_RING,
        GL_BARCODE_SHAPE_HEXAGON,
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
 * glBarcodeShapeBox:
 *
 * @ =  origin (x,y) from top left corner of barcode
 *
 *              @---------+
 *              |         |
 *              |         |
 *              |         |
 *              |         | height
 *              |         |
 *              |         |
 *              |         |
 *              +---------+
 *                 width
 */
typedef struct {

        /* Begin Common Fields */
        glBarcodeShapeType  type; /* Always GL_BARCODE_SHAPE_BOX. */
        gdouble             x;
        gdouble             y;
        /* End Common Fields */

        gdouble             width;
        gdouble             height;

} glBarcodeShapeBox;

/*
 * glBarcodeShapeChar:
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
        glBarcodeShapeType  type; /* Always GL_BARCODE_SHAPE_CHAR. */
        gdouble             x;
        gdouble             y;
        /* End Common Fields */

        gdouble             fsize;
        gchar               c;

} glBarcodeShapeChar;

/*
 * glBarcodeShapeString:
 *
 * @ =  origin (x,y) from top left corner of barcode
 *
 *              ____        _  ------------------
 *             /    \      | |                  ^
 *            /  /\  \     | |                  |
 *           /  /__\  \    | |___     ____      |
 *          /  ______  \   | ._  \   /  __|     | ~fsize
 *         /  /      \  \  | |_)  | |  (__      |
 *        /__/        \__\ |_.___/   \____|     |
 *                                              v
 *                           @ ------------------
 *                           x = horizontal center
 */
typedef struct {

        /* Begin Common Fields */
        glBarcodeShapeType  type; /* Always GL_BARCODE_SHAPE_STRING. */
        gdouble             x;
        gdouble             y;
        /* End Common Fields */

        gdouble             fsize;
        gchar              *string;

} glBarcodeShapeString;

/*
 * glBarcodeShapeRing:
 *
 * @ = origin (x,y) is centre of circle
 *
 *                v  line_width
 *           _.-""""-._
 *         .'   ____   `.
 *        /   .'  ^ `.   \
 *       |   /        \   |
 *       |   |    @---|---|------
 *       |   \        /   |     ^
 *        \   `.____.'   /      | radius
 *         `._    ...._.'.......|
 *            `-....-'
 */

typedef struct {

        /* Begin Common Fields */
        glBarcodeShapeType  type; /* Always GL_BARCODE_SHAPE_RING. */
        gdouble             x;
        gdouble             y;
        /* End Common Fields */

        gdouble             radius;
        gdouble             line_width;

} glBarcodeShapeRing;

/*
 * glBarcodeShapeHexagon;
 *
 * @ = origin (x,y) is top of hexagon
 *
 *                  @
 *              _-"   "-_
 *          _-"           "-_
 *       +"                   "+
 *       |                     |
 *       |                     |
 *       |                     |
 *       |                     |
 *       |                     |
 *       +_                   _+
 *         "-_             _-"
 *            "-_       _-"
 *               "-_ _-"
 *                  "
 *
 * NOTE: For Maxicode hexagons height is always 1.02mm, width is always 0.88mm
 */

typedef struct {

        /* Begin Common Fields */
        glBarcodeShapeType  type; /* Always GL_BARCODE_SHAPE_HEXAGON. */
        gdouble             x;
        gdouble             y;
        /* End Common Fields */

} glBarcodeShapeHexagon;

typedef union {

        glBarcodeShapeType    type;
        glBarcodeShapeAny     any;

        glBarcodeShapeLine    line;
        glBarcodeShapeBox     box;
        glBarcodeShapeChar    bchar;
        glBarcodeShapeString  string;
        glBarcodeShapeRing    ring;
        glBarcodeShapeHexagon hexagon;

} glBarcodeShape;


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
