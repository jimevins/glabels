/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  tools.c:  toolbar module
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

#include "tools.h"
#include "mdi.h"
#include "display.h"
#include "merge_properties.h"

/*===========================================*/
/* Private types.                            */
/*===========================================*/

/*===========================================*/
/* Private globals.                          */
/*===========================================*/

/*===========================================*/
/* Private function prototypes.              */
/*===========================================*/



/*****************************************************************************/
/* arrow callback.                                                           */
/*****************************************************************************/
void
gl_tools_arrow_cb (GtkWidget * widget,
	  gpointer data)
{
	GnomeMDIChild *child = gnome_mdi_get_active_child (GNOME_MDI (data));
	glDisplay *display;

	display = gl_mdi_get_display (child);
	if (display != NULL) {
		gl_display_arrow_mode (display);
	}
}

/*****************************************************************************/
/* text callback.                                                            */
/*****************************************************************************/
void
gl_tools_text_cb (GtkWidget * widget,
	 gpointer data)
{
	GnomeMDIChild *child = gnome_mdi_get_active_child (GNOME_MDI (data));
	glDisplay *display;

	display = gl_mdi_get_display (child);
	if (display != NULL) {
		gl_display_object_create_mode (display, GL_LABEL_OBJECT_TEXT);
	}
}

/*****************************************************************************/
/* box callback.                                                             */
/*****************************************************************************/
void
gl_tools_box_cb (GtkWidget * widget,
	gpointer data)
{
	GnomeMDIChild *child = gnome_mdi_get_active_child (GNOME_MDI (data));
	glDisplay *display;

	display = gl_mdi_get_display (child);
	if (display != NULL) {
		gl_display_object_create_mode (display, GL_LABEL_OBJECT_BOX);
	}
}

/*****************************************************************************/
/* line callback.                                                            */
/*****************************************************************************/
void
gl_tools_line_cb (GtkWidget * widget,
	 gpointer data)
{
	GnomeMDIChild *child = gnome_mdi_get_active_child (GNOME_MDI (data));
	glDisplay *display;

	display = gl_mdi_get_display (child);
	if (display != NULL) {
		gl_display_object_create_mode (display, GL_LABEL_OBJECT_LINE);
	}
}

/*****************************************************************************/
/* ellipse callback.                                                         */
/*****************************************************************************/
void
gl_tools_ellipse_cb (GtkWidget * widget,
	    gpointer data)
{
	GnomeMDIChild *child = gnome_mdi_get_active_child (GNOME_MDI (data));
	glDisplay *display;

	display = gl_mdi_get_display (child);
	if (display != NULL) {
		gl_display_object_create_mode (display,
					       GL_LABEL_OBJECT_ELLIPSE);
	}
}

/*****************************************************************************/
/* image callback.                                                           */
/*****************************************************************************/
void
gl_tools_image_cb (GtkWidget * widget,
	  gpointer data)
{
	GnomeMDIChild *child = gnome_mdi_get_active_child (GNOME_MDI (data));
	glDisplay *display;

	display = gl_mdi_get_display (child);
	if (display != NULL) {
		gl_display_object_create_mode (display, GL_LABEL_OBJECT_IMAGE);
	}
}

/*****************************************************************************/
/* barcode callback.                                                         */
/*****************************************************************************/
void
gl_tools_barcode_cb (GtkWidget * widget,
	    gpointer data)
{
	GnomeMDIChild *child = gnome_mdi_get_active_child (GNOME_MDI (data));
	glDisplay *display;

	display = gl_mdi_get_display (child);
	if (display != NULL) {
		gl_display_object_create_mode (display,
					       GL_LABEL_OBJECT_BARCODE);
	}
}

/*****************************************************************************/
/* zoom in callback.                                                         */
/*****************************************************************************/
void
gl_tools_zoomin_cb (GtkWidget * widget,
	   gpointer data)
{
	GnomeMDIChild *child = gnome_mdi_get_active_child (GNOME_MDI (data));
	glDisplay *display;

	display = gl_mdi_get_display (child);
	if (display != NULL) {
		gl_display_zoom_in (display);
	}
}

/*****************************************************************************/
/* zoom out callback.                                                        */
/*****************************************************************************/
void
gl_tools_zoomout_cb (GtkWidget * widget,
	    gpointer data)
{
	GnomeMDIChild *child = gnome_mdi_get_active_child (GNOME_MDI (data));
	glDisplay *display;

	display = gl_mdi_get_display (child);
	if (display != NULL) {
		gl_display_zoom_out (display);
	}
}

/*****************************************************************************/
/* zoom 1:1 callback.                                                        */
/*****************************************************************************/
void
gl_tools_zoom1to1_cb (GtkWidget * widget,
	     gpointer data)
{
	GnomeMDIChild *child = gnome_mdi_get_active_child (GNOME_MDI (data));
	glDisplay *display;

	display = gl_mdi_get_display (child);
	if (display != NULL) {
		gl_display_set_zoom (display, 1.0);
	}
}

/*****************************************************************************/
/* edit merge properties callback.                                           */
/*****************************************************************************/
void
gl_tools_merge_properties_cb (GtkWidget * widget,
		     gpointer data)
{
	GnomeMDIChild *child = gnome_mdi_get_active_child (GNOME_MDI (data));
	glDisplay *display;

	display = gl_mdi_get_display (child);
	if (display != NULL) {
		gl_merge_properties_dialog (display, child->name);
	}
}

