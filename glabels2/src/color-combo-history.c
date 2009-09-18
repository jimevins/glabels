/*
 *  color-combo-history.c
 *  Copyright (C) 2008  Jim Evins <evins@snaught.com>.
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

#include "color-combo-history.h"

#include "color.h"


/*========================================================*/
/* Private types.                                         */
/*========================================================*/

/** GL_COLOR_COMBO_HISTORY Private fields */
struct _glColorComboHistoryPrivate {

        guint       max_n;

        guint       n;
        guint      *color;
};


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void gl_color_combo_history_finalize      (GObject             *object);


/*****************************************************************************/
/* Object infrastructure.                                                    */
/*****************************************************************************/
G_DEFINE_TYPE (glColorComboHistory, gl_color_combo_history, G_TYPE_OBJECT);


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
static void
gl_color_combo_history_class_init (glColorComboHistoryClass *class)
{
        GObjectClass              *gobject_class = (GObjectClass *) class;

        gl_color_combo_history_parent_class = g_type_class_peek_parent (class);

        gobject_class->finalize = gl_color_combo_history_finalize;
}


/*****************************************************************************/
/* Object Instance Init Function.                                            */
/*****************************************************************************/
static void
gl_color_combo_history_init (glColorComboHistory *this)
{
        this->priv = g_new0 (glColorComboHistoryPrivate, 1);
}


/*****************************************************************************/
/* Finalize Method.                                                          */
/*****************************************************************************/
static void
gl_color_combo_history_finalize (GObject *object)
{
        glColorComboHistory    *this;

        g_return_if_fail (object && IS_GL_COLOR_COMBO_HISTORY (object));
        this = GL_COLOR_COMBO_HISTORY (object);

        g_free (this->priv->color);
        g_free (this->priv);

        G_OBJECT_CLASS (gl_color_combo_history_parent_class)->finalize (object);
}


/*****************************************************************************/
/** New Object Generator.                                                    */
/*****************************************************************************/
glColorComboHistory *
gl_color_combo_history_new (guint n)
{
        glColorComboHistory *this;

        this = g_object_new (TYPE_GL_COLOR_COMBO_HISTORY, NULL);

        this->priv->max_n = n;
        this->priv->n     = 0;
        if (n > 0)
        {
                this->priv->color = g_new0 (guint, n);
        }

        return this;
}


/*****************************************************************************/
/* Add color to history.                                                     */
/*****************************************************************************/
void
gl_color_combo_history_add_color (glColorComboHistory *this,
                                  guint                color)
{
        guint i;

        /*
         * First check for duplicate color.
         */
        for ( i=0; i < this->priv->n; i++ )
        {
                if ( this->priv->color[i] == color )
                {
                        return;
                }
        }

        /*
         * Simple case.
         */
        if ( this->priv->n < this->priv->max_n )
        {
                this->priv->color[ this->priv->n ] = color;
                this->priv->n++;

                return;
        }

        /* Move colors down, dropping oldest. */
        for ( i=0; i < (this->priv->n - 1); i++ )
        {
                this->priv->color[i] = this->priv->color[i+1];
        }
        this->priv->color[ this->priv->n - 1 ] = color;
}

/*****************************************************************************/
/* Get color.                                                                */
/*****************************************************************************/
guint
gl_color_combo_history_get_color (glColorComboHistory *this,
                                  guint                i)
{
        return this->priv->color[i];
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
