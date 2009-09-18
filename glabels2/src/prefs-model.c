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

#include <config.h>

#include "prefs-model.h"

#include <glib.h>
#include <gtk/gtk.h>
#include <string.h>

#include <libglabels/libglabels.h>
#include "marshal.h"
#include "util.h"
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

#define PREF_DRAWING_TOOLBAR_VISIBLE        "/drawing-toolbar-visible"

#define PREF_PROPERTY_TOOLBAR_VISIBLE        "/property-toolbar-visible"

#define PREF_GRID_VISIBLE                   "/grid-visible"
#define PREF_MARKUP_VISIBLE                 "/markup-visible"

#define PREF_MAX_RECENTS                    "/max-recents"

#define PREF_RECENT_TEMPLATES               "/recent-templates"
#define PREF_MAX_RECENT_TEMPLATES           "/max-recent-templates"

/* Default values */
#define DEFAULT_UNITS_STRING_US    units_to_string (LGL_UNITS_INCH)
#define DEFAULT_PAGE_SIZE_US       "US-Letter"

#define DEFAULT_UNITS_STRING_METRIC units_to_string (LGL_UNITS_MM)
#define DEFAULT_PAGE_SIZE_METRIC   "A4"

#define DEFAULT_FONT_FAMILY        "Sans"
#define DEFAULT_FONT_SIZE          14.0
#define DEFAULT_FONT_WEIGHT_STRING gl_util_weight_to_string (PANGO_WEIGHT_NORMAL)
#define DEFAULT_FONT_ITALIC_FLAG   FALSE
#define DEFAULT_TEXT_ALIGN_STRING  gl_util_align_to_string (PANGO_ALIGN_LEFT)
#define DEFAULT_TEXT_COLOR         GL_COLOR (0,0,0)
#define DEFAULT_TEXT_LINE_SPACING  1.0

#define DEFAULT_LINE_WIDTH         1.0
#define DEFAULT_LINE_COLOR         GL_COLOR_A (0, 0, 0, 255)

#define DEFAULT_FILL_COLOR         GL_COLOR_A (0, 255, 0, 255)


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

static guint signals[LAST_SIGNAL] = {0};


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

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

static lglUnitsType   string_to_units              (const gchar         *string);
static const gchar   *units_to_string              (lglUnitsType         units);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glPrefsModel, gl_prefs_model, G_TYPE_OBJECT);


