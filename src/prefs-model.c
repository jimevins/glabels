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
#include <gconf/gconf-client.h>

#include <libglabels.h>
#include "marshal.h"
#include "str-util.h"
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

#define PREF_DRAWING_TOOLBAR_VISIBLE        "/drawing-toolbar-visible"

#define PREF_PROPERTY_TOOLBAR_VISIBLE        "/property-toolbar-visible"

#define PREF_GRID_VISIBLE                   "/grid-visible"
#define PREF_MARKUP_VISIBLE                 "/markup-visible"

#define PREF_MAX_RECENTS                    "/max-recents"


/* Default values */
#define DEFAULT_UNITS_STRING_US     lgl_units_get_id (LGL_UNITS_INCH)
#define DEFAULT_PAGE_SIZE_US        "US-Letter"

#define DEFAULT_UNITS_STRING_METRIC lgl_units_get_id (LGL_UNITS_MM)
#define DEFAULT_PAGE_SIZE_METRIC    "A4"

#define DEFAULT_FONT_FAMILY        "Sans"
#define DEFAULT_FONT_SIZE          14.0
#define DEFAULT_FONT_WEIGHT_STRING gl_str_util_weight_to_string (PANGO_WEIGHT_NORMAL)
#define DEFAULT_FONT_ITALIC_FLAG   FALSE
#define DEFAULT_TEXT_ALIGN_STRING  gl_str_util_align_to_string (PANGO_ALIGN_LEFT)
#define DEFAULT_TEXT_COLOR         GL_COLOR (0,0,0)
#define DEFAULT_TEXT_LINE_SPACING  1.0

#define DEFAULT_LINE_WIDTH         1.0
#define DEFAULT_LINE_COLOR         GL_COLOR_A (0, 0, 0, 255)

#define DEFAULT_FILL_COLOR         GL_COLOR_A (0, 255, 0, 255)


/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glPrefsModelPrivate {

	GConfClient *gconf_client;

};

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
gl_prefs_model_init (glPrefsModel *this)
{
	gl_debug (DEBUG_PREFS, "START");

        this->priv = g_new0 (glPrefsModelPrivate, 1);

        this->priv->gconf_client = gconf_client_get_default ();

        g_return_if_fail (this->priv->gconf_client != NULL);
 
        gconf_client_add_dir (this->priv->gconf_client,
                              BASE_KEY,
                              GCONF_CLIENT_PRELOAD_ONELEVEL,
                              NULL);
         
        gconf_client_notify_add (this->priv->gconf_client,
                                 BASE_KEY,
                                 (GConfClientNotifyFunc)notify_cb, this,
                                 NULL, NULL);

	gl_debug (DEBUG_PREFS, "END");
}


static void
gl_prefs_model_finalize (GObject *object)
{
	glPrefsModel *this = GL_PREFS_MODEL (object);

	gl_debug (DEBUG_PREFS, "START");

	g_return_if_fail (object && GL_IS_PREFS_MODEL (object));

	g_object_unref (G_OBJECT(this->priv->gconf_client));
        g_free (this->priv);

	G_OBJECT_CLASS (gl_prefs_model_parent_class)->finalize (object);

	gl_debug (DEBUG_PREFS, "END");
}


/*****************************************************************************/
/* New prefs_model object.                                                   */
/*****************************************************************************/
glPrefsModel *
gl_prefs_model_new (void)
{
	glPrefsModel *this;

	gl_debug (DEBUG_PREFS, "START");

	this = GL_PREFS_MODEL (g_object_new (gl_prefs_model_get_type(), NULL));

	gl_debug (DEBUG_PREFS, "END");

	return this;
}


/*****************************************************************************/
/* Set units.                                                                */
/*****************************************************************************/
void
gl_prefs_model_set_units (glPrefsModel     *this,
                          lglUnits          units)
{
	gconf_client_set_string (this->priv->gconf_client,
				 BASE_KEY PREF_UNITS,
				 lgl_units_get_id (units),
				 NULL);
}


