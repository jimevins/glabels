/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prefs.h:  Application preferences module header file
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

#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>

#include <gconf/gconf-client.h>

#include "prefs.h"
#include "util.h"

#include "debug.h"

glPreferences      *gl_prefs     = NULL;

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

#define PREF_DEFAULT_LINE_WIDTH             "/default-line-width"
#define PREF_DEFAULT_LINE_COLOR             "/default-line-color"

#define PREF_DEFAULT_FILL_COLOR             "/default-fill-color"

#define PREF_MAIN_TOOLBAR_VISIBLE           "/main-toolbar-visible"
#define PREF_MAIN_TOOLBAR_BUTTONS_STYLE     "/main-toolbar-buttons-style"
#define PREF_MAIN_TOOLBAR_VIEW_TOOLTIPS     "/main-toolbar-view-tooltips"

#define PREF_DRAWING_TOOLBAR_VISIBLE        "/drawing-toolbar-visible"
#define PREF_DRAWING_TOOLBAR_BUTTONS_STYLE  "/drawing-toolbar-buttons-style"
#define PREF_DRAWING_TOOLBAR_VIEW_TOOLTIPS  "/drawing-toolbar-view-tooltips"

#define PREF_MAX_RECENTS                    "/max-recents"

/* Default values */
#define DEFAULT_UNITS_STRING       units_to_string (GL_PREFS_UNITS_INCHES)
#define DEFAULT_PAGE_SIZE          "US Letter"

#define DEFAULT_FONT_FAMILY        "Helvetica"
#define DEFAULT_FONT_SIZE          14.0
#define DEFAULT_FONT_WEIGHT_STRING gl_util_weight_to_string (GNOME_FONT_BOOK)
#define DEFAULT_FONT_ITALIC_FLAG   FALSE
#define DEFAULT_TEXT_JUST_STRING   gl_util_just_to_string (GTK_JUSTIFY_LEFT)
#define DEFAULT_TEXT_COLOR         GNOME_CANVAS_COLOR (0,0,0)

#define DEFAULT_LINE_WIDTH         1.0
#define DEFAULT_LINE_COLOR         GNOME_CANVAS_COLOR_A (0, 0, 0, 255)

#define DEFAULT_FILL_COLOR         GNOME_CANVAS_COLOR_A (0, 255, 0, 255)

/*========================================================*/
/* Private types.                                         */
/*========================================================*/


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/
static GConfClient *gconf_client = NULL;

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void notify_cb (GConfClient *client,
		       guint cnxn_id,
		       GConfEntry *entry,
		       gpointer user_data);

static gchar *get_string (GConfClient* client, const gchar* key, const gchar* def);
static gboolean get_bool (GConfClient* client, const gchar* key, gboolean def);
static gint get_int (GConfClient* client, const gchar* key, gint def);
static gdouble get_float (GConfClient* client, const gchar* key, gdouble def);

static glPrefsUnits string_to_units (const gchar *string);
static const gchar *units_to_string (glPrefsUnits units);



/*****************************************************************************/
/* Initialize preferences module.                                            */
/*****************************************************************************/
void 
gl_prefs_init (void)
{
	gl_debug (DEBUG_PREFS, "");

	gconf_client = gconf_client_get_default ();
	
	g_return_if_fail (gconf_client != NULL);

	gconf_client_add_dir (gconf_client,
			      BASE_KEY,
			      GCONF_CLIENT_PRELOAD_ONELEVEL,
			      NULL);
	
	gconf_client_notify_add (gconf_client,
				 BASE_KEY,
				 notify_cb,
				 NULL, NULL, NULL);
}


