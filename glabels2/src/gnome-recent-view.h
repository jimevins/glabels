#ifndef __GNOME_RECENT_VIEW_H__
#define __GNOME_RECENT_VIEW_H__


#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>
#include "gnome-recent-model.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GNOME_TYPE_RECENT_VIEW             (gnome_recent_view_get_type ())
#define GNOME_RECENT_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GNOME_TYPE_RECENT_VIEW, GnomeRecentView))
#define GNOME_RECENT_VIEW_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), GNOME_TYPE_RECENT_VIEW, GnomeRecentViewClass))
#define GNOME_IS_RECENT_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GNOME_TYPE_RECENT_VIEW))
#define GNOME_IS_RECENT_VIEW_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), GNOME_TYPE_RECENT_VIEW))
#define GNOME_RECENT_VIEW_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), GNOME_TYPE_RECENT_VIEW, GnomeRecentViewClass))

typedef struct _GnomeRecentView       GnomeRecentView;         /* Dummy typedef */
typedef struct _GnomeRecentViewClass  GnomeRecentViewClass;

struct _GnomeRecentViewClass
{
  GTypeInterface		   base_iface;
  
  /* vtable, not signals */
  void (* do_clear)				(GnomeRecentView *view);
  void (* do_set_model)				(GnomeRecentView *view,
						 GnomeRecentModel *model);
  GnomeRecentModel * (* do_get_model)		(GnomeRecentView *view);
};

GtkType  gnome_recent_view_get_type		(void) G_GNUC_CONST;
void     gnome_recent_view_set_list		(GnomeRecentView *view,
						 GSList *list);
void     gnome_recent_view_clear		(GnomeRecentView *view);
GnomeRecentModel *gnome_recent_view_get_model   (GnomeRecentView *view);
void	 gnome_recent_view_set_model		(GnomeRecentView *view,
						 GnomeRecentModel *model);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GNOME_RECENT_VIEW_H__ */
