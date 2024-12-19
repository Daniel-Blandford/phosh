
#define G_LOG_DOMAIN "phosh-desktop"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "phoshdesktop.h"

#include "phosh-config.h"
#include "phosh-enums.h"
#include "feedback-manager.h"
#include "util.h"

#include <handy.h>

#define PHOSH_DESKTOP_DRAG_THRESHOLD 0.3

enum {
  PROP_0,
  PROP_DESKTOP_STATE,
  PROP_OSK_ENABLED,
  PROP_LAST_PROP,
};
static GParamSpec *props[PROP_LAST_PROP];

struct _PhoshDesktop
{
  GtkBox parent;

  PhoshDesktopState state;

  GtkGesture     *click_gesture; /* needed so that the gesture isn't destroyed immediately */

  PhoshMonitor    *monitor;
  //PhoshBackground *background;
  gboolean         use_background;
};

G_DEFINE_TYPE (PhoshDesktop, phoshdesktop, GTK_TYPE_BOX)

GType
phosh_desktop_state_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { PHOSH_DESKTOP_STATE_FOLDED, "PHOSH_DESKTOP_STATE_FOLDED", "folded" },
      { PHOSH_DESKTOP_STATE_UNFOLDED, "PHOSH_DESKTOP_STATE_UNFOLDED", "unfolded" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("PhoshDesktopState", values);
  }
  return etype;
}

static gboolean
desktop_touch(GtkWidget* widget, GdkEventTouch* event)
{
  g_print ("phoshdesktop - desktop_touch\n");
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

static gboolean
phoshdesktop_motion_notify_event (GtkWidget *widget, GdkEventMotion *event)
{
  //g_debug ("IT LIVES!!! Phoshdesktop - motion_notify_event: x=%f, y=%f\n", event->x, event->y);
  gfloat initial_y = event->y;
  // ...
  return TRUE;
}

static gboolean
phoshdesktop_scroll_event (GtkWidget *widget, GdkEventScroll *event)
{
  g_debug ("phoshdesktop - scroll_event: direction=%d, x=%f, y=%f\n", 
           event->direction, event->x, event->y);
  // Add your scroll handling logic here
  return TRUE;
}

static void
on_drag_state_changed (PhoshDesktop *self)
{
  g_print ("phoshdesktop - on_drag_state_changed\n");
  PhoshDesktopState state = self->state;
  PhoshDragSurfaceState drag_state;
  //gboolean kbd_interactivity = FALSE;

  drag_state = phosh_drag_surface_get_drag_state (PHOSH_DRAG_SURFACE (self));

  switch (drag_state) {
  case PHOSH_DRAG_SURFACE_STATE_UNFOLDED:
    state = PHOSH_DESKTOP_STATE_UNFOLDED;
    /*kbd_interactivity = TRUE;
    if (self->focus_app_search) {
      phosh_overview_focus_app_search (PHOSH_OVERVIEW (self->overview));
      self->focus_app_search = FALSE;
    }
    phosh_desktop_set_background_alpha (self, 1.0);*/
    break;
  case PHOSH_DRAG_SURFACE_STATE_FOLDED:
    state = PHOSH_DESKTOP_STATE_FOLDED;
    //phosh_desktop_set_background_alpha (self, 0.0);
    break;
  case PHOSH_DRAG_SURFACE_STATE_DRAGGED:
    //if (self->state == PHOSH_DESKTOP_STATE_FOLDED)
    //  phosh_overview_reset (PHOSH_OVERVIEW (self->overview));
    break;
  default:
    g_return_if_reached ();
    return;
  }

  if (self->state != state) {
    self->state = state;
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DESKTOP_STATE]);
  }

  //phosh_desktop_update_desktop_bar (self);

  //phosh_layer_surface_set_kbd_interactivity (PHOSH_LAYER_SURFACE (self), kbd_interactivity);
  //update_drag_handle (self, FALSE);
  gtk_widget_queue_draw (GTK_WIDGET (self));
}

