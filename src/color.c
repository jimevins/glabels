/*
 *  color.c
 *  Copyright (C) 2002-2009  Jim Evins <evins@snaught.com>.
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

#include "color.h"

#include <string.h>


/*****************************************************************************/
/* Apply given opacity to given color.                                       */
/*****************************************************************************/
guint
gl_color_set_opacity (guint   color,
                      gdouble opacity)
{
        guint new_color;

        new_color = (color & 0xFFFFFF00) | (((guint)(255.0*opacity)) & 0xFF);

        return new_color;
}



/*****************************************************************************/
/* Convert gLabels color into a GdkColor                                     */
/*****************************************************************************/
GdkColor *
gl_color_to_gdk_color (guint color)
{
        GdkColor *gdk_color;

        gdk_color = g_new0 (GdkColor, 1);

        gdk_color->red   = GL_COLOR_F_RED   (color) * 65535;
        gdk_color->green = GL_COLOR_F_GREEN (color) * 65535;
        gdk_color->blue  = GL_COLOR_F_BLUE  (color) * 65535;

        return gdk_color;
}


/*****************************************************************************/
/* Convert GdkColor into a gLabels color                                     */
/*****************************************************************************/
guint
gl_color_from_gdk_color (GdkColor *gdk_color)
{
        guint color;

        color = GL_COLOR ((gdk_color->red   >>8),
                          (gdk_color->green >>8),
                          (gdk_color->blue  >>8));

        return color;
}


/****************************************************************************/
/* Create a single color node with default color.                           */
/****************************************************************************/
glColorNode *
gl_color_node_new_default (void)
{
        glColorNode* color_node;
        
        color_node = g_new0(glColorNode,1);
        
        color_node->field_flag = FALSE;
        color_node->color = GL_COLOR_NONE;
        color_node->key = NULL;

        return color_node;
}


/****************************************************************************/
/* Copy a single color node.                                                 */
/****************************************************************************/
glColorNode *
gl_color_node_dup (glColorNode *src)
{
        glColorNode *dst;

        if ( src == NULL ) return NULL;

        dst = g_new0 (glColorNode, 1);

        dst->field_flag = src->field_flag;
        if (src->key != NULL)
        {
                dst->key = g_strdup (src->key);
        }
        else
        {
                dst->key = NULL;
        }
        dst->color = src->color;

        return dst;
}


/****************************************************************************/
/* Compare 2 color nodes for equality.                                       */
/****************************************************************************/
gboolean
gl_color_node_equal (glColorNode     *color_node1,
                     glColorNode     *color_node2)
{
        /* First take care of the case of either or both being NULL. */
        if ( color_node1 == NULL )
        {
                return ( color_node2 == NULL );
        }
        else
        {
                if ( color_node2 == NULL )
                {
                        return FALSE;
                }
        }

        /* Bail if field flags differ. */
        if ( color_node1->field_flag != color_node2->field_flag )
        {
                return FALSE;
        }

        /* Now take care of the case of either or both color fields being different. */
        if ( color_node1->color != color_node2->color )
        {
                return FALSE;
        }
        
        /* Then take care of the case of either or both key fields being NULL. */
        if ( color_node1->key == NULL )
        {
                return ( color_node2->key == NULL );
        }
        else
        {
                if ( color_node2->key == NULL )
                {
                        return FALSE;
                }
        }

        /* Field flags are identical, so now compare the keys. */
        return (strcmp (color_node1->key, color_node2->key) == 0);
}


/****************************************************************************/
/* Expand single node into representative color.                            */
/****************************************************************************/
guint
gl_color_node_expand (glColorNode    *color_node,
                      glMergeRecord  *record)
{
        gchar    *text;
        GdkColor *gdk_color;
        guint     color;

        if (color_node->field_flag)
        {
                if (record == NULL)
                {
                        return GL_COLOR_NONE;
                }
                else
                {
                        text = gl_merge_eval_key (record, color_node->key);
                        if (text != NULL)
                        {
                                gdk_color = g_new0 (GdkColor, 1);
                                if (gdk_color_parse (text, gdk_color))
                                {
                                        color = gl_color_from_gdk_color (gdk_color);
                                        g_free (gdk_color);
                                        return color;
                                }
                                else
                                {
                                        g_free (gdk_color);
                                        return GL_COLOR_NONE;
                                }
                        }
                        else
                        {
                                return GL_COLOR_NONE;
                        }
                }
        }
        else
        {
                return color_node->color;
        }
}


/****************************************************************************/
/* Free a single color node.                                                */
/****************************************************************************/
void
gl_color_node_free (glColorNode **color_node)
{
        if ( *color_node == NULL ) return;

        g_free ((*color_node)->key);
        (*color_node)->key = NULL;
        g_free (*color_node);
        *color_node = NULL;
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
