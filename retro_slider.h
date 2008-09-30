/* retro_slider.h */

#ifndef __RETRO_SLIDER_H
#define __RETRO_SLIDER_H

#include <gtk/gtk.h>
#include <cairo.h>

G_BEGIN_DECLS


#define GTK_rslider(obj) GTK_CHECK_CAST(obj, gtk_rslider_get_type (), GtkRslider)
#define GTK_rslider_CLASS(klass) GTK_CHECK_CLASS_CAST(klass, gtk_rslider_get_type(), GtkRsliderClass)
#define GTK_IS_rslider(obj) GTK_CHECK_TYPE(obj, gtk_rslider_get_type())


typedef struct _GtkRslider GtkRslider;
typedef struct _GtkRsliderClass GtkRsliderClass;


struct _GtkRslider {
  GtkWidget widget;

  gint sel;
  gint bar_height;
  gint bar_width;
  gint bar_margins;
  gint seg_thickness;
  gint seg_spacing;
};

struct _GtkRsliderClass {
  GtkWidgetClass parent_class;
};


GtkType gtk_rslider_get_type(void);
void gtk_rslider_set_sel(GtkRslider *rslider, gint sel);
GtkWidget * gtk_rslider_new();


G_END_DECLS

#endif /* __RETRO_SLIDER_H */
