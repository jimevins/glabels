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
#include <gdk-pixbuf/gdk-pixdata.h>
#include "stock.h"
#include "stock-pixmaps/stockpixbufs.h"

void
gl_stock_init (void)
{
	GdkPixbuf *pixbuf;
	GtkIconFactory *factory;
	GtkIconSet *icon_set;
	GdkPixdata *pixdata;

	static GtkStockItem items[] = {
		{ GL_STOCK_ARROW,   "Arrow",   0, 0, NULL },
		{ GL_STOCK_TEXT,    "Text",    0, 0, NULL },
		{ GL_STOCK_LINE,    "Line",    0, 0, NULL },
		{ GL_STOCK_BOX,     "Box",     0, 0, NULL },
		{ GL_STOCK_ELLIPSE, "Ellipse", 0, 0, NULL },
		{ GL_STOCK_IMAGE,   "Image",   0, 0, NULL },
		{ GL_STOCK_BARCODE, "Barcode", 0, 0, NULL },
		{ GL_STOCK_MERGE,   "Merge",   0, 0, NULL },
	};

	gtk_stock_add (items, G_N_ELEMENTS (items));

	factory = gtk_icon_factory_new ();
	gtk_icon_factory_add_default (factory);

	pixdata = g_new0 (GdkPixdata, 1);

	/* Arrow */
	gdk_pixdata_deserialize (pixdata,
				 sizeof(stock_arrow_24), stock_arrow_24,
				 NULL);
	pixbuf = gdk_pixbuf_from_pixdata (pixdata, FALSE, NULL);
	icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);
	gtk_icon_factory_add (factory, GL_STOCK_ARROW, icon_set);
	gtk_icon_set_unref (icon_set);
	g_object_unref (G_OBJECT(pixbuf));

	/* Text */
	gdk_pixdata_deserialize (pixdata,
				 sizeof(stock_text_24), stock_text_24,
				 NULL);
	pixbuf = gdk_pixbuf_from_pixdata (pixdata, FALSE, NULL);
	icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);
	gtk_icon_factory_add (factory, GL_STOCK_TEXT, icon_set);
	gtk_icon_set_unref (icon_set);
	g_object_unref (G_OBJECT(pixbuf));

	/* Line */
	gdk_pixdata_deserialize (pixdata,
				 sizeof(stock_line_24), stock_line_24,
				 NULL);
	pixbuf = gdk_pixbuf_from_pixdata (pixdata, FALSE, NULL);
	icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);
	gtk_icon_factory_add (factory, GL_STOCK_LINE, icon_set);
	gtk_icon_set_unref (icon_set);
	g_object_unref (G_OBJECT(pixbuf));

	/* Box */
	gdk_pixdata_deserialize (pixdata,
				 sizeof(stock_box_24), stock_box_24,
				 NULL);
	pixbuf = gdk_pixbuf_from_pixdata (pixdata, FALSE, NULL);
	icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);
	gtk_icon_factory_add (factory, GL_STOCK_BOX, icon_set);
	gtk_icon_set_unref (icon_set);
	g_object_unref (G_OBJECT(pixbuf));

	/* Ellipse */
	gdk_pixdata_deserialize (pixdata,
				 sizeof(stock_ellipse_24), stock_ellipse_24,
				 NULL);
	pixbuf = gdk_pixbuf_from_pixdata (pixdata, FALSE, NULL);
	icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);
	gtk_icon_factory_add (factory, GL_STOCK_ELLIPSE, icon_set);
	gtk_icon_set_unref (icon_set);
	g_object_unref (G_OBJECT(pixbuf));

	/* Image */
	gdk_pixdata_deserialize (pixdata,
				 sizeof(stock_image_24), stock_image_24,
				 NULL);
	pixbuf = gdk_pixbuf_from_pixdata (pixdata, FALSE, NULL);
	icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);
	gtk_icon_factory_add (factory, GL_STOCK_IMAGE, icon_set);
	gtk_icon_set_unref (icon_set);
	g_object_unref (G_OBJECT(pixbuf));

	/* Barcode */
	gdk_pixdata_deserialize (pixdata,
				 sizeof(stock_barcode_24), stock_barcode_24,
				 NULL);
	pixbuf = gdk_pixbuf_from_pixdata (pixdata, FALSE, NULL);
	icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);
	gtk_icon_factory_add (factory, GL_STOCK_BARCODE, icon_set);
	gtk_icon_set_unref (icon_set);
	g_object_unref (G_OBJECT(pixbuf));

	/* Merge */
	gdk_pixdata_deserialize (pixdata,
				 sizeof(stock_merge_24), stock_merge_24,
				 NULL);
	pixbuf = gdk_pixbuf_from_pixdata (pixdata, FALSE, NULL);
	icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);
	gtk_icon_factory_add (factory, GL_STOCK_MERGE, icon_set);
	gtk_icon_set_unref (icon_set);
	g_object_unref (G_OBJECT(pixbuf));

	g_free (pixdata);
	g_object_unref (G_OBJECT(factory));
}
