#include <gtk/gtk.h>
#include "phoshdesktop.h"

struct _PhoshDesktop
{
  GtkBox parent;
};

G_DEFINE_TYPE (PhoshDesktop, phoshdesktop, GTK_TYPE_BOX)

static void
phoshdesktop_constructed (GObject *object)
{
  PhoshDesktop *self = PHOSH_DESKTOP (object);

  G_OBJECT_CLASS (phoshdesktop_parent_class)->constructed (object);

  gtk_style_context_add_class (
      gtk_widget_get_style_context (GTK_WIDGET (self)),
      "phosh-desktop");
}

static void
phoshdesktop_class_init (PhoshDesktopClass *klass)
{
    GObjectClass *object_class = (GObjectClass *)klass;
    object_class->constructed = phoshdesktop_constructed;
}

static void
phoshdesktop_init (PhoshDesktop *self)
{
}

GtkWidget *
phoshdesktop_new (void)
{
    return GTK_WIDGET (g_object_new (PHOSH_TYPE_DESKTOP, NULL));
}