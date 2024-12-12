#ifndef PHOSH_DESKTOP_H
#define PHOSH_DESKTOP_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PHOSH_TYPE_DESKTOP (phoshdesktop_get_type())
#define PHOSH_DESKTOP(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), PHOSH_TYPE_DESKTOP, PhoshDesktop))
#define PHOSH_DESKTOP_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), PHOSH_TYPE_DESKTOP, PhoshDesktopClass))
#define PHOSH_IS_DESKTOP(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), PHOSH_TYPE_DESKTOP))
#define PHOSH_IS_DESKTOP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), PHOSH_TYPE_DESKTOP))
#define PHOSH_DESKTOP_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), PHOSH_TYPE_DESKTOP, PhoshDesktopClass))

typedef struct _PhoshDesktop PhoshDesktop;
typedef struct _PhoshDesktopClass PhoshDesktopClass;

GType phoshdesktop_get_type(void);

GtkWidget *phoshdesktop_new(void);

G_END_DECLS

#endif /* PHOSH_DESKTOP_H */