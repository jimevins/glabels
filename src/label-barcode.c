/*
 *  label-barcode.c
 *  Copyright (C) 2001-2009  Jim Evins <evins@snaught.com>.
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

#include "label-barcode.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <pango/pangocairo.h>
#include "bc-backends.h"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

#define FONT_SCALE (72.0/96.0)
#define PI 3.141592654

#define GL_BARCODE_FONT_FAMILY      "Sans"

#define HANDLE_OUTLINE_RGBA_ARGS   0.5,   0.5,   0.5,   0.75
#define HANDLE_OUTLINE_WIDTH_PIXELS   2.0

#define SELECTION_SLOP_PIXELS 4.0


/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glLabelBarcodePrivate {

        glTextNode          *text_node;
        glLabelBarcodeStyle *style;
        glColorNode         *color_node;

        /* Cached info.  Only regenerate when text_node,
         * style, or raw size changed */
        lglBarcode          *display_gbc;
        gdouble              w, h;

};


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void  gl_label_barcode_finalize      (GObject             *object);

static void  copy                           (glLabelObject       *dst_object,
                                             glLabelObject       *src_object);

static void  update_barcode                 (glLabelBarcode      *lbc);

static void  set_size                       (glLabelObject       *object,
                                             gdouble              w,
                                             gdouble              h,
                                             gboolean             checkpoint);

static void  get_size                       (glLabelObject       *object,
                                             gdouble             *w,
                                             gdouble             *h);

static void  set_line_color                 (glLabelObject       *object,
                                             glColorNode         *line_color,
                                             gboolean             checkpoint);

static glColorNode *get_line_color          (glLabelObject       *object);

static void     draw_object                 (glLabelObject       *object,
                                             cairo_t             *cr,
                                             gboolean             screen_flag,
                                             glMergeRecord       *record);

static gboolean object_at                   (glLabelObject       *object,
                                             cairo_t             *cr,
                                             gdouble              x_pixels,
                                             gdouble              y_pixels);

static void     draw_handles                (glLabelObject       *object,
                                             cairo_t             *cr);

static void     create_alt_msg_path         (cairo_t             *cr,
                                             gchar               *text);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glLabelBarcode, gl_label_barcode, GL_TYPE_LABEL_OBJECT);


static void
gl_label_barcode_class_init (glLabelBarcodeClass *class)
{
        GObjectClass       *object_class       = G_OBJECT_CLASS (class);
        glLabelObjectClass *label_object_class = GL_LABEL_OBJECT_CLASS (class);

        gl_label_barcode_parent_class = g_type_class_peek_parent (class);

        label_object_class->copy           = copy;
        label_object_class->set_size       = set_size;
        label_object_class->get_size       = get_size;
        label_object_class->set_line_color = set_line_color;
        label_object_class->get_line_color = get_line_color;
        label_object_class->draw_object    = draw_object;
        label_object_class->draw_shadow    = NULL;
        label_object_class->object_at      = object_at;
        label_object_class->draw_handles   = draw_handles;

        object_class->finalize = gl_label_barcode_finalize;
}


static void
gl_label_barcode_init (glLabelBarcode *lbc)
{
        lbc->priv = g_new0 (glLabelBarcodePrivate, 1);
        lbc->priv->text_node  = gl_text_node_new_from_text ("");
}


static void
gl_label_barcode_finalize (GObject *object)
{
        glLabelBarcode *lbc = GL_LABEL_BARCODE (object);

        g_return_if_fail (object && GL_IS_LABEL_BARCODE (object));

        gl_text_node_free (&lbc->priv->text_node);
        gl_label_barcode_style_free (lbc->priv->style);
        gl_color_node_free (&(lbc->priv->color_node));
        lgl_barcode_free (lbc->priv->display_gbc);
        g_free (lbc->priv);

        G_OBJECT_CLASS (gl_label_barcode_parent_class)->finalize (object);
}


