/*
 *  view.h
 *  Copyright (C) 2001-2009  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of gLabels.
 *
 *  gLabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gLabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gLabels.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VIEW_H__
#define __VIEW_H__

#include <gtk/gtk.h>

#include "label-object.h"

typedef enum {
	GL_VIEW_MODE_ARROW,
	GL_VIEW_MODE_OBJECT_CREATE
} glViewMode;

typedef enum {
        GL_VIEW_IDLE = 0,
        GL_VIEW_ARROW_SELECT_REGION,
        GL_VIEW_ARROW_MOVE,
        GL_VIEW_ARROW_RESIZE,
        GL_VIEW_CREATE_DRAG,
} glViewState;

#define GL_TYPE_VIEW            (gl_view_get_type ())
#define GL_VIEW(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj), GL_TYPE_VIEW, glView ))
#define GL_VIEW_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_VIEW, glViewClass))
#define GL_IS_VIEW(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_VIEW))
#define GL_IS_VIEW_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_VIEW))

typedef struct _glView      glView;
typedef struct _glViewClass glViewClass;



struct _glView {
	GtkVBox            parent_widget;

	glLabel           *label;

	GtkWidget         *canvas;
	gdouble            zoom;
	gboolean           zoom_to_fit_flag;
	gdouble            home_scale;
        gdouble            x0, y0;
        gdouble            w, h;

        gboolean           update_scheduled_flag;
        gboolean           grid_visible;
	gdouble            grid_spacing;

        gboolean           markup_visible;

	glViewMode         mode;
	glLabelObjectType  create_type;
        glViewState        state;
        gboolean           grabbed_flag;

        /* GL_VIEW_ARROW_MOVE state */
        gdouble            move_last_x;
        gdouble            move_last_y;

        /* GL_VIEW_ARROW_SELECT_REGION state */
        gboolean           select_region_visible;
        glLabelRegion      select_region;

        /* GL_VIEW_ARROW_RESIZE state */
        glLabelObject      *resize_object;
        glLabelObjectHandle resize_handle;
        gboolean            resize_honor_aspect;

        /* GL_VIEW_CREATE_DRAG state */
        glLabelObject     *create_object;
        gdouble            create_x0;
        gdouble            create_y0;

};

struct _glViewClass {
	GtkVBoxClass      parent_class;

	/* Signal to launch a context menu */
	void (*context_menu_activate) (glView   *view,
				       gint      button,
				       guint32   activate_time,
				       gpointer  user_data);

	/* Signals to support a status bar */
	void (*zoom_changed)      (glView   *view,
				   gdouble   zoom,
				   gpointer  user_data);
	void (*pointer_moved)     (glView   *view,
				   gdouble   x,
				   gdouble   y,
				   gpointer  user_data);
	void (*pointer_exit)      (glView   *view,
				   gpointer  user_data);
	void (*mode_changed)      (glView   *view,
				   gpointer  user_data);
};

G_BEGIN_DECLS

GType      gl_view_get_type                (void) G_GNUC_CONST;

GtkWidget *gl_view_new                     (glLabel           *label);


void       gl_view_update                  (glView            *view);

void       gl_view_update_region           (glView            *view,
                                            cairo_t           *cr,
                                            glLabelRegion     *region);

void       gl_view_show_grid               (glView            *view);

void       gl_view_hide_grid               (glView            *view);

void       gl_view_set_grid_spacing        (glView            *view,
					    gdouble            spacing);

void       gl_view_show_markup             (glView            *view);

void       gl_view_hide_markup             (glView            *view);

void       gl_view_arrow_mode              (glView            *view);

void       gl_view_object_create_mode      (glView            *view,
					    glLabelObjectType  type);


void       gl_view_zoom_in                 (glView            *view);

void       gl_view_zoom_out                (glView            *view);

void       gl_view_zoom_to_fit             (glView            *view);

void       gl_view_set_zoom                (glView            *view,
					    gdouble            zoom);

gdouble    gl_view_get_zoom                (glView            *view);

gboolean   gl_view_is_zoom_max             (glView            *view);

gboolean   gl_view_is_zoom_min             (glView            *view);




G_END_DECLS

#endif



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