/*****************************************************************************/
/* Get units.                                                                */
/*****************************************************************************/
lglUnits
gl_prefs_model_get_units (glPrefsModel     *this)
{
        const gchar  *pgsize;
        const gchar  *default_units_string;
        gchar        *string;
        lglUnits      units;

        /* Make educated guess about locale default. */
        pgsize = gtk_paper_size_get_default ();
        if ( strcmp (pgsize, GTK_PAPER_NAME_LETTER) == 0 )
        {
                default_units_string = DEFAULT_UNITS_STRING_US;
        }
        else
        {
                default_units_string = DEFAULT_UNITS_STRING_METRIC;
        }

	string = get_string (this->priv->gconf_client,
                             BASE_KEY PREF_UNITS,
                             default_units_string);
	units = lgl_units_from_id (string);
	g_free (string);

        /* If invalid, make an educated guess from locale. */
        if (units == LGL_UNITS_INVALID)
        {
                if ( strcmp (pgsize, GTK_PAPER_NAME_LETTER) == 0 )
                {
                        units = LGL_UNITS_INCH;
                }
                else
                {
                        units = LGL_UNITS_MM;
                }
        }

        return units;
}


/*****************************************************************************/
/* Set default page size.                                                    */
/*****************************************************************************/
void
gl_prefs_model_set_default_page_size (glPrefsModel     *this,
                                      const gchar      *page_size)
{
	gconf_client_set_string (this->priv->gconf_client,
				 BASE_KEY PREF_DEFAULT_PAGE_SIZE,
				 page_size,
				 NULL);
}


/*****************************************************************************/
/* Get default page size.                                                    */
/*****************************************************************************/
gchar *
gl_prefs_model_get_default_page_size (glPrefsModel     *this)
{
        const gchar *pgsize;
        const gchar *default_page_size;
        gchar       *page_size;
	lglPaper    *paper;

        /* Make educated guess about locale default. */
        pgsize = gtk_paper_size_get_default ();
        if ( strcmp (pgsize, GTK_PAPER_NAME_LETTER) == 0 )
        {
                default_page_size = DEFAULT_PAGE_SIZE_US;
        }
        else
        {
                default_page_size = DEFAULT_PAGE_SIZE_METRIC;
        }

	page_size = get_string (this->priv->gconf_client,
                                BASE_KEY PREF_DEFAULT_PAGE_SIZE,
                                default_page_size);

	/* Proof read the default page size -- it must be a valid id. */
	/* (For compatability with older versions.) */
	paper = lgl_db_lookup_paper_from_id (page_size);
	if ( paper == NULL ) {
                g_free (page_size);
		page_size = g_strdup (DEFAULT_PAGE_SIZE_US);
	} else {
		lgl_paper_free (paper);
	}

        return page_size;
}


/*****************************************************************************/
/* Set default font family.                                                  */
/*****************************************************************************/
void
gl_prefs_model_set_default_font_family (glPrefsModel     *this,
                                        const gchar      *family)
{
	gconf_client_set_string (this->priv->gconf_client,
				 BASE_KEY PREF_DEFAULT_FONT_FAMILY,
				 family,
				 NULL);
}


/*****************************************************************************/
/* Get default font family.                                                  */
/*****************************************************************************/
gchar *
gl_prefs_model_get_default_font_family (glPrefsModel     *this)
{
        gchar *family;

        family = get_string (this->priv->gconf_client,
                             BASE_KEY PREF_DEFAULT_FONT_FAMILY,
                             DEFAULT_FONT_FAMILY);

        return family;
}


/*****************************************************************************/
/* Set default font size.                                                    */
/*****************************************************************************/
void
gl_prefs_model_set_default_font_size (glPrefsModel     *this,
                                      gdouble           size)
{
	gconf_client_set_float (this->priv->gconf_client,
                                BASE_KEY PREF_DEFAULT_FONT_SIZE,
                                size,
                                NULL);
}