/*****************************************************************************/
/* NEW label "barcode" object.                                               */
/*****************************************************************************/
GObject *
gl_label_barcode_new (glLabel *label,
                      gboolean checkpoint)
{
        glLabelBarcode      *lbc;
        glLabelBarcodeStyle *style;
        glColorNode         *line_color_node;

        lbc = g_object_new (gl_label_barcode_get_type(), NULL);

        if (label != NULL)
        {
                if ( checkpoint )
                {
                        gl_label_checkpoint (label, _("Create barcode object"));
                }

                /* Default barcode style and properties. */
                style = gl_label_barcode_style_new ();
                gl_label_barcode_style_set_backend_id (style, gl_barcode_backends_backend_name_to_id (NULL));
                gl_label_barcode_style_set_style_id (style, gl_barcode_backends_style_name_to_id (style->backend_id, NULL));
                style->text_flag     = gl_barcode_backends_style_can_text (style->backend_id, style->id);
                style->checksum_flag = gl_barcode_backends_style_can_csum (style->backend_id, style->id);
                style->format_digits = gl_barcode_backends_style_get_prefered_n (style->backend_id, style->id);
                lbc->priv->style = style;
                update_barcode (lbc);

                line_color_node = gl_color_node_new_default ();
                line_color_node->color = gl_label_get_default_line_color(label);
                lbc->priv->color_node = line_color_node;

                gl_label_add_object (label, GL_LABEL_OBJECT (lbc));
                gl_label_object_set_parent (GL_LABEL_OBJECT (lbc), label);
        }

        return G_OBJECT (lbc);
}