/*****************************************************************************/
/* Save all settings.                                                        */
/*****************************************************************************/
void 
gl_prefs_save_settings (void)
{
	gl_debug (DEBUG_PREFS, "START");
	
	g_return_if_fail (gconf_client != NULL);
	g_return_if_fail (gl_prefs != NULL);

	/* Units */
	gconf_client_set_string (gconf_client,
				 BASE_KEY PREF_UNITS,
				 units_to_string(gl_prefs->units),
				 NULL);

	/* Default page size */
	gconf_client_set_string (gconf_client,
				 BASE_KEY PREF_DEFAULT_PAGE_SIZE,
				 gl_prefs->default_page_size,
				 NULL);


	/* Text properties */
	gconf_client_set_string (gconf_client,
				 BASE_KEY PREF_DEFAULT_FONT_FAMILY,
				 gl_prefs->default_font_family,
				 NULL);

	gconf_client_set_float  (gconf_client,
				 BASE_KEY PREF_DEFAULT_FONT_SIZE,
				 gl_prefs->default_font_size,
				 NULL);

	gconf_client_set_string (gconf_client,
				 BASE_KEY PREF_DEFAULT_FONT_WEIGHT,
				 gl_util_weight_to_string(gl_prefs->default_font_weight),
				 NULL);

	gconf_client_set_int    (gconf_client,
				 BASE_KEY PREF_DEFAULT_TEXT_COLOR,
				 gl_prefs->default_text_color,
				 NULL);

	gconf_client_set_string (gconf_client,
				 BASE_KEY PREF_DEFAULT_TEXT_ALIGNMENT,
				 gl_util_just_to_string(gl_prefs->default_text_alignment),
				 NULL);


	/* Line properties */
	gconf_client_set_float  (gconf_client,
				 BASE_KEY PREF_DEFAULT_LINE_WIDTH,
				 gl_prefs->default_line_width,
				 NULL);

	gconf_client_set_int    (gconf_client,
				 BASE_KEY PREF_DEFAULT_LINE_COLOR,
				 gl_prefs->default_line_color,
				 NULL);


	/* Fill properties */
	gconf_client_set_int    (gconf_client,
				 BASE_KEY PREF_DEFAULT_FILL_COLOR,
				 gl_prefs->default_fill_color,
				 NULL);


	/* Main Toolbar */
	gconf_client_set_bool (gconf_client,
			       BASE_KEY PREF_MAIN_TOOLBAR_VISIBLE,
			       gl_prefs->main_toolbar_visible,
			       NULL);

	gconf_client_set_int (gconf_client,
			      BASE_KEY PREF_MAIN_TOOLBAR_BUTTONS_STYLE,
			      gl_prefs->main_toolbar_buttons_style,
			      NULL);

	gconf_client_set_bool (gconf_client,
			       BASE_KEY PREF_MAIN_TOOLBAR_VIEW_TOOLTIPS,
			       gl_prefs->main_toolbar_view_tooltips,
			       NULL);

	/* Drawing Toolbar */
	gconf_client_set_bool (gconf_client,
			       BASE_KEY PREF_DRAWING_TOOLBAR_VISIBLE,
			       gl_prefs->drawing_toolbar_visible,
			       NULL);

	gconf_client_set_int (gconf_client,
			      BASE_KEY PREF_DRAWING_TOOLBAR_BUTTONS_STYLE,
			      gl_prefs->drawing_toolbar_buttons_style,
			      NULL);

	gconf_client_set_bool (gconf_client,
			       BASE_KEY PREF_DRAWING_TOOLBAR_VIEW_TOOLTIPS,
			       gl_prefs->drawing_toolbar_view_tooltips,
			       NULL);


	/* Recent files */
	gconf_client_set_int (gconf_client,
			      BASE_KEY PREF_MAX_RECENTS,
			      gl_prefs->max_recents,
			      NULL);


	gconf_client_suggest_sync (gconf_client, NULL);
	
	gl_debug (DEBUG_PREFS, "END");
}

