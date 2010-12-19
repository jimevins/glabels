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

#include <gio/gio.h>
#include <gtk/gtk.h>
#include <string.h>

#include <libglabels.h>
#include "marshal.h"
#include "str-util.h"
#include "color.h"

#include "debug.h"


/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

/* Default values */
#define DEFAULT_UNITS_STRING_US     lgl_units_get_id (LGL_UNITS_INCH)
#define DEFAULT_PAGE_SIZE_US        "US-Letter"

#define DEFAULT_UNITS_STRING_METRIC lgl_units_get_id (LGL_UNITS_MM)
#define DEFAULT_PAGE_SIZE_METRIC    "A4"


/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glPrefsModelPrivate {

        GSettings *locale;
        GSettings *objects;
        GSettings *ui;

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

static void           gsettings_changed_cb         (glPrefsModel        *prefs_model);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glPrefsModel, gl_prefs_model, G_TYPE_OBJECT)


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

        this->priv->locale  = g_settings_new ("org.gnome.glabels-3.locale");
        this->priv->objects = g_settings_new ("org.gnome.glabels-3.objects");
        this->priv->ui      = g_settings_new ("org.gnome.glabels-3.ui");

        g_return_if_fail (this->priv->locale != NULL);
        g_return_if_fail (this->priv->objects != NULL);
        g_return_if_fail (this->priv->ui != NULL);
 
        g_signal_connect_swapped (G_OBJECT (this->priv->locale), "changed",
                                  G_CALLBACK (gsettings_changed_cb), this);
        g_signal_connect_swapped (G_OBJECT (this->priv->objects), "changed",
                                  G_CALLBACK (gsettings_changed_cb), this);
        g_signal_connect_swapped (G_OBJECT (this->priv->ui), "changed",
                                  G_CALLBACK (gsettings_changed_cb), this);

        gl_debug (DEBUG_PREFS, "END");
}


static void
gl_prefs_model_finalize (GObject *object)
{
        glPrefsModel *this = GL_PREFS_MODEL (object);

        gl_debug (DEBUG_PREFS, "START");

        g_return_if_fail (object && GL_IS_PREFS_MODEL (object));

        g_object_unref (G_OBJECT(this->priv->locale));
        g_object_unref (G_OBJECT(this->priv->objects));
        g_object_unref (G_OBJECT(this->priv->ui));
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
        g_settings_set_string (this->priv->locale,
                               "units",
                               lgl_units_get_id (units));
}


