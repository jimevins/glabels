/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prefs-model.c:  Application preferences model module
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
#include <config.h>


#include "marshal.h"
#include "prefs-model.h"
#include "util.h"
#include <libglabels/paper.h>
#include "color.h"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

/* GConf keys */
#define BASE_KEY                            "/apps/glabels"

#define PREF_UNITS                          "/units"
#define PREF_DEFAULT_PAGE_SIZE              "/default-page-size"

#define PREF_DEFAULT_FONT_FAMILY            "/default-font-family"
#define PREF_DEFAULT_FONT_SIZE              "/default-font-size"
#define PREF_DEFAULT_FONT_WEIGHT            "/default-font-weight"
#define PREF_DEFAULT_FONT_ITALIC_FLAG       "/default-font-italic-flag"
#define PREF_DEFAULT_TEXT_COLOR             "/default-text-color"
#define PREF_DEFAULT_TEXT_ALIGNMENT         "/default-text-alignment"
#define PREF_DEFAULT_TEXT_LINE_SPACING      "/default-text-line-spacing"

#define PREF_DEFAULT_LINE_WIDTH             "/default-line-width"
#define PREF_DEFAULT_LINE_COLOR             "/default-line-color"

#define PREF_DEFAULT_FILL_COLOR             "/default-fill-color"

#define PREF_MAIN_TOOLBAR_VISIBLE           "/main-toolbar-visible"
#define PREF_MAIN_TOOLBAR_BUTTONS_STYLE     "/main-toolbar-buttons-style"
#define PREF_MAIN_TOOLBAR_VIEW_TOOLTIPS     "/main-toolbar-view-tooltips"

#define PREF_DRAWING_TOOLBAR_VISIBLE        "/drawing-toolbar-visible"
#define PREF_DRAWING_TOOLBAR_BUTTONS_STYLE  "/drawing-toolbar-buttons-style"
#define PREF_DRAWING_TOOLBAR_VIEW_TOOLTIPS  "/drawing-toolbar-view-tooltips"

#define PREF_PROPERTY_TOOLBAR_VISIBLE        "/property-toolbar-visible"
#define PREF_PROPERTY_TOOLBAR_VIEW_TOOLTIPS  "/property-toolbar-view-tooltips"

#define PREF_GRID_VISIBLE                   "/grid-visible"
#define PREF_MARKUP_VISIBLE                 "/markup-visible"

#define PREF_MAX_RECENTS                    "/max-recents"

/* Default values */
#define DEFAULT_UNITS_STRING       units_to_string (GL_UNITS_INCHES)
#define DEFAULT_PAGE_SIZE          "US-Letter"

#define DEFAULT_FONT_FAMILY        "Sans"
#define DEFAULT_FONT_SIZE          14.0
#define DEFAULT_FONT_WEIGHT_STRING gl_util_weight_to_string (GNOME_FONT_BOOK)
#define DEFAULT_FONT_ITALIC_FLAG   FALSE
#define DEFAULT_TEXT_JUST_STRING   gl_util_just_to_string (GTK_JUSTIFY_LEFT)
#define DEFAULT_TEXT_COLOR         GNOME_CANVAS_COLOR (0,0,0)
#define DEFAULT_TEXT_LINE_SPACING  1.0

#define DEFAULT_LINE_WIDTH         1.0
#define DEFAULT_LINE_COLOR         GNOME_CANVAS_COLOR_A (0, 0, 0, 255)

#define DEFAULT_FILL_COLOR         GNOME_CANVAS_COLOR_A (0, 255, 0, 255)

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

enum {
	CHANGED,
	LAST_SIGNAL
};



/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static GObjectClass *parent_class = NULL;

static guint signals[LAST_SIGNAL] = {0};

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void           gl_prefs_model_class_init    (glPrefsModelClass   *klass);
static void           gl_prefs_model_instance_init (glPrefsModel        *object);
static void           gl_prefs_model_finalize      (GObject             *object);

static void           notify_cb                    (GConfClient         *client,
						    guint                cnxn_id,
						    GConfEntry          *entry,
						    glPrefsModel        *prefs_model);

static gchar         *get_string                   (GConfClient         *client,
						    const gchar         *key,
						    const gchar         *def);

static gboolean       get_bool                     (GConfClient         *client,
						    const gchar         *key,
						    gboolean             def);

static gint           get_int                      (GConfClient         *client,
						    const gchar         *key,
						    gint                 def);

