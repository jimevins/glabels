/*
 *  prefs-model.h
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

#ifndef __PREFS_MODEL_H__
#define __PREFS_MODEL_H__

#include <glib-object.h>
#include <gconf/gconf-client.h>
#include <pango/pango.h>
#include <libglabels/libglabels.h>

G_BEGIN_DECLS

typedef enum {
	GL_TOOLBAR_SYSTEM = 0,
	GL_TOOLBAR_ICONS,
	GL_TOOLBAR_ICONS_AND_TEXT
} glToolbarSetting;

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
	lglUnitsType      units;

	/* Page size */
	gchar            *default_page_size;

	/* Text properties */
	gchar            *default_font_family;
	gdouble           default_font_size;
	PangoWeight       default_font_weight;
	gboolean          default_font_italic_flag;
	guint             default_text_color;
	PangoAlignment    default_text_alignment;
	gdouble           default_text_line_spacing;
	
	/* Line properties */
	gdouble           default_line_width;
	guint             default_line_color;
	
	/* Fill properties */
	guint             default_fill_color;

	/* User Interface/Main Toolbar */
	gboolean	  main_toolbar_visible;
	glToolbarSetting  main_toolbar_buttons_style; 

	/* User Interface/Drawing Toolbar */
	gboolean          drawing_toolbar_visible;

	/* User Interface/Property Toolbar */
	gboolean          property_toolbar_visible;

	/* View properties */
	gboolean          grid_visible;
	gboolean          markup_visible;

	/* Recent files */
	gint              max_recents;

	/* Recent templates */
        GSList           *recent_templates;
	gint              max_recent_templates;
};

struct _glPrefsModelClass {
	GObjectClass     parent_class;

        void (*changed)          (glPrefsModel *prefs_model, gpointer user_data);
};


GType         gl_prefs_model_get_type            (void) G_GNUC_CONST;

glPrefsModel *gl_prefs_model_new                 (void);

void          gl_prefs_model_save_settings       (glPrefsModel   *prefs_model);

void          gl_prefs_model_load_settings       (glPrefsModel   *prefs_model);

G_END_DECLS

#endif /* __PREFS_MODEL_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
