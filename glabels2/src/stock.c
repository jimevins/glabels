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


/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static add_icons (GtkIconFactory *factory,
		  const gchar    *stock_id,
		  const guchar   *inline_24,
		  const guchar   *inline_16);


/****************************************************************************/
/* Initialize our stock icons.                                              */
/****************************************************************************/
void
gl_stock_init (void)
{
	GtkIconFactory *factory;

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

	add_icons (factory, GL_STOCK_ARROW,   stock_arrow_24,   stock_arrow_16);
	add_icons (factory, GL_STOCK_TEXT,    stock_text_24,    stock_text_16);
	add_icons (factory, GL_STOCK_LINE,    stock_line_24,    stock_line_16);
	add_icons (factory, GL_STOCK_BOX,     stock_box_24,     stock_box_16);
	add_icons (factory, GL_STOCK_ELLIPSE, stock_ellipse_24, stock_ellipse_16);
	add_icons (factory, GL_STOCK_IMAGE,   stock_image_24,   stock_image_16);
	add_icons (factory, GL_STOCK_BARCODE, stock_barcode_24, stock_barcode_16);
	add_icons (factory, GL_STOCK_MERGE,   stock_merge_24,   stock_merge_16);

	g_object_unref (G_OBJECT(factory));
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Unpack and associate given icons with stock_id.                */
/*--------------------------------------------------------------------------*/
static add_icons (GtkIconFactory *factory,
		  const gchar    *stock_id,
		  const guchar   *inline_24,
		  const guchar   *inline_16)
{
	GdkPixbuf      *pixbuf;
	GtkIconSet     *icon_set;
	GtkIconSource  *icon_source;

	/* Create icon set with default image */
	pixbuf = gdk_pixbuf_new_from_inline (-1, inline_24, FALSE, NULL);
	icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);
	g_object_unref (G_OBJECT(pixbuf));

	/* Add 16x16 icon for menus to icon set */
	if ( inline_16 != NULL ) {
		icon_source = gtk_icon_source_new ();
		pixbuf = gdk_pixbuf_new_from_inline (-1, inline_16,
						     FALSE, NULL);
		gtk_icon_source_set_pixbuf (icon_source, pixbuf);
		g_object_unref (G_OBJECT(pixbuf));
		gtk_icon_source_set_size_wildcarded (icon_source, FALSE);
		gtk_icon_source_set_size (icon_source, GTK_ICON_SIZE_MENU);
		gtk_icon_set_add_source (icon_set, icon_source);
		g_free (icon_source);
	}

	/* Now associate icon set with stock id */
	gtk_icon_factory_add (factory, stock_id, icon_set);
	gtk_icon_set_unref (icon_set);
}