static void
phoshdesktop_constructed (GObject *object)
{
  g_print ("phoshdesktop_constructed\n");
  PhoshDesktop *self = PHOSH_DESKTOP (object);

  G_OBJECT_CLASS (phoshdesktop_parent_class)->constructed (object);

  //g_signal_connect (self, "notify::drag-state", G_CALLBACK (on_drag_state_changed), NULL);

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

    widget_class->motion_notify_event = phoshdesktop_motion_notify_event;
    widget_class->scroll_event = phoshdesktop_scroll_event;
    widget_class->touch_event = desktop_touch;

    object_class->constructed = phoshdesktop_constructed;

  /**
   * PhoshDesktop:state:
   *
   * Whether the desktop widget is currently folded (app carousel is visible) or unfolded (desktop is fullscreen). The property is
   * changed when the widget reaches it's target state.
   */
  props[PROP_DESKTOP_STATE] =
    g_param_spec_enum ("state", "", "",
                       PHOSH_TYPE_DESKTOP_STATE,
                       PHOSH_DESKTOP_STATE_FOLDED,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

  //gtk_widget_class_set_css_name (widget_class, "phosh-desktop");
}

void
phosh_desktop_on_touch_event (GtkWidget *widget, GdkEventTouch *event, gpointer user_data)
{
  g_debug ("PhoshDesktop touch event: type=%d, x=%f, y=%f",
           event->type, event->x, event->y);
  // Add your touch event handling logic here
}

void
on_motion_event (GtkEventControllerMotion *controller,
                 gdouble                   x,
                 gdouble                   y,
                 gpointer                  user_data)
{
  g_debug ("Actual PhoshDesktop motion event: x=%f, y=%f", x, y);
  // Add your motion event handling logic here
}

static void
phoshdesktop_init (PhoshDesktop *self)
{
  GtkEventController *motion_controller;

  g_print ("phoshdesktop_init\n");

  gtk_widget_add_events(GTK_WIDGET(self), 
                        GDK_POINTER_MOTION_MASK |
                        GDK_TOUCH_MASK |
                        GDK_BUTTON_PRESS_MASK |
                        GDK_BUTTON_RELEASE_MASK |
                        GDK_SCROLL_MASK);  // Add scroll mask

  gtk_widget_set_sensitive(GTK_WIDGET(self), TRUE);
  gtk_widget_set_visible(GTK_WIDGET(self), TRUE);

  // Connect to the "touch-event" signal
  g_signal_connect (self, "touch-event", G_CALLBACK (phosh_desktop_on_touch_event), NULL);

  // Connect to the "motion-notify-event" signal
  g_signal_connect (self, "motion-notify-event", G_CALLBACK (phoshdesktop_motion_notify_event), NULL);

  // Connect to the "scroll-event" signal
  g_signal_connect (self, "scroll-event", G_CALLBACK (phoshdesktop_scroll_event), NULL);

  // Add a motion controller for tracking motion events
  motion_controller = GTK_EVENT_CONTROLLER (gtk_event_controller_motion_new (GTK_WIDGET (self)));
  g_signal_connect (motion_controller, "motion", G_CALLBACK (on_motion_event), self);
}

GtkWidget *
phoshdesktop_new (void)
{
  g_print ("phoshdesktop_new\n");
    return GTK_WIDGET (g_object_new (PHOSH_TYPE_DESKTOP, NULL));
}

/**
 * phosh_desktop_get_state:
 * @self: The desktop surface
 *
 * Get the current state of the desktop widget. See [property@Desktop:state] for details.
 *
 * Returns: The desktop widget's state
 */
PhoshDesktopState
phosh_desktop_get_state (PhoshDesktop *self)
{
  g_return_val_if_fail (PHOSH_IS_DESKTOP (self), PHOSH_DESKTOP_STATE_FOLDED);

  return self->state;
}

/**
 * phosh_desktop_set_state:
 * @self: The desktop surface
 * @state: The state to set
 *
 * Set the state of the desktop screen. See #PhoshDesktopState.
 */
void
phosh_desktop_set_state (PhoshDesktop *self, PhoshDesktopState state)
{
  g_autofree char *state_name = NULL;
  PhoshDragSurfaceState drag_state = phosh_drag_surface_get_drag_state (PHOSH_DRAG_SURFACE (self));
  PhoshDragSurfaceState target_state = PHOSH_DRAG_SURFACE_STATE_FOLDED;

  g_return_if_fail (PHOSH_IS_DESKTOP (self));

  if (self->state == state)
    return;

  if (drag_state == PHOSH_DRAG_SURFACE_STATE_DRAGGED)
    return;

  state_name = g_enum_to_string (PHOSH_TYPE_DESKTOP_STATE, state);
  g_debug ("Setting state to %s", state_name);

  if (state == PHOSH_DESKTOP_STATE_UNFOLDED)
    target_state = PHOSH_DRAG_SURFACE_STATE_UNFOLDED;

  phosh_drag_surface_set_drag_state (PHOSH_DRAG_SURFACE (self), target_state);
}
