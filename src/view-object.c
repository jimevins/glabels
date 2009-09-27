/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view_object.c:  GLabels label object base class
 *
 *  Copyright (C) 2001-2007  Jim Evins <evins@snaught.com>.
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


#include "view-object.h"

#include <glib.h>

#include "debug.h"

/*========================================================*/
/* Private constants and macros.                          */
/*========================================================*/

#define HANDLE_FILL_RGBA_ARGS      0.0,   0.75,  0.0,   0.4
#define HANDLE_OUTLINE_RGBA_ARGS   0.0,   0.0,   0.0,   0.8

#define HANDLE_OUTLINE_WIDTH_PIXELS   1.0
#define HANDLE_PIXELS 7

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glViewObjectPrivate {

	glLabelObject              *object;

	glView                     *view;
        glViewObjectHandlesStyle    handles_style;

	GtkWidget                  *property_editor;
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void     gl_view_object_finalize      (GObject             *object);

static void     object_removed_cb            (glViewObject        *view_object,
                                              glLabelObject       *object);

static void     object_top_cb                (glViewObject        *view_object,
                                              glLabelObject       *object);

static void     object_bottom_cb             (glViewObject        *view_object,
                                              glLabelObject       *object);






/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glViewObject, gl_view_object, G_TYPE_OBJECT);

static void
gl_view_object_class_init (glViewObjectClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	gl_debug (DEBUG_VIEW, "START");

	gl_view_object_parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_view_object_finalize;

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_object_init (glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	view_object->priv = g_new0 (glViewObjectPrivate, 1);

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_object_finalize (GObject *object)
{
	glViewObject  *view_object = GL_VIEW_OBJECT (object);
	glView        *view;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (object && GL_IS_VIEW_OBJECT (object));

	view = view_object->priv->view;
	view->object_list = g_list_remove (view->object_list, object);
	view->selected_object_list =
		g_list_remove (view->selected_object_list, object);

	g_object_unref (view_object->priv->object);
	if (view_object->priv->property_editor) {
                gl_debug (DEBUG_VIEW, "Destroy editor");
		g_object_unref (G_OBJECT(view_object->priv->property_editor));
	}
	g_free (view_object->priv);

	G_OBJECT_CLASS (gl_view_object_parent_class)->finalize (object);

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
	
	view_object->priv->view = view;

	view->object_list = g_list_append (view->object_list, view_object);

	g_signal_connect_swapped (G_OBJECT (view_object->priv->object), "changed",
				  G_CALLBACK (gl_view_update), view);
	g_signal_connect_swapped (G_OBJECT (view_object->priv->object), "moved",
				  G_CALLBACK (gl_view_update), view);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Set Label object to follow.                                               */
/*****************************************************************************/
void
gl_view_object_set_object     (glViewObject            *view_object,
			       glLabelObject           *object,
			       glViewObjectHandlesStyle style)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));
	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));
	
	view_object->priv->object = g_object_ref (G_OBJECT (object));
	view_object->priv->handles_style = style;

	g_signal_connect_swapped (G_OBJECT (object), "removed",
				  G_CALLBACK (object_removed_cb), view_object);
	g_signal_connect_swapped (G_OBJECT (object), "top",
				  G_CALLBACK (object_top_cb), view_object);
	g_signal_connect_swapped (G_OBJECT (object), "bottom",
				  G_CALLBACK (object_bottom_cb), view_object);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Handle object "removed" signal.                                 */
/*---------------------------------------------------------------------------*/
static void
object_removed_cb (glViewObject  *view_object,
                   glLabelObject *object)
{
        glView *view;
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));
	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

        view = gl_view_object_get_view (view_object);
        view->object_list = g_list_remove (view->object_list, view_object);
        g_object_unref (G_OBJECT (view_object));

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. Handle object "top" signal.                                      */
/*---------------------------------------------------------------------------*/
static void
object_top_cb (glViewObject  *view_object,
               glLabelObject *object)
{
        glView *view;
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));
	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

        view = gl_view_object_get_view (view_object);

	/* Move to end of list, representing front most object */
        view->object_list = g_list_remove (view->object_list, view_object);
        view->object_list = g_list_append (view->object_list, view_object);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. Handle object "removed" signal.                                  */
/*---------------------------------------------------------------------------*/
static void
object_bottom_cb (glViewObject  *view_object,
                  glLabelObject *object)
{
        glView *view;
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));
	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

        view = gl_view_object_get_view (view_object);

	/* Move to front of list, representing rear most object */
        view->object_list = g_list_remove (view->object_list, view_object);
        view->object_list = g_list_prepend (view->object_list, view_object);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Return parent view associated with this view.                             */
