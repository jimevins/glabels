/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef __GNOME_RECENT_MODEL_H__
#define __GNOME_RECENT_MODEL_H__

#include <libbonoboui.h>

G_BEGIN_DECLS

#define GNOME_RECENT_MODEL(obj)		G_TYPE_CHECK_INSTANCE_CAST (obj, gnome_recent_model_get_type (), GnomeRecentModel)
#define GNOME_RECENT_MODEL_CLASS(klass) 	G_TYPE_CHECK_CLASS_CAST (klass, gnome_recent_model_get_type (), GnomeRecentModelClass)
#define GNOME_IS_RECENT_MODEL(obj)		G_TYPE_CHECK_INSTANCE_TYPE (obj, gnome_recent_model_get_type ())

typedef struct _GnomeRecentModel GnomeRecentModel;

typedef struct _GnomeRecentModelClass GnomeRecentModelClass;

GType                gnome_recent_model_get_type     (void);

/* constructors */
GnomeRecentModel *   gnome_recent_model_new          (const gchar *appname,
						      gint limit);
GnomeRecentModel *   gnome_recent_model_new_global   (void);



/* public methods */
gboolean gnome_recent_model_add          (GnomeRecentModel *recent,
                                          const gchar *uri);
gboolean gnome_recent_model_delete       (GnomeRecentModel *recent,
                                          const gchar *uri);
void     gnome_recent_model_clear        (GnomeRecentModel *recent);
GSList * gnome_recent_model_get_list     (GnomeRecentModel *recent);
void     gnome_recent_model_set_limit    (GnomeRecentModel *recent,
                                          gint limit);
gint     gnome_recent_model_get_limit    (GnomeRecentModel *recent);
gchar   *gnome_recent_model_get_appname  (GnomeRecentModel *recent);

G_END_DECLS

#endif /* __GNOME_RECENT_MODEL_H__ */