/*****************************************************************************/
/* Get default font size.                                                    */
/*****************************************************************************/
gdouble
gl_prefs_model_get_default_font_size (glPrefsModel     *this)
{
        gdouble size;

	size = get_float (this->priv->gconf_client,
                          BASE_KEY PREF_DEFAULT_FONT_SIZE,
                          DEFAULT_FONT_SIZE);

        return size;
}


/*****************************************************************************/
/* Set default font weight.                                                  */
/*****************************************************************************/
void
gl_prefs_model_set_default_font_weight (glPrefsModel     *this,
                                        PangoWeight       weight)
{
	gconf_client_set_string (this->priv->gconf_client,
				 BASE_KEY PREF_DEFAULT_FONT_WEIGHT,
				 gl_str_util_weight_to_string(weight),
				 NULL);
}


/*****************************************************************************/
/* Get default font weight.                                                  */
/*****************************************************************************/
PangoWeight
gl_prefs_model_get_default_font_weight (glPrefsModel     *this)
{
        gchar       *string;
        PangoWeight  weight;

	string = get_string (this->priv->gconf_client,
                             BASE_KEY PREF_DEFAULT_FONT_WEIGHT,
                             DEFAULT_FONT_WEIGHT_STRING);
	weight = gl_str_util_string_to_weight (string);
	g_free (string);

        return weight;
}


/*****************************************************************************/
/* Set default font italic flag.                                             */
/*****************************************************************************/
void
gl_prefs_model_set_default_font_italic_flag (glPrefsModel     *this,
                                             gboolean          italic_flag)
{
	gconf_client_set_bool (this->priv->gconf_client,
			       BASE_KEY PREF_DEFAULT_FONT_ITALIC_FLAG,
			       italic_flag,
			       NULL);
}


/*****************************************************************************/
/* Get default font italic flag.                                             */
/*****************************************************************************/
gboolean
gl_prefs_model_get_default_font_italic_flag (glPrefsModel     *this)
{
        gboolean italic_flag;

	italic_flag = get_bool (this->priv->gconf_client,
                                BASE_KEY PREF_DEFAULT_FONT_ITALIC_FLAG,
                                DEFAULT_FONT_ITALIC_FLAG);

        return italic_flag;
}


/*****************************************************************************/
/* Set default text color.                                                   */
/*****************************************************************************/
void
gl_prefs_model_set_default_text_color (glPrefsModel     *this,
                                       guint             color)
{
	gconf_client_set_int    (this->priv->gconf_client,
				 BASE_KEY PREF_DEFAULT_TEXT_COLOR,
				 color,
				 NULL);
}


/*****************************************************************************/
/* Get default text color.                                                   */
/*****************************************************************************/
guint
gl_prefs_model_get_default_text_color (glPrefsModel     *this)
{
        guint color;

	color =	get_int (this->priv->gconf_client,
			 BASE_KEY PREF_DEFAULT_TEXT_COLOR,
			 DEFAULT_TEXT_COLOR);

        return color;
}


/*****************************************************************************/
/* Set default text alignment.                                               */
/*****************************************************************************/
void
gl_prefs_model_set_default_text_alignment (glPrefsModel     *this,
                                           PangoAlignment    alignment)
{
	gconf_client_set_string (this->priv->gconf_client,
				 BASE_KEY PREF_DEFAULT_TEXT_ALIGNMENT,
				 gl_str_util_align_to_string(alignment),
				 NULL);
}


/*****************************************************************************/
/* Get default text alignment.                                               */
/*****************************************************************************/
PangoAlignment
gl_prefs_model_get_default_text_alignment (glPrefsModel     *this)
{
        gchar          *string;
        PangoAlignment  alignment;

	string = get_string (this->priv->gconf_client,
                             BASE_KEY PREF_DEFAULT_TEXT_ALIGNMENT,
                             DEFAULT_TEXT_ALIGN_STRING);
	alignment = gl_str_util_string_to_align (string);
	g_free (string);

        return alignment;
}