/*****************************************************************************/
glView *
gl_view_object_get_view   (glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object), NULL);

	gl_debug (DEBUG_VIEW, "END");

	return view_object->priv->view;
}

/*****************************************************************************/
/* Return label object that we are following.                                */
/*****************************************************************************/
glLabelObject *
gl_view_object_get_object (glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object), NULL);
	
	gl_debug (DEBUG_VIEW, "END");

	return view_object->priv->object;
}

/*****************************************************************************/
/* Get property editor.                                                      */
/*****************************************************************************/
GtkWidget *
gl_view_object_get_editor (glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object), NULL);

	if ( GL_VIEW_OBJECT_GET_CLASS(view_object)->construct_editor != NULL ) {

		if (view_object->priv->property_editor == NULL) {
			view_object->priv->property_editor =
				GL_VIEW_OBJECT_GET_CLASS(view_object)->construct_editor (view_object);
		}
		g_signal_connect (G_OBJECT (view_object->priv->property_editor),
				  "destroy",
				  G_CALLBACK (gtk_widget_destroyed),
				  &view_object->priv->property_editor);
	
		gtk_widget_show (view_object->priv->property_editor);

	}


	gl_debug (DEBUG_VIEW, "END");

	return view_object->priv->property_editor;
}


/*****************************************************************************/
/* Select object.                                                            */
/*****************************************************************************/
void
gl_view_object_select (glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	gl_view_select_object(view_object->priv->view, view_object);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Is object at (x,y)?                                                       */
/*****************************************************************************/
gboolean
gl_view_object_at (glViewObject  *view_object,
                   cairo_t       *cr,
                   gdouble        x,
                   gdouble        y)
{
        glLabelObject *object = view_object->priv->object;
        gdouble        x0, y0;
        cairo_matrix_t matrix;
        gboolean       return_val = FALSE;

	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object), FALSE);
	g_return_val_if_fail (object && GL_IS_LABEL_OBJECT (object), FALSE);

        gl_label_object_get_position (object, &x0, &y0);
        gl_label_object_get_matrix (object, &matrix);

        cairo_save (cr);
        cairo_translate (cr, x0, y0);
        cairo_transform (cr, &matrix);

        cairo_device_to_user (cr, &x, &y);

        if ( GL_VIEW_OBJECT_GET_CLASS(view_object)->object_at != NULL ) {

                return_val = GL_VIEW_OBJECT_GET_CLASS(view_object)->object_at (view_object, cr, x, y);

        }

        cairo_restore (cr);

        return return_val;

	gl_debug (DEBUG_VIEW, "END");
}


/*--------------------------------------------------------------------------*/
/* Create handle path                                                       */
/*--------------------------------------------------------------------------*/
static void
create_handle_path (cairo_t                *cr,
                    glLabelObject          *object,
                    gdouble                 scale,
                    gdouble                 x_handle,
                    gdouble                 y_handle)
{
        gl_debug (DEBUG_VIEW, "START");

        cairo_rectangle (cr,
                         x_handle - HANDLE_PIXELS/scale/2, y_handle - HANDLE_PIXELS/scale/2,
                         HANDLE_PIXELS/scale, HANDLE_PIXELS/scale);

        gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Draw object handles                                                       */
/*****************************************************************************/
void
gl_view_object_draw_handles (glViewObject *view_object,
                             cairo_t      *cr)
{
        glLabelObject *object = view_object->priv->object;
        gdouble        x0, y0;
        cairo_matrix_t matrix;
        gdouble        scale;
        gdouble        w, h;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));
	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

        scale = view_object->priv->view->home_scale * view_object->priv->view->zoom;

        gl_label_object_get_position (object, &x0, &y0);
        gl_label_object_get_matrix (object, &matrix);

        gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);

        cairo_save (cr);

        cairo_translate (cr, x0, y0);
        cairo_transform (cr, &matrix);

        switch (view_object->priv->handles_style)
        {
        case GL_VIEW_OBJECT_HANDLES_BOX:

                /* North */
                cairo_new_path (cr);
                create_handle_path (cr, object, scale, w/2, 0);

                /* North East */
                cairo_new_sub_path (cr);
                create_handle_path (cr, object, scale, w, 0);

                /* East */
                cairo_new_sub_path (cr);
                create_handle_path (cr, object, scale, w, h/2);

                /* South East */
                cairo_new_sub_path (cr);
                create_handle_path (cr, object, scale, w, h);

                /* South */
                cairo_new_sub_path (cr);
                create_handle_path (cr, object, scale, w/2, h);

                /* South West */
                cairo_new_sub_path (cr);
                create_handle_path (cr, object, scale, 0, h);

                /* West */
                cairo_new_sub_path (cr);
                create_handle_path (cr, object, scale, 0, h/2);

                /* North West */
                cairo_new_sub_path (cr);
                create_handle_path (cr, object, scale, 0, 0);

                break;

        case GL_VIEW_OBJECT_HANDLES_LINE:
                /* P1 */
                cairo_new_path (cr);
                create_handle_path (cr, object, scale, 0, 0);

                /* P2 */
                cairo_new_sub_path (cr);
                create_handle_path (cr, object, scale, w, h);

                break;
        }

        cairo_set_source_rgba (cr, HANDLE_FILL_RGBA_ARGS);
        cairo_fill_preserve (cr);
                               
        cairo_set_line_width (cr, HANDLE_OUTLINE_WIDTH_PIXELS/scale);
        cairo_set_source_rgba (cr, HANDLE_OUTLINE_RGBA_ARGS);
        cairo_stroke (cr);

        cairo_restore (cr);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Is there an object handle at (x,y).                                       */
