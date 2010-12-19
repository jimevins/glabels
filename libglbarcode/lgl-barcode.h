/*
 *  lgl-barcode.h
 *  Copyright (C) 2001-2010  Jim Evins <evins@snaught.com>.
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

#ifndef __LGL_BARCODE_H__
#define __LGL_BARCODE_H__

#include <glib.h>

G_BEGIN_DECLS


/********************************/
/* Barcode Intermediate Format. */
/********************************/

/**
 * lglBarcode:
 *  @width:    Width of barcode bounding box (points)
 *  @height:   Height of barcode bounding box (points)
 *  @shapes:   List of #lglBarcodeShape drawing primitives
 *
 * This structure contains the libglbarcode intermediate barcode format.  This
 * structure contains a simple vectorized representation of the barcode.  This
 * vectorized representation is easy to interpret by a rendering backend for
 * either vector or raster formats.  A simple API is provided for constructing
 * barcodes in this format.
 *
 */
typedef struct {

        gdouble  width;
        gdouble  height;

        GList   *shapes;    /* List of lglBarcodeShape drawing primitives */

} lglBarcode;


/********************************/
/* Barcode Construction.        */
/********************************/

lglBarcode      *lgl_barcode_new              (void);

void             lgl_barcode_free             (lglBarcode     *bc);

void             lgl_barcode_add_line         (lglBarcode     *bc,
                                               gdouble         x,
                                               gdouble         y,
                                               gdouble         length,
                                               gdouble         width);

void             lgl_barcode_add_box          (lglBarcode     *bc,
                                               gdouble         x,
                                               gdouble         y,
                                               gdouble         width,
                                               gdouble         height);

void             lgl_barcode_add_char         (lglBarcode     *bc,
                                               gdouble         x,
                                               gdouble         y,
                                               gdouble         fsize,
                                               gchar           c);

void             lgl_barcode_add_string       (lglBarcode     *bc,
                                               gdouble         x,
                                               gdouble         y,
                                               gdouble         fsize,
                                               gchar          *string,
                                               gsize           length);

void             lgl_barcode_add_ring         (lglBarcode     *bc,
                                               gdouble         x,
                                               gdouble         y,
                                               gdouble         radius,
                                               gdouble         line_width);

void             lgl_barcode_add_hexagon      (lglBarcode     *bc,
                                               gdouble         x,
                                               gdouble         y,
                                               gdouble         height);

/*******************************/
/* Barcode Drawing Primitives. */
/*******************************/

typedef enum {
        LGL_BARCODE_SHAPE_LINE,
        LGL_BARCODE_SHAPE_BOX,
        LGL_BARCODE_SHAPE_CHAR,
        LGL_BARCODE_SHAPE_STRING,
        LGL_BARCODE_SHAPE_RING,
        LGL_BARCODE_SHAPE_HEXAGON
} lglBarcodeShapeType;


typedef struct {

        /* Begin Common Fields */
        lglBarcodeShapeType  type;
        gdouble              x;
        gdouble              y;
        /* End Common Fields */

} lglBarcodeShapeAny;


/**
 * lglBarcodeShapeLine:
 * @type:   Always %LGL_BARCODE_SHAPE_LINE
 * @x:      x coordinate of top of line
 * @y:      y coordinate of top of line
 * @length: Length of line
 * @width:  Width of line
 *
 * A vertical line drawing primitive.
 *
 *<programlisting>
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
 *
 *</programlisting>
 *
 * All units are in points ( 1 point = 1/72 inch ).
 */
typedef struct {

        /* Begin Common Fields */
        lglBarcodeShapeType  type; /* Always LGL_BARCODE_SHAPE_LINE. */
        gdouble              x;
        gdouble              y;
        /* End Common Fields */

        gdouble              length;
        gdouble              width;

} lglBarcodeShapeLine;


/**
 * lglBarcodeShapeBox:
 * @type:   Always %LGL_BARCODE_SHAPE_BOX
 * @x:      x coordinate of top left corner of box
 * @y:      y coordinate of top left corner of box
 * @width:  Width of box
 * @height: Height of box
 *
 * A solid box drawing primitive.
 *
 *<programlisting>
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
 *
 *</programlisting>
 *
 * All units are in points ( 1 point = 1/72 inch ).
 */
