#include <gtk/gtk.h>
#include "phoshdesktop.h"

struct _PhoshDesktop {
    GtkBox parent;
};

struct _PhoshDesktopClass {
    GtkBoxClass parent_class;
};

G_DEFINE_TYPE(PhoshDesktop, phoshdesktop, GTK_TYPE_BOX)

static void phoshdesktop_init(PhoshDesktop *self) {
    g_print("PhoshDesktop Widget phoshdesktop_init");
}

static void phoshdesktop_class_init(PhoshDesktopClass *klass) {
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    gtk_widget_class_set_template_from_resource(widget_class, "/sm/puri/phosh/ui/phoshdesktop.ui");
    gtk_widget_class_bind_template_child(widget_class, PhoshDesktop, desktop_clock);
}

GtkWidget *phoshdesktop_new(void) {
    return GTK_WIDGET(g_object_new(PHOSH_TYPE_DESKTOP, NULL));
}

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    // Create a new instance of the custom widget
    PhoshDesktop *desktop = g_object_new(PHOSH_TYPE_DESKTOP, NULL);

    // Show the custom widget
    gtk_widget_show(GTK_WIDGET(desktop));

    // Run the main loop
    gtk_main();

    return 0;
}