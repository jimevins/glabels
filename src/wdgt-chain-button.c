/*
 *  wdgt-chain-button.c
 *  Modified version of gimpchainbutton.h for gLabels:
 *
 *  LIBGIMP - The GIMP Library
 *  Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 *  gimpchainbutton.h
 *  Copyright (C) 1999-2000 Sven Neumann <sven@gimp.org>
 *
 *  Modified or gLabels by Jim Evins <evins@snaught.com>
 *
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

#include "wdgt-chain-button.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>


enum
{
  TOGGLED,
  LAST_SIGNAL
};


static void      gl_wdgt_chain_button_clicked_callback (GtkWidget          *widget,
							glWdgtChainButton  *button);
static gboolean  gl_wdgt_chain_button_draw_lines       (GtkWidget          *widget,
                                                        cairo_t            *cr,
							glWdgtChainButton  *button);


static const gchar *gl_wdgt_chain_stock_items[] =
{
  "changes-prevent",
  "changes-allow",
  "changes-prevent",
  "changes-allow"
};


static guint gl_wdgt_chain_button_signals[LAST_SIGNAL] = { 0 };



G_DEFINE_TYPE (glWdgtChainButton, gl_wdgt_chain_button, GTK_TYPE_TABLE)


static void
gl_wdgt_chain_button_class_init (glWdgtChainButtonClass *class)
{
	gl_wdgt_chain_button_parent_class = g_type_class_peek_parent (class);

	gl_wdgt_chain_button_signals[TOGGLED] =
		g_signal_new ("toggled",
			      G_TYPE_FROM_CLASS (class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (glWdgtChainButtonClass, toggled),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);

	class->toggled = NULL;
}

static void
gl_wdgt_chain_button_init (glWdgtChainButton *button)
{
	button->position = GL_WDGT_CHAIN_TOP;
	button->active   = FALSE;

	button->line1    = gtk_drawing_area_new ();
	button->line2    = gtk_drawing_area_new ();
	button->image    = gtk_image_new ();

	button->button   = gtk_toggle_button_new ();

	gtk_button_set_relief (GTK_BUTTON (button->button), GTK_RELIEF_NONE);
        gtk_button_set_focus_on_click (GTK_BUTTON (button->button), FALSE);
	gtk_container_add (GTK_CONTAINER (button->button), button->image);
	gtk_widget_show (button->image);

	g_signal_connect (button->button, "clicked",
			  G_CALLBACK (gl_wdgt_chain_button_clicked_callback),
			  button);
	g_signal_connect (button->line1, "draw",
			  G_CALLBACK (gl_wdgt_chain_button_draw_lines),
			  button);
	g_signal_connect (button->line2, "draw",
			  G_CALLBACK (gl_wdgt_chain_button_draw_lines),
			  button);

}


/**
 * gl_wdgt_chain_button_new:
 * @position: The position you are going to use for the button
 *            with respect to the widgets you want to chain.
 *
 * Creates a new #glWdgtChainButton widget.
 *
 * This returns a button showing either a broken or a linked chain and
 * small clamps attached to both sides that visually group the two widgets
 * you want to connect. This widget looks best when attached
 * to a table taking up two columns (or rows respectively) next
 * to the widgets that it is supposed to connect. It may work
 * for more than two widgets, but the look is optimized for two.
 *
 * Returns: Pointer to the new #glWdgtChainButton, which is inactive
 *          by default. Use gl_wdgt_chain_button_set_active() to
 *          change its state.
 */
GtkWidget *
gl_wdgt_chain_button_new (glWdgtChainPosition position)
{
  glWdgtChainButton *button;

  button = g_object_new (GL_WDGT_TYPE_CHAIN_BUTTON, NULL);

  button->position = position;

  gtk_image_set_from_icon_name
    (GTK_IMAGE (button->image),
     gl_wdgt_chain_stock_items[((position & GL_WDGT_CHAIN_LEFT) << 1) + ! button->active],
     GTK_ICON_SIZE_BUTTON);

  if (position & GL_WDGT_CHAIN_LEFT) /* are we a vertical chainbutton? */
    {
      gtk_table_resize (GTK_TABLE (button), 3, 1);
      gtk_table_attach (GTK_TABLE (button), button->button, 0, 1, 1, 2,
			GTK_SHRINK, GTK_SHRINK, 0, 0);
      gtk_table_attach (GTK_TABLE (button), button->line1, 0, 1, 0, 1,
                        GTK_FILL, GTK_EXPAND|GTK_FILL, 0, 0 );
      gtk_table_attach (GTK_TABLE (button), button->line2, 0, 1, 2, 3,
                        GTK_FILL, GTK_EXPAND|GTK_FILL, 0, 0 );
    }
  else
    {
      gtk_table_resize (GTK_TABLE (button), 1, 3);
      gtk_table_attach (GTK_TABLE (button), button->button, 1, 2, 0, 1,
			GTK_SHRINK, GTK_SHRINK, 0, 0);
      gtk_table_attach (GTK_TABLE (button), button->line1, 0, 1, 0, 1,
                        GTK_EXPAND|GTK_FILL, GTK_FILL, 0, 0 );
      gtk_table_attach (GTK_TABLE (button), button->line2, 2, 3, 0, 1,
                        GTK_EXPAND|GTK_FILL, GTK_FILL, 0, 0 );
    }

  gtk_widget_show (button->button);
  gtk_widget_show (button->line1);
  gtk_widget_show (button->line2);

  return GTK_WIDGET (button);
}

