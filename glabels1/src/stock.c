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

#include <config.h>
#include <gnome.h>
#include "stock.h"
#include "pixmaps/arrow.xpm"
#include "pixmaps/text.xpm"
#include "pixmaps/line.xpm"
#include "pixmaps/box.xpm"
#include "pixmaps/ellipse.xpm"
#include "pixmaps/image.xpm"
#include "pixmaps/barcode.xpm"
#include "pixmaps/zoomin.xpm"
#include "pixmaps/zoomout.xpm"
#include "pixmaps/zoom1to1.xpm"
#include "pixmaps/merge.xpm"

void
gl_stock_init (void)
{
	static GnomeStockPixmapEntry entries[11];

	entries[0].data.type = GNOME_STOCK_PIXMAP_TYPE_DATA;
	entries[0].data.width = 24;
	entries[0].data.height = 24;
	entries[0].data.xpm_data = arrow_xpm;

	entries[1].data.type = GNOME_STOCK_PIXMAP_TYPE_DATA;
	entries[1].data.width = 24;
	entries[1].data.height = 24;
	entries[1].data.xpm_data = text_xpm;

	entries[2].data.type = GNOME_STOCK_PIXMAP_TYPE_DATA;
	entries[2].data.width = 24;
	entries[2].data.height = 24;
	entries[2].data.xpm_data = line_xpm;

	entries[3].data.type = GNOME_STOCK_PIXMAP_TYPE_DATA;
	entries[3].data.width = 24;
	entries[3].data.height = 24;
	entries[3].data.xpm_data = box_xpm;

	entries[4].data.type = GNOME_STOCK_PIXMAP_TYPE_DATA;
	entries[4].data.width = 24;
	entries[4].data.height = 24;
	entries[4].data.xpm_data = ellipse_xpm;

	entries[5].data.type = GNOME_STOCK_PIXMAP_TYPE_DATA;
	entries[5].data.width = 24;
	entries[5].data.height = 24;
	entries[5].data.xpm_data = image_xpm;

	entries[6].data.type = GNOME_STOCK_PIXMAP_TYPE_DATA;
	entries[6].data.width = 24;
	entries[6].data.height = 24;
	entries[6].data.xpm_data = barcode_xpm;

	entries[7].data.type = GNOME_STOCK_PIXMAP_TYPE_DATA;
	entries[7].data.width = 24;
	entries[7].data.height = 24;
	entries[7].data.xpm_data = zoomin_xpm;

	entries[8].data.type = GNOME_STOCK_PIXMAP_TYPE_DATA;
	entries[8].data.width = 24;
	entries[8].data.height = 24;
	entries[8].data.xpm_data = zoomout_xpm;

	entries[9].data.type = GNOME_STOCK_PIXMAP_TYPE_DATA;
	entries[9].data.width = 24;
	entries[9].data.height = 24;
	entries[9].data.xpm_data = zoom1to1_xpm;

	entries[10].data.type = GNOME_STOCK_PIXMAP_TYPE_DATA;
	entries[10].data.width = 24;
	entries[10].data.height = 24;
	entries[10].data.xpm_data = merge_xpm;

	gnome_stock_pixmap_register (GL_STOCK_PIXMAP_ARROW,
				     GNOME_STOCK_PIXMAP_REGULAR, &entries[0]);
	gnome_stock_pixmap_register (GL_STOCK_PIXMAP_TEXT,
				     GNOME_STOCK_PIXMAP_REGULAR, &entries[1]);
	gnome_stock_pixmap_register (GL_STOCK_PIXMAP_LINE,
				     GNOME_STOCK_PIXMAP_REGULAR, &entries[2]);
	gnome_stock_pixmap_register (GL_STOCK_PIXMAP_BOX,
				     GNOME_STOCK_PIXMAP_REGULAR, &entries[3]);
	gnome_stock_pixmap_register (GL_STOCK_PIXMAP_ELLIPSE,
				     GNOME_STOCK_PIXMAP_REGULAR, &entries[4]);
	gnome_stock_pixmap_register (GL_STOCK_PIXMAP_IMAGE,
				     GNOME_STOCK_PIXMAP_REGULAR, &entries[5]);
	gnome_stock_pixmap_register (GL_STOCK_PIXMAP_BARCODE,
				     GNOME_STOCK_PIXMAP_REGULAR, &entries[6]);
	gnome_stock_pixmap_register (GL_STOCK_PIXMAP_ZOOMIN,
				     GNOME_STOCK_PIXMAP_REGULAR, &entries[7]);
	gnome_stock_pixmap_register (GL_STOCK_PIXMAP_ZOOMOUT,
				     GNOME_STOCK_PIXMAP_REGULAR, &entries[8]);
	gnome_stock_pixmap_register (GL_STOCK_PIXMAP_ZOOM1TO1,
				     GNOME_STOCK_PIXMAP_REGULAR, &entries[9]);
	gnome_stock_pixmap_register (GL_STOCK_PIXMAP_MERGE,
				     GNOME_STOCK_PIXMAP_REGULAR, &entries[10]);

}
