/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  object-editor.c:  object properties editor module
 *
 *  Copyright (C) 2003  Jim Evins <evins@snaught.com>.
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
#include <config.h>

#include <gnome.h>
#include <math.h>

#include "object-editor.h"
#include "prefs.h"
#include "mygal/widget-color-combo.h"
#include "color.h"

#include "object-editor-private.h"

#include "debug.h"

/*===========================================*/
/* Private macros                            */
/*===========================================*/

/*===========================================*/
/* Private data types                        */
/*===========================================*/

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare fill page.                                             */
/*--------------------------------------------------------------------------*/
void
gl_object_editor_prepare_fill_page (glObjectEditor *editor)
{
	GdkColor *gdk_color;

	gl_debug (DEBUG_EDITOR, "START");

	/* Extract widgets from XML tree. */
	editor->priv->fill_page_vbox   = glade_xml_get_widget (editor->priv->gui,
							       "fill_page_vbox");
	editor->priv->fill_color_combo = glade_xml_get_widget (editor->priv->gui,
							       "fill_color_combo");

	/* Modify widgets based on configuration */
	gdk_color = gl_color_to_gdk_color (gl_prefs->default_fill_color);
	color_combo_set_color (COLOR_COMBO(editor->priv->fill_color_combo), gdk_color);
	g_free (gdk_color);

	/* Un-hide */
	gtk_widget_show_all (editor->priv->fill_page_vbox);

	/* Connect signals */
	g_signal_connect_swapped (G_OBJECT (editor->priv->fill_color_combo),
				  "color_changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));

	gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Set fill color.                                                           */
/*****************************************************************************/
void
gl_object_editor_set_fill_color (glObjectEditor      *editor,
				 guint                color)
{
	GdkColor *gdk_color;

	gl_debug (DEBUG_EDITOR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->fill_color_combo),
					 gl_object_editor_changed_cb,
					 editor);

	gdk_color = gl_color_to_gdk_color (color);
	color_combo_set_color (COLOR_COMBO(editor->priv->fill_color_combo), gdk_color);
	g_free (gdk_color);

	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->fill_color_combo),
					   gl_object_editor_changed_cb,
					   editor);

	gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Query fill color.                                                         */
/*****************************************************************************/
guint
gl_object_editor_get_fill_color (glObjectEditor      *editor)
{
        GdkColor *gdk_color;
        gboolean  is_default;
	guint     color;
 
	gl_debug (DEBUG_EDITOR, "START");

        gdk_color = color_combo_get_color (COLOR_COMBO(editor->priv->fill_color_combo),
                                           &is_default);
 
        if (is_default) {
                color = GL_COLOR_NONE;
        } else {
                color = gl_color_from_gdk_color (gdk_color);
        }

	gl_debug (DEBUG_EDITOR, "END");

	return color;
}