/*****************************************************************************/
glViewObjectHandle
gl_view_object_handle_at (glViewObject *view_object,
                          cairo_t      *cr,
                          gdouble       x,
                          gdouble       y)
{
        glLabelObject *object = view_object->priv->object;
        gdouble        x0, y0;
        cairo_matrix_t matrix;
        gdouble        scale;
        gdouble        w, h;
        glViewObjectHandle handle = GL_VIEW_OBJECT_HANDLE_NONE;

	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object), handle);
	g_return_val_if_fail (object && GL_IS_LABEL_OBJECT (object), handle);

        scale = view_object->priv->view->home_scale * view_object->priv->view->zoom;

        gl_label_object_get_position (object, &x0, &y0);
        gl_label_object_get_matrix (object, &matrix);

        gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);

        cairo_save (cr);

        cairo_translate (cr, x0, y0);
        cairo_transform (cr, &matrix);

        cairo_device_to_user (cr, &x, &y);

        switch (view_object->priv->handles_style)
        {
        case GL_VIEW_OBJECT_HANDLES_BOX:

                /* South East */
                create_handle_path (cr, object, scale, w, h);
                if (cairo_in_fill (cr, x, y))   {handle = GL_VIEW_OBJECT_HANDLE_SE; break;}

                /* South West */
                create_handle_path (cr, object, scale, 0, h);
                if (cairo_in_fill (cr, x, y))   {handle = GL_VIEW_OBJECT_HANDLE_SW; break;}

                /* North East */
                create_handle_path (cr, object, scale, w, 0);
                if (cairo_in_fill (cr, x, y))   {handle = GL_VIEW_OBJECT_HANDLE_NE; break;}

                /* North West */
                create_handle_path (cr, object, scale, 0, 0);
                if (cairo_in_fill (cr, x, y))   {handle = GL_VIEW_OBJECT_HANDLE_NW; break;}

                /* East */
                create_handle_path (cr, object, scale, w, h/2);
                if (cairo_in_fill (cr, x, y))   {handle = GL_VIEW_OBJECT_HANDLE_E; break;}

                /* South */
                create_handle_path (cr, object, scale, w/2, h);
                if (cairo_in_fill (cr, x, y))   {handle = GL_VIEW_OBJECT_HANDLE_S; break;}

                /* West */
                create_handle_path (cr, object, scale, 0, h/2);
                if (cairo_in_fill (cr, x, y))   {handle = GL_VIEW_OBJECT_HANDLE_W; break;}

                /* North */
                create_handle_path (cr, object, scale, w/2, 0);
                if (cairo_in_fill (cr, x, y))   {handle = GL_VIEW_OBJECT_HANDLE_N; break;}

                break;

        case GL_VIEW_OBJECT_HANDLES_LINE:
                /* P2 */
                create_handle_path (cr, object, scale, w, h);
                if (cairo_in_fill (cr, x, y))   {handle = GL_VIEW_OBJECT_HANDLE_P2; break;}

                /* P1 */
                create_handle_path (cr, object, scale, 0, 0);
                if (cairo_in_fill (cr, x, y))   {handle = GL_VIEW_OBJECT_HANDLE_P1; break;}

                break;

        }

        cairo_restore (cr);

        return handle;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Resize object.                                                  */