/*****************************************************************************/
/* Set default text line spacing.                                            */
/*****************************************************************************/
void
gl_prefs_model_set_default_text_line_spacing (glPrefsModel     *this,
                                              gdouble           spacing)
{
	gconf_client_set_float  (this->priv->gconf_client,
				 BASE_KEY PREF_DEFAULT_TEXT_LINE_SPACING,
				 spacing,
				 NULL);
}


/*****************************************************************************/
/* Get default text line spacing.                                            */
/*****************************************************************************/
gdouble
gl_prefs_model_get_default_text_line_spacing (glPrefsModel     *this)
{
        gdouble spacing;

	spacing = get_float (this->priv->gconf_client,
                             BASE_KEY PREF_DEFAULT_TEXT_LINE_SPACING,
                             DEFAULT_TEXT_LINE_SPACING);

        return spacing;
}


/*****************************************************************************/
/* Set default line width.                                                   */
/*****************************************************************************/
void
gl_prefs_model_set_default_line_width (glPrefsModel     *this,
                                       gdouble           width)
{
	gconf_client_set_float  (this->priv->gconf_client,
				 BASE_KEY PREF_DEFAULT_LINE_WIDTH,
				 width,
				 NULL);
}


/*****************************************************************************/
/* Get default line width.                                                   */
/*****************************************************************************/
gdouble
gl_prefs_model_get_default_line_width (glPrefsModel     *this)
{
        gdouble width;

	width = get_float (this->priv->gconf_client,
			   BASE_KEY PREF_DEFAULT_LINE_WIDTH,
			   DEFAULT_LINE_WIDTH);

        return width;
}


/*****************************************************************************/
/* Set default line color.                                                   */
/*****************************************************************************/
void
gl_prefs_model_set_default_line_color (glPrefsModel     *this,
                                       guint             color)
{
	gconf_client_set_int (this->priv->gconf_client,
                              BASE_KEY PREF_DEFAULT_LINE_COLOR,
                              color,
                              NULL);
}


/*****************************************************************************/
/* Get default line color.                                                   */
/*****************************************************************************/
guint
gl_prefs_model_get_default_line_color (glPrefsModel     *this)
{
        guint color;

	color = get_int (this->priv->gconf_client,
			 BASE_KEY PREF_DEFAULT_LINE_COLOR,
			 DEFAULT_LINE_COLOR);

        return color;
}


/*****************************************************************************/
/* Set default fill color.                                                   */
/*****************************************************************************/
void
gl_prefs_model_set_default_fill_color (glPrefsModel     *this,
                                       guint             color)
{
	gconf_client_set_int (this->priv->gconf_client,
                              BASE_KEY PREF_DEFAULT_FILL_COLOR,
                              color,
                              NULL);
}


/*****************************************************************************/
/* Get default fill color.                                                   */
/*****************************************************************************/
guint
gl_prefs_model_get_default_fill_color (glPrefsModel     *this)
{
        guint color;

	color = get_int (this->priv->gconf_client,
			 BASE_KEY PREF_DEFAULT_FILL_COLOR,
			 DEFAULT_FILL_COLOR);

        return color;
}


/*****************************************************************************/
/* Set main toolbar visible flag.                                            */
/*****************************************************************************/
void
gl_prefs_model_set_main_toolbar_visible (glPrefsModel     *this,
                                         gboolean          visible)
{
	gconf_client_set_bool (this->priv->gconf_client,
			       BASE_KEY PREF_MAIN_TOOLBAR_VISIBLE,
			       visible,
			       NULL);
}


/*****************************************************************************/
/* Get main toolbar visible flag.                                            */
/*****************************************************************************/
gboolean
gl_prefs_model_get_main_toolbar_visible (glPrefsModel     *this)
{
        gboolean visible;

	visible = get_bool (this->priv->gconf_client,
                            BASE_KEY PREF_MAIN_TOOLBAR_VISIBLE,
                            TRUE);

        return visible;
}


/*****************************************************************************/
/* Set drawing toolbar visible flag.                                         */
/*****************************************************************************/
void
gl_prefs_model_set_drawing_toolbar_visible (glPrefsModel     *this,
                                            gboolean          visible)
{
	gconf_client_set_bool (this->priv->gconf_client,
			       BASE_KEY PREF_DRAWING_TOOLBAR_VISIBLE,
			       visible,
			       NULL);
}


