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

#define GL_STOCK_ARROW         "gl_stock_arrow"
#define GL_STOCK_TEXT          "gl_stock_text"
#define GL_STOCK_LINE          "gl_stock_line"
#define GL_STOCK_BOX           "gl_stock_box"
#define GL_STOCK_ELLIPSE       "gl_stock_ellipse"
#define GL_STOCK_IMAGE         "gl_stock_image"
#define GL_STOCK_BARCODE       "gl_stock_barcode"
#define GL_STOCK_MERGE         "gl_stock_merge"
#define GL_STOCK_ALIGN_LEFT    "gl_stock_align_left"
#define GL_STOCK_ALIGN_RIGHT   "gl_stock_align_right"
#define GL_STOCK_ALIGN_HCENTER "gl_stock_align_hcenter"
#define GL_STOCK_ALIGN_TOP     "gl_stock_align_top"
#define GL_STOCK_ALIGN_BOTTOM  "gl_stock_align_bottom"
#define GL_STOCK_ALIGN_VCENTER "gl_stock_align_vcenter"

void gl_stock_init (void);

#endif
