/* retro_slider.c */
/*
  The bulk of this code was reused from the example at
  http://zetcode.com/tutorials/gtktutorial/customwidget/
  Thank you much.
*/

#include "retro_slider.h"

#define RSLIDER_DEFAULT_HEIGHT 160
#define RSLIDER_DEFAULT_WIDTH 20
#define RSLIDER_DEFAULT_MARGINS 2
#define RSLIDER_DEFAULT_SEG_THICKNESS 3
#define RSLIDER_DEFAULT_SEG_SPACING 4


static void gtk_rslider_class_init(GtkRsliderClass *klass);
static void gtk_rslider_init(GtkRslider *rslider);
static void gtk_rslider_size_request(GtkWidget *widget,
    GtkRequisition *requisition);
static void gtk_rslider_size_allocate(GtkWidget *widget,
    GtkAllocation *allocation);
static void gtk_rslider_realize(GtkWidget *widget);
static gboolean gtk_rslider_expose(GtkWidget *widget,
    GdkEventExpose *event);
static void gtk_rslider_paint(GtkWidget *widget);
static void gtk_rslider_destroy(GtkObject *object);


GtkType
gtk_rslider_get_type(void)
{
  static GtkType gtk_rslider_type = 0;


  if (!gtk_rslider_type) {
      static const GtkTypeInfo gtk_rslider_info = {
          "GtkRslider",
          sizeof(GtkRslider),
          sizeof(GtkRsliderClass),
          (GtkClassInitFunc) gtk_rslider_class_init,
          (GtkObjectInitFunc) gtk_rslider_init,
          NULL,
          NULL,
          (GtkClassInitFunc) NULL
      };
      gtk_rslider_type = gtk_type_unique(GTK_TYPE_WIDGET, &gtk_rslider_info);
  }


  return gtk_rslider_type;
}

void
gtk_rslider_set_state(GtkRslider *rslider, gint num)
{
   rslider->sel = num;
   gtk_rslider_paint(GTK_WIDGET(rslider));
}


GtkWidget * gtk_rslider_new()
{
   return GTK_WIDGET(gtk_type_new(gtk_rslider_get_type()));
}


static void
gtk_rslider_class_init(GtkRsliderClass *klass)
{
  GtkWidgetClass *widget_class;
  GtkObjectClass *object_class;


  widget_class = (GtkWidgetClass *) klass;
  object_class = (GtkObjectClass *) klass;

  widget_class->realize = gtk_rslider_realize;
  widget_class->size_request = gtk_rslider_size_request;
  widget_class->size_allocate = gtk_rslider_size_allocate;
  widget_class->expose_event = gtk_rslider_expose;

  object_class->destroy = gtk_rslider_destroy;
}


static void
gtk_rslider_init(GtkRslider *rslider)
{
   rslider->sel = 0;
   rslider->bar_height = RSLIDER_DEFAULT_HEIGHT;
   rslider->bar_width = RSLIDER_DEFAULT_WIDTH;
   rslider->bar_margins = RSLIDER_DEFAULT_MARGINS;
   rslider->seg_thickness = RSLIDER_DEFAULT_SEG_THICKNESS;
   rslider->seg_spacing = RSLIDER_DEFAULT_SEG_SPACING;
}


static void
gtk_rslider_size_request(GtkWidget *widget,
    GtkRequisition *requisition)
{
  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_rslider(widget));
  g_return_if_fail(requisition != NULL);

  requisition->width = RSLIDER_DEFAULT_WIDTH;
  requisition->height = RSLIDER_DEFAULT_HEIGHT;
}


static void
gtk_rslider_size_allocate(GtkWidget *widget,
    GtkAllocation *allocation)
{
  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_rslider(widget));
  g_return_if_fail(allocation != NULL);

  widget->allocation = *allocation;

  if (GTK_WIDGET_REALIZED(widget)) {
     gdk_window_move_resize(
         widget->window,
         allocation->x, allocation->y,
         allocation->width, allocation->height
     );
   }
}


static void
gtk_rslider_realize(GtkWidget *widget)
{
  GdkWindowAttr attributes;
  guint attributes_mask;

  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_rslider(widget));

  GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  GTK_rslider(widget)->bar_height = attributes.height;
  GTK_rslider(widget)->bar_width = attributes.width;

  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;

  attributes_mask = GDK_WA_X | GDK_WA_Y;

  widget->window = gdk_window_new(
     gtk_widget_get_parent_window (widget),
     & attributes, attributes_mask
  );

  gdk_window_set_user_data(widget->window, widget);

  widget->style = gtk_style_attach(widget->style, widget->window);
  gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);
}


static gboolean
gtk_rslider_expose(GtkWidget *widget,
    GdkEventExpose *event)
{
  g_return_val_if_fail(widget != NULL, FALSE);
  g_return_val_if_fail(GTK_IS_rslider(widget), FALSE);
  g_return_val_if_fail(event != NULL, FALSE);

  gtk_rslider_paint(widget);

  return FALSE;
}


static void
gtk_rslider_paint(GtkWidget *widget)
{
  cairo_t *cr;

  cr = gdk_cairo_create(widget->window);

  gint bar_height = GTK_rslider(widget)->bar_height;
  gint bar_width = GTK_rslider(widget)->bar_width;
  gint bar_margins = GTK_rslider(widget)->bar_margins;
  gint seg_thickness = GTK_rslider(widget)->seg_thickness;
  gint seg_spacing = GTK_rslider(widget)->seg_spacing;


  cairo_translate(cr, 0, bar_margins+(seg_spacing-seg_thickness));

  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_paint(cr);

  gint num_bars;
  num_bars = (bar_height-2*bar_margins)/seg_spacing;

  gint pos = GTK_rslider(widget)->sel;
  gint rect = pos * num_bars / 100;

  gint i;
  for ( i = 0; i < num_bars; i++) {
      if (i > num_bars - rect - 1) {
          cairo_set_source_rgb(cr, 1.0, 0.8, 0.0);
      } else {
          cairo_set_source_rgb(cr, 0.6, 0.2, 0.0);
      }
      
      cairo_rectangle(cr, bar_margins, i*seg_spacing, bar_width-(2*bar_margins), seg_thickness);
      cairo_fill(cr);
  }

  cairo_destroy(cr);
}


static void
gtk_rslider_destroy(GtkObject *object)
{
  GtkRslider *rslider;
  GtkRsliderClass *klass;

  g_return_if_fail(object != NULL);
  g_return_if_fail(GTK_IS_rslider(object));

  rslider = GTK_rslider(object);

  klass = gtk_type_class(gtk_widget_get_type());

  if (GTK_OBJECT_CLASS(klass)->destroy) {
     (* GTK_OBJECT_CLASS(klass)->destroy) (object);
  }
}
