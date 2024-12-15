
#define G_LOG_DOMAIN "phosh-desktop"

#include <gtk/gtk.h>
#include "phoshdesktop.h"

struct _PhoshDesktop
{
  GtkBox parent;
};

G_DEFINE_TYPE (PhoshDesktop, phoshdesktop, GTK_TYPE_BOX)

static gboolean
desktop_touch(GtkWidget* widget, GdkEventTouch* event)
{
  if(GTK_IS_CONTAINER(widget)) {
      GList *children = gtk_container_get_children(GTK_CONTAINER(widget));
      GList *iter = children;
      while (iter != NULL) {
          GtkWidget *child = iter->data;
          // Check the child's name, class, or other properties to identify the desired child
          
    g_debug ("desktop_touch_event widget=%s",gtk_widget_get_name(child));
          /*if (gtk_widget_get_name(child) == "desired_child_name") {
              // Found the desired child, use it as needed
              break;
          }*/
          iter = g_list_next(iter);
      }
    //gtk_widget_set_margin_top (children->, event->y);
  }
    return TRUE;
}

static void
phoshdesktop_constructed (GObject *object)
{
  g_print ("phoshdesktop_constructed\n");
  PhoshDesktop *self = PHOSH_DESKTOP (object);

  G_OBJECT_CLASS (phoshdesktop_parent_class)->constructed (object);

  gtk_style_context_add_class (
      gtk_widget_get_style_context (GTK_WIDGET (self)),
      "phosh-desktop\n");
}

static void
phoshdesktop_class_init (PhoshDesktopClass *klass)
{
  g_print ("phoshdesktop_class_init\n");
    GObjectClass *object_class = (GObjectClass *)klass;
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    object_class->constructed = phoshdesktop_constructed;


  widget_class->touch_event = desktop_touch;

  //gtk_widget_class_set_css_name (widget_class, "phosh-desktop");
}

static void
phoshdesktop_init (PhoshDesktop *self)
{
  g_print ("phoshdesktop_init\n");
}

GtkWidget *
phoshdesktop_new (void)
{
  g_print ("phoshdesktop_new\n");
    return GTK_WIDGET (g_object_new (PHOSH_TYPE_DESKTOP, NULL));
}