/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  tools.h:  toolbar module header file
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
#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <bonobo/bonobo-ui-component.h>

G_BEGIN_DECLS

void gl_tools_arrow                    (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

void gl_tools_text                     (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

void gl_tools_box                      (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

void gl_tools_line                     (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

void gl_tools_ellipse                  (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

void gl_tools_image                    (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

void gl_tools_barcode                  (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

void gl_tools_zoomin                   (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

void gl_tools_zoomout                  (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

void gl_tools_zoom1to1                 (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

void gl_tools_merge_properties         (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

void gl_tools_object_properties        (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

void gl_tools_raise_objects            (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

void gl_tools_lower_objects            (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

void gl_tools_rotate_objects_left      (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

void gl_tools_rotate_objects_right     (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

void gl_tools_flip_objects_horiz       (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

void gl_tools_flip_objects_vert        (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

void gl_tools_align_objects_left       (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

void gl_tools_align_objects_right      (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

void gl_tools_align_objects_hcenter    (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

void gl_tools_align_objects_top        (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

void gl_tools_align_objects_bottom     (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

void gl_tools_align_objects_vcenter    (BonoboUIComponent *uic,
					gpointer           user_data,
					const gchar       *verbname);

G_END_DECLS

#endif
