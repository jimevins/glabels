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

#include <libglabels/libglabels.h>
#include "marshal.h"


/*========================================================*/
/* Private types.                                         */
/*========================================================*/

/** GL_FONT_HISTORY_MODEL Private fields */
struct _glFontHistoryModelPrivate {

        guint       max_n;

        GList      *family_list;
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


/*****************************************************************************/
/* Object infrastructure.                                                    */
/*****************************************************************************/
G_DEFINE_TYPE (glFontHistoryModel, gl_font_history_model, G_TYPE_OBJECT);


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
}


/*****************************************************************************/
/* Finalize Method.                                                          */
/*****************************************************************************/
static void
gl_font_history_model_finalize (GObject *object)
{
        glFontHistoryModel    *this;
        GList                 *p;

        g_return_if_fail (object && IS_GL_FONT_HISTORY_MODEL (object));
        this = GL_FONT_HISTORY_MODEL (object);

        for ( p = this->priv->family_list; p; p=p->next )
        {
                g_free (p->data);
        }
        g_list_free (this->priv->family_list);

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
        GList *p;

        /*
         * If already in list, remove that entry.
         */
        p = g_list_find_custom (this->priv->family_list,
                                family,
                                (GCompareFunc)lgl_str_utf8_casecmp);
        if (p)
        {
                this->priv->family_list =
                        g_list_remove_link (this->priv->family_list, p);
                g_free (p->data);
                g_list_free_1 (p);
        }

        /*
         * Now prepend to list.
         */
        this->priv->family_list =
                g_list_prepend (this->priv->family_list, g_strdup (family));

        /*
         * Truncate list to maximum size
         */
        while (g_list_length (this->priv->family_list) > this->priv->max_n)
        {
                p = g_list_last (this->priv->family_list);
                this->priv->family_list =
                        g_list_remove_link (this->priv->family_list, p);
                g_free (p->data);
                g_list_free_1 (p);
        }

        g_signal_emit (G_OBJECT(this), signals[CHANGED], 0);
}


/*****************************************************************************/
/* Get font.                                                                 */
/*****************************************************************************/
const GList *
gl_font_history_model_get_family_list (glFontHistoryModel *this)
{
        return this->priv->family_list;
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