/**
 * gl_wdgt_chain_button_set_active:
 * @button: Pointer to a #glWdgtChainButton.
 * @active: The new state.
 *
 * Sets the state of the #glWdgtChainButton to be either locked (%TRUE) or
 * unlocked (%FALSE) and changes the showed pixmap to reflect the new state.
 */
void
gl_wdgt_chain_button_set_active (glWdgtChainButton  *button,
                                 gboolean          active)
{
  g_return_if_fail (GL_WDGT_IS_CHAIN_BUTTON (button));

  if (button->active != active)
    {
      guint num;

      button->active = active ? TRUE : FALSE;

      num = ((button->position & GL_WDGT_CHAIN_LEFT) << 1) + (active ? 0 : 1);

      gtk_image_set_from_icon_name (GTK_IMAGE (button->image),
                                    gl_wdgt_chain_stock_items[num],
                                    GTK_ICON_SIZE_MENU);

      g_signal_handlers_block_by_func (G_OBJECT (button->button),
                                       gl_wdgt_chain_button_clicked_callback, button);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button->button), active);
      g_signal_handlers_unblock_by_func (G_OBJECT (button->button),
                                         gl_wdgt_chain_button_clicked_callback, button);
    }
}

/**
 * gl_wdgt_chain_button_get_active
 * @button: Pointer to a #glWdgtChainButton.
 *
 * Checks the state of the #glWdgtChainButton.
 *
 * Returns: %TRUE if the #glWdgtChainButton is active (locked).
 */
gboolean
gl_wdgt_chain_button_get_active (glWdgtChainButton *button)
{
  g_return_val_if_fail (GL_WDGT_IS_CHAIN_BUTTON (button), FALSE);

  return button->active;
}

static void
gl_wdgt_chain_button_clicked_callback (GtkWidget       *widget,
                                       glWdgtChainButton *button)
{
  g_return_if_fail (GL_WDGT_IS_CHAIN_BUTTON (button));

  gl_wdgt_chain_button_set_active (button, ! button->active);

  g_signal_emit (button, gl_wdgt_chain_button_signals[TOGGLED], 0);
}

static gboolean
gl_wdgt_chain_button_draw_lines (GtkWidget         *widget,
                                 cairo_t           *cr,
				 glWdgtChainButton *button)
{
  GtkAllocation        allocation;
  gdouble              w, h;
  glWdgtChainPosition  position;
  gint                 which_line;

  g_return_val_if_fail (GL_WDGT_IS_CHAIN_BUTTON (button), FALSE);

  gtk_widget_get_allocation (widget, &allocation);
  w = allocation.width;
  h = allocation.height;

  which_line = (widget == button->line1) ? 1 : -1;

  position = button->position;

  if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
    switch (position)
      {
      case GL_WDGT_CHAIN_LEFT:
        position = GL_WDGT_CHAIN_RIGHT;
        break;
      case GL_WDGT_CHAIN_RIGHT:
        position = GL_WDGT_CHAIN_LEFT;
        break;
      default:
        break;
      }

  switch (position)
    {
    case GL_WDGT_CHAIN_LEFT:
      cairo_move_to (cr, w-2, (which_line == 1) ? 0.6*h : 0.4*h);
      cairo_line_to (cr, w/2, (which_line == 1) ? 0.6*h : 0.4*h);
      cairo_line_to (cr, w/2, (which_line == 1) ? h-2 : 1);
      break;
    case GL_WDGT_CHAIN_RIGHT:
      cairo_move_to (cr, 1,   (which_line == 1) ? 0.6*h : 0.4*h);
      cairo_line_to (cr, w/2, (which_line == 1) ? 0.6*h : 0.4*h);
      cairo_line_to (cr, w/2, (which_line == 1) ? h-2 : 1);
      break;
    case GL_WDGT_CHAIN_TOP:
      cairo_move_to (cr, w/2, h-2);
      cairo_line_to (cr, w/2, h/2);
      cairo_line_to (cr, (which_line == 1) ? w-2 : 1, h/2);
      break;
    case GL_WDGT_CHAIN_BOTTOM:
      cairo_move_to (cr, w/2, 1);
      cairo_line_to (cr, w/2, h/2);
      cairo_line_to (cr, (which_line == 1) ? w-2 : 1, h/2);
      break;
    default:
      return FALSE;
    }

  cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
  cairo_set_line_width (cr, 1.0);
  cairo_set_source_rgb (cr, 0, 0, 0);

  cairo_stroke (cr);

  return TRUE;
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
