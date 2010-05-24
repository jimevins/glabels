/*
 *  color-combo-menu.c
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

#include "color-combo-menu.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "color-combo-color-menu-item.h"
#include "color-history-model.h"
#include "color.h"
#include "marshal.h"


/*===========================================*/
/* Private macros and constants.             */
/*===========================================*/

#define PALETTE_COLS     7
#define PALETTE_ROWS    (G_N_ELEMENTS(color_table)/PALETTE_COLS + (G_N_ELEMENTS(color_table)%PALETTE_COLS ? 1 : 0))

#define ROW_DEFAULT (0)
#define ROW_SEP_1   (ROW_DEFAULT + 1)
#define ROW_PALETTE (ROW_SEP_1   + 1)
#define ROW_SEP_2   (ROW_PALETTE + PALETTE_ROWS)
#define ROW_HISTORY (ROW_SEP_2   + 1)
#define ROW_SEP_3   (ROW_HISTORY + 1)
#define ROW_CUSTOM  (ROW_SEP_3   + 1)

/*===========================================*/
/* Private types                             */
/*===========================================*/

struct _glColorComboMenuPrivate {

        GtkWidget *default_menu_item;
        GtkWidget *custom_menu_item;

        GtkWidget *history_menu_item[PALETTE_COLS];

        guint      color;
};

enum {
        COLOR_CHANGED,
        LAST_SIGNAL
};

typedef struct {
        guint  color;
        gchar *name;
} ColorTableEntry;


/*===========================================*/
/* Private globals                           */
/*===========================================*/

static guint signals[LAST_SIGNAL] = {0};

static ColorTableEntry color_table[] =
{

        { GL_COLOR (139,   0,   0), N_("Dark Red") },
        { GL_COLOR (165,  42,  42), N_("Brown") },
        { GL_COLOR (205, 149,  12), N_("Dark Goldenrod") },
        { GL_COLOR (  0, 100,   0), N_("Dark Green") },
        { GL_COLOR (  0, 139, 139), N_("Dark Cyan") },
        { GL_COLOR (  0,   0, 128), N_("Navy Blue") },
        { GL_COLOR (148,   0, 211), N_("Dark Violet") },

        { GL_COLOR (255,   0,   0), N_("Red") },
        { GL_COLOR (255, 165,   0), N_("Orange") },
        { GL_COLOR (205, 205,   0), N_("Dark Yellow") },
        { GL_COLOR (  0, 205,   0), N_("Medium green") },
        { GL_COLOR ( 64, 224, 208), N_("Turquoise") },
        { GL_COLOR (  0,   0, 255), N_("Blue") },
        { GL_COLOR (160,  32, 240), N_("Purple") },

        { GL_COLOR (250, 128, 114), N_("Salmon") },
        { GL_COLOR (255, 215,   0), N_("Gold") },
        { GL_COLOR (255, 255,   0), N_("Yellow") },
        { GL_COLOR (  0, 255,   0), N_("Green") },
        { GL_COLOR (  0, 255, 255), N_("Cyan") },
        { GL_COLOR (135, 206, 235), N_("SkyBlue") },
        { GL_COLOR (238, 130, 238), N_("Violet") },

        { GL_COLOR (255, 192, 203), N_("Pink") },
        { GL_COLOR (255, 246, 143), N_("Khaki") },
        { GL_COLOR (255, 255, 224), N_("Light Yellow") },
        { GL_COLOR (144, 238, 144), N_("Light Green") },
        { GL_COLOR (224, 255, 255), N_("Light Cyan") },
        { GL_COLOR (198, 226, 255), N_("Slate Gray") },
        { GL_COLOR (216, 191, 216), N_("Thistle") },

        { GL_COLOR (255, 255, 255), N_("White") },
        /* xgettext: no-c-format */
        { GL_COLOR (230, 230, 230), N_("10% Gray") },
        /* xgettext: no-c-format */
        { GL_COLOR (192, 192, 192), N_("25% Gray") },
        /* xgettext: no-c-format */
        { GL_COLOR (153, 153, 153), N_("40% Gray") },
        /* xgettext: no-c-format */
        { GL_COLOR (128, 128, 128), N_("50% Gray") },
        /* xgettext: no-c-format */
        { GL_COLOR (102, 102, 102), N_("60% Gray") },
        { GL_COLOR (  0,   0,   0), N_("Black") },

};

static glColorHistoryModel *custom_color_history = NULL;


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_color_combo_menu_finalize  (GObject          *object);

static void load_custom_color_history     (glColorComboMenu *this);

static gboolean map_event_cb              (GtkWidget    *widget,
                                           GdkEventAny  *event);

static void default_menu_item_activate_cb (glColorComboMenu *this);
static void custom_menu_item_activate_cb  (glColorComboMenu *this);

static void palette_menu_item_activate_cb (GtkMenuItem      *item,
                                           glColorComboMenu *this);

