/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  tools.c:  implementation of drawing toolbar verbs
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
#include "window.h"

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
		gpointer           user_data,
		const gchar       *verbname)
{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_arrow_mode (GL_VIEW(window->view));
	}
}

/*****************************************************************************/
/* text callback.                                                            */
/*****************************************************************************/
void
gl_tools_text (BonoboUIComponent *uic,
	       gpointer           user_data,
	       const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_object_create_mode (GL_VIEW(window->view),
					    GL_LABEL_OBJECT_TEXT);
	}
}

/*****************************************************************************/
/* box callback.                                                             */
/*****************************************************************************/
void
gl_tools_box (BonoboUIComponent *uic,
	      gpointer           user_data,
	      const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_object_create_mode (GL_VIEW(window->view),
					    GL_LABEL_OBJECT_BOX);
	}
}

/*****************************************************************************/
/* line callback.                                                            */
/*****************************************************************************/
void
gl_tools_line (BonoboUIComponent *uic,
	       gpointer           user_data,
	       const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_object_create_mode (GL_VIEW(window->view),
					    GL_LABEL_OBJECT_LINE);
	}
}

/*****************************************************************************/
/* ellipse callback.                                                         */
/*****************************************************************************/
void
gl_tools_ellipse (BonoboUIComponent *uic,
		  gpointer           user_data,
		  const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_object_create_mode (GL_VIEW(window->view),
					    GL_LABEL_OBJECT_ELLIPSE);
	}
}

/*****************************************************************************/
/* image callback.                                                           */
/*****************************************************************************/
void
gl_tools_image (BonoboUIComponent *uic,
		gpointer           user_data,
		const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_object_create_mode (GL_VIEW(window->view),
					    GL_LABEL_OBJECT_IMAGE);
	}
}

/*****************************************************************************/
/* barcode callback.                                                         */
/*****************************************************************************/
void
gl_tools_barcode (BonoboUIComponent *uic,
		  gpointer           user_data,
		  const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_object_create_mode (GL_VIEW(window->view),
					    GL_LABEL_OBJECT_BARCODE);
	}
}

/*****************************************************************************/
/* zoom in callback.                                                         */
/*****************************************************************************/
void
gl_tools_zoomin (BonoboUIComponent *uic,
		 gpointer           user_data,
		 const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_zoom_in (GL_VIEW(window->view));
	}
}

/*****************************************************************************/
/* zoom out callback.                                                        */
/*****************************************************************************/
void
gl_tools_zoomout (BonoboUIComponent *uic,
		  gpointer           user_data,
		  const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_zoom_out (GL_VIEW(window->view));
	}
}

/*****************************************************************************/
/* zoom 1:1 callback.                                                        */
/*****************************************************************************/
void
gl_tools_zoom1to1 (BonoboUIComponent *uic,
		   gpointer           user_data,
		   const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_set_zoom (GL_VIEW(window->view), 1.0);
	}
}

/*****************************************************************************/
/* zoom to fit callback.                                                     */
/*****************************************************************************/
void
gl_tools_zoom_to_fit (BonoboUIComponent *uic,
		      gpointer           user_data,
		      const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_zoom_best_fit (GL_VIEW(window->view));
	}
}

/*****************************************************************************/
/* edit merge properties callback.                                           */
/*****************************************************************************/
void
gl_tools_merge_properties (BonoboUIComponent *uic,
			   gpointer           user_data,
			   const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_edit_merge_props (GL_VIEW(window->view));
	}
}

/*****************************************************************************/
/* Raise objects to top callback.                                            */
/*****************************************************************************/
void
gl_tools_raise_objects (BonoboUIComponent *uic,
			gpointer           user_data,
			const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_raise_selection (GL_VIEW(window->view));
	}
}

/*****************************************************************************/
/* Lower objects to bottom callback.                                         */
/*****************************************************************************/
void
gl_tools_lower_objects (BonoboUIComponent *uic,
			gpointer           user_data,
			const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_lower_selection (GL_VIEW(window->view));
	}
}

/*****************************************************************************/
/* Rotate objects left 90 degrees.                                           */
/*****************************************************************************/
void
gl_tools_rotate_objects_left (BonoboUIComponent *uic,
			      gpointer           user_data,
			      const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_rotate_selection_left (GL_VIEW(window->view));
	}
}

/*****************************************************************************/
/* Rotate objects right 90 degrees.                                          */
/*****************************************************************************/
void
gl_tools_rotate_objects_right (BonoboUIComponent *uic,
			       gpointer           user_data,
			       const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_rotate_selection_right (GL_VIEW(window->view));
	}
}

/*****************************************************************************/
/* Flip objects horizontally callback.                                       */
/*****************************************************************************/
void
gl_tools_flip_objects_horiz (BonoboUIComponent *uic,
			     gpointer           user_data,
			     const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_flip_selection_horiz (GL_VIEW(window->view));
	}
}

/*****************************************************************************/
/* Flip objects vertically callback.                                         */
/*****************************************************************************/
void
gl_tools_flip_objects_vert (BonoboUIComponent *uic,
			    gpointer           user_data,
			    const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_flip_selection_vert (GL_VIEW(window->view));
	}
}

/*****************************************************************************/
/* Align objects left callback.                                              */
/*****************************************************************************/
void
gl_tools_align_objects_left (BonoboUIComponent *uic,
			     gpointer           user_data,
			     const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_align_selection_left (GL_VIEW(window->view));
	}
}

/*****************************************************************************/
/* Align objects right callback.                                             */
/*****************************************************************************/
void
gl_tools_align_objects_right (BonoboUIComponent *uic,
			      gpointer           user_data,
			      const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_align_selection_right (GL_VIEW(window->view));
	}
}

/*****************************************************************************/
/* Align objects horizontal center callback.                                 */
/*****************************************************************************/
void
gl_tools_align_objects_hcenter (BonoboUIComponent *uic,
				gpointer           user_data,
				const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_align_selection_hcenter (GL_VIEW(window->view));
	}
}

/*****************************************************************************/
/* Align objects top callback.                                               */
/*****************************************************************************/
void
gl_tools_align_objects_top (BonoboUIComponent *uic,
			    gpointer           user_data,
			    const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_align_selection_top (GL_VIEW(window->view));
	}
}

/*****************************************************************************/
/* Align objects bottom callback.                                            */
/*****************************************************************************/
void
gl_tools_align_objects_bottom (BonoboUIComponent *uic,
			       gpointer           user_data,
			       const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_align_selection_bottom (GL_VIEW(window->view));
	}
}

/*****************************************************************************/
/* Align objects vertical center callback.                                   */
/*****************************************************************************/
void
gl_tools_align_objects_vcenter (BonoboUIComponent *uic,
				gpointer           user_data,
				const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_align_selection_vcenter (GL_VIEW(window->view));
	}
}

/*****************************************************************************/
/* Center objects horizontally callback.                                     */
/*****************************************************************************/
void
gl_tools_center_objects_horiz (BonoboUIComponent *uic,
			       gpointer           user_data,
			       const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_center_selection_horiz (GL_VIEW(window->view));
	}
}

/*****************************************************************************/
/* Center objects vertically callback.                                       */
/*****************************************************************************/
void
gl_tools_center_objects_vert (BonoboUIComponent *uic,
			      gpointer           user_data,
			      const gchar       *verbname)

{
	glWindow *window;

	window = GL_WINDOW (user_data);

	if (window->view != NULL) {
		gl_view_center_selection_vert (GL_VIEW(window->view));
	}
}