/*---------------------------------------------------------------------------*/
void
gl_view_object_resize_event (glViewObject       *view_object,
                             glViewObjectHandle  handle,
                             gboolean            honor_aspect,
                             cairo_t            *cr,
                             gdouble             x,
                             gdouble             y)
{
        glLabelObject *object;
        cairo_matrix_t matrix;
        gdouble        x0, y0, x1, y1, x2, y2;
        gdouble        w, h;
        gdouble        dx=0, dy=0;

	gl_debug (DEBUG_VIEW, "x,y world = %g, %g", x, y);

        object = view_object->priv->object;

        /*
         * Change to item relative coordinates
         */
        cairo_save (cr);
        gl_label_object_get_position (object, &x0, &y0);
        cairo_translate (cr, x0, y0);
        gl_label_object_get_matrix (object, &matrix);
        cairo_transform (cr, &matrix);

        /*
         * Initialize origin and 2 corners in object relative coordinates.
         */
        x0 = 0.0;
        y0 = 0.0;

        x1 = 0.0;
        y1 = 0.0;

        gl_label_object_get_size (object, &x2, &y2);

	gl_debug (DEBUG_VIEW, "x0,y0 object = %g, %g", x0, y0);
	gl_debug (DEBUG_VIEW, "x1,y1 object = %g, %g", x1, y1);
	gl_debug (DEBUG_VIEW, "x2,y2 object = %g, %g", x2, y2);

        /*
         * Translate x,y into object relative coordinates.
         */
        cairo_device_to_user (cr, &x, &y);

	gl_debug (DEBUG_VIEW, "x,y object = %g, %g", x, y);
        
        /*
         * Get new size
         */
        switch (handle)
        {

        case GL_VIEW_OBJECT_HANDLE_NW:
                w = MAX (x2 - x, 0);
                h = MAX (y2 - y, 0);
                break;

        case GL_VIEW_OBJECT_HANDLE_N:
                w = x2 - x1;
                h = MAX (y2 - y, 0);
                break;

        case GL_VIEW_OBJECT_HANDLE_NE:
                w = MAX (x - x1, 0);
                h = MAX (y2 - y, 0);
                break;

        case GL_VIEW_OBJECT_HANDLE_E:
                w = MAX (x - x1, 0);
                h = y2 - y1;
                break;

        case GL_VIEW_OBJECT_HANDLE_SE:
                w = MAX (x - x1, 0);
                h = MAX (y - y1, 0);
                break;

        case GL_VIEW_OBJECT_HANDLE_S:
                w = x2 - x1;
                h = MAX (y - y1, 0);
                break;

        case GL_VIEW_OBJECT_HANDLE_SW:
                w = MAX (x2 - x, 0);
                h = MAX (y - y1, 0);
                break;

        case GL_VIEW_OBJECT_HANDLE_W:
                w = MAX (x2 - x, 0);
                h = y2 - y1;
                break;
        case GL_VIEW_OBJECT_HANDLE_P1:
                x1 = x;
                y1 = y;
                dx = (x2 - x);
                dy = (y2 - y);
                x0 = x0 + x1;
                y0 = y0 + y1;
                break;

        case GL_VIEW_OBJECT_HANDLE_P2:
                dx = x - x1;
                dy = y - y1;
                x0 = x0 + x1;
                y0 = y0 + y1;
                break;

        default:
                g_print ("Invalid handle.\n");  /* Should not happen! */

        }
        if ( (handle != GL_VIEW_OBJECT_HANDLE_P1) && (handle != GL_VIEW_OBJECT_HANDLE_P2) )
        {
                if ( honor_aspect )
                {
                        gl_label_object_set_size_honor_aspect (object, w, h);
                }
                else
                {
                        gl_label_object_set_size (object, w, h);
                }

                /*
                 * Query the new size in case it was constrained.
                 */
                gl_label_object_get_size (object, &w, &h);

                /*
                 * Get new position
                 */
                switch (handle)
                {

                case GL_VIEW_OBJECT_HANDLE_NW:
                        x0 += x2 - w;
                        y0 += y2 - h;
                        break;

                case GL_VIEW_OBJECT_HANDLE_N:
                case GL_VIEW_OBJECT_HANDLE_NE:
                        /* x unchanged */
                        y0 += y2 - h;
                        break;

                case GL_VIEW_OBJECT_HANDLE_E:
                case GL_VIEW_OBJECT_HANDLE_SE:
                case GL_VIEW_OBJECT_HANDLE_S:
                        /* unchanged */
                        break;

                case GL_VIEW_OBJECT_HANDLE_SW:
                case GL_VIEW_OBJECT_HANDLE_W:
                        x0 += x2 - w;
                        /* y unchanged */
                        break;

                default:
                        g_print ("Invalid handle.\n");  /* Should not happen! */
                }
        }
        else
        {
                gl_label_object_set_size (object, dx, dy);
        }

        /*
         * Put new origin back into world coordinates and set.
         */
        cairo_user_to_device (cr, &x0, &y0);
        cairo_restore (cr);
        cairo_device_to_user (cr, &x0, &y0);
        gl_label_object_set_position (object, x0, y0);
}
