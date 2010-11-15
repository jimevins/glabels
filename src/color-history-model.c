/*
 *  color-history-model.c
 *  Copyright (C) 2008-2009  Jim Evins <evins@snaught.com>.
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

#include "color-history-model.h"

#include <gio/gio.h>

#include "marshal.h"


/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glColorHistoryModelPrivate {

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

static void    gl_color_history_model_finalize  (GObject             *object);

static void    history_changed_cb               (glColorHistoryModel *this);

static guint  *get_color_array                  (glColorHistoryModel *this,
                                                 guint               *n_elements);
static void    set_color_array                  (glColorHistoryModel *this,
                                                 guint               *array,
                                                 guint                n_elements);


/*****************************************************************************/
/* Object infrastructure.                                                    */
/*****************************************************************************/
G_DEFINE_TYPE (glColorHistoryModel, gl_color_history_model, G_TYPE_OBJECT);


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
static void
gl_color_history_model_class_init (glColorHistoryModelClass *class)
{
        GObjectClass  *gobject_class = (GObjectClass *) class;

        gl_color_history_model_parent_class = g_type_class_peek_parent (class);

        gobject_class->finalize = gl_color_history_model_finalize;

        signals[CHANGED] =
                g_signal_new ("changed",
                              G_OBJECT_CLASS_TYPE (gobject_class),
                              G_SIGNAL_RUN_LAST,
                              G_STRUCT_OFFSET (glColorHistoryModelClass, changed),
                              NULL, NULL,
                              gl_marshal_VOID__VOID,
                              G_TYPE_NONE,
                              0);
}


/*****************************************************************************/
/* Object Instance Init Function.                                            */
/*****************************************************************************/
static void
gl_color_history_model_init (glColorHistoryModel *this)
{
        this->priv = g_new0 (glColorHistoryModelPrivate, 1);

        this->priv->history = g_settings_new ("org.gnome.glabels-3.history");

        g_return_if_fail (this->priv->history != NULL);

        g_signal_connect_swapped (G_OBJECT (this->priv->history),
                                  "changed::recent-colors",
                                  G_CALLBACK (history_changed_cb), this);
}


/*****************************************************************************/
/* Finalize Method.                                                          */
/*****************************************************************************/
static void
gl_color_history_model_finalize (GObject *object)
{
        glColorHistoryModel   *this;

        g_return_if_fail (object && IS_GL_COLOR_HISTORY_MODEL (object));
        this = GL_COLOR_HISTORY_MODEL (object);

        g_object_unref (G_OBJECT(this->priv->history));
        g_free (this->priv);

        G_OBJECT_CLASS (gl_color_history_model_parent_class)->finalize (object);
}


/*****************************************************************************/
/** New Object Generator.                                                    */
/*****************************************************************************/
glColorHistoryModel *
gl_color_history_model_new (guint n)
{
        glColorHistoryModel *this;

        this = g_object_new (TYPE_GL_COLOR_HISTORY_MODEL, NULL);

        this->priv->max_n = n;

        return this;
}


/*****************************************************************************/
/* Add color to history.                                                      */
/*****************************************************************************/
void
gl_color_history_model_add_color (glColorHistoryModel *this,
                                  guint                color)
{
        guint  *old;
        guint  *new;
        guint   i, n;

        old = get_color_array (this, &n);
                                   
        new = g_new0 (guint, this->priv->max_n);

        new[0] = color;

        for ( i = 0; (i < (this->priv->max_n-1)) && (i < n); i++ )
        {
                new[i+1] = old[i];
        }

        set_color_array (this, new, i+1);

        g_free (old);
        g_free (new);
}


/*****************************************************************************/
/* History changed callback.                                                 */
/*****************************************************************************/
static void
history_changed_cb (glColorHistoryModel  *this)
{
        g_signal_emit (G_OBJECT(this), signals[CHANGED], 0);
}


/*****************************************************************************/
/* Get list of colors.                                                       */
/*****************************************************************************/
static guint *
get_color_array (glColorHistoryModel *this,
                 guint               *n_elements)
{
        GVariant *value;
        GVariant *child_value;
        gsize     i;
        guint    *array;

        value = g_settings_get_value (this->priv->history, "recent-colors");
        *n_elements = g_variant_n_children (value);

        array = g_new0 (guint, *n_elements);

        for ( i = 0; i < *n_elements; i++ )
        {
                child_value = g_variant_get_child_value (value, i);
                array[i] = g_variant_get_uint32 (child_value);
                g_variant_unref (child_value);
        }

        g_variant_unref (value);

        return array;
}


/*****************************************************************************/
/* Set list of colors.                                                       */
/*****************************************************************************/
static void
set_color_array (glColorHistoryModel *this,
                 guint               *array,
                 guint                n_elements)
{
        GVariant  *value;
        GVariant **child_values;
        gsize      i;

        child_values = g_new (GVariant *, n_elements);

        for ( i = 0; i < n_elements; i++ )
        {
                child_values[i] = g_variant_new_uint32 (array[i]);
        }

        value = g_variant_new_array (G_VARIANT_TYPE_UINT32, child_values, n_elements);

        g_settings_set_value (this->priv->history, "recent-colors", value);

        g_free (child_values);
}


/*****************************************************************************/
/* Get color.                                                                */
/*****************************************************************************/
guint
gl_color_history_model_get_color (glColorHistoryModel *this,
                                  guint                i)
{
        guint  *array;
        guint   color = 0;
        guint   n;

        array = get_color_array (this, &n);
        if ( array && (i < n) )
        {
                color = array[i];
        }
        g_free (array);

        return color;
}




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
