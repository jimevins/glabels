/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * mygal-combo-box.h - a customizable combobox
 * Copyright 2000, 2001, Ximian, Inc.
 *
 * Authors:
 *   Miguel de Icaza <miguel@ximian.com>
 *
 * Modified for gLabels by:
 *   Jim Evins <evins@snaught.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License, version 2, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef _MYGAL_COMBO_BOX_H_
#define _MYGAL_COMBO_BOX_H_

#include <gtk/gtkhbox.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MYGAL_COMBO_BOX_TYPE          (mygal_combo_box_get_type())
#define MYGAL_COMBO_BOX(obj)	      G_TYPE_CHECK_INSTANCE_CAST (obj, mygal_combo_box_get_type (), MygalComboBox)
#define MYGAL_COMBO_BOX_CLASS(klass)  G_TYPE_CHECK_CLASS_CAST (klass, mygal_combo_box_get_type (), MygalComboBoxClass)
#define MYGAL_IS_COMBO_BOX(obj)       G_TYPE_CHECK_INSTANCE_TYPE (obj, mygal_combo_box_get_type ())

typedef struct _MygalComboBox	     MygalComboBox;
typedef struct _MygalComboBoxPrivate MygalComboBoxPrivate;
typedef struct _MygalComboBoxClass   MygalComboBoxClass;

struct _MygalComboBox {
	GtkHBox hbox;
	MygalComboBoxPrivate *priv;
};

struct _MygalComboBoxClass {
	GtkHBoxClass parent_class;

	GtkWidget *(*pop_down_widget) (MygalComboBox *cbox);

	/*
	 * invoked when the popup has been hidden, if the signal
	 * returns TRUE, it means it should be killed from the
	 */ 
	gboolean  *(*pop_down_done)   (MygalComboBox *cbox, GtkWidget *);

	/*
	 * Notification signals.
	 */
	void      (*pre_pop_down)     (MygalComboBox *cbox);
	void      (*post_pop_hide)    (MygalComboBox *cbox);
};

GtkType    mygal_combo_box_get_type    (void);
void       mygal_combo_box_construct   (MygalComboBox *combo_box,
					GtkWidget   *display_widget,
					GtkWidget   *optional_pop_down_widget);
void       mygal_combo_box_get_pos     (MygalComboBox *combo_box, int *x, int *y);

GtkWidget *mygal_combo_box_new         (GtkWidget *display_widget,
					GtkWidget *optional_pop_down_widget);
void       mygal_combo_box_popup_hide  (MygalComboBox *combo_box);

void       mygal_combo_box_set_display (MygalComboBox *combo_box,
					GtkWidget *display_widget);

void       mygal_combo_box_set_title   (MygalComboBox *combo,
					const gchar *title);

void       mygal_combo_box_set_tearable        (MygalComboBox *combo,
						gboolean tearable);
void       mygal_combo_box_set_arrow_sensitive (MygalComboBox *combo,
						gboolean sensitive);
void       mygal_combo_box_set_arrow_relief    (MygalComboBox *cc,
						GtkReliefStyle relief);
#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* _MYGAL_COMBO_BOX_H_ */
