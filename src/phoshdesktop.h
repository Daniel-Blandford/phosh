#pragma once

#include "drag-surface.h"
#include "monitor/monitor.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#define PHOSH_TYPE_DESKTOP (phoshdesktop_get_type())

/**
 * PhoshDesktopState:
 * @PHOSH_DESKTOP_STATE_FOLDED: App carousel is visible
 * @PHOSH_DESKTOP_STATE_UNFOLDED: Desktop screen takes the whole screen except the app drawer
 *
 * The state of #PhoshDesktop.
 */
typedef enum {
  PHOSH_DESKTOP_STATE_FOLDED,
  PHOSH_DESKTOP_STATE_UNFOLDED,
} PhoshDesktopState;

GType phosh_desktop_state_get_type (void) G_GNUC_CONST;
#define PHOSH_TYPE_DESKTOP_STATE (phosh_desktop_state_get_type())

G_DECLARE_FINAL_TYPE (PhoshDesktop, phoshdesktop, PHOSH, DESKTOP, GtkBox)

GtkWidget *phoshdesktop_new (void);
PhoshDesktopState phosh_desktop_get_state (PhoshDesktop *self);
void           phosh_desktop_set_state (PhoshDesktop *self, PhoshDesktopState state);
void phosh_desktop_on_touch_event (GtkWidget *widget, GdkEventTouch *event, gpointer user_data);
// New function declarations
/*void on_touch_event (GtkWidget *widget, GdkEventTouch *event, gpointer user_data);*/
void on_motion_event (GtkEventControllerMotion *controller,
                      gdouble                   x,
                      gdouble                   y,
                      gpointer                  user_data);
// gboolean phoshdesktop_motion_notify_event (GtkWidget *widget, GdkEventMotion *event);
// gboolean phoshdesktop_scroll_event (GtkWidget *widget, GdkEventScroll *event);