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

#include <gtk/gtk.h>


enum
{
  TOGGLED,
  LAST_SIGNAL
};


static void      gl_wdgt_chain_button_clicked_callback (GtkWidget          *widget,
							glWdgtChainButton  *button);
static gboolean  gl_wdgt_chain_button_draw_lines       (GtkWidget          *widget,
							GdkEventExpose     *eevent,
							glWdgtChainButton  *button);


static const gchar *gl_wdgt_chain_stock_items[] =
{
  "glabels-hchain",
  "glabels-hchain-broken",
  "glabels-vchain",
  "glabels-vchain-broken"
};


static guint gl_wdgt_chain_button_signals[LAST_SIGNAL] = { 0 };



G_DEFINE_TYPE (glWdgtChainButton, gl_wdgt_chain_button, GTK_TYPE_TABLE);


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

	button->button   = gtk_button_new ();

	gtk_button_set_relief (GTK_BUTTON (button->button), GTK_RELIEF_NONE);
	gtk_container_add (GTK_CONTAINER (button->button), button->image);
	gtk_widget_show (button->image);

	g_signal_connect (button->button, "clicked",
			  G_CALLBACK (gl_wdgt_chain_button_clicked_callback),
			  button);
	g_signal_connect (button->line1, "expose_event",
			  G_CALLBACK (gl_wdgt_chain_button_draw_lines),
			  button);
	g_signal_connect (button->line2, "expose_event",
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
      gtk_table_attach_defaults (GTK_TABLE (button),
				 button->line1, 0, 1, 0, 1);
      gtk_table_attach_defaults (GTK_TABLE (button),
				 button->line2, 0, 1, 2, 3);
    }
  else
    {
      gtk_table_resize (GTK_TABLE (button), 1, 3);
      gtk_table_attach (GTK_TABLE (button), button->button, 1, 2, 0, 1,
			GTK_SHRINK, GTK_SHRINK, 0, 0);
      gtk_table_attach_defaults (GTK_TABLE (button),
				 button->line1, 0, 1, 0, 1);
      gtk_table_attach_defaults (GTK_TABLE (button),
				 button->line2, 2, 3, 0, 1);
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
                                    GTK_ICON_SIZE_BUTTON);
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
				 GdkEventExpose    *eevent,
				 glWdgtChainButton *button)
{
  GtkAllocation        allocation;
  GdkPoint             points[3];
  GdkPoint             buf;
  GtkShadowType	       shadow;
  glWdgtChainPosition  position;
  gint                 which_line;

#define SHORT_LINE 4
  /* don't set this too high, there's no check against drawing outside
     the widgets bounds yet (and probably never will be) */

  g_return_val_if_fail (GL_WDGT_IS_CHAIN_BUTTON (button), FALSE);

  gtk_widget_get_allocation (widget, &allocation);
  points[0].x = allocation.width / 2;
  points[0].y = allocation.height / 2;

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
      points[0].x += SHORT_LINE;
      points[1].x = points[0].x - SHORT_LINE;
      points[1].y = points[0].y;
      points[2].x = points[1].x;
      points[2].y = (which_line == 1) ? allocation.height - 1 : 0;
      shadow = GTK_SHADOW_ETCHED_IN;
      break;
    case GL_WDGT_CHAIN_RIGHT:
      points[0].x -= SHORT_LINE;
      points[1].x = points[0].x + SHORT_LINE;
      points[1].y = points[0].y;
      points[2].x = points[1].x;
      points[2].y = (which_line == 1) ? allocation.height - 1 : 0;
      shadow = GTK_SHADOW_ETCHED_OUT;
      break;
    case GL_WDGT_CHAIN_TOP:
      points[0].y += SHORT_LINE;
      points[1].x = points[0].x;
      points[1].y = points[0].y - SHORT_LINE;
      points[2].x = (which_line == 1) ? allocation.width - 1 : 0;
      points[2].y = points[1].y;
      shadow = GTK_SHADOW_ETCHED_OUT;
      break;
    case GL_WDGT_CHAIN_BOTTOM:
      points[0].y -= SHORT_LINE;
      points[1].x = points[0].x;
      points[1].y = points[0].y + SHORT_LINE;
      points[2].x = (which_line == 1) ? allocation.width - 1 : 0;
      points[2].y = points[1].y;
      shadow = GTK_SHADOW_ETCHED_IN;
      break;
    default:
      return FALSE;
    }

  if ( ((shadow == GTK_SHADOW_ETCHED_OUT) && (which_line == -1)) ||
       ((shadow == GTK_SHADOW_ETCHED_IN) && (which_line == 1)) )
    {
      buf = points[0];
      points[0] = points[2];
      points[2] = buf;
    }

  gtk_paint_polygon (gtk_widget_get_style (widget),
		     gtk_widget_get_window (widget),
		     GTK_STATE_NORMAL,
		     shadow,
		     &eevent->area,
		     widget,
		     "chainbutton",
		     points,
		     3,
		     FALSE);

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