static void history_menu_item_activate_cb (GtkMenuItem      *item,
                                           glColorComboMenu *this);


/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
G_DEFINE_TYPE (glColorComboMenu, gl_color_combo_menu, GTK_TYPE_MENU);


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
static void
gl_color_combo_menu_class_init (glColorComboMenuClass *class)
{
	GObjectClass   *gobject_class = G_OBJECT_CLASS (class);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);

        gl_color_combo_menu_parent_class = g_type_class_peek_parent (class);

	gobject_class->finalize = gl_color_combo_menu_finalize;

        widget_class->map_event = map_event_cb;

        signals[COLOR_CHANGED] =
                g_signal_new ("color_changed",
                              G_OBJECT_CLASS_TYPE (gobject_class),
                              G_SIGNAL_RUN_LAST,
                              G_STRUCT_OFFSET (glColorComboMenuClass, color_changed),
                              NULL, NULL,
                              gl_marshal_VOID__UINT_BOOLEAN,
                              G_TYPE_NONE,
                              2, G_TYPE_POINTER, G_TYPE_BOOLEAN);
}


/*****************************************************************************/
/* Object Instance Init Function.                                            */
/*****************************************************************************/
static void
gl_color_combo_menu_init (glColorComboMenu *this)
{
        GtkWidget *separator_menu_item;
        gint       i, i_row, i_col;
        GtkWidget *palette_menu_item;


        /*
         * Initialize history, if needed.
         */
        if ( !custom_color_history )
        {
                custom_color_history = gl_color_history_model_new (PALETTE_COLS);
        }


	this->priv = g_new0 (glColorComboMenuPrivate, 1);

        this->priv->default_menu_item = gtk_menu_item_new_with_label ("");
        gtk_menu_attach (GTK_MENU (this), this->priv->default_menu_item,
                         0, PALETTE_COLS, ROW_DEFAULT, ROW_DEFAULT+1);

        separator_menu_item = gtk_separator_menu_item_new ();
        gtk_menu_attach (GTK_MENU (this), separator_menu_item,
                         0, PALETTE_COLS, ROW_SEP_1, ROW_SEP_1+1);

        for ( i=0; i < G_N_ELEMENTS(color_table); i++ )
        {

                i_row = i / PALETTE_COLS;
                i_col = i % PALETTE_COLS;

                palette_menu_item = gl_color_combo_color_menu_item_new (i,
                                                                        color_table[i].color,
                                                                        gettext (color_table[i].name));
                g_signal_connect (palette_menu_item, "activate",
                                  G_CALLBACK (palette_menu_item_activate_cb), this);

                gtk_menu_attach (GTK_MENU (this), palette_menu_item,
                                 i_col, i_col+1,
                                 ROW_PALETTE+i_row, ROW_PALETTE+i_row+1);
        }

        separator_menu_item = gtk_separator_menu_item_new ();
        gtk_menu_attach (GTK_MENU (this), separator_menu_item,
                         0, PALETTE_COLS, ROW_SEP_2, ROW_SEP_2+1);

        for ( i=0; i < PALETTE_COLS; i++ )
        {
                this->priv->history_menu_item[i] =
                        gl_color_combo_color_menu_item_new (i, 0, NULL);
                gtk_widget_set_sensitive (this->priv->history_menu_item[i], FALSE);
                g_signal_connect (this->priv->history_menu_item[i], "activate",
                                  G_CALLBACK (history_menu_item_activate_cb), this);
                gtk_menu_attach (GTK_MENU (this), this->priv->history_menu_item[i],
                                 i, i+1,
                                 ROW_HISTORY, ROW_HISTORY+1);
        }

        separator_menu_item = gtk_separator_menu_item_new ();
        gtk_menu_attach (GTK_MENU (this), separator_menu_item,
                         0, PALETTE_COLS, ROW_SEP_3, ROW_SEP_3+1);

        this->priv->custom_menu_item = gtk_menu_item_new_with_label (_("Custom Color"));
        gtk_menu_attach (GTK_MENU (this), this->priv->custom_menu_item,
                         0, PALETTE_COLS, ROW_CUSTOM, ROW_CUSTOM+1);

        g_signal_connect_swapped (this->priv->default_menu_item, "activate",
                                  G_CALLBACK (default_menu_item_activate_cb), this);
        g_signal_connect_swapped (this->priv->custom_menu_item, "activate",
                                  G_CALLBACK (custom_menu_item_activate_cb), this);

}


/*****************************************************************************/
/* Finalize Method.                                                          */
/*****************************************************************************/
static void
gl_color_combo_menu_finalize (GObject *object)
{
	glColorComboMenu *this = GL_COLOR_COMBO_MENU (object);

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_COLOR_COMBO_MENU (object));

	g_free (this->priv);

	G_OBJECT_CLASS (gl_color_combo_menu_parent_class)->finalize (object);
}


