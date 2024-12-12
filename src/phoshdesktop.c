#include <gtk/gtk.h>
#include "phoshdesktop.h"

G_DEFINE_TYPE (PhoshDesktop, phoshdesktop, GTK_TYPE_BOX)

static void
phoshdesktop_init (PhoshDesktop *self)
{
    gtk_widget_init_template (GTK_WIDGET (self));
}

static void
phoshdesktop_class_init (PhoshDesktopClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    gtk_widget_class_set_template_from_resource (widget_class, "/sm/puri/phosh/ui/phoshdesktop.ui");
    //gtk_widget_class_bind_template_child (widget_class, PhoshDesktop, desktop_clock);
}

GtkWidget *
phoshdesktop_new (void)
{
    return GTK_WIDGET (g_object_new (PHOSH_TYPE_DESKTOP, NULL));
}