/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view_object.c:  GLabels label object base class
 *
 *  Copyright (C) 2001-2002  Jim Evins <evins@snaught.com>.
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

#include <glib.h>

#include "view-object.h"
#include "libart_lgpl/libart.h"

#include "debug.h"

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glViewObjectPrivate {

	glView                     *view;
	glLabelObject              *object;

	GnomeCanvasItem            *group;
	glViewHighlight            *highlight;

	gdouble                    affine[6];

	GtkWidget                  *menu;
	GtkWidget                  *property_dialog;

	glViewObjectDlgConstructor  dialog_constructor;
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static GObjectClass *parent_class = NULL;


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void     gl_view_object_class_init    (glViewObjectClass   *klass);
static void     gl_view_object_instance_init (glViewObject        *view_object);
static void     gl_view_object_finalize      (GObject             *object);

static GtkMenu *new_menu                     (glViewObject        *view_object);

static void     object_moved_cb              (glLabelObject       *object,
					      gdouble              x,
					      gdouble              y,
					      glViewObject        *view_object);

static void     raise_object_cb              (GtkWidget           *widget,
					      glViewObject        *view_object);

static void     lower_object_cb              (GtkWidget           *widget,
					      glViewObject        *view_object);

static void     flip_rotate_object_cb        (GtkWidget           *widget,
					      glLabelObjectFlip    flip,
					      gdouble              rotate_degs,
					      glViewObject        *view_object);



/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_view_object_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (glViewObjectClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_view_object_class_init,
			NULL,
			NULL,
			sizeof (glViewObject),
			0,
			(GInstanceInitFunc) gl_view_object_instance_init,
		};

		type = g_type_register_static (G_TYPE_OBJECT,
					       "glViewObject", &info, 0);
	}

	return type;
}

static void
gl_view_object_class_init (glViewObjectClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	gl_debug (DEBUG_VIEW, "START");

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gl_view_object_finalize;

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_object_instance_init (glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	view_object->private = g_new0 (glViewObjectPrivate, 1);

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_object_finalize (GObject *object)
{
	glLabel       *parent;
	glView        *view;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (object && GL_IS_VIEW_OBJECT (object));

	view = GL_VIEW_OBJECT(object)->private->view;
	view->object_list = g_list_remove (view->object_list, object);
	view->selected_object_list =
		g_list_remove (view->selected_object_list, object);

	g_object_unref (GL_VIEW_OBJECT(object)->private->object);
	g_object_unref (G_OBJECT(GL_VIEW_OBJECT(object)->private->highlight));
	gtk_object_destroy (GTK_OBJECT(GL_VIEW_OBJECT(object)->private->group));
	gtk_object_destroy (GTK_OBJECT(GL_VIEW_OBJECT(object)->private->menu));
	if (GL_VIEW_OBJECT(object)->private->property_dialog) {
		gtk_object_destroy (GTK_OBJECT(GL_VIEW_OBJECT(object)->private->property_dialog));
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* NEW object view.                                                          */
/*****************************************************************************/
GObject *
gl_view_object_new (void)
{
	glViewObject *view_object;

	gl_debug (DEBUG_VIEW, "START");

	view_object = g_object_new (gl_view_object_get_type(), NULL);

	gl_debug (DEBUG_VIEW, "END");

	return G_OBJECT (view_object);
}

/*****************************************************************************/
/* Set parent view to which this object view belongs.                        */
/*****************************************************************************/
void
gl_view_object_set_view       (glViewObject *view_object,
			       glView *view)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));
	g_return_if_fail (view && GL_IS_VIEW (view));
	
	view_object->private->view = view;

	view->object_list = g_list_prepend (view->object_list, view_object);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Set Label object to follow.                                               */
/*****************************************************************************/
void
gl_view_object_set_object     (glViewObject         *view_object,
			       glLabelObject        *object,
			       glViewHighlightStyle style)
{
	GnomeCanvas        *canvas;
	GnomeCanvasGroup   *root;
	gdouble            x, y, w, h;
	glLabelObjectFlip  flip;
	gdouble            a[6];

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));
	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));
	
	view_object->private->object = object;

	gl_label_object_get_position (GL_LABEL_OBJECT(object), &x, &y);
	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);

	/* create canvas group to contain items representing object */
	view_object->private->group =
		gnome_canvas_item_new (view_object->private->view->label_group,
				       gnome_canvas_group_get_type (),
				       "x", x,
				       "y", y,
				       NULL);

	/* create affine to handle flipping and rotation transformations */
	art_affine_identity (view_object->private->affine);
#if 1
	/* Apply appropriate flipping */
	flip = gl_label_object_get_flip (object);
	g_print ("Flip = %d\n", flip);
	if ( flip & GL_LABEL_OBJECT_FLIP_HORIZ ) {
		art_affine_translate (a, -w, 0.0);
		art_affine_multiply (view_object->private->affine,
				     view_object->private->affine, a);
		art_affine_scale (a, -1.0, 1.0);
		art_affine_multiply (view_object->private->affine,
				     view_object->private->affine, a);
	}
	if ( flip & GL_LABEL_OBJECT_FLIP_VERT ) {
		art_affine_translate (a, 0.0, -h);
		art_affine_multiply (view_object->private->affine,
				     view_object->private->affine, a);
		art_affine_scale (a, 1.0, -1.0);
		art_affine_multiply (view_object->private->affine,
				     view_object->private->affine, a);
	}
