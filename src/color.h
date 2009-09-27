/*
 *  color.h
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

#ifndef __COLOR_H__
#define __COLOR_H__

#include "merge.h"
#include <gdk/gdk.h>

G_BEGIN_DECLS

/*
 * gLabels stores colors as a 32-bit unsigned integer in RGBA format (MSByte = Red)
 */
#define GL_COLOR(r,g,b)     (  (((unsigned int) (r) & 0xff) << 24) \
                             | (((unsigned int) (g) & 0xff) << 16) \
                             | (((unsigned int) (b) & 0xff) << 8)  \
                             | 0xff )

#define GL_COLOR_A(r,g,b,a) (  (((unsigned int) (r) & 0xff) << 24) \
                             | (((unsigned int) (g) & 0xff) << 16) \
                             | (((unsigned int) (b) & 0xff) << 8)  \
                             | ( (unsigned int) (a) & 0xff ) )


#define GL_COLOR_NONE                  GL_COLOR_A(0,0,0,0)
#define GL_COLOR_BLACK                 GL_COLOR_A(0,0,0,255)
#define GL_COLOR_WHITE                 GL_COLOR_A(255,255,255,255)

#define GL_COLOR_TEXT_DEFAULT          GL_COLOR_BLACK
#define GL_COLOR_BC_DEFAULT            GL_COLOR_BLACK
#define GL_COLOR_NO_LINE               GL_COLOR_NONE
#define GL_COLOR_NO_FILL               GL_COLOR_NONE
#define GL_COLOR_FILL_MERGE_DEFAULT    GL_COLOR_A(255,255,255,128)
#define GL_COLOR_MERGE_DEFAULT         GL_COLOR_A(0,0,0,128)
#define GL_COLOR_SHADOW_DEFAULT        GL_COLOR(0,0,0)
#define GL_COLOR_SHADOW_MERGE_DEFAULT  GL_COLOR_A(0,0,0,255)


/*
 * Extract components as floating point (0.0 .. 1.0)
 */
#define GL_COLOR_F_RED(x)   ( (((x)>>24) & 0xff) / 255.0 )
#define GL_COLOR_F_GREEN(x) ( (((x)>>16) & 0xff) / 255.0 )
#define GL_COLOR_F_BLUE(x)  ( (((x)>>8)  & 0xff) / 255.0 )
#define GL_COLOR_F_ALPHA(x) ( ( (x)      & 0xff) / 255.0 )

/*
 * Extract arguments for cairo_set_source_rgb()
 */
#define GL_COLOR_RGB_ARGS(x)  \
        GL_COLOR_F_RED(x),    \
        GL_COLOR_F_GREEN(x),  \
        GL_COLOR_F_BLUE(x)

/*
 * Extract arguments for cairo_set_source_rgba()
 */
#define GL_COLOR_RGBA_ARGS(x) \
        GL_COLOR_F_RED(x),    \
        GL_COLOR_F_GREEN(x),  \
        GL_COLOR_F_BLUE(x),   \
        GL_COLOR_F_ALPHA(x)

                


guint     gl_color_set_opacity            (guint            color,
					   gdouble          opacity);

guint     gl_color_shadow                 (guint            base_color,
					   gdouble          opacity,
					   guint            object_color);

/*
 * Routines to convert to/from GdkColor.
 */
GdkColor *gl_color_to_gdk_color           (guint            color);

guint     gl_color_from_gdk_color         (GdkColor        *gdk_color);



/*
 * Color nodes
 */
typedef struct {
        gboolean field_flag;
        guint color;
        gchar *key;
} glColorNode;

glColorNode *gl_color_node_new_default    (void);

glColorNode *gl_color_node_dup            (glColorNode     *color_node);
gboolean     gl_color_node_equal          (glColorNode     *color_node1,
                                           glColorNode     *color_node2);
guint        gl_color_node_expand         (glColorNode     *color_node,
                                           glMergeRecord   *record);
void         gl_color_node_free           (glColorNode    **color_node);


G_END_DECLS

#endif /* __COLOR_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
