/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  stock.h:  Stock image module header file
 *
 *  Copyright (C) 2001-2002  Jim Evins <evins@snaught.com>.
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

#ifndef __STOCK_H__
#define __STOCK_H__

/* Stock names */

#define GL_STOCK_PIXMAP_ARROW        "GL_stock_arrow"
#define GL_STOCK_PIXMAP_TEXT         "GL_stock_text"
#define GL_STOCK_PIXMAP_LINE         "GL_stock_line"
#define GL_STOCK_PIXMAP_BOX          "GL_stock_box"
#define GL_STOCK_PIXMAP_ELLIPSE      "GL_stock_ellipse"
#define GL_STOCK_PIXMAP_IMAGE        "GL_stock_image"
#define GL_STOCK_PIXMAP_BARCODE      "GL_stock_barcode"
#define GL_STOCK_PIXMAP_ZOOMIN       "GL_stock_zoomin"
#define GL_STOCK_PIXMAP_ZOOMOUT      "GL_stock_zoomout"
#define GL_STOCK_PIXMAP_ZOOM1TO1     "GL_stock_zoom1to1"
#define GL_STOCK_PIXMAP_MERGE        "GL_stock_merge"

void gl_stock_init (void);

#endif