/*****************************************************************************/
/** New Object Generator.                                                    */
/*****************************************************************************/
GtkWidget *
gl_color_combo_menu_new (const gchar *default_label,
                         guint        color)
{
	glColorComboMenu *this;

	this = g_object_new (gl_color_combo_menu_get_type (), NULL);

        gtk_label_set_text ( GTK_LABEL (gtk_bin_get_child (GTK_BIN (this->priv->default_menu_item))),
                            default_label);

        this->priv->color = color;

	return GTK_WIDGET (this);
}


/*****************************************************************************/
/* Load menu with custom color history.                                      */
/*****************************************************************************/
static void
load_custom_color_history (glColorComboMenu *this)
{
        guint  i;
        guint  color;
        gchar *tip;

        for ( i=0; i < PALETTE_COLS; i++ )
        {
                color = gl_color_history_model_get_color (custom_color_history, i);

                if (color)
                {
                        tip = g_strdup_printf (_("Custom Color #%u"), i+1);

                        gl_color_combo_color_menu_item_set_color (GL_COLOR_COMBO_COLOR_MENU_ITEM (this->priv->history_menu_item[i]),
                                                                  i,
                                                                  color,
                                                                  tip);
                        g_free (tip);
                        gtk_widget_set_sensitive (this->priv->history_menu_item[i], TRUE);
                }
        }
}


/*****************************************************************************/
/* Map event callback.                                                       */
/*****************************************************************************/
static gboolean
map_event_cb (GtkWidget    *widget,
              GdkEventAny  *event)
{
        glColorComboMenu *this = GL_COLOR_COMBO_MENU (widget);

        load_custom_color_history (this);

        return FALSE;
}


/*****************************************************************************/
/* "Default" menu_item activate callback.                                    */
/*****************************************************************************/
static void
default_menu_item_activate_cb (glColorComboMenu *this)
{
        g_signal_emit (this, signals[COLOR_CHANGED], 0,
                      NULL, TRUE);

        gtk_widget_hide (GTK_WIDGET (this));
}


/*****************************************************************************/
/* "Custom color" menu_item activate callback.                               */
/*****************************************************************************/
static void
custom_menu_item_activate_cb (glColorComboMenu *this)
{
        GtkWidget *custom_dialog;
        GtkWidget *colorsel;
        gint       response;
        GdkColor   color;

        gtk_widget_hide (GTK_WIDGET (this));

        custom_dialog = gtk_color_selection_dialog_new (_("Custom Color"));

        colorsel = gtk_color_selection_dialog_get_color_selection (GTK_COLOR_SELECTION_DIALOG (custom_dialog));
        color.red   = GL_COLOR_F_RED   (this->priv->color) * 65535;
        color.green = GL_COLOR_F_GREEN (this->priv->color) * 65535;
        color.blue  = GL_COLOR_F_BLUE  (this->priv->color) * 65535;

        gtk_color_selection_set_current_color (GTK_COLOR_SELECTION (colorsel),
                                               &color);

        response = gtk_dialog_run (GTK_DIALOG (custom_dialog));

        switch (response) {

        case GTK_RESPONSE_OK:
                gtk_color_selection_get_current_color (GTK_COLOR_SELECTION (colorsel),
                                                       &color);
                this->priv->color = GL_COLOR ((color.red   >>8),
                                              (color.green >>8),
                                              (color.blue  >>8));

                gl_color_history_model_add_color (custom_color_history,
                                                  this->priv->color);

                g_signal_emit (this, signals[COLOR_CHANGED], 0,
                               this->priv->color, FALSE);

                gtk_widget_destroy (custom_dialog);
                break;
        default:
                gtk_widget_destroy (custom_dialog);
                break;
        }
}


/*****************************************************************************/
/* Palette color changed callback.                                           */
/*****************************************************************************/
static void
palette_menu_item_activate_cb (GtkMenuItem      *item,
                               glColorComboMenu *this)
{
        gint i;

        i = gl_color_combo_color_menu_item_get_id (GL_COLOR_COMBO_COLOR_MENU_ITEM (item));

        this->priv->color = color_table[i].color;

        g_signal_emit (this, signals[COLOR_CHANGED], 0,
                       this->priv->color, FALSE);

        gtk_widget_hide (GTK_WIDGET (this));
}

/*****************************************************************************/
/* History color menu item callback.                                         */
/*****************************************************************************/
static void
history_menu_item_activate_cb (GtkMenuItem      *item,
                               glColorComboMenu *this)
{
        gint i;

        i = gl_color_combo_color_menu_item_get_id (GL_COLOR_COMBO_COLOR_MENU_ITEM (item));

        this->priv->color = gl_color_history_model_get_color (custom_color_history, i);

        g_signal_emit (this, signals[COLOR_CHANGED], 0,
                       this->priv->color, FALSE);

        gtk_widget_hide (GTK_WIDGET (this));
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
