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

#include <gconf/gconf-client.h>

#include "marshal.h"


#define BASE_KEY          "/apps/glabels"
#define RECENT_COLORS_KEY  BASE_KEY "/recent-colors"


/*========================================================*/
/* Private types.                                         */
/*========================================================*/

/** GL_COLOR_HISTORY_MODEL Private fields */
struct _glColorHistoryModelPrivate {

	GConfClient *gconf_client;

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

static void gl_color_history_model_finalize     (GObject             *object);

static void conf_notify_cb                      (GConfClient         *client,
                                                 guint                cnxn_id,
                                                 GConfEntry          *entry,
                                                 glColorHistoryModel *this);

static GSList *get_color_list                   (glColorHistoryModel *this);


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

        this->priv->gconf_client = gconf_client_get_default ();

        g_return_if_fail (this->priv->gconf_client != NULL);

        gconf_client_add_dir (this->priv->gconf_client,
                              BASE_KEY,
                              GCONF_CLIENT_PRELOAD_ONELEVEL,
                              NULL);

        gconf_client_notify_add (this->priv->gconf_client,
                                 RECENT_COLORS_KEY,
                                 (GConfClientNotifyFunc)conf_notify_cb, this,
                                 NULL, NULL);
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

        g_object_unref (G_OBJECT(this->priv->gconf_client));
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
        GSList  *list = NULL;
        GSList  *old_list;
        GSList  *p;

        /*
         * Start new list with this color.
         */
        list = g_slist_append (list, GINT_TO_POINTER (color));

        /*
         * Transfer old list to new list, ignoring any duplicate of this color
         */
        old_list = get_color_list (this);
        for ( p = old_list; p; p=p->next )
        {
                if ( color != (guint)GPOINTER_TO_INT (p->data) )
                {
                        list = g_slist_append (list, p->data);
                }
        }
        g_slist_free (old_list);

        /*
         * Truncate list to maximum size
         */
        while (g_slist_length (list) > this->priv->max_n)
        {
                p = g_slist_last (list);
                list = g_slist_remove_link (list, p);
                g_slist_free_1 (p);
        }

        /*
         * Update conf
         */
        gconf_client_set_list (this->priv->gconf_client,
                               RECENT_COLORS_KEY,
                               GCONF_VALUE_INT,
                               list,
                               NULL);
}


/*****************************************************************************/
/* GConf notify callback.                                                    */
/*****************************************************************************/
static void
conf_notify_cb (GConfClient         *client,
                guint                cnxn_id,
                GConfEntry          *entry,
                glColorHistoryModel  *this)
{
        g_signal_emit (G_OBJECT(this), signals[CHANGED], 0);
}


/*****************************************************************************/
/* Get list of colors.                                                       */
/*****************************************************************************/
static GSList *
get_color_list (glColorHistoryModel *this)
{
        GSList *list;

        /*
         * Get color list.
         */
	list = gconf_client_get_list (this->priv->gconf_client,
                                      RECENT_COLORS_KEY,
                                      GCONF_VALUE_INT,
                                      NULL);
        return list;
}


/*****************************************************************************/
/* Get color.                                                                */
/*****************************************************************************/
guint
gl_color_history_model_get_color (glColorHistoryModel *this,
                                  guint                i)
{
        guint   color = 0;
        GSList *list;
        GSList *p;

        list = get_color_list (this);
        p = g_slist_nth (list, i);
        if (p)
        {
                color = GPOINTER_TO_INT (p->data);
        }
        g_slist_free (list);

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
