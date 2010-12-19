/*
 *  font-history-model.c
 *  Copyright (C) 2009  Jim Evins <evins@snaught.com>.
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

#include "font-history-model.h"

#include <gio/gio.h>

#include <libglabels.h>
#include "font-util.h"
#include "marshal.h"


/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glFontHistoryModelPrivate {

        GSettings   *history;

        guint        max_n;
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

static void gl_font_history_model_finalize      (GObject             *object);

static void history_changed_cb                  (glFontHistoryModel  *this);


/*****************************************************************************/
/* Object infrastructure.                                                    */
/*****************************************************************************/
G_DEFINE_TYPE (glFontHistoryModel, gl_font_history_model, G_TYPE_OBJECT)


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
static void
gl_font_history_model_class_init (glFontHistoryModelClass *class)
{
        GObjectClass  *gobject_class = (GObjectClass *) class;

        gl_font_history_model_parent_class = g_type_class_peek_parent (class);

        gobject_class->finalize = gl_font_history_model_finalize;

        signals[CHANGED] =
                g_signal_new ("changed",
                              G_OBJECT_CLASS_TYPE (gobject_class),
                              G_SIGNAL_RUN_LAST,
                              G_STRUCT_OFFSET (glFontHistoryModelClass, changed),
                              NULL, NULL,
                              gl_marshal_VOID__VOID,
                              G_TYPE_NONE,
                              0);
}


/*****************************************************************************/
/* Object Instance Init Function.                                            */
/*****************************************************************************/
static void
gl_font_history_model_init (glFontHistoryModel *this)
{
        this->priv = g_new0 (glFontHistoryModelPrivate, 1);

        this->priv->history = g_settings_new ("org.gnome.glabels-3.history");

        g_return_if_fail (this->priv->history != NULL);

        g_signal_connect_swapped (G_OBJECT (this->priv->history),
                                  "changed::recent-fonts",
                                  G_CALLBACK (history_changed_cb), this);
}


/*****************************************************************************/
/* Finalize Method.                                                          */
/*****************************************************************************/
static void
gl_font_history_model_finalize (GObject *object)
{
        glFontHistoryModel    *this;

        g_return_if_fail (object && IS_GL_FONT_HISTORY_MODEL (object));
        this = GL_FONT_HISTORY_MODEL (object);

        g_object_unref (G_OBJECT(this->priv->history));
        g_free (this->priv);

        G_OBJECT_CLASS (gl_font_history_model_parent_class)->finalize (object);
}


/*****************************************************************************/
/** New Object Generator.                                                    */
/*****************************************************************************/
glFontHistoryModel *
gl_font_history_model_new (guint n)
{
        glFontHistoryModel *this;

        this = g_object_new (TYPE_GL_FONT_HISTORY_MODEL, NULL);

        this->priv->max_n = n;

        return this;
}


/*****************************************************************************/
/* Add font to history.                                                      */
/*****************************************************************************/
void
gl_font_history_model_add_family (glFontHistoryModel *this,
                                  const gchar        *family)
{
        gchar **old;
        gchar **new;
        gint    i, j;

        old = g_settings_get_strv (this->priv->history, "recent-fonts");
                                   
        new = g_new0 (gchar *, this->priv->max_n+1);

        /* Put in first slot. */
        new[0] = g_strdup (family);

        /* Push everthing else down, pruning any duplicate found. */
        for ( i = 0, j = 1; (j < (this->priv->max_n-1)) && old[i]; i++ )
        {
                if ( lgl_str_utf8_casecmp (family, old[i]) != 0 )
                {
                        new[j++] = g_strdup (old[i]);
                }
        }

        g_settings_set_strv (this->priv->history, "recent-fonts",
                             (const gchar * const *)new);

        g_strfreev (old);
        g_strfreev (new);
}


/*****************************************************************************/
/* History changed callback.                                                 */
/*****************************************************************************/
static void
history_changed_cb                  (glFontHistoryModel  *this)
{
        g_signal_emit (G_OBJECT(this), signals[CHANGED], 0);
}


/*****************************************************************************/
/* Get list of font families.                                                */
/*****************************************************************************/
GList *
gl_font_history_model_get_family_list (glFontHistoryModel *this)
{
        gchar **strv;
        GList  *list = NULL;
        gint    i;

        strv = g_settings_get_strv (this->priv->history, "recent-fonts");

        /*
         * Proof read name list; transfer storage to new list.
         */
        for ( i = 0; strv[i]; i++ )
        {
                if ( gl_font_util_is_family_installed (strv[i]) )
                {
                        list = g_list_append (list, g_strdup (strv[i]));
                }
        }
        g_strfreev (strv);

        return list;
}


/*****************************************************************************/
/* Free font family list.                                                    */
/*****************************************************************************/
void
gl_font_history_model_free_family_list (GList *list)
{
        GList *p;

        for ( p = list; p; p=p->next )
        {
                g_free (p->data);
        }
        g_list_free (list);
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
