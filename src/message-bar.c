/*
 *  message-bar.c
 *  Copyright (C) 2009  Jim Evins <evins@snaught.com>.
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

#include "message-bar.h"

#include "debug.h"


/*===========================================*/
/* Private data types                        */
/*===========================================*/

struct _glMessageBarPrivate {

        GtkWidget *icon;
        GtkWidget *label;

        gchar     *primary_text;
        gchar     *secondary_text;
};


/*===========================================*/
/* Private globals                           */
/*===========================================*/


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void       gl_message_bar_finalize       (GObject               *object);

static void       gl_message_bar_construct      (glMessageBar          *this,
                                                 GtkMessageType         type,
                                                 GtkButtonsType         buttons,
                                                 const gchar           *message_format,
                                                 va_list                args);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glMessageBar, gl_message_bar, GTK_TYPE_INFO_BAR)


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
static void
gl_message_bar_class_init (glMessageBarClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

  	gl_message_bar_parent_class = g_type_class_peek_parent (class);

  	object_class->finalize = gl_message_bar_finalize;  	
}


/*****************************************************************************/
/* Object Instance Init Function.                                            */
/*****************************************************************************/
static void
gl_message_bar_init (glMessageBar *dialog)
{
	g_return_if_fail (GL_IS_MESSAGE_BAR (dialog));

	dialog->priv = g_new0 (glMessageBarPrivate, 1);
}


/*****************************************************************************/
/* Finalize Function.                                                        */
/*****************************************************************************/
static void 
gl_message_bar_finalize (GObject *object)
{
	glMessageBar* dialog = GL_MESSAGE_BAR (object);;
	
	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_MESSAGE_BAR (dialog));
	g_return_if_fail (dialog->priv != NULL);

	g_free (dialog->priv->primary_text);
	g_free (dialog->priv->secondary_text);
	g_free (dialog->priv);

	G_OBJECT_CLASS (gl_message_bar_parent_class)->finalize (object);
}


/*****************************************************************************/
/* NEW message bar.                                                          */
/*****************************************************************************/
GtkWidget *
gl_message_bar_new (GtkMessageType  type,
                    GtkButtonsType  buttons,
                    const gchar    *message_format,
                    ...)
{
	GtkWidget *widget;
        va_list    args;

	widget = GTK_WIDGET (g_object_new (GL_TYPE_MESSAGE_BAR, NULL));
        
        va_start (args, message_format);
        gl_message_bar_construct (GL_MESSAGE_BAR (widget), type, buttons, message_format, args);
        va_end (args);

	return widget;
}


/*****************************************************************************/
/* Construct message bar.                                                    */
/*****************************************************************************/
static void
gl_message_bar_construct (glMessageBar    *this,
                          GtkMessageType   type,
                          GtkButtonsType   buttons,
                          const gchar     *message_format,
                          va_list          args)
{
        GtkWidget *hbox;

        gtk_info_bar_set_message_type (GTK_INFO_BAR (this), type);

        this->priv->icon = gtk_image_new ();
        switch (type)
        {
        case GTK_MESSAGE_INFO:
                gtk_image_set_from_stock (GTK_IMAGE (this->priv->icon),
                                          GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG);
                break;
        case GTK_MESSAGE_WARNING:
                gtk_image_set_from_stock (GTK_IMAGE (this->priv->icon),
                                          GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_DIALOG);
                break;
        case GTK_MESSAGE_QUESTION:
                gtk_image_set_from_stock (GTK_IMAGE (this->priv->icon),
                                          GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_DIALOG);
                break;
        case GTK_MESSAGE_ERROR:
                gtk_image_set_from_stock (GTK_IMAGE (this->priv->icon),
                                          GTK_STOCK_DIALOG_ERROR, GTK_ICON_SIZE_DIALOG);
                break;
        default:
                break;
        }

        this->priv->primary_text = g_strdup_vprintf (message_format, args);
        this->priv->label = gtk_label_new (this->priv->primary_text);

        hbox = gtk_info_bar_get_content_area (GTK_INFO_BAR (this));
        gtk_box_pack_start (GTK_BOX (hbox), this->priv->icon, FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (hbox), this->priv->label, FALSE, FALSE, 0);

        switch (buttons)
        {
        case GTK_BUTTONS_OK:
                gtk_info_bar_add_button (GTK_INFO_BAR (this),
                                         GTK_STOCK_OK, GTK_RESPONSE_ACCEPT);
                break;
        case GTK_BUTTONS_CLOSE:
                gtk_info_bar_add_button (GTK_INFO_BAR (this),
                                         GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT);
                break;
        case GTK_BUTTONS_CANCEL:
                gtk_info_bar_add_button (GTK_INFO_BAR (this),
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT);
                break;
        case GTK_BUTTONS_YES_NO:
                gtk_info_bar_add_button (GTK_INFO_BAR (this),
                                         GTK_STOCK_YES, GTK_RESPONSE_ACCEPT);
                gtk_info_bar_add_button (GTK_INFO_BAR (this),
                                         GTK_STOCK_NO, GTK_RESPONSE_REJECT);
                break;
        case GTK_BUTTONS_OK_CANCEL:
                gtk_info_bar_add_button (GTK_INFO_BAR (this),
                                         GTK_STOCK_OK, GTK_RESPONSE_ACCEPT);
                gtk_info_bar_add_button (GTK_INFO_BAR (this),
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT);
                break;
        default:
                break;
        }
}


/*****************************************************************************/
/* Set secondary text.                                                       */
/*****************************************************************************/
void
gl_message_bar_format_secondary_text (glMessageBar   *this,
                                      const gchar    *message_format,
                                      ...)
{
        va_list    args;
        gchar     *markup;

        g_free (this->priv->secondary_text);

        va_start (args, message_format);
        this->priv->secondary_text = g_strdup_vprintf (message_format, args);
        va_end (args);

        markup = g_strdup_printf ("<span weight=\"bold\" size=\"larger\">%s</span>\n%s",
                                  this->priv->primary_text,
                                  this->priv->secondary_text);

        gtk_label_set_markup (GTK_LABEL (this->priv->label), markup);

        g_free (markup);
}






/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