/*****************************************************************************/
/* Copy object contents.                                                     */
/*****************************************************************************/
static void
copy (glLabelObject *dst_object,
      glLabelObject *src_object)
{
        glLabelBarcode      *lbc     = (glLabelBarcode *)src_object;
        glLabelBarcode      *new_lbc = (glLabelBarcode *)dst_object;
        glTextNode          *text_node;
        glLabelBarcodeStyle *style;
        glColorNode         *color_node;

        gl_debug (DEBUG_LABEL, "START");

        g_return_if_fail (lbc && GL_IS_LABEL_BARCODE (lbc));
        g_return_if_fail (new_lbc && GL_IS_LABEL_BARCODE (new_lbc));

        text_node = gl_label_barcode_get_data (lbc);
        style = gl_label_barcode_get_style (lbc);
        color_node = get_line_color (src_object);

        new_lbc->priv->text_node  = text_node;
        new_lbc->priv->style      = style;
        new_lbc->priv->color_node = color_node;

        update_barcode (new_lbc);

        gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set object params.                                                        */
/*****************************************************************************/
void
gl_label_barcode_set_data (glLabelBarcode   *lbc,
                           const glTextNode *text_node,
                           gboolean          checkpoint)
{
        glLabel *label;

        gl_debug (DEBUG_LABEL, "START");

        g_return_if_fail (lbc && GL_IS_LABEL_BARCODE (lbc));

        if (!gl_text_node_equal (lbc->priv->text_node, text_node))
        {
                if ( checkpoint )
                {
                        label = gl_label_object_get_parent (GL_LABEL_OBJECT (lbc));
                        gl_label_checkpoint (label, _("Barcode data"));
                }
                
                gl_text_node_free (&lbc->priv->text_node);
                lbc->priv->text_node = gl_text_node_dup (text_node);

                update_barcode (lbc);

                gl_label_object_emit_changed (GL_LABEL_OBJECT(lbc));
        }

        gl_debug (DEBUG_LABEL, "END");
}


void
gl_label_barcode_set_style (glLabelBarcode            *lbc,
                            const glLabelBarcodeStyle *style,
                            gboolean                   checkpoint)
{
        glLabel *label;

        gl_debug (DEBUG_LABEL, "START");

        g_return_if_fail (lbc && GL_IS_LABEL_BARCODE (lbc));

        if ( !gl_label_barcode_style_is_equal (style, lbc->priv->style) )
        {
                if ( checkpoint )
                {
                        label = gl_label_object_get_parent (GL_LABEL_OBJECT (lbc));
                        gl_label_checkpoint (label, _("Barcode property"));
                }

                gl_label_barcode_style_free (lbc->priv->style);
                lbc->priv->style = gl_label_barcode_style_dup (style);

                update_barcode (lbc);

                gl_label_object_emit_changed (GL_LABEL_OBJECT(lbc));
        }

        gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Get object params.                                                        */
/*****************************************************************************/
glTextNode *
gl_label_barcode_get_data (glLabelBarcode *lbc)
{
        g_return_val_if_fail (lbc && GL_IS_LABEL_BARCODE (lbc), NULL);

        return gl_text_node_dup (lbc->priv->text_node);
}


glLabelBarcodeStyle *
gl_label_barcode_get_style (glLabelBarcode *lbc)
{
        g_return_val_if_fail (lbc && GL_IS_LABEL_BARCODE (lbc), NULL);

        return gl_label_barcode_style_dup (lbc->priv->style);
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Update cached lglBarcode.                                       */
/*---------------------------------------------------------------------------*/
static void
update_barcode (glLabelBarcode *lbc)
{
        gdouble              w_raw, h_raw;
        gchar               *data;

        gl_debug (DEBUG_LABEL, "START");

        g_return_if_fail (lbc && GL_IS_LABEL_BARCODE (lbc));

        gl_label_object_get_raw_size (GL_LABEL_OBJECT (lbc), &w_raw, &h_raw);

        lgl_barcode_free (lbc->priv->display_gbc);

        if (lbc->priv->text_node->field_flag)
        {
                data = gl_barcode_backends_style_default_digits (lbc->priv->style->backend_id,
                                                                 lbc->priv->style->id,
                                                                 lbc->priv->style->format_digits);
        }
        else
        {
                data = gl_text_node_expand (lbc->priv->text_node, NULL);
        }

        lbc->priv->display_gbc = gl_barcode_backends_new_barcode (lbc->priv->style->backend_id,
                                                                  lbc->priv->style->id,
                                                                  lbc->priv->style->text_flag,
                                                                  lbc->priv->style->checksum_flag,
                                                                  w_raw,
                                                                  h_raw,
                                                                  data);
        g_free (data);

        if ( lbc->priv->display_gbc == NULL )
        {
                lglBarcode *gbc;

                /* Try again with default digits, but don't save -- just extract size. */
                data = gl_barcode_backends_style_default_digits (lbc->priv->style->backend_id,
                                                                 lbc->priv->style->id,
                                                                 lbc->priv->style->format_digits);
                gbc = gl_barcode_backends_new_barcode (lbc->priv->style->backend_id,
                                                       lbc->priv->style->id,
                                                       lbc->priv->style->text_flag,
                                                       lbc->priv->style->checksum_flag,
                                                       w_raw,
                                                       h_raw,
                                                       data);
                g_free (data);

                if ( gbc != NULL )
                {
                        lbc->priv->w = gbc->width;
                        lbc->priv->h = gbc->height;
                }
                else
                {
                        /* If we still can't render, just set a default size. */
                        lbc->priv->w = 144;
                        lbc->priv->h = 72;
                }

                lgl_barcode_free (gbc);
        }
        else
        {
                lbc->priv->w = lbc->priv->display_gbc->width;
                lbc->priv->h = lbc->priv->display_gbc->height;
        }

        gl_debug (DEBUG_LABEL, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set object size method.                                         */
/*---------------------------------------------------------------------------*/
static void
set_size (glLabelObject       *object,
          gdouble              w,
          gdouble              h,
          gboolean             checkpoint)
{
        gl_debug (DEBUG_LABEL, "START");

        g_return_if_fail (object && GL_IS_LABEL_BARCODE (object));

        gl_label_object_set_raw_size (object, w, h, checkpoint);
        update_barcode (GL_LABEL_BARCODE (object));

        gl_debug (DEBUG_LABEL, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Get object size method.                                         */
/*---------------------------------------------------------------------------*/
static void
get_size (glLabelObject *object,
          gdouble       *w,
          gdouble       *h)
{
        gl_debug (DEBUG_LABEL, "START");

        g_return_if_fail (object && GL_IS_LABEL_BARCODE (object));

        *w = GL_LABEL_BARCODE (object)->priv->w;
        *h = GL_LABEL_BARCODE (object)->priv->h;

        gl_debug (DEBUG_LABEL, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set line color method.                                          */
/*---------------------------------------------------------------------------*/
static void
set_line_color (glLabelObject *object,
                glColorNode   *line_color_node,
                gboolean       checkpoint)
{
        glLabelBarcode *lbarcode = (glLabelBarcode *)object;
        glLabel        *label;

        g_return_if_fail (lbarcode && GL_IS_LABEL_BARCODE (lbarcode));

        if ( !gl_color_node_equal(lbarcode->priv->color_node, line_color_node) )
        {
                if ( checkpoint )
                {
                        label = gl_label_object_get_parent (GL_LABEL_OBJECT (lbarcode));
                        gl_label_checkpoint (label, _("Barcode data"));
                }

                gl_color_node_free (&(lbarcode->priv->color_node));
                lbarcode->priv->color_node = gl_color_node_dup (line_color_node);
                gl_label_object_emit_changed (GL_LABEL_OBJECT(lbarcode));
        }
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Get line color method.                                          */
/*---------------------------------------------------------------------------*/
static glColorNode*
get_line_color (glLabelObject *object)
{
        glLabelBarcode *lbarcode = (glLabelBarcode *)object;

        g_return_val_if_fail (lbarcode && GL_IS_LABEL_BARCODE (lbarcode), NULL);

        return gl_color_node_dup (lbarcode->priv->color_node);
}


/*****************************************************************************/
/* Draw object method.                                                       */
/*****************************************************************************/
static void
draw_object (glLabelObject *object,
             cairo_t       *cr,
             gboolean       screen_flag,
             glMergeRecord *record)
{
        glLabelBarcode       *lbc     = (glLabelBarcode *)object;
        gdouble               x0, y0;
        cairo_matrix_t        matrix;
        lglBarcode           *gbc;
        gchar                *text;
        glTextNode           *text_node;
        glLabelBarcodeStyle  *style;
        guint                 color;
        glColorNode          *color_node;
        gdouble               w, h;

        gl_debug (DEBUG_LABEL, "START");

        gl_label_object_get_position (object, &x0, &y0);
        gl_label_object_get_matrix (object, &matrix);

        text_node = gl_label_barcode_get_data (GL_LABEL_BARCODE (object));
        style = gl_label_barcode_get_style (GL_LABEL_BARCODE (object));

        color_node = gl_label_object_get_line_color (object);
        color = gl_color_node_expand (color_node, record);
        if (color_node->field_flag && screen_flag)
        {
                color = GL_COLOR_MERGE_DEFAULT;
        }

        cairo_set_source_rgba (cr, GL_COLOR_RGBA_ARGS (color));

        if (text_node->field_flag && !screen_flag) {

                gl_label_object_get_raw_size (object, &w, &h);

                text = gl_text_node_expand (text_node, record);
                gbc = gl_barcode_backends_new_barcode (style->backend_id, style->id, style->text_flag, style->checksum_flag, w, h, text);
                g_free (text);

                if ( gbc != NULL )
                {
                        lgl_barcode_render_to_cairo (gbc, cr);
                        lgl_barcode_free (gbc);
                }

        }
        else
        {

                if (lbc->priv->display_gbc == NULL)
                {
                        create_alt_msg_path (cr, text_node->data);
                        cairo_fill (cr);
                }
                else
                {
                        lgl_barcode_render_to_cairo (lbc->priv->display_gbc, cr);
                }

        }

        gl_text_node_free (&text_node);
        gl_label_barcode_style_free (style);
        gl_color_node_free (&color_node);

        gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Is object at coordinates?                                                 */
/*****************************************************************************/
static gboolean
object_at (glLabelObject *object,
           cairo_t       *cr,
           gdouble        x,
           gdouble        y)
{
        glLabelBarcode       *lbc     = (glLabelBarcode *)object;
        gdouble               w, h;
        glTextNode           *text_node;
        gdouble               scale_x, scale_y;

        gl_label_object_get_size (object, &w, &h);

        if ( (x >= 0) && (x <= w) && (y >= 0) && (y <= h) )
        {

                text_node = gl_label_barcode_get_data(GL_LABEL_BARCODE(object));

                if ( lbc->priv->display_gbc == NULL )
                {
                        create_alt_msg_path (cr, text_node->data);
                }
                else
                {
                        lgl_barcode_render_to_cairo_path (lbc->priv->display_gbc, cr);
                }

                if (cairo_in_fill (cr, x, y))
                {
                        return TRUE;
                }


                scale_x = 1.0;
                scale_y = 1.0;
                cairo_device_to_user_distance (cr, &scale_x, &scale_y);

                cairo_set_line_width (cr, 2*SELECTION_SLOP_PIXELS*scale_x);

                if (cairo_in_stroke (cr, x, y))
                {
                        return TRUE;
                }


                if (gl_label_object_is_selected (object))
                {
                        cairo_new_path (cr);
                        cairo_rectangle (cr, 0, 0, w, h);

                        scale_x = 1.0;
                        scale_y = 1.0;
                        cairo_device_to_user_distance (cr, &scale_x, &scale_y);

                        cairo_set_line_width (cr, (HANDLE_OUTLINE_WIDTH_PIXELS + 2*SELECTION_SLOP_PIXELS)*scale_x);

                        if (cairo_in_stroke (cr, x, y))
                        {
                                return TRUE;
                        }
                }

        }

        return FALSE;
}


/*****************************************************************************/
/* Draw barcode style handles.                                               */
/*****************************************************************************/
static void
draw_handles (glLabelObject     *object,
              cairo_t           *cr)
{
        gdouble w, h;
        gdouble scale_x, scale_y;
        gdouble dashes[2] = { 2, 2 };

        gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);

        cairo_save (cr);

        cairo_rectangle (cr, 0, 0, w, h);

        scale_x = 1.0;
        scale_y = 1.0;
        cairo_device_to_user_distance (cr, &scale_x, &scale_y);
        cairo_scale (cr, scale_x, scale_y);

        cairo_set_dash (cr, dashes, 2, 0);
        cairo_set_line_width (cr, HANDLE_OUTLINE_WIDTH_PIXELS);
        cairo_set_source_rgba (cr, HANDLE_OUTLINE_RGBA_ARGS);
        cairo_stroke (cr);

        cairo_restore (cr);

        gl_label_object_draw_handles_box (object, cr);
}


/*****************************************************************************/
/* Create a cairo path with apropos message.                                 */
/*****************************************************************************/
static void
create_alt_msg_path (cairo_t *cr,
                     gchar   *text)
{
        PangoLayout          *layout;
        PangoFontDescription *desc;

        layout = pango_cairo_create_layout (cr);

        desc = pango_font_description_new ();
        pango_font_description_set_family (desc, GL_BARCODE_FONT_FAMILY);
        pango_font_description_set_size   (desc, 12 * PANGO_SCALE * FONT_SCALE);
        pango_layout_set_font_description (layout, desc);
        pango_font_description_free       (desc);

        if (text == NULL || *text == '\0')
        {
                pango_layout_set_text (layout, _("Barcode data empty"), -1);
        }
        else
        {
                pango_layout_set_text (layout, _("Invalid barcode data"), -1);
        }

        cairo_move_to (cr, 0, 0);
        pango_cairo_layout_path (cr, layout);

        g_object_unref (layout);
}


/*****************************************************************************/
/* Barcode style utilities.                                                  */
/*****************************************************************************/
glLabelBarcodeStyle *
gl_label_barcode_style_new (void)
{
        return g_new0 (glLabelBarcodeStyle, 1);
}


glLabelBarcodeStyle *
gl_label_barcode_style_dup (const glLabelBarcodeStyle *style)
{
        glLabelBarcodeStyle *style2;

        style2 = gl_label_barcode_style_new ();

        /* Shallow copy first. */
        *style2 = *style;

        /* Now go deep. */
        style2->backend_id = g_strdup (style->backend_id);
        style2->id         = g_strdup (style->id);

        return style2;
}


void
gl_label_barcode_style_free (glLabelBarcodeStyle *style)
{
        if ( style )
        {
                g_free (style->backend_id);
                g_free (style->id);

                g_free (style);
        }
}


gboolean
gl_label_barcode_style_is_equal (const glLabelBarcodeStyle *style1,
                                 const glLabelBarcodeStyle *style2)
{

        /* First take care of the case of either or both being NULL. */
        if ( style1 == NULL )
        {
                return ( style2 == NULL );
        }
        else
        {
                if ( style2 == NULL )
                {
                        return FALSE;
                }
        }

        /* Compare field by field, bail on first difference. */
        if ( style1->text_flag != style2->text_flag )
        {
                return FALSE;
        }
        if ( style1->checksum_flag != style2->checksum_flag )
        {
                return FALSE;
        }
        if ( style1->format_digits != style2->format_digits )
        {
                return FALSE;
        }
        if ( style1->backend_id && style2->backend_id )
        {
                if ( strcmp (style1->backend_id, style2->backend_id) != 0 )
                {
                        return FALSE;
                }
        }
        else
        {
                if ( style1->backend_id != style2->backend_id )
                {
                        return FALSE;
                }
        }
        if ( style1->id && style2->id )
        {
                if ( strcmp (style1->id, style2->id) != 0 )
                {
                        return FALSE;
                }
        }
        else
        {
                if ( style1->id != style2->id )
                {
                        return FALSE;
                }
        }

        /* Passed all tests. */
        return TRUE;
}


void
gl_label_barcode_style_set_backend_id (glLabelBarcodeStyle *style,
                                       const gchar         *backend_id)
{
        style->backend_id = g_strdup (backend_id);
}


void
gl_label_barcode_style_set_style_id (glLabelBarcodeStyle *style,
                                     const gchar         *id)
{
        style->id = g_strdup (id);
}





/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
