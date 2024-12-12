// phoshdesktop.h
#ifndef PHOSH_DESKTOP_H
#define PHOSH_DESKTOP_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PHOSH_TYPE_DESKTOP (phoshdesktop_get_type())
G_DECLARE_DERIVABLE_TYPE (PhoshDesktop, phoshdesktop, PHOSH, DESKTOP, GtkBox)

GType phoshdesktop_get_type(void);

struct _PhoshDesktopClass
{
  GtkBoxClass parent_class;
};

GtkWidget *phoshdesktop_new (void);

G_END_DECLS

#endif // PHOSH_DESKTOP_H