/*****************************************************************************/
/* Get drawing toolbar visible flag.                                         */
/*****************************************************************************/
gboolean
gl_prefs_model_get_drawing_toolbar_visible (glPrefsModel     *this)
{
        gboolean visible;

	visible = get_bool (this->priv->gconf_client,
                            BASE_KEY PREF_DRAWING_TOOLBAR_VISIBLE,
                            TRUE);

        return visible;
}


/*****************************************************************************/
/* Set property toolbar visible flag.                                        */
/*****************************************************************************/
void
gl_prefs_model_set_property_toolbar_visible (glPrefsModel     *this,
                                             gboolean          visible)
{
	gconf_client_set_bool (this->priv->gconf_client,
			       BASE_KEY PREF_PROPERTY_TOOLBAR_VISIBLE,
			       visible,
			       NULL);
}


/*****************************************************************************/
/* Get property toolbar visible flag.                                        */
/*****************************************************************************/
gboolean
gl_prefs_model_get_property_toolbar_visible (glPrefsModel     *this)
{
        gboolean visible;

	visible = get_bool (this->priv->gconf_client,
                            BASE_KEY PREF_PROPERTY_TOOLBAR_VISIBLE,
                            TRUE);

        return visible;
}


/*****************************************************************************/
/* Set grid visible flag.                                                    */
/*****************************************************************************/
void
gl_prefs_model_set_grid_visible (glPrefsModel     *this,
                                 gboolean          visible)
{
	gconf_client_set_bool (this->priv->gconf_client,
			       BASE_KEY PREF_GRID_VISIBLE,
			       visible,
			       NULL);
}


/*****************************************************************************/
/* Get grid visible flag.                                                    */
/*****************************************************************************/
gboolean
gl_prefs_model_get_grid_visible (glPrefsModel     *this)
{
        gboolean visible;

	visible = get_bool (this->priv->gconf_client,
                            BASE_KEY PREF_GRID_VISIBLE,
                            TRUE);

        return visible;
}


/*****************************************************************************/
/* Set markup visible flag.                                                  */
/*****************************************************************************/
void
gl_prefs_model_set_markup_visible (glPrefsModel     *this,
                                   gboolean          visible)
{
	gconf_client_set_bool (this->priv->gconf_client,
			       BASE_KEY PREF_MARKUP_VISIBLE,
			       visible,
			       NULL);
}


/*****************************************************************************/
/* Get markup visible flag.                                                  */
/*****************************************************************************/
gboolean
gl_prefs_model_get_markup_visible (glPrefsModel     *this)
{
        gboolean visible;

	visible = get_bool (this->priv->gconf_client,
                            BASE_KEY PREF_MARKUP_VISIBLE,
                            TRUE);

        return visible;
}


/*****************************************************************************/
/* Set max recents.                                                          */
/*****************************************************************************/
void
gl_prefs_model_set_max_recents (glPrefsModel     *this,
                                gint              max_recents)
{
	gconf_client_set_int (this->priv->gconf_client,
			      BASE_KEY PREF_MAX_RECENTS,
			      max_recents,
			      NULL);
}


/*****************************************************************************/
/* Get max recents.                                                          */
/*****************************************************************************/
gint
gl_prefs_model_get_max_recents (glPrefsModel     *this)
{
        gint max_recents;

	max_recents = get_int (this->priv->gconf_client,
                               BASE_KEY PREF_MAX_RECENTS,
                               -1);

        return max_recents;
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Key changed callback.                                           */
/*---------------------------------------------------------------------------*/
static void 
notify_cb (GConfClient  *client,
	   guint         cnxn_id,
	   GConfEntry   *entry,
	   glPrefsModel *this)
{
	gl_debug (DEBUG_PREFS, "Key was changed: %s", entry->key);

	g_signal_emit (G_OBJECT(this), signals[CHANGED], 0);
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





/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