/*****************************************************************************/
/* Get units.                                                                */
/*****************************************************************************/
lglUnits
gl_prefs_model_get_units (glPrefsModel     *this)
{
        const gchar  *pgsize;
        gchar        *string;
        lglUnits      units;

        string = g_settings_get_string (this->priv->locale, "units");
        
        /* If not set, make educated guess about locale default. */
        if ( !string || (*string == '\0') )
        {
                pgsize = gtk_paper_size_get_default ();
                if ( strcmp (pgsize, GTK_PAPER_NAME_LETTER) == 0 )
                {
                        string = g_strdup (DEFAULT_UNITS_STRING_US);
                }
                else
                {
                        string = g_strdup (DEFAULT_UNITS_STRING_METRIC);
                }
        }

        units = lgl_units_from_id (string);
        g_free (string);

        /* If invalid, make an educated guess from locale. */
        if (units == LGL_UNITS_INVALID)
        {
                pgsize = gtk_paper_size_get_default ();
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
        g_settings_set_string (this->priv->locale,
                               "default-page-size",
                               page_size);
}


/*****************************************************************************/
/* Get default page size.                                                    */
/*****************************************************************************/
gchar *
gl_prefs_model_get_default_page_size (glPrefsModel     *this)
{
        const gchar *pgsize;
        gchar       *page_size;
        lglPaper    *paper;

        page_size = g_settings_get_string (this->priv->locale, "default-page-size");

        /* If not set, make educated guess about locale default. */
        if ( !page_size || (*page_size == '\0') )
        {
                pgsize = gtk_paper_size_get_default ();
                if ( strcmp (pgsize, GTK_PAPER_NAME_LETTER) == 0 )
                {
                        page_size = g_strdup (DEFAULT_PAGE_SIZE_US);
                }
                else
                {
                        page_size = g_strdup (DEFAULT_PAGE_SIZE_METRIC);
                }
        }

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
        g_settings_set_string (this->priv->objects,
                               "default-font-family",
                               family);
}


/*****************************************************************************/
/* Get default font family.                                                  */
/*****************************************************************************/
gchar *
gl_prefs_model_get_default_font_family (glPrefsModel     *this)
{
        gchar *family;

        family = g_settings_get_string (this->priv->objects,
                                        "default-font-family");

        return family;
}


/*****************************************************************************/
/* Set default font size.                                                    */
/*****************************************************************************/
void
gl_prefs_model_set_default_font_size (glPrefsModel     *this,
                                      gdouble           size)
{
        g_settings_set_double (this->priv->objects,
                               "default-font-size",
                               size);
}


/*****************************************************************************/
/* Get default font size.                                                    */
/*****************************************************************************/
gdouble
gl_prefs_model_get_default_font_size (glPrefsModel     *this)
{
        gdouble size;

        size = g_settings_get_double (this->priv->objects,
                                      "default-font-size");

        return size;
}


/*****************************************************************************/
/* Set default font weight.                                                  */
/*****************************************************************************/
void
gl_prefs_model_set_default_font_weight (glPrefsModel     *this,
                                        PangoWeight       weight)
{
        g_settings_set_string (this->priv->objects,
                               "default-font-weight",
                               gl_str_util_weight_to_string(weight));
}


/*****************************************************************************/
/* Get default font weight.                                                  */
/*****************************************************************************/
PangoWeight
gl_prefs_model_get_default_font_weight (glPrefsModel     *this)
{
        gchar       *string;
        PangoWeight  weight;

        string = g_settings_get_string (this->priv->objects,
                                        "default-font-weight");
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
        g_settings_set_boolean (this->priv->objects,
                                "default-font-italic-flag",
                                italic_flag);
}


/*****************************************************************************/
/* Get default font italic flag.                                             */
/*****************************************************************************/
gboolean
gl_prefs_model_get_default_font_italic_flag (glPrefsModel     *this)
{
        gboolean italic_flag;

        italic_flag = g_settings_get_boolean (this->priv->objects,
                                              "default-font-italic-flag");

        return italic_flag;
}


/*****************************************************************************/
/* Set default text color.                                                   */
/*****************************************************************************/
void
gl_prefs_model_set_default_text_color (glPrefsModel     *this,
                                       guint             color)
{
        g_settings_set_value (this->priv->objects,
                              "default-text-color",
                              g_variant_new_uint32 (color));
}


/*****************************************************************************/
/* Get default text color.                                                   */
/*****************************************************************************/
guint
gl_prefs_model_get_default_text_color (glPrefsModel     *this)
{
        GVariant *value;
        guint     color;

        value = g_settings_get_value (this->priv->objects, "default-text-color");
        color = g_variant_get_uint32 (value);
        g_variant_unref (value);

        return color;
}


/*****************************************************************************/
/* Set default text alignment.                                               */
/*****************************************************************************/
void
gl_prefs_model_set_default_text_alignment (glPrefsModel     *this,
                                           PangoAlignment    alignment)
{
        g_settings_set_string (this->priv->objects,
                               "default-text-alignment",
                               gl_str_util_align_to_string(alignment));
}


/*****************************************************************************/
/* Get default text alignment.                                               */
/*****************************************************************************/
PangoAlignment
gl_prefs_model_get_default_text_alignment (glPrefsModel     *this)
{
        gchar          *string;
        PangoAlignment  alignment;

        string = g_settings_get_string (this->priv->objects,
                                        "default-text-alignment");
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
        g_settings_set_double (this->priv->objects,
                               "default-text-line-spacing",
                               spacing);
}


/*****************************************************************************/
/* Get default text line spacing.                                            */
/*****************************************************************************/
gdouble
gl_prefs_model_get_default_text_line_spacing (glPrefsModel     *this)
{
        gdouble spacing;

        spacing = g_settings_get_double (this->priv->objects,
                                         "default-text-line-spacing");

        return spacing;
}


/*****************************************************************************/
/* Set default line width.                                                   */
/*****************************************************************************/
void
gl_prefs_model_set_default_line_width (glPrefsModel     *this,
                                       gdouble           width)
{
        g_settings_set_double (this->priv->objects,
                               "default-line-width",
                               width);
}


/*****************************************************************************/
/* Get default line width.                                                   */
/*****************************************************************************/
gdouble
gl_prefs_model_get_default_line_width (glPrefsModel     *this)
{
        gdouble width;

        width = g_settings_get_double (this->priv->objects,
                                       "default-line-width");

        return width;
}


/*****************************************************************************/
/* Set default line color.                                                   */
/*****************************************************************************/
void
gl_prefs_model_set_default_line_color (glPrefsModel     *this,
                                       guint             color)
{
        g_settings_set_value (this->priv->objects,
                              "default-line-color",
                              g_variant_new_uint32 (color));
}


/*****************************************************************************/
/* Get default line color.                                                   */
/*****************************************************************************/
guint
gl_prefs_model_get_default_line_color (glPrefsModel     *this)
{
        GVariant *value;
        guint     color;

        value = g_settings_get_value (this->priv->objects, "default-line-color");
        color = g_variant_get_uint32 (value);
        g_variant_unref (value);

        return color;
}


/*****************************************************************************/
/* Set default fill color.                                                   */
/*****************************************************************************/
void
gl_prefs_model_set_default_fill_color (glPrefsModel     *this,
                                       guint             color)
{
        g_settings_set_value (this->priv->objects,
                              "default-fill-color",
                              g_variant_new_uint32 (color));
}


/*****************************************************************************/
/* Get default fill color.                                                   */
/*****************************************************************************/
guint
gl_prefs_model_get_default_fill_color (glPrefsModel     *this)
{
        GVariant *value;
        guint     color;

        value = g_settings_get_value (this->priv->objects, "default-fill-color");
        color = g_variant_get_uint32 (value);
        g_variant_unref (value);

        return color;
}


/*****************************************************************************/
/* Set main toolbar visible flag.                                            */
/*****************************************************************************/
void
gl_prefs_model_set_main_toolbar_visible (glPrefsModel     *this,
                                         gboolean          visible)
{
        g_settings_set_boolean (this->priv->ui,
                                "main-toolbar-visible",
                                visible);
}


/*****************************************************************************/
/* Get main toolbar visible flag.                                            */
/*****************************************************************************/
gboolean
gl_prefs_model_get_main_toolbar_visible (glPrefsModel     *this)
{
        gboolean visible;

        visible = g_settings_get_boolean (this->priv->ui,
                                          "main-toolbar-visible");

        return visible;
}


/*****************************************************************************/
/* Set drawing toolbar visible flag.                                         */
/*****************************************************************************/
void
gl_prefs_model_set_drawing_toolbar_visible (glPrefsModel     *this,
                                            gboolean          visible)
{
        g_settings_set_boolean (this->priv->ui,
                                "drawing-toolbar-visible",
                                visible);
}


/*****************************************************************************/
/* Get drawing toolbar visible flag.                                         */
/*****************************************************************************/
gboolean
gl_prefs_model_get_drawing_toolbar_visible (glPrefsModel     *this)
{
        gboolean visible;

        visible = g_settings_get_boolean (this->priv->ui,
                                          "drawing-toolbar-visible");

        return visible;
}


/*****************************************************************************/
/* Set property toolbar visible flag.                                        */
/*****************************************************************************/
void
gl_prefs_model_set_property_toolbar_visible (glPrefsModel     *this,
                                             gboolean          visible)
{
        g_settings_set_boolean (this->priv->ui,
                                "property-toolbar-visible",
                                visible);
}


/*****************************************************************************/
/* Get property toolbar visible flag.                                        */
/*****************************************************************************/
gboolean
gl_prefs_model_get_property_toolbar_visible (glPrefsModel     *this)
{
        gboolean visible;

        visible = g_settings_get_boolean (this->priv->ui,
                                          "property-toolbar-visible");

        return visible;
}


/*****************************************************************************/
/* Set grid visible flag.                                                    */
/*****************************************************************************/
void
gl_prefs_model_set_grid_visible (glPrefsModel     *this,
                                 gboolean          visible)
{
        g_settings_set_boolean (this->priv->ui,
                                "grid-visible",
                                visible);
}


/*****************************************************************************/
/* Get grid visible flag.                                                    */
/*****************************************************************************/
gboolean
gl_prefs_model_get_grid_visible (glPrefsModel     *this)
{
        gboolean visible;

        visible = g_settings_get_boolean (this->priv->ui,
                                          "grid-visible");

        return visible;
}


/*****************************************************************************/
/* Set markup visible flag.                                                  */
/*****************************************************************************/
void
gl_prefs_model_set_markup_visible (glPrefsModel     *this,
                                   gboolean          visible)
{
        g_settings_set_boolean (this->priv->ui,
                                "markup-visible",
                                visible);
}


/*****************************************************************************/
/* Get markup visible flag.                                                  */
/*****************************************************************************/
gboolean
gl_prefs_model_get_markup_visible (glPrefsModel     *this)
{
        gboolean visible;

        visible = g_settings_get_boolean (this->priv->ui,
                                          "markup-visible");

        return visible;
}


/*****************************************************************************/
/* Set max recents.                                                          */
/*****************************************************************************/
void
gl_prefs_model_set_max_recents (glPrefsModel     *this,
                                gint              max_recents)
{
        g_settings_set_int (this->priv->ui,
                            "max-recents",
                            max_recents);
}


/*****************************************************************************/
/* Get max recents.                                                          */
/*****************************************************************************/
gint
gl_prefs_model_get_max_recents (glPrefsModel     *this)
{
        gint max_recents;

        max_recents = g_settings_get_int (this->priv->ui,
                                          "max-recents");

        return max_recents;
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Key changed callback.                                           */
/*---------------------------------------------------------------------------*/
static void 
gsettings_changed_cb (glPrefsModel *this)
{
        g_signal_emit (G_OBJECT(this), signals[CHANGED], 0);
}




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
