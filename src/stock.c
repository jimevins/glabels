/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

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

#include "stock.h"

#include <glib/gi18n.h>
#include <gtk/gtkiconfactory.h>
#include <gtk/gtkstock.h>
#include <gdk-pixbuf/gdk-pixdata.h>

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

static void add_icons (GtkIconFactory *factory,
                       const gchar    *stock_id,
                       const guchar   *inline_24,
                       const guchar   *inline_16);

static void add_button_icon (GtkIconFactory *factory,
                             const gchar    *stock_id,
                             const guchar   *inline_24);


/****************************************************************************/
/* Initialize our stock icons.                                              */
/****************************************************************************/
void
gl_stock_init (void)
{
	GtkIconFactory *factory;

	static GtkStockItem items[] = {
		{ GL_STOCK_ARROW,         N_("_Select Mode"),          0, 0, NULL },
		{ GL_STOCK_TEXT,          N_("_Text"),                 0, 0, NULL },
		{ GL_STOCK_LINE,          N_("_Line"),                 0, 0, NULL },
		{ GL_STOCK_BOX,           N_("_Box"),                  0, 0, NULL },
		{ GL_STOCK_ELLIPSE,       N_("_Ellipse"),              0, 0, NULL },
		{ GL_STOCK_IMAGE,         N_("_Image"),                0, 0, NULL },
		{ GL_STOCK_BARCODE,       N_("Bar_code"),              0, 0, NULL },
		{ GL_STOCK_MERGE,         N_("_Merge Properties"),     0, 0, NULL },
		{ GL_STOCK_PROPERTIES,    N_("Object _Properties"),    0, 0, NULL },
		{ GL_STOCK_ORDER_TOP,     N_("Bring to _Front"),       0, 0, NULL },
		{ GL_STOCK_ORDER_BOTTOM,  N_("Send to _Back"),         0, 0, NULL },
		{ GL_STOCK_ROTATE_LEFT,   N_("Rotate _Left"),          0, 0, NULL },
		{ GL_STOCK_ROTATE_RIGHT,  N_("Rotate _Right"),         0, 0, NULL },
		{ GL_STOCK_FLIP_HORIZ,    N_("Flip _Horizontally"),    0, 0, NULL },
		{ GL_STOCK_FLIP_VERT,     N_("Flip _Vertically"),      0, 0, NULL },
		{ GL_STOCK_ALIGN_LEFT,    N_("_Lefts"),                0, 0, NULL },
		{ GL_STOCK_ALIGN_RIGHT,   N_("_Rights"),               0, 0, NULL },
		{ GL_STOCK_ALIGN_HCENTER, N_("_Centers"),              0, 0, NULL },
		{ GL_STOCK_ALIGN_TOP,     N_("_Tops"),                 0, 0, NULL },
		{ GL_STOCK_ALIGN_BOTTOM,  N_("Bottoms"),               0, 0, NULL },
		{ GL_STOCK_ALIGN_VCENTER, N_("Centers"),               0, 0, NULL },
		{ GL_STOCK_CENTER_HORIZ,  N_("Label Ce_nter"),         0, 0, NULL },
		{ GL_STOCK_CENTER_VERT,   N_("Label Ce_nter"),         0, 0, NULL },
		{ GL_STOCK_BUCKET_FILL,   N_("Fill color"),            0, 0, NULL },
		{ GL_STOCK_PENCIL,        N_("Line color"),            0, 0, NULL },
		{ GL_STOCK_HCHAIN,        N_("Linked"),                0, 0, NULL },
		{ GL_STOCK_HCHAIN_BROKEN, N_("Not Linked"),            0, 0, NULL },
		{ GL_STOCK_VCHAIN,        N_("Linked"),                0, 0, NULL },
		{ GL_STOCK_VCHAIN_BROKEN, N_("Not Linked"),            0, 0, NULL },
	};

	gtk_stock_add (items, G_N_ELEMENTS (items));

	factory = gtk_icon_factory_new ();
	gtk_icon_factory_add_default (factory);

	add_icons (factory, GL_STOCK_ARROW,         stock_arrow_24,   stock_arrow_16);
	add_icons (factory, GL_STOCK_TEXT,          stock_text_24,    stock_text_16);
	add_icons (factory, GL_STOCK_LINE,          stock_line_24,    stock_line_16);
	add_icons (factory, GL_STOCK_BOX,           stock_box_24,     stock_box_16);
	add_icons (factory, GL_STOCK_ELLIPSE,       stock_ellipse_24, stock_ellipse_16);
	add_icons (factory, GL_STOCK_IMAGE,         stock_image_24,   stock_image_16);
	add_icons (factory, GL_STOCK_BARCODE,       stock_barcode_24, stock_barcode_16);
	add_icons (factory, GL_STOCK_MERGE,         stock_merge_24,   stock_merge_16);

	add_icons (factory, GL_STOCK_PROPERTIES,
		   stock_properties_24,
		   stock_properties_16);

	add_icons (factory, GL_STOCK_ORDER_TOP,     NULL, stock_order_top_16);
	add_icons (factory, GL_STOCK_ORDER_BOTTOM,  NULL, stock_order_bottom_16);
	add_icons (factory, GL_STOCK_ROTATE_LEFT,   NULL, stock_rotate_left_16);
	add_icons (factory, GL_STOCK_ROTATE_RIGHT,  NULL, stock_rotate_right_16);
	add_icons (factory, GL_STOCK_FLIP_HORIZ,    NULL, stock_flip_horiz_16);
	add_icons (factory, GL_STOCK_FLIP_VERT,     NULL, stock_flip_vert_16);
	add_icons (factory, GL_STOCK_ALIGN_LEFT,    NULL, stock_align_left_16);
	add_icons (factory, GL_STOCK_ALIGN_RIGHT,   NULL, stock_align_right_16);
	add_icons (factory, GL_STOCK_ALIGN_HCENTER, NULL, stock_align_hcenter_16);
	add_icons (factory, GL_STOCK_ALIGN_TOP,     NULL, stock_align_top_16);
	add_icons (factory, GL_STOCK_ALIGN_BOTTOM,  NULL, stock_align_bottom_16);
	add_icons (factory, GL_STOCK_ALIGN_VCENTER, NULL, stock_align_vcenter_16);
	add_icons (factory, GL_STOCK_CENTER_HORIZ,  NULL, stock_center_horiz_16);
	add_icons (factory, GL_STOCK_CENTER_VERT,   NULL, stock_center_vert_16);

	add_icons (factory, GL_STOCK_BUCKET_FILL,   stock_bucket_fill_24, stock_bucket_fill_16);
	add_icons (factory, GL_STOCK_PENCIL,        stock_pencil_24,      stock_pencil_16);

	add_button_icon (factory, GL_STOCK_HCHAIN,        stock_hchain_24);
	add_button_icon (factory, GL_STOCK_HCHAIN_BROKEN, stock_hchain_broken_24);
	add_button_icon (factory, GL_STOCK_VCHAIN,        stock_vchain_24);
	add_button_icon (factory, GL_STOCK_VCHAIN_BROKEN, stock_vchain_broken_24);

	g_object_unref (G_OBJECT(factory));
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Unpack and associate given icons with stock_id.                */
/*--------------------------------------------------------------------------*/
static void
add_icons (GtkIconFactory *factory,
           const gchar    *stock_id,
           const guchar   *inline_24,
           const guchar   *inline_16)
{
	GdkPixbuf      *pixbuf;
	GtkIconSet     *icon_set;
	GtkIconSource  *icon_source;

	if ( inline_24 != NULL ) {
		/* Create icon set with default image */
		pixbuf = gdk_pixbuf_new_from_inline (-1, inline_24, FALSE, NULL);
		icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);
		g_object_unref (G_OBJECT(pixbuf));
	} else {
		/* Create an empty icon set */
		icon_set = gtk_icon_set_new ();
	}

	if ( inline_16 != NULL ) {
		/* Add 16x16 icon for menus to icon set */
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

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Unpack and associate given button icon with stock_id.          */
/*--------------------------------------------------------------------------*/
static void
add_button_icon (GtkIconFactory *factory,
                 const gchar    *stock_id,
                 const guchar   *inline_24)
{
	GtkIconSet     *icon_set;
	GdkPixbuf      *pixbuf;
	GtkIconSource  *icon_source;

	icon_set = gtk_icon_set_new ();

	icon_source = gtk_icon_source_new ();
	pixbuf = gdk_pixbuf_new_from_inline (-1, inline_24, FALSE, NULL);
	gtk_icon_source_set_pixbuf (icon_source, pixbuf);
	g_object_unref (G_OBJECT(pixbuf));
	gtk_icon_source_set_size_wildcarded (icon_source, FALSE);
	gtk_icon_source_set_size (icon_source, GTK_ICON_SIZE_BUTTON);
	gtk_icon_set_add_source (icon_set, icon_source);
	g_free (icon_source);

	/* Now associate icon set with stock id */
	gtk_icon_factory_add (factory, stock_id, icon_set);
	gtk_icon_set_unref (icon_set);
}







