#include <gtk/gtk.h>

typedef struct _PhoshDesktop PhoshDesktop;
//typedef struct _PhoshDesktopClass PhoshDesktopClass;

struct _PhoshDesktop {
    //GtkBox parent;
} _PhoshDesktop;


G_DEFINE_TYPE(PhoshDesktop, phoshdesktop, GTK_TYPE_BOX)

static void phoshdesktop_init(PhoshDesktop *self) {
    GtkBox *box = GTK_BOX(self);
    gtk_widget_init_template(GTK_WIDGET(self));
    gtk_box_pack_end(box, gtk_label_new("12:34:56"), FALSE, FALSE, 0);
}

static void phoshdesktop_class_init(PhoshDesktopClass *klass) {
    GtkBoxClass *box_class = GTK_BOX_CLASS(klass);
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(klass), "/sm/puri/phosh/ui/phoshdesktop.ui");
}

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    // Register the type of the custom widget
    phoshdesktop_get_type();

    // Load the UI file
    GtkBuilder *builder = gtk_builder_new_from_file("phoshdesktop.ui");
    return 0;
}