static void
gl_prefs_model_class_init (glPrefsModelClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	gl_debug (DEBUG_PREFS, "START");

	gl_prefs_model_parent_class = g_type_class_peek_parent (class);

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
gl_prefs_model_init (glPrefsModel *prefs_model)
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
	glPrefsModel *prefs_model = GL_PREFS_MODEL (object);

	gl_debug (DEBUG_PREFS, "START");

	g_return_if_fail (object && GL_IS_PREFS_MODEL (object));

	g_object_unref (G_OBJECT(prefs_model->gconf_client));
	g_free (prefs_model->default_page_size);
	g_free (prefs_model->default_font_family);

	G_OBJECT_CLASS (gl_prefs_model_parent_class)->finalize (object);

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
        lgl_xml_set_default_units (prefs_model->units);

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
				 gl_util_align_to_string(prefs_model->default_text_alignment),
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

	/* Drawing Toolbar */
	gconf_client_set_bool (prefs_model->gconf_client,
			       BASE_KEY PREF_DRAWING_TOOLBAR_VISIBLE,
			       prefs_model->drawing_toolbar_visible,
			       NULL);

	/* Property Toolbar */
	gconf_client_set_bool (prefs_model->gconf_client,
			       BASE_KEY PREF_PROPERTY_TOOLBAR_VISIBLE,
			       prefs_model->property_toolbar_visible,
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

	/* Recent templates */
	gconf_client_set_list (prefs_model->gconf_client,
			       BASE_KEY PREF_RECENT_TEMPLATES,
                               GCONF_VALUE_STRING,
                               prefs_model->recent_templates,
                               NULL);
	gconf_client_set_int (prefs_model->gconf_client,
			      BASE_KEY PREF_MAX_RECENT_TEMPLATES,
			      prefs_model->max_recent_templates,
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
        const gchar *pgsize, *default_units_string, *default_page_size;
	gchar    *string;
	lglPaper *paper;
        GSList   *p, *p_next;

	gl_debug (DEBUG_PREFS, "START");
	
	g_return_if_fail (prefs_model && GL_IS_PREFS_MODEL(prefs_model));
	g_return_if_fail (prefs_model->gconf_client != NULL);

        /* Make educated guess about locale defaults. */
        pgsize = gtk_paper_size_get_default ();
        if ( strcmp (pgsize,GTK_PAPER_NAME_LETTER) == 0 )
        {
                default_units_string = DEFAULT_UNITS_STRING_US;
                default_page_size    = DEFAULT_PAGE_SIZE_US;
        }
        else
        {
                default_units_string = DEFAULT_UNITS_STRING_METRIC;
                default_page_size    = DEFAULT_PAGE_SIZE_METRIC;
        }

	/* Units */
	string =
		get_string (prefs_model->gconf_client,
			    BASE_KEY PREF_UNITS,
			    default_units_string);
	prefs_model->units = string_to_units( string );
	g_free( string );
        lgl_xml_set_default_units (prefs_model->units);


	/* Page size */
        g_free (prefs_model->default_page_size);
	prefs_model->default_page_size =
		get_string (prefs_model->gconf_client,
			    BASE_KEY PREF_DEFAULT_PAGE_SIZE,
			    default_page_size);

	/* Text properties */
        g_free (prefs_model->default_font_family);
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
			    DEFAULT_TEXT_ALIGN_STRING);
	prefs_model->default_text_alignment = gl_util_string_to_align( string );
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

	/* User Inferface/Drawing Toolbar */
	prefs_model->drawing_toolbar_visible =
		get_bool (prefs_model->gconf_client,
			  BASE_KEY PREF_DRAWING_TOOLBAR_VISIBLE,
			  TRUE);

	/* User Inferface/Property Toolbar */
	prefs_model->property_toolbar_visible =
		get_bool (prefs_model->gconf_client,
			  BASE_KEY PREF_PROPERTY_TOOLBAR_VISIBLE,
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
			 -1);

	/* Recent templates */
        for (p=prefs_model->recent_templates; p != NULL; p=p->next)
        {
                g_free (p->data);
        }
        g_slist_free (prefs_model->recent_templates);
	prefs_model->recent_templates =
		gconf_client_get_list (prefs_model->gconf_client,
                                       BASE_KEY PREF_RECENT_TEMPLATES,
                                       GCONF_VALUE_STRING,
                                       NULL);
	prefs_model->max_recent_templates =
		get_int (prefs_model->gconf_client,
			 BASE_KEY PREF_MAX_RECENT_TEMPLATES,
			 5);


	/* Proof read the default page size -- it must be a valid id. */
	/* (For compatability with older versions.) */
	paper = lgl_db_lookup_paper_from_id (prefs_model->default_page_size);
	if ( paper == NULL ) {
		prefs_model->default_page_size = g_strdup (DEFAULT_PAGE_SIZE_US);
	} else {
		lgl_paper_free (paper);
		paper = NULL;
	}

        /* Proof read the recent templates list.  Make sure the template names */
        /* are valid.  Remove from list if not. */
        for (p=prefs_model->recent_templates; p != NULL; p=p_next)
        {
                p_next = p->next;

                if ( !lgl_db_does_template_name_exist (p->data) )
                {
                        g_free (p->data);
                        prefs_model->recent_templates = g_slist_delete_link (prefs_model->recent_templates, p);
                }
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
static lglUnitsType
string_to_units (const gchar *string)
{
	lglUnitsType units;

	if (g_ascii_strcasecmp (string, "Points") == 0) {
		units = LGL_UNITS_POINT;
	} else if (g_ascii_strcasecmp (string, "Inches") == 0) {
		units = LGL_UNITS_INCH;
	} else if (g_ascii_strcasecmp (string, "Millimeters") == 0) {
		units = LGL_UNITS_MM;
	} else {
		units = LGL_UNITS_INCH;
	}

	return units;
}


static const
gchar *units_to_string (lglUnitsType units)
{
	switch (units) {
	case LGL_UNITS_POINT:
		return "Points";
		break;
	case LGL_UNITS_INCH:
		return "Inches";
		break;
	case LGL_UNITS_MM:
		return "Millimeters";
		break;
	default:
		return "Inches";
		break;
	}
}





/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