#endif

	/* Create appropriate selection highlight canvas item. */
	view_object->private->highlight =
		GL_VIEW_HIGHLIGHT (gl_view_highlight_new (view_object, style));

	gl_view_raise_fg (view_object->private->view);

	view_object->private->menu = GTK_WIDGET(new_menu (view_object));

	g_signal_connect (G_OBJECT (object), "moved",
			  G_CALLBACK (object_moved_cb),
			  view_object);

	g_signal_connect (G_OBJECT (object), "top",
			  G_CALLBACK (raise_object_cb),
			  view_object);

	g_signal_connect (G_OBJECT (object), "bottom",
			  G_CALLBACK (lower_object_cb),
			  view_object);

	g_signal_connect (G_OBJECT (object), "flip_rotate",
			  G_CALLBACK (flip_rotate_object_cb),
			  view_object);

	g_signal_connect (G_OBJECT (view_object->private->group), "event",
			  G_CALLBACK (gl_view_item_event_handler),
			  view_object);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Set dialog for controlling/viewing object properties.                     */
/*****************************************************************************/
void
gl_view_object_set_dlg_constructor (glViewObject               *view_object,
				    glViewObjectDlgConstructor  dlg_constructor)

{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));
	
	view_object->private->dialog_constructor = dlg_constructor;

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Return parent view associated with this view.                             */
/*****************************************************************************/
glView *
gl_view_object_get_view   (glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));

	gl_debug (DEBUG_VIEW, "END");

	return view_object->private->view;
}

/*****************************************************************************/
/* Return label object that we are following.                                */
/*****************************************************************************/
glLabelObject *
gl_view_object_get_object (glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));
	
	gl_debug (DEBUG_VIEW, "END");

	return view_object->private->object;
}

/*****************************************************************************/
/* Return canvas item representing our object in this view.                  */
/*****************************************************************************/
GnomeCanvasItem *
gl_view_object_get_group   (glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));
	
	gl_debug (DEBUG_VIEW, "END");

	return view_object->private->group;
}

/*****************************************************************************/
/* Create canvas item for this object.                                       */
/*****************************************************************************/
GnomeCanvasItem *
gl_view_object_item_new (glViewObject *view_object,
			 GType         type,
			 const gchar  *first_arg_name,
			 ...)
{
	GnomeCanvasItem *item;
	va_list          args;

	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object), NULL);

	item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(view_object->private->group),
				      type, NULL);
	
        va_start (args, first_arg_name);
        gnome_canvas_item_set_valist (item, first_arg_name, args);
        va_end (args);

	g_print ("Affine = {%f, %f, %f, %f, %f, %f}\n",
		 view_object->private->affine[0],
		 view_object->private->affine[1],
		 view_object->private->affine[2],
		 view_object->private->affine[3],
		 view_object->private->affine[4],
		 view_object->private->affine[5]);

#if 1
	gnome_canvas_item_affine_absolute (item, view_object->private->affine);
#endif

	gl_debug (DEBUG_VIEW, "END");

	return item;
}

/*****************************************************************************/
/* Return dialog for controlling/viewing object properties.                  */
/*****************************************************************************/
GtkWidget *
gl_view_object_get_dialog (glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object), NULL);
	
	gl_debug (DEBUG_VIEW, "END");

	return view_object->private->property_dialog;
}

/*****************************************************************************/
/* Popup menu for this object.                                               */
/*****************************************************************************/
GtkMenu *
gl_view_object_get_menu (glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object), NULL);

	gl_debug (DEBUG_VIEW, "END");

	return GTK_MENU(view_object->private->menu);
}

/*****************************************************************************/
/* Highlight view of object.                                                 */
/*****************************************************************************/
void
gl_view_object_show_highlight     (glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));
	g_return_if_fail (view_object->private->highlight);
	
	gl_view_highlight_show (view_object->private->highlight);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Remove highlight from view of object.                                     */
/*****************************************************************************/
void
gl_view_object_hide_highlight   (glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));
	g_return_if_fail (view_object->private->highlight);
	
	gl_view_highlight_hide (view_object->private->highlight);

	gl_debug (DEBUG_VIEW, "END");
}