typedef struct {

        /* Begin Common Fields */
        lglBarcodeShapeType  type; /* Always LGL_BARCODE_SHAPE_BOX. */
        gdouble              x;
        gdouble              y;
        /* End Common Fields */

        gdouble              width;
        gdouble              height;

} lglBarcodeShapeBox;


/**
 * lglBarcodeShapeChar:
 * @type:   Always %LGL_BARCODE_SHAPE_CHAR
 * @x:      x coordinate of left baseline of character
 * @y:      y coordinate of left baseline of character
 * @fsize:  Font size
 * @c:      Character to add
 *
 * An single byte character drawing primitive.
 *
 *<programlisting>
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
 *
 *</programlisting>
 *
 * All units are in points ( 1 point = 1/72 inch ).
 */
typedef struct {

        /* Begin Common Fields */
        lglBarcodeShapeType  type; /* Always LGL_BARCODE_SHAPE_CHAR. */
        gdouble              x;
        gdouble              y;
        /* End Common Fields */

        gdouble              fsize;
        gchar                c;

} lglBarcodeShapeChar;


/**
 * lglBarcodeShapeString:
 * @type:   Always %LGL_BARCODE_SHAPE_STRING
 * @x:      x coordinate of horizontal center of baseline of string
 * @y:      y coordinate of horizontal center of baseline of string
 * @fsize:  Font size
 * @string: String to add
 * @length: Number of bytes in string
 *
 * A character string drawing primitive.
 *
 *<programlisting>
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
 *
 *</programlisting>
 *
 * All units are in points ( 1 point = 1/72 inch ).
 */
typedef struct {

        /* Begin Common Fields */
        lglBarcodeShapeType  type; /* Always LGL_BARCODE_SHAPE_STRING. */
        gdouble              x;
        gdouble              y;
        /* End Common Fields */

        gdouble              fsize;
        gchar               *string;

} lglBarcodeShapeString;


/**
 * lglBarcodeShapeRing:
 * @type:       Always %LGL_BARCODE_SHAPE_RING
 * @x:          x coordinate of center of circle
 * @y:          y coordinate of center of circle
 * @radius:     Radius of ring (center of line)
 * @line_width: Width of line
 *
 * A ring (an open circle) drawing primitive.
 *
 *<programlisting>
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
 *
 *</programlisting>
 *
 * All units are in points ( 1 point = 1/72 inch ).
 */

typedef struct {

        /* Begin Common Fields */
        lglBarcodeShapeType  type; /* Always LGL_BARCODE_SHAPE_RING. */
        gdouble              x;
        gdouble              y;
        /* End Common Fields */

        gdouble              radius;
        gdouble              line_width;

} lglBarcodeShapeRing;


/**
 * lglBarcodeShapeHexagon:
 * @type:   Always %LGL_BARCODE_SHAPE_HEXAGON
 * @x:      x coordinate of top point of hexagon
 * @y:      y coordinate of top point of hexagon
 * @height: Height of hexagon
 *
 * A solid regular hexagon (oriented with vertexes at top and bottom) drawing primitive.
 *
 *<programlisting>
 *
 * @ = origin (x,y) is top of hexagon
 *
 *                  @ ------------------
 *              _-"   "-_              ^
 *          _-"           "-_          |
 *       +"                   "+       |
 *       |                     |       |
 *       |                     |       |
 *       |                     |       | height
 *       |                     |       |
 *       |                     |       |
 *       +_                   _+       |
 *         "-_             _-"         |
 *            "-_       _-"            |
 *               "-_ _-"               v
 *                  " ------------------
 *
 *</programlisting>
 *
 * All units are in points ( 1 point = 1/72 inch ).
 */

typedef struct {

        /* Begin Common Fields */
        lglBarcodeShapeType  type; /* Always LGL_BARCODE_SHAPE_HEXAGON. */
        gdouble              x;
        gdouble              y;
        /* End Common Fields */

        gdouble              height;

} lglBarcodeShapeHexagon;


typedef union {

        lglBarcodeShapeType    type;
        lglBarcodeShapeAny     any;

        lglBarcodeShapeLine    line;
        lglBarcodeShapeBox     box;
        lglBarcodeShapeChar    bchar;
        lglBarcodeShapeString  string;
        lglBarcodeShapeRing    ring;
        lglBarcodeShapeHexagon hexagon;

} lglBarcodeShape;


G_END_DECLS

#endif /* __LGL_BARCODE_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
