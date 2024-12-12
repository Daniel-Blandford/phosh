#pragma once

#include <gtk/gtk.h>

#define PHOSH_TYPE_DESKTOP (phoshdesktop_get_type())

G_DECLARE_FINAL_TYPE (PhoshDesktop, phoshdesktop, PHOSH, DESKTOP, GtkBox)

GtkWidget *phoshdesktop_new (void);