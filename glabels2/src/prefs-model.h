/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prefs-model.h:  Application preferences model module header file
 *
 *  Copyright (C) 2001-2003  Jim Evins <evins@snaught.com>.
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
#ifndef __PREFS_MODEL_H__
#define __PREFS_MODEL_H__

#include <glib-object.h>
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>
#include <libgnomeprint/gnome-font.h>

G_BEGIN_DECLS

typedef enum {
	GL_TOOLBAR_SYSTEM = 0,
	GL_TOOLBAR_ICONS,
	GL_TOOLBAR_ICONS_AND_TEXT
} glToolbarSetting;

typedef enum {
	GL_UNITS_PTS,
	GL_UNITS_INCHES,
	GL_UNITS_MM,
} glPrefsUnits;

#define GL_TYPE_PREFS_MODEL              (gl_prefs_model_get_type ())
#define GL_PREFS_MODEL(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_PREFS_MODEL, glPrefsModel))
#define GL_PREFS_MODEL_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_PREFS_MODEL, glPrefsModelClass))
#define GL_IS_PREFS_MODEL(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_PREFS_MODEL))
#define GL_IS_PREFS_MODEL_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_PREFS_MODEL))
#define GL_PREFS_MODEL_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), GL_TYPE_PREFS_MODEL, glPrefsModelClass))


typedef struct _glPrefsModel          glPrefsModel;
typedef struct _glPrefsModelClass     glPrefsModelClass;

typedef struct _glPrefsModelPrivate   glPrefsModelPrivate;


struct _glPrefsModel {
	GObject          *object;

	GConfClient      *gconf_client;

	/* Units */
	glPrefsUnits      units;

	/* Page size */
	gchar            *default_page_size;

	/* Text properties */
	gchar            *default_font_family;
	gdouble           default_font_size;
	GnomeFontWeight   default_font_weight;
	gboolean          default_font_italic_flag;
	guint             default_text_color;
	GtkJustification  default_text_alignment;
	
	/* Line properties */
	gdouble           default_line_width;
	guint             default_line_color;
	
	/* Fill properties */
	guint             default_fill_color;

	/* User Interface/Main Toolbar */
	gboolean	  main_toolbar_visible;
	glToolbarSetting  main_toolbar_buttons_style; 
	gboolean	  main_toolbar_view_tooltips;

	/* User Interface/Drawing Toolbar */
	gboolean          drawing_toolbar_visible;
	glToolbarSetting  drawing_toolbar_buttons_style; 
	gboolean	  drawing_toolbar_view_tooltips;

	/* User Interface/Property Toolbar */
	gboolean          property_toolbar_visible;
	gboolean	  property_toolbar_view_tooltips;

	/* View properties */
	gboolean          grid_visible;
	gboolean          markup_visible;

	/* Recent files */
	gint              max_recents;
};

struct _glPrefsModelClass {
	GObjectClass     parent_class;

        void (*changed)          (glPrefsModel *prefs_model, gpointer user_data);
};


GType         gl_prefs_model_get_type            (void);

glPrefsModel *gl_prefs_model_new                 (void);

void          gl_prefs_model_save_settings       (glPrefsModel   *prefs_model);

void          gl_prefs_model_load_settings       (glPrefsModel   *prefs_model);

G_END_DECLS

#endif /* __PREFS_MODEL_H__ */

