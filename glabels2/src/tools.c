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
#include "glabels.h"
#include "merge-properties.h"

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
gl_tools_arrow (BonoboUIComponent *uic,
		gpointer user_data,
		const gchar* verbname)
{
	glView *view;

	view = glabels_get_active_view ();
	if (view != NULL) {
		gl_view_arrow_mode (view);
	}
}

/*****************************************************************************/
/* text callback.                                                            */
/*****************************************************************************/
void
gl_tools_text (BonoboUIComponent *uic,
	       gpointer user_data,
	       const gchar* verbname)

{
	glView *view;

	view = glabels_get_active_view ();
	if (view != NULL) {
		gl_view_object_create_mode (view, GL_LABEL_OBJECT_TEXT);
	}
}

/*****************************************************************************/
/* box callback.                                                             */
/*****************************************************************************/
void
gl_tools_box (BonoboUIComponent *uic,
	      gpointer user_data,
	      const gchar* verbname)

{
	glView *view;

	view = glabels_get_active_view ();
	if (view != NULL) {
		gl_view_object_create_mode (view, GL_LABEL_OBJECT_BOX);
	}
}

/*****************************************************************************/
/* line callback.                                                            */
/*****************************************************************************/
void
gl_tools_line (BonoboUIComponent *uic,
	       gpointer user_data,
	       const gchar* verbname)

{
	glView *view;

	view = glabels_get_active_view ();
	if (view != NULL) {
		gl_view_object_create_mode (view, GL_LABEL_OBJECT_LINE);
	}
}

/*****************************************************************************/
/* ellipse callback.                                                         */
/*****************************************************************************/
void
gl_tools_ellipse (BonoboUIComponent *uic,
		  gpointer user_data,
		  const gchar* verbname)

{
	glView *view;

	view = glabels_get_active_view ();
	if (view != NULL) {
		gl_view_object_create_mode (view,
					       GL_LABEL_OBJECT_ELLIPSE);
	}
}

/*****************************************************************************/
/* image callback.                                                           */
/*****************************************************************************/
void
gl_tools_image (BonoboUIComponent *uic,
		gpointer user_data,
		const gchar* verbname)

{
	glView *view;

	view = glabels_get_active_view ();
	if (view != NULL) {
		gl_view_object_create_mode (view, GL_LABEL_OBJECT_IMAGE);
	}
}

/*****************************************************************************/
/* barcode callback.                                                         */
/*****************************************************************************/
void
gl_tools_barcode (BonoboUIComponent *uic,
		  gpointer user_data,
		  const gchar* verbname)

{
	glView *view;

	view = glabels_get_active_view ();
	if (view != NULL) {
		gl_view_object_create_mode (view,
					       GL_LABEL_OBJECT_BARCODE);
	}
}

/*****************************************************************************/
/* zoom in callback.                                                         */
/*****************************************************************************/
void
gl_tools_zoomin (BonoboUIComponent *uic,
		 gpointer user_data,
		 const gchar* verbname)

{
	glView *view;

	view = glabels_get_active_view ();
	if (view != NULL) {
		gl_view_zoom_in (view);
	}
}

/*****************************************************************************/
/* zoom out callback.                                                        */
/*****************************************************************************/
void
gl_tools_zoomout (BonoboUIComponent *uic,
		  gpointer user_data,
		  const gchar* verbname)

{
	glView *view;

	view = glabels_get_active_view ();
	if (view != NULL) {
		gl_view_zoom_out (view);
	}
}

/*****************************************************************************/
/* zoom 1:1 callback.                                                        */
/*****************************************************************************/
void
gl_tools_zoom1to1 (BonoboUIComponent *uic,
		   gpointer user_data,
		   const gchar* verbname)

{
	glView *view;

	view = glabels_get_active_view ();
	if (view != NULL) {
		gl_view_set_zoom (view, 1.0);
	}
}

/*****************************************************************************/
/* edit merge properties callback.                                           */
/*****************************************************************************/
void
gl_tools_merge_properties (BonoboUIComponent *uic,
			   gpointer user_data,
			   const gchar* verbname)

{
	glView *view;

	view = glabels_get_active_view ();
	if (view != NULL) {
		gl_merge_properties_dialog (view);
	}
}

/*****************************************************************************/
/* edit object properties callback.                                          */
/*****************************************************************************/
void
gl_tools_object_properties (BonoboUIComponent *uic,
			    gpointer user_data,
			    const gchar* verbname)

{
	glView *view;

	view = glabels_get_active_view ();
	if (view != NULL) {
		gl_view_edit_object_props (view);
	}
}

/*****************************************************************************/
/* Raise objects to top callback.                                            */
/*****************************************************************************/
void
gl_tools_raise_objects (BonoboUIComponent *uic,
			gpointer user_data,
			const gchar* verbname)

{
	glView *view;

	view = glabels_get_active_view ();
	if (view != NULL) {
		gl_view_raise_selection (view);
	}
}

/*****************************************************************************/
/* Lower objects to bottom callback.                                         */
/*****************************************************************************/
void
gl_tools_lower_objects (BonoboUIComponent *uic,
			gpointer user_data,
			const gchar* verbname)

{
	glView *view;

	view = glabels_get_active_view ();
	if (view != NULL) {
		gl_view_lower_selection (view);
	}
}