/*****************************************************************************/
/* Load all settings.                                                        */
/*****************************************************************************/
void
gl_prefs_load_settings (void)
{
	gchar *string;

	gl_debug (DEBUG_PREFS, "START");
	
	if (gl_prefs == NULL)
		gl_prefs = g_new0 (glPreferences, 1);

	if (gconf_client == NULL)
	{
		/* TODO: in any case set default values */
		g_warning ("Cannot load settings.");
		return;
	}


	/* Units */
	string =
		get_string (gconf_client,
			    BASE_KEY PREF_UNITS,
			    DEFAULT_UNITS_STRING);
	gl_prefs->units = string_to_units( string );
	g_free( string );


	/* Page size */
	gl_prefs->default_page_size =
		get_string (gconf_client,
			    BASE_KEY PREF_DEFAULT_PAGE_SIZE,
			    DEFAULT_PAGE_SIZE);

	/* Text properties */
	gl_prefs->default_font_family =
		get_string (gconf_client,
			    BASE_KEY PREF_DEFAULT_FONT_FAMILY,
			    DEFAULT_FONT_FAMILY);

	gl_prefs->default_font_size =
		get_float (gconf_client,
			   BASE_KEY PREF_DEFAULT_FONT_SIZE,
			   DEFAULT_FONT_SIZE);

	string =
		get_string (gconf_client,
			    BASE_KEY PREF_DEFAULT_FONT_WEIGHT,
			    DEFAULT_FONT_WEIGHT_STRING);
	gl_prefs->default_font_weight =
		gl_util_string_to_weight( string );
	g_free( string );

	gl_prefs->default_text_color =
		get_int (gconf_client,
			 BASE_KEY PREF_DEFAULT_TEXT_COLOR,
			 DEFAULT_TEXT_COLOR);

	string =
		get_string (gconf_client,
			    BASE_KEY PREF_DEFAULT_TEXT_ALIGNMENT,
			    DEFAULT_TEXT_JUST_STRING);
	gl_prefs->default_font_weight =	gl_util_string_to_just( string );
	g_free( string );


	/* Line properties */
	gl_prefs->default_line_width =
		get_float (gconf_client,
			   BASE_KEY PREF_DEFAULT_LINE_WIDTH,
			   DEFAULT_LINE_WIDTH);
	gl_prefs->default_line_color =
		get_int (gconf_client,
			 BASE_KEY PREF_DEFAULT_LINE_COLOR,
			 DEFAULT_LINE_COLOR);

	/* Fill properties */
	gl_prefs->default_fill_color =
		get_int (gconf_client,
			 BASE_KEY PREF_DEFAULT_FILL_COLOR,
			 DEFAULT_FILL_COLOR);


	/* User Inferface/Main Toolbar */
	gl_prefs->main_toolbar_visible =
		get_bool (gconf_client,
			  BASE_KEY PREF_MAIN_TOOLBAR_VISIBLE,
			  TRUE);

	gl_prefs->main_toolbar_buttons_style =
		get_int (gconf_client,
			 BASE_KEY PREF_MAIN_TOOLBAR_BUTTONS_STYLE,
			 GL_TOOLBAR_SYSTEM);

	gl_prefs->main_toolbar_view_tooltips =
		get_bool (gconf_client,
			  BASE_KEY PREF_MAIN_TOOLBAR_VIEW_TOOLTIPS,
			  TRUE);

	/* User Inferface/Drawing Toolbar */
	gl_prefs->drawing_toolbar_visible =
		get_bool (gconf_client,
			  BASE_KEY PREF_DRAWING_TOOLBAR_VISIBLE,
			  TRUE);

	gl_prefs->drawing_toolbar_buttons_style =
		get_int (gconf_client,
			 BASE_KEY PREF_DRAWING_TOOLBAR_BUTTONS_STYLE,
			 GL_TOOLBAR_SYSTEM);

	gl_prefs->drawing_toolbar_view_tooltips =
		get_bool (gconf_client,
			  BASE_KEY PREF_DRAWING_TOOLBAR_VIEW_TOOLTIPS,
			  TRUE);


	/* Recent files */
	gl_prefs->max_recents =
		get_int (gconf_client,
			 BASE_KEY PREF_MAX_RECENTS,
			 4);


	gl_debug (DEBUG_PREFS, "max_recents = %d", gl_prefs->max_recents);

	gl_debug (DEBUG_PREFS, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Key changed callback.                                           */
/*---------------------------------------------------------------------------*/
static void 
notify_cb (GConfClient *client,
	   guint cnxn_id,
	   GConfEntry *entry,
	   gpointer user_data)
{
	gl_debug (DEBUG_PREFS, "Key was changed: %s", entry->key);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Utilities to get values with defaults.                          */
/*---------------------------------------------------------------------------*/
static gchar*
get_string (GConfClient* client,
	    const gchar* key,
	    const gchar* def)
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
get_bool (GConfClient* client,
	  const gchar* key,
	  gboolean def)
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
get_int (GConfClient* client,
	 const gchar* key,
	 gint def)
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
get_float (GConfClient* client,
	   const gchar* key,
	   gdouble def)
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
static glPrefsUnits string_to_units (const gchar *string)
{
	glPrefsUnits units;

	if (g_strcasecmp (string, "Points") == 0) {
		units = GL_PREFS_UNITS_PTS;
	} else if (g_strcasecmp (string, "Inches") == 0) {
		units = GL_PREFS_UNITS_INCHES;
	} else if (g_strcasecmp (string, "Millimeters") == 0) {
		units = GL_PREFS_UNITS_MM;
	} else {
		units = GL_PREFS_UNITS_INCHES;
	}

	return units;
}

static const gchar *units_to_string (glPrefsUnits units)
{
	switch (units) {
	case GL_PREFS_UNITS_PTS:
		return "Points";
		break;
	case GL_PREFS_UNITS_INCHES:
		return "Inches";
		break;
	case GL_PREFS_UNITS_MM:
		return "Millimeters";
		break;
	default:
		return "Inches";
		break;
	}
}



/****************************************************************************/
/* Get string representing desired units.                                   */
/****************************************************************************/
const gchar *
gl_prefs_get_page_size (void)
{
	return (gl_prefs->default_page_size);
}

/****************************************************************************/
/* Get desired units.                                                       */
/****************************************************************************/
glPrefsUnits
gl_prefs_get_units (void)
{
	return gl_prefs->units;
}

/****************************************************************************/
/* Get desired units per point.                                             */
/****************************************************************************/
gdouble
gl_prefs_get_units_per_point (void)
{

	switch (gl_prefs->units) {
	case GL_PREFS_UNITS_PTS:
		return 1.0;	/* points */
	case GL_PREFS_UNITS_INCHES:
		return 1.0 / 72.0;	/* inches */
	case GL_PREFS_UNITS_MM:
		return 0.35277778;	/* mm */
	default:
		g_warning ("Illegal units");	/* Should not happen */
		return 1.0;
	}
}

/****************************************************************************/
/* Get precision for desired units.                                         */
/****************************************************************************/
gint
gl_prefs_get_units_precision (void)
{

	switch (gl_prefs->units) {
	case GL_PREFS_UNITS_PTS:
		return 1;	/* points */
	case GL_PREFS_UNITS_INCHES:
		return 3;	/* inches */
	case GL_PREFS_UNITS_MM:
		return 1;	/* mm */
	default:
		g_warning ("Illegal units");	/* Should not happen */
		return 1.0;
	}
}

/****************************************************************************/
/* Get step size for desired units.                                         */
/****************************************************************************/
gdouble
gl_prefs_get_units_step_size (void)
{

	switch (gl_prefs->units) {
	case GL_PREFS_UNITS_PTS:
		return 0.1;	/* points */
	case GL_PREFS_UNITS_INCHES:
		return 0.001;	/* inches */
	case GL_PREFS_UNITS_MM:
		return 0.1;	/* mm */
	default:
		g_warning ("Illegal units");	/* Should not happen */
		return 1.0;
	}
}

/****************************************************************************/
/* Get string representing desired units.                                   */
/****************************************************************************/
const gchar *
gl_prefs_get_units_string (void)
{

	switch (gl_prefs->units) {
	case GL_PREFS_UNITS_PTS:
		return _("points");
	case GL_PREFS_UNITS_INCHES:
		return _("inches");
	case GL_PREFS_UNITS_MM:
		return _("mm");
	default:
		g_warning ("Illegal units");	/* Should not happen */
		return _("points");
	}
}