static gdouble        get_float                    (GConfClient         *client,
						    const gchar         *key,
						    gdouble              def);

static glPrefsUnits   string_to_units              (const gchar         *string);
static const gchar   *units_to_string              (glPrefsUnits         units);



/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_prefs_model_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (glPrefsModelClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_prefs_model_class_init,
			NULL,
			NULL,
			sizeof (glPrefsModel),
			0,
			(GInstanceInitFunc) gl_prefs_model_instance_init,
			NULL
		};

		type = g_type_register_static (G_TYPE_OBJECT,
					       "glPrefsModel", &info, 0);
	}

	return type;
}

static void
gl_prefs_model_class_init (glPrefsModelClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	gl_debug (DEBUG_PREFS, "START");

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gl_prefs_model_finalize;

	signals[CHANGED] =
		g_signal_new ("changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (glPrefsModelClass, changed),
			      NULL, NULL,
			      gl_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

	gl_debug (DEBUG_PREFS, "END");
}

static void
gl_prefs_model_instance_init (glPrefsModel *prefs_model)
{
	gl_debug (DEBUG_PREFS, "START");

        prefs_model->gconf_client = gconf_client_get_default ();

        g_return_if_fail (prefs_model->gconf_client != NULL);
 
        gconf_client_add_dir (prefs_model->gconf_client,
                              BASE_KEY,
                              GCONF_CLIENT_PRELOAD_ONELEVEL,
                              NULL);
         
        gconf_client_notify_add (prefs_model->gconf_client,
                                 BASE_KEY,
                                 (GConfClientNotifyFunc)notify_cb, prefs_model,
                                 NULL, NULL);

	gl_debug (DEBUG_PREFS, "END");
}

static void
gl_prefs_model_finalize (GObject *object)
{
	gl_debug (DEBUG_PREFS, "START");

	g_return_if_fail (object && GL_IS_PREFS_MODEL (object));

	g_object_unref (G_OBJECT(GL_PREFS_MODEL(object)->gconf_client));

	g_free (GL_PREFS_MODEL(object)->default_page_size);
	g_free (GL_PREFS_MODEL(object)->default_font_family);

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gl_debug (DEBUG_PREFS, "END");
}

/*****************************************************************************/
/* New prefs_model object.                                                   */
/*****************************************************************************/
glPrefsModel *
gl_prefs_model_new (void)
{
	glPrefsModel *prefs_model;

	gl_debug (DEBUG_PREFS, "START");

	prefs_model = GL_PREFS_MODEL (g_object_new (gl_prefs_model_get_type(), NULL));

	gl_debug (DEBUG_PREFS, "END");

	return prefs_model;
}



/*****************************************************************************/
/* Save all settings.                                                        */
/*****************************************************************************/
void 
gl_prefs_model_save_settings (glPrefsModel *prefs_model)
{
	gl_debug (DEBUG_PREFS, "START");
	
	g_return_if_fail (prefs_model && GL_IS_PREFS_MODEL(prefs_model));
	g_return_if_fail (prefs_model->gconf_client != NULL);

	/* We are saving settings because presumably some of them have been changed. */
	g_signal_emit (G_OBJECT(prefs_model), signals[CHANGED], 0);

	/* Units */
	gconf_client_set_string (prefs_model->gconf_client,
				 BASE_KEY PREF_UNITS,
				 units_to_string(prefs_model->units),
				 NULL);

	/* Default page size */
	gconf_client_set_string (prefs_model->gconf_client,
				 BASE_KEY PREF_DEFAULT_PAGE_SIZE,
				 prefs_model->default_page_size,
				 NULL);


	/* Text properties */
	gconf_client_set_string (prefs_model->gconf_client,
				 BASE_KEY PREF_DEFAULT_FONT_FAMILY,
				 prefs_model->default_font_family,
				 NULL);

	gconf_client_set_float  (prefs_model->gconf_client,
				 BASE_KEY PREF_DEFAULT_FONT_SIZE,
				 prefs_model->default_font_size,
				 NULL);

	gconf_client_set_string (prefs_model->gconf_client,
				 BASE_KEY PREF_DEFAULT_FONT_WEIGHT,
				 gl_util_weight_to_string(prefs_model->default_font_weight),
				 NULL);

	gconf_client_set_int    (prefs_model->gconf_client,
				 BASE_KEY PREF_DEFAULT_TEXT_COLOR,
				 prefs_model->default_text_color,
				 NULL);

	gconf_client_set_string (prefs_model->gconf_client,
				 BASE_KEY PREF_DEFAULT_TEXT_ALIGNMENT,
				 gl_util_just_to_string(prefs_model->default_text_alignment),
				 NULL);

	gconf_client_set_float  (prefs_model->gconf_client,
				 BASE_KEY PREF_DEFAULT_TEXT_LINE_SPACING,
				 prefs_model->default_text_line_spacing,
				 NULL);

	/* Line properties */
	gconf_client_set_float  (prefs_model->gconf_client,
				 BASE_KEY PREF_DEFAULT_LINE_WIDTH,
				 prefs_model->default_line_width,
				 NULL);

	gconf_client_set_int    (prefs_model->gconf_client,
				 BASE_KEY PREF_DEFAULT_LINE_COLOR,
				 prefs_model->default_line_color,
				 NULL);


	/* Fill properties */
	gconf_client_set_int    (prefs_model->gconf_client,
				 BASE_KEY PREF_DEFAULT_FILL_COLOR,
				 prefs_model->default_fill_color,
				 NULL);


	/* Main Toolbar */
	gconf_client_set_bool (prefs_model->gconf_client,
			       BASE_KEY PREF_MAIN_TOOLBAR_VISIBLE,
			       prefs_model->main_toolbar_visible,
			       NULL);

	gconf_client_set_int (prefs_model->gconf_client,
			      BASE_KEY PREF_MAIN_TOOLBAR_BUTTONS_STYLE,
			      prefs_model->main_toolbar_buttons_style,
			      NULL);

	gconf_client_set_bool (prefs_model->gconf_client,
			       BASE_KEY PREF_MAIN_TOOLBAR_VIEW_TOOLTIPS,
			       prefs_model->main_toolbar_view_tooltips,
			       NULL);

	/* Drawing Toolbar */
	gconf_client_set_bool (prefs_model->gconf_client,
			       BASE_KEY PREF_DRAWING_TOOLBAR_VISIBLE,
			       prefs_model->drawing_toolbar_visible,
			       NULL);

	gconf_client_set_int (prefs_model->gconf_client,
			      BASE_KEY PREF_DRAWING_TOOLBAR_BUTTONS_STYLE,
			      prefs_model->drawing_toolbar_buttons_style,
			      NULL);

	gconf_client_set_bool (prefs_model->gconf_client,
			       BASE_KEY PREF_DRAWING_TOOLBAR_VIEW_TOOLTIPS,
			       prefs_model->drawing_toolbar_view_tooltips,
			       NULL);

	/* Property Toolbar */
	gconf_client_set_bool (prefs_model->gconf_client,
			       BASE_KEY PREF_PROPERTY_TOOLBAR_VISIBLE,
			       prefs_model->property_toolbar_visible,
			       NULL);

	gconf_client_set_bool (prefs_model->gconf_client,
			       BASE_KEY PREF_PROPERTY_TOOLBAR_VIEW_TOOLTIPS,
			       prefs_model->property_toolbar_view_tooltips,
			       NULL);

	/* View properties */
	gconf_client_set_bool (prefs_model->gconf_client,
			       BASE_KEY PREF_GRID_VISIBLE,
			       prefs_model->grid_visible,
			       NULL);

	gconf_client_set_bool (prefs_model->gconf_client,
			       BASE_KEY PREF_MARKUP_VISIBLE,
			       prefs_model->markup_visible,
			       NULL);

	/* Recent files */
	gconf_client_set_int (prefs_model->gconf_client,
			      BASE_KEY PREF_MAX_RECENTS,
			      prefs_model->max_recents,
			      NULL);


	gconf_client_suggest_sync (prefs_model->gconf_client, NULL);
	
	gl_debug (DEBUG_PREFS, "END");
}

/*****************************************************************************/
/* Load all settings.                                                        */
/*****************************************************************************/
void
gl_prefs_model_load_settings (glPrefsModel *prefs_model)
{
	gchar   *string;
	glPaper *paper;

	gl_debug (DEBUG_PREFS, "START");
	
	g_return_if_fail (prefs_model && GL_IS_PREFS_MODEL(prefs_model));
	g_return_if_fail (prefs_model->gconf_client != NULL);

	/* Units */
	string =
		get_string (prefs_model->gconf_client,
			    BASE_KEY PREF_UNITS,
			    DEFAULT_UNITS_STRING);
	prefs_model->units = string_to_units( string );
	g_free( string );


	/* Page size */
	prefs_model->default_page_size =
		get_string (prefs_model->gconf_client,
			    BASE_KEY PREF_DEFAULT_PAGE_SIZE,
			    DEFAULT_PAGE_SIZE);

	/* Text properties */
	prefs_model->default_font_family =
		get_string (prefs_model->gconf_client,
			    BASE_KEY PREF_DEFAULT_FONT_FAMILY,
			    DEFAULT_FONT_FAMILY);

	prefs_model->default_font_size =
		get_float (prefs_model->gconf_client,
			   BASE_KEY PREF_DEFAULT_FONT_SIZE,
			   DEFAULT_FONT_SIZE);

	string =
		get_string (prefs_model->gconf_client,
			    BASE_KEY PREF_DEFAULT_FONT_WEIGHT,
			    DEFAULT_FONT_WEIGHT_STRING);
	prefs_model->default_font_weight = gl_util_string_to_weight( string );
	g_free( string );

	prefs_model->default_text_color =
		get_int (prefs_model->gconf_client,
			 BASE_KEY PREF_DEFAULT_TEXT_COLOR,
			 DEFAULT_TEXT_COLOR);

	string =
		get_string (prefs_model->gconf_client,
			    BASE_KEY PREF_DEFAULT_TEXT_ALIGNMENT,
			    DEFAULT_TEXT_JUST_STRING);
	prefs_model->default_text_alignment = gl_util_string_to_just( string );
	g_free( string );

	prefs_model->default_text_line_spacing =
		get_float (prefs_model->gconf_client,
			   BASE_KEY PREF_DEFAULT_TEXT_LINE_SPACING,
			   DEFAULT_TEXT_LINE_SPACING);

	gl_debug (DEBUG_PREFS, "text_line_spacing = %f", prefs_model->default_text_line_spacing);

	/* Line properties */
	prefs_model->default_line_width =
		get_float (prefs_model->gconf_client,
			   BASE_KEY PREF_DEFAULT_LINE_WIDTH,
			   DEFAULT_LINE_WIDTH);
	prefs_model->default_line_color =
		get_int (prefs_model->gconf_client,
			 BASE_KEY PREF_DEFAULT_LINE_COLOR,
			 DEFAULT_LINE_COLOR);

	/* Fill properties */
	prefs_model->default_fill_color =
		get_int (prefs_model->gconf_client,
			 BASE_KEY PREF_DEFAULT_FILL_COLOR,
			 DEFAULT_FILL_COLOR);


	/* User Inferface/Main Toolbar */
	prefs_model->main_toolbar_visible =
		get_bool (prefs_model->gconf_client,
			  BASE_KEY PREF_MAIN_TOOLBAR_VISIBLE,
			  TRUE);

	prefs_model->main_toolbar_buttons_style =
		get_int (prefs_model->gconf_client,
			 BASE_KEY PREF_MAIN_TOOLBAR_BUTTONS_STYLE,
			 GL_TOOLBAR_SYSTEM);

	prefs_model->main_toolbar_view_tooltips =
		get_bool (prefs_model->gconf_client,
			  BASE_KEY PREF_MAIN_TOOLBAR_VIEW_TOOLTIPS,
			  TRUE);

	/* User Inferface/Drawing Toolbar */
	prefs_model->drawing_toolbar_visible =
		get_bool (prefs_model->gconf_client,
			  BASE_KEY PREF_DRAWING_TOOLBAR_VISIBLE,
			  TRUE);

	prefs_model->drawing_toolbar_buttons_style =
		get_int (prefs_model->gconf_client,
			 BASE_KEY PREF_DRAWING_TOOLBAR_BUTTONS_STYLE,
			 GL_TOOLBAR_SYSTEM);

	prefs_model->drawing_toolbar_view_tooltips =
		get_bool (prefs_model->gconf_client,
			  BASE_KEY PREF_DRAWING_TOOLBAR_VIEW_TOOLTIPS,
			  TRUE);

	/* User Inferface/Property Toolbar */
	prefs_model->property_toolbar_visible =
		get_bool (prefs_model->gconf_client,
			  BASE_KEY PREF_PROPERTY_TOOLBAR_VISIBLE,
			  TRUE);

	prefs_model->property_toolbar_view_tooltips =
		get_bool (prefs_model->gconf_client,
			  BASE_KEY PREF_PROPERTY_TOOLBAR_VIEW_TOOLTIPS,
			  TRUE);


	/* View properties */
	prefs_model->grid_visible =
		get_bool (prefs_model->gconf_client,
			  BASE_KEY PREF_GRID_VISIBLE,
			  TRUE);

	prefs_model->markup_visible =
		get_bool (prefs_model->gconf_client,
			  BASE_KEY PREF_MARKUP_VISIBLE,
			  TRUE);

	/* Recent files */
	prefs_model->max_recents =
		get_int (prefs_model->gconf_client,
			 BASE_KEY PREF_MAX_RECENTS,
			 4);


	/* Proof read the default page size -- it must be a valid id. */
	/* (For compatability with older versions.) */
	/* Note: paper module must be initialized for this to work. */
	paper = gl_paper_from_id (prefs_model->default_page_size);
	if ( paper == NULL ) {
		prefs_model->default_page_size = g_strdup (DEFAULT_PAGE_SIZE);
	} else {
		gl_paper_free (paper);
		paper = NULL;
	}

	gl_debug (DEBUG_PREFS, "max_recents = %d", prefs_model->max_recents);


	g_signal_emit (G_OBJECT(prefs_model), signals[CHANGED], 0);

	gl_debug (DEBUG_PREFS, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Key changed callback.                                           */
/*---------------------------------------------------------------------------*/
static void 
notify_cb (GConfClient  *client,
	   guint         cnxn_id,
	   GConfEntry   *entry,
	   glPrefsModel *prefs_model)
{
	gl_debug (DEBUG_PREFS, "Key was changed: %s", entry->key);

	gl_prefs_model_load_settings (prefs_model);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Utilities to get values with defaults.                          */
/*---------------------------------------------------------------------------*/
static gchar*
get_string (GConfClient *client,
	    const gchar *key,
	    const gchar *def)
{
	gchar* val;

	val = gconf_client_get_string (client, key, NULL);

	if (val != NULL) {

		return val;

	} else {

		return def ? g_strdup (def) : NULL;

	}
}

static gboolean
get_bool (GConfClient *client,
	  const gchar *key,
	  gboolean     def)
{
	GConfValue* val;
	gboolean retval;

	val = gconf_client_get (client, key, NULL);

	if (val != NULL) {

		if ( val->type == GCONF_VALUE_BOOL ) {
			retval = gconf_value_get_bool (val);
		} else {
			retval = def;
		}

		gconf_value_free (val);

		return retval;

	} else {

		return def;

	}
}

static gint
get_int (GConfClient *client,
	 const gchar *key,
	 gint         def)
{
	GConfValue* val;
	gint retval;

	val = gconf_client_get (client, key, NULL);

	if (val != NULL) {

		if ( val->type == GCONF_VALUE_INT) {
			retval = gconf_value_get_int(val);
		} else {
			retval = def;
		}

		gconf_value_free (val);

		return retval;

	} else {

		return def;

	}
}

static gdouble
get_float (GConfClient *client,
	   const gchar *key,
	   gdouble      def)
{
	GConfValue* val;
	gdouble retval;

	val = gconf_client_get (client, key, NULL);

	if (val != NULL) {

		if ( val->type == GCONF_VALUE_FLOAT ) {
			retval = gconf_value_get_float(val);
		} else {
			retval = def;
		}

		gconf_value_free (val);

		return retval;

	} else {
		return def;

	}
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Utilities to deal with units.                                   */
/*---------------------------------------------------------------------------*/
static glPrefsUnits
string_to_units (const gchar *string)
{
	glPrefsUnits units;

	if (g_strcasecmp (string, "Points") == 0) {
		units = GL_UNITS_PTS;
	} else if (g_strcasecmp (string, "Inches") == 0) {
		units = GL_UNITS_INCHES;
	} else if (g_strcasecmp (string, "Millimeters") == 0) {
		units = GL_UNITS_MM;
	} else {
		units = GL_UNITS_INCHES;
	}

	return units;
}

static const
gchar *units_to_string (glPrefsUnits units)
{
	switch (units) {
	case GL_UNITS_PTS:
		return "Points";
		break;
	case GL_UNITS_INCHES:
		return "Inches";
		break;
	case GL_UNITS_MM:
		return "Millimeters";
		break;
	default:
		return "Inches";
		break;
	}
}