/*---------------------------------------------------------------------------*/
/* Create a popup menu for this object view.                                 */
/*---------------------------------------------------------------------------*/
static GtkMenu *
new_menu (glViewObject *view_object)
{
	GtkWidget *menu, *menuitem;

	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (view_object && GL_VIEW_OBJECT(view_object), NULL);

	menu = gtk_menu_new ();

	menuitem = gtk_menu_item_new_with_label (_("Edit properties..."));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	gtk_widget_show (menuitem);
	g_signal_connect_swapped (G_OBJECT (menuitem), "activate",
				  G_CALLBACK (gl_view_object_show_dialog),
				  view_object);

	menuitem = gtk_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	gtk_widget_show (menuitem);

	menuitem = gtk_menu_item_new_with_label (_("Delete"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	gtk_widget_show (menuitem);
	g_signal_connect_swapped (G_OBJECT (menuitem), "activate",
				  G_CALLBACK (g_object_unref), view_object);

	menuitem = gtk_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	gtk_widget_show (menuitem);

	menuitem = gtk_menu_item_new_with_label (_("Bring to front"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	gtk_widget_show (menuitem);
	g_signal_connect_swapped (G_OBJECT (menuitem), "activate",
				  G_CALLBACK (gl_label_object_raise_to_top),
				  view_object->private->object);

	menuitem = gtk_menu_item_new_with_label (_("Send to back"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	gtk_widget_show (menuitem);
	g_signal_connect_swapped (G_OBJECT (menuitem), "activate",
				  G_CALLBACK (gl_label_object_lower_to_bottom),
				  view_object->private->object);

	gl_debug (DEBUG_VIEW, "END");

	return GTK_MENU(menu);
}

/*****************************************************************************/
/* Show property dialog.                                                     */
/*****************************************************************************/
void
gl_view_object_show_dialog (glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));

	if (view_object->private->property_dialog != NULL) {
		gtk_window_present (GTK_WINDOW (view_object->private->property_dialog));
		return;
	}

	view_object->private->property_dialog =
		view_object->private->dialog_constructor (view_object);

	g_signal_connect (G_OBJECT (view_object->private->property_dialog),
			  "destroy",
			  G_CALLBACK (gtk_widget_destroyed),
			  &view_object->private->property_dialog);
	
	gtk_widget_show_all (view_object->private->property_dialog);


	gl_debug (DEBUG_VIEW, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Object moved callback.                                          */
/*---------------------------------------------------------------------------*/
static void
object_moved_cb (glLabelObject *object,
		 gdouble        dx,
		 gdouble        dy,
		 glViewObject  *view_object)
{
	GnomeCanvasItem    *item, *highlight;

	gl_debug (DEBUG_VIEW, "START");

	/* Adjust location of analogous canvas group. */
	gnome_canvas_item_move (view_object->private->group, dx, dy);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  raise item to front callback.                                   */
/*---------------------------------------------------------------------------*/
static void
raise_object_cb (GtkWidget    *widget,
		 glViewObject *view_object)
{
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	/* send to top */
	gnome_canvas_item_raise_to_top (view_object->private->group);

	/* send highlight to top */
	gl_view_highlight_show (view_object->private->highlight);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  lower item to back callback.                                    */
/*---------------------------------------------------------------------------*/
static void
lower_object_cb (GtkWidget    *widget,
		 glViewObject *view_object)
{
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	/* Send to bottom */
	gnome_canvas_item_lower_to_bottom (view_object->private->group);

	/* now raise it above all items that form the backgound */
	gnome_canvas_item_lower_to_bottom (GNOME_CANVAS_ITEM(view_object->private->view->markup_group));
	gnome_canvas_item_lower_to_bottom (GNOME_CANVAS_ITEM(view_object->private->view->grid_group));
	gnome_canvas_item_lower_to_bottom (GNOME_CANVAS_ITEM(view_object->private->view->bg_group));

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Flip/rotate object callback.                                    */
/*---------------------------------------------------------------------------*/
static void
flip_rotate_object_cb (GtkWidget         *widget,
		       glLabelObjectFlip  flip,
		       gdouble            rotate_degs,
		       glViewObject      *view_object)
{
	glLabelObject   *object;
	gdouble          a[6];
	gdouble          w, h;
	GList           *p, *item_list;
	GnomeCanvasItem *item;

	gl_debug (DEBUG_VIEW, "START");

	gl_label_object_get_size (view_object->private->object, &w, &h);

	/* Reset to identity affine */
	art_affine_identity (view_object->private->affine);

	/* Apply appropriate flipping */
	if ( flip & GL_LABEL_OBJECT_FLIP_HORIZ ) {
		art_affine_translate (a, -w, 0.0);
		art_affine_multiply (view_object->private->affine, view_object->private->affine, a);
		art_affine_scale (a, -1.0, 1.0);
		art_affine_multiply (view_object->private->affine,
				     view_object->private->affine, a);
	}
	if ( flip & GL_LABEL_OBJECT_FLIP_VERT ) {
		art_affine_translate (a, 0.0, -h);
		art_affine_multiply (view_object->private->affine,
				     view_object->private->affine, a);
		art_affine_scale (a, 1.0, -1.0);
		art_affine_multiply (view_object->private->affine,
				     view_object->private->affine, a);
	}

#if 1
	/* Apply newly constructed affine */
	item_list = GNOME_CANVAS_GROUP(view_object->private->group)->item_list;
	for ( p=item_list; p != NULL; p=p->next) {
		item = GNOME_CANVAS_ITEM(p->data);
		gnome_canvas_item_affine_absolute (item, view_object->private->affine);
	}
#endif

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Select object.                                                            */
/*****************************************************************************/
void
gl_view_object_select (glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	gl_view_select_object(view_object->private->view, view_object);

	gl_debug (DEBUG_VIEW, "END");
}

