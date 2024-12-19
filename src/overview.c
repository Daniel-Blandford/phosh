/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Author: Guido Günther <agx@sigxcpu.org>
 */

#define G_LOG_DOMAIN "phosh-overview"

#include "phosh-config.h"
#include <glib.h>
#include "activity.h"
#include "app-grid-button.h"
#include "app-grid.h"
#include "phoshdesktop.h"
#include "overview.h"
#include "wlr-screencopy-unstable-v1-client-protocol.h"
#include "phosh-private-client-protocol.h"
#include "phosh-wayland.h"
#include "shell.h"
#include "toplevel-manager.h"
#include "toplevel-thumbnail.h"
#include "swipe-away-bin.h"
#include "util.h"

#include <gio/gdesktopappinfo.h>

#include <handy.h>

#define OVERVIEW_ICON_SIZE 64

/**
 * PhoshOverview:
 *
 * The overview shows running apps and the app grid to launch new
 * applications.
 *
 * The #PhoshOverview shows running apps (#PhoshActivity) and
 * the app grid (#PhoshAppGrid) to launch new applications.
 */

enum {
  ACTIVITY_LAUNCHED,
  ACTIVITY_RAISED,
  ACTIVITY_CLOSED,
  SELECTION_ABORTED,
  N_SIGNALS
};
static guint signals[N_SIGNALS] = { 0 };

enum {
  PROP_0,
  PROP_HAS_ACTIVITIES,
  LAST_PROP,
};
static GParamSpec *props[LAST_PROP];


typedef struct
{
  /* Running activities */
  GtkWidget *carousel_running_activities;
  GtkWidget *app_grid;
  GtkWidget *phoshdesktop;
  GtkWidget *app_grid_positional_wrapper;
  PhoshActivity *activity;

  int       has_activities;
} PhoshOverviewPrivate;

struct _PhoshOverview
{
  GtkBox parent;
  PhoshDesktop *phoshdesktop;
  GtkWidget *app_grid_positional_wrapper;
  gdouble drag_start_x;
  gdouble drag_start_y;
  gboolean is_dragging;
  gint64 last_motion_event_time;
  gint initial_height;
  gint current_height;
  PhoshOverviewPrivate *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE (PhoshOverview, phosh_overview, GTK_TYPE_BOX)

static gboolean on_button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data);

static void
phosh_overview_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  PhoshOverview *self = PHOSH_OVERVIEW (object);
  PhoshOverviewPrivate *priv = phosh_overview_get_instance_private (self);

  switch (property_id) {
  case PROP_HAS_ACTIVITIES:
    g_value_set_boolean (value, priv->has_activities);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static PhoshToplevel *
get_toplevel_from_activity (PhoshActivity *activity)
{
  PhoshToplevel *toplevel;
  g_return_val_if_fail (PHOSH_IS_ACTIVITY (activity), NULL);
  toplevel = g_object_get_data (G_OBJECT (activity), "toplevel");
  g_return_val_if_fail (PHOSH_IS_TOPLEVEL (toplevel), NULL);

  return toplevel;
}


static PhoshActivity *
find_activity_by_toplevel (PhoshOverview        *self,
                           PhoshToplevel        *needle)
{
  g_autoptr(GList) children;
  PhoshActivity *activity = NULL;
  PhoshOverviewPrivate *priv = phosh_overview_get_instance_private (self);

  children = gtk_container_get_children (GTK_CONTAINER (priv->carousel_running_activities));
  for (GList *l = children; l; l = l->next) {
    PhoshToplevel *toplevel;

    activity = PHOSH_ACTIVITY (l->data);
    toplevel = get_toplevel_from_activity (activity);
    if (toplevel == needle)
      break;
  }

  g_return_val_if_fail (activity, NULL);
  return activity;
}


static void
scroll_to_activity (PhoshOverview *self, PhoshActivity *activity)
{
  PhoshOverviewPrivate *priv = phosh_overview_get_instance_private (self);
  hdy_carousel_scroll_to (HDY_CAROUSEL (priv->carousel_running_activities), GTK_WIDGET (activity));
  gtk_widget_grab_focus (GTK_WIDGET (activity));
}

static void
on_activity_clicked (PhoshOverview *self, PhoshActivity *activity)
{
  PhoshToplevel *toplevel;
  g_return_if_fail (PHOSH_IS_OVERVIEW (self));
  g_return_if_fail (PHOSH_IS_ACTIVITY (activity));

  toplevel = get_toplevel_from_activity (activity);
  g_return_if_fail (toplevel);

  g_debug("Will raise %s (%s)",
          phosh_activity_get_app_id (activity),
          phosh_toplevel_get_title (toplevel));

  phosh_toplevel_activate (toplevel, phosh_wayland_get_wl_seat (phosh_wayland_get_default ()));
  g_signal_emit (self, signals[ACTIVITY_RAISED], 0);
}


static void
on_activity_closed (PhoshOverview *self, PhoshActivity *activity)
{
  PhoshToplevel *toplevel;

  g_return_if_fail (PHOSH_IS_OVERVIEW (self));
  g_return_if_fail (PHOSH_IS_ACTIVITY (activity));

  toplevel = g_object_get_data (G_OBJECT (activity), "toplevel");
  g_return_if_fail (PHOSH_IS_TOPLEVEL (toplevel));

  g_debug ("Will close %s (%s)",
           phosh_activity_get_app_id (activity),
           phosh_toplevel_get_title (toplevel));

  phosh_toplevel_close (toplevel);
  phosh_trigger_feedback ("window-close");
  g_signal_emit (self, signals[ACTIVITY_CLOSED], 0);
}


static void
on_toplevel_closed (PhoshToplevel *toplevel, PhoshOverview *overview)
{
  PhoshActivity *activity;
  PhoshOverviewPrivate *priv;

  g_return_if_fail (PHOSH_IS_TOPLEVEL (toplevel));
  g_return_if_fail (PHOSH_IS_OVERVIEW (overview));
  priv = phosh_overview_get_instance_private (overview);

  activity = find_activity_by_toplevel (overview, toplevel);
  g_return_if_fail (PHOSH_IS_ACTIVITY (activity));
  gtk_widget_destroy (GTK_WIDGET (activity));

  if (priv->activity == activity)
    priv->activity = NULL;
}


static void
on_toplevel_activated_changed (PhoshToplevel *toplevel, GParamSpec *pspec, PhoshOverview *overview)
{
  PhoshActivity *activity;
  PhoshOverviewPrivate *priv;
  g_return_if_fail (PHOSH_IS_OVERVIEW (overview));
  g_return_if_fail (PHOSH_IS_TOPLEVEL (toplevel));
  priv = phosh_overview_get_instance_private (overview);

  if (phosh_toplevel_is_activated (toplevel)) {
    activity = find_activity_by_toplevel (overview, toplevel);
    priv->activity = activity;
    scroll_to_activity (overview, activity);
  }
}


static void
on_thumbnail_ready_changed (PhoshThumbnail *thumbnail, GParamSpec *pspec, PhoshActivity *activity)
{
  g_return_if_fail (PHOSH_IS_THUMBNAIL (thumbnail));
  g_return_if_fail (PHOSH_IS_ACTIVITY (activity));

  phosh_activity_set_thumbnail (activity, thumbnail);
}


static void
request_thumbnail (PhoshActivity *activity, PhoshToplevel *toplevel)
{
  PhoshToplevelThumbnail *thumbnail;
  GtkAllocation allocation;
  int scale;
  g_return_if_fail (PHOSH_IS_ACTIVITY (activity));
  g_return_if_fail (PHOSH_IS_TOPLEVEL (toplevel));
  scale = gtk_widget_get_scale_factor (GTK_WIDGET (activity));
  phosh_activity_get_thumbnail_allocation (activity, &allocation);
  thumbnail = phosh_toplevel_thumbnail_new_from_toplevel (toplevel, allocation.width * scale, allocation.height * scale);
  g_signal_connect_object (thumbnail, "notify::ready", G_CALLBACK (on_thumbnail_ready_changed), activity, 0);
}


static void
on_activity_resized (PhoshActivity *activity, GtkAllocation *alloc, PhoshToplevel *toplevel)
{
  request_thumbnail (activity, toplevel);
}


static void
on_activity_has_focus_changed (PhoshOverview *self, GParamSpec *pspec, PhoshActivity *activity)
{
  PhoshOverviewPrivate *priv;

  g_return_if_fail (PHOSH_IS_ACTIVITY (activity));
  g_return_if_fail (PHOSH_IS_OVERVIEW (self));
  priv = phosh_overview_get_instance_private (self);

  if (gtk_widget_has_focus (GTK_WIDGET (activity)))
    hdy_carousel_scroll_to (HDY_CAROUSEL (priv->carousel_running_activities), GTK_WIDGET (activity));
}


static void
add_activity (PhoshOverview *self, PhoshToplevel *toplevel)
{
  PhoshOverviewPrivate *priv;
  GtkWidget *activity;
  const char *app_id, *title;
  const char *parent_app_id = NULL;
  int width, height;
  PhoshToplevelManager *m = phosh_shell_get_toplevel_manager (phosh_shell_get_default ());
  PhoshToplevel *parent = NULL;

  g_return_if_fail (PHOSH_IS_OVERVIEW (self));
  priv = phosh_overview_get_instance_private (self);

  app_id = phosh_toplevel_get_app_id (toplevel);
  title = phosh_toplevel_get_title (toplevel);

  if (phosh_toplevel_get_parent_handle (toplevel))
    parent = phosh_toplevel_manager_get_parent (m, toplevel);
  if (parent)
    parent_app_id = phosh_toplevel_get_app_id (parent);

  g_debug ("Building activator for '%s' (%s)", app_id, title);
  phosh_shell_get_usable_area (phosh_shell_get_default (), NULL, NULL, &width, &height);
  activity = g_object_new (PHOSH_TYPE_ACTIVITY,
                           "app-id", app_id,
                           "parent-app-id", parent_app_id,
                           "win-width", width,
                           "win-height", height,
                           "maximized", phosh_toplevel_is_maximized (toplevel),
                           "fullscreen", phosh_toplevel_is_fullscreen (toplevel),
                           NULL);
  g_object_set_data (G_OBJECT (activity), "toplevel", toplevel);

  gtk_container_add (GTK_CONTAINER (priv->carousel_running_activities), activity);
  gtk_widget_show (activity);

  g_signal_connect_swapped (activity, "clicked", G_CALLBACK (on_activity_clicked), self);
  g_signal_connect_swapped (activity, "closed", G_CALLBACK (on_activity_closed), self);

  g_signal_connect_object (toplevel, "closed", G_CALLBACK (on_toplevel_closed), self, 0);
  g_signal_connect_object (toplevel, "notify::activated", G_CALLBACK (on_toplevel_activated_changed), self, 0);
  g_object_bind_property (toplevel, "maximized", activity, "maximized", G_BINDING_DEFAULT);
  g_object_bind_property (toplevel, "fullscreen", activity, "fullscreen", G_BINDING_DEFAULT);

  g_signal_connect (activity, "resized", G_CALLBACK (on_activity_resized), toplevel);
  g_signal_connect_swapped (activity, "notify::has-focus", G_CALLBACK (on_activity_has_focus_changed), self);

  phosh_connect_feedback (activity);

  if (phosh_toplevel_is_activated (toplevel)) {
    scroll_to_activity (self, PHOSH_ACTIVITY (activity));
    priv->activity = PHOSH_ACTIVITY (activity);
  }
}


static void
get_running_activities (PhoshOverview *self)
{
  PhoshOverviewPrivate *priv;
  PhoshToplevelManager *toplevel_manager = phosh_shell_get_toplevel_manager (phosh_shell_get_default ());
  guint toplevels_num = phosh_toplevel_manager_get_num_toplevels (toplevel_manager);
  g_return_if_fail (PHOSH_IS_OVERVIEW (self));
  priv = phosh_overview_get_instance_private (self);

  priv->has_activities = !!toplevels_num;
  if (toplevels_num == 0)
    gtk_widget_hide (priv->carousel_running_activities);

  for (guint i = 0; i < toplevels_num; i++) {
    PhoshToplevel *toplevel = phosh_toplevel_manager_get_toplevel (toplevel_manager, i);
    add_activity (self, toplevel);
  }
}


static void
toplevel_added_cb (PhoshOverview        *self,
                   PhoshToplevel        *toplevel,
                   PhoshToplevelManager *manager)
{
  g_return_if_fail (PHOSH_IS_OVERVIEW (self));
  g_return_if_fail (PHOSH_IS_TOPLEVEL (toplevel));
  g_return_if_fail (PHOSH_IS_TOPLEVEL_MANAGER (manager));
  add_activity (self, toplevel);
}


static void
toplevel_changed_cb (PhoshOverview        *self,
                     PhoshToplevel        *toplevel,
                     PhoshToplevelManager *manager)
{
  PhoshActivity *activity;

  g_return_if_fail (PHOSH_IS_OVERVIEW (self));
  g_return_if_fail (PHOSH_IS_TOPLEVEL (toplevel));
  g_return_if_fail (PHOSH_IS_TOPLEVEL_MANAGER (manager));

  if (phosh_shell_get_state (phosh_shell_get_default ()) & PHOSH_STATE_OVERVIEW)
    return;

  activity = find_activity_by_toplevel (self, toplevel);
  g_return_if_fail (activity);

  request_thumbnail (activity, toplevel);
}


static void
num_toplevels_cb (PhoshOverview        *self,
                  GParamSpec           *pspec,
                  PhoshToplevelManager *manager)
{
  PhoshOverviewPrivate *priv;
  gboolean has_activities;

  g_return_if_fail (PHOSH_IS_OVERVIEW (self));
  g_return_if_fail (PHOSH_IS_TOPLEVEL_MANAGER (manager));
  priv = phosh_overview_get_instance_private (self);

  has_activities = !!phosh_toplevel_manager_get_num_toplevels (manager);
  if (priv->has_activities == has_activities)
    return;

  priv->has_activities = has_activities;
  gtk_widget_set_visible (priv->carousel_running_activities, has_activities);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HAS_ACTIVITIES]);
}

static void
phosh_overview_size_allocate (GtkWidget     *widget,
                              GtkAllocation *alloc)
{
  PhoshOverview *self = PHOSH_OVERVIEW (widget);
  PhoshOverviewPrivate *priv = phosh_overview_get_instance_private (self);
  g_autoptr (GList) children = NULL;
  int width, height;

  phosh_shell_get_usable_area (phosh_shell_get_default (), NULL, NULL, &width, &height);
  children = gtk_container_get_children (GTK_CONTAINER (priv->carousel_running_activities));

  for (GList *l = children; l; l = l->next) {
    g_object_set (l->data,
                  "win-width", width,
                  "win-height", height,
                  NULL);
  }

  GTK_WIDGET_CLASS (phosh_overview_parent_class)->size_allocate (widget, alloc);
}


static void
app_launched_cb (PhoshOverview *self,
                 GAppInfo      *info,
                 GtkWidget     *widget)
{
  g_return_if_fail (PHOSH_IS_OVERVIEW (self));

  g_signal_emit (self, signals[ACTIVITY_LAUNCHED], 0);
}


static void
page_changed_cb (PhoshOverview *self,
                 guint          index,
                 HdyCarousel   *carousel)
{
  PhoshActivity *activity;
  PhoshToplevel *toplevel;
  g_autoptr (GList) list = NULL;
  g_return_if_fail (PHOSH_IS_OVERVIEW (self));
  g_return_if_fail (HDY_IS_CAROUSEL (carousel));

  /* Carousel is empty */
  if (((int)index < 0))
    return;

  /* don't raise on scroll in docked mode */
  if (phosh_shell_get_docked (phosh_shell_get_default ()))
    return;

  /* ignore page changes when overview is not open */
  if (!(phosh_shell_get_state (phosh_shell_get_default ()) & PHOSH_STATE_OVERVIEW))
    return;

  list = gtk_container_get_children (GTK_CONTAINER (carousel));
  activity = PHOSH_ACTIVITY (g_list_nth_data (list, index));
  toplevel = get_toplevel_from_activity (activity);
  phosh_toplevel_activate (toplevel, phosh_wayland_get_wl_seat (phosh_wayland_get_default ()));

  if (!gtk_widget_has_focus (GTK_WIDGET (activity)))
    gtk_widget_grab_focus (GTK_WIDGET (activity));
}

/* Experimental input events */
gboolean
on_button_release_event (GtkWidget      *widget,
                         GdkEventButton *event,
                         gpointer        user_data)
{
  PhoshOverview *self = PHOSH_OVERVIEW(user_data);
  PhoshOverviewPrivate *priv = phosh_overview_get_instance_private (self);

    self->drag_start_x = 0;
    self->drag_start_y = 0;
    self->is_dragging = FALSE;
    self->initial_height = 0;

  return GDK_EVENT_PROPAGATE;
}

gboolean
on_scroll_event (GtkWidget      *widget,
                 GdkEventScroll *event,
                 gpointer        user_data)
{
  g_debug ("Overview scroll event: direction=%d", event->direction);

  PhoshOverview *self = PHOSH_OVERVIEW(user_data);
  PhoshOverviewPrivate *priv = phosh_overview_get_instance_private (self);
  
  if (priv->phoshdesktop) {
    return gtk_widget_event(GTK_WIDGET(priv->phoshdesktop), (GdkEvent *)event);
  }
  
  return GDK_EVENT_PROPAGATE;
}

#define SMOOTHING_FACTOR 0.2

gboolean
on_motion_notify_event (GtkWidget      *widget,
                        GdkEventMotion *event,
                        gpointer        user_data)
{
  PhoshOverview *self = PHOSH_OVERVIEW(user_data);
  PhoshOverviewPrivate *priv = phosh_overview_get_instance_private (self);
  
  if (!self->is_dragging) {
    self->initial_height = gtk_widget_get_allocated_height(priv->app_grid_positional_wrapper);
    self->current_height = self->initial_height;
    self->drag_start_x = event->x;
    self->drag_start_y = event->y;
    self->is_dragging = TRUE;
  }

  gdouble dy = event->y - self->drag_start_y;
  
  if (priv->app_grid_positional_wrapper) {
    // Get the current scale factor
    gdouble scale_factor = gtk_widget_get_scale_factor(widget);
    
    // Apply the scale factor to the dy value
    gdouble scaled_dy = dy * scale_factor;
    
    int target_height = self->initial_height - scaled_dy;
    target_height = MAX(target_height, 100);  // Clamp to minimum height
    
    // Apply low-pass filter for smoothing
    self->current_height = self->current_height * (1 - SMOOTHING_FACTOR) + target_height * SMOOTHING_FACTOR;
    
    // Round to nearest integer to avoid sub-pixel jitter
    int rounded_height = round(self->current_height);
    
    gtk_widget_set_size_request(priv->app_grid_positional_wrapper, -1, rounded_height);

    // Log widget position
    int widget_x, widget_y;
    gtk_widget_translate_coordinates(priv->app_grid_positional_wrapper, 
                                     gtk_widget_get_toplevel(widget), 
                                     0, 0, &widget_x, &widget_y);
    
    // Log finger position
    gdouble finger_x, finger_y;
    gdk_event_get_root_coords((GdkEvent*)event, &finger_x, &finger_y);

    g_debug("Widget position: (%d, %d), Finger position: (%.2f, %.2f), Scale factor: %.2f", 
            widget_x, widget_y, finger_x, finger_y, scale_factor);
  }

  // Remove debouncing and always queue a redraw
  gtk_widget_queue_draw(GTK_WIDGET(self));

  return GDK_EVENT_PROPAGATE;
}

gboolean
on_touch_event (GtkWidget     *widget,
                GdkEventTouch *event,
                gpointer       user_data)
{
    GdkEventTouch *touch_event = (GdkEventTouch *)event;
    // Get the touch ID and coordinates
    //guint touch_id = touch_event->touch_id;
    gdouble x = touch_event->x;
    gdouble y = touch_event->y;

    // Handle the touch event
    g_print("Touch event: ID u, x %f, y %f\n", x, y);

  PhoshOverview *self = PHOSH_OVERVIEW(user_data);
  PhoshDesktop *desktop = phosh_overview_get_phoshdesktop(self);

  // Forward the touch event to PhoshDesktop
  if (desktop) {
    return gtk_widget_event(GTK_WIDGET(desktop), (GdkEvent *)event);
  }

  return GDK_EVENT_PROPAGATE;
}

static void
phosh_overview_constructed (GObject *object)
{
  PhoshOverview *self = PHOSH_OVERVIEW (object);
  PhoshOverviewPrivate *priv = phosh_overview_get_instance_private (self);
  PhoshToplevelManager *toplevel_manager =
      phosh_shell_get_toplevel_manager (phosh_shell_get_default ());

  G_OBJECT_CLASS (phosh_overview_parent_class)->constructed (object);

  g_signal_connect_object (toplevel_manager, "toplevel-added",
                           G_CALLBACK (toplevel_added_cb),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (toplevel_manager, "toplevel-changed",
                           G_CALLBACK (toplevel_changed_cb),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (toplevel_manager, "notify::num-toplevels",
                           G_CALLBACK (num_toplevels_cb),
                           self,
                           G_CONNECT_SWAPPED);

  get_running_activities (self);

  g_signal_connect_swapped (priv->app_grid, "app-launched",
                            G_CALLBACK (app_launched_cb), self);

  g_signal_connect_swapped (priv->carousel_running_activities, "page-changed",
                            G_CALLBACK (page_changed_cb), self);

    // Set up event handling for phoshdesktop
  gtk_widget_add_events (GTK_WIDGET (priv->phoshdesktop), 
                         GDK_TOUCH_MASK | GDK_POINTER_MOTION_MASK);
  g_signal_connect (priv->phoshdesktop, "button-release-event", 
                    G_CALLBACK (on_button_release_event), self);
  g_signal_connect (priv->phoshdesktop, "touch-event", 
                    G_CALLBACK (on_touch_event), self);
  g_signal_connect (priv->phoshdesktop, "motion-notify-event", 
                    G_CALLBACK (on_motion_notify_event), self);
}

static void
phosh_overview_class_init (PhoshOverviewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->constructed = phosh_overview_constructed;
  object_class->get_property = phosh_overview_get_property;
  widget_class->size_allocate = phosh_overview_size_allocate;

  props[PROP_HAS_ACTIVITIES] =
    g_param_spec_boolean (
      "has-activities",
      "Has activities",
      "Whether the overview has running activities",
      FALSE,
      G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /* ensure used custom types */
  PHOSH_TYPE_APP_GRID;
  PHOSH_TYPE_DESKTOP;
  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/sm/puri/phosh/ui/overview.ui");

  gtk_widget_class_bind_template_child_private (widget_class, PhoshOverview, carousel_running_activities);
  gtk_widget_class_bind_template_child_private (widget_class, PhoshOverview, app_grid_positional_wrapper);
  gtk_widget_class_bind_template_child_private (widget_class, PhoshOverview, app_grid);
  gtk_widget_class_bind_template_child_private (widget_class, PhoshOverview, phoshdesktop);


  gtk_widget_class_bind_template_callback (widget_class, on_touch_event);
  gtk_widget_class_bind_template_callback (widget_class, on_scroll_event);
  /*gtk_widget_class_bind_template_callback (widget_class, on_touchpad_swipe_event);
  gtk_widget_class_bind_template_callback (widget_class, on_touchpad_pinch_event);*/
  gtk_widget_class_bind_template_callback (widget_class, on_motion_notify_event);

  signals[ACTIVITY_LAUNCHED] = g_signal_new ("activity-launched",
      G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL,
      NULL, G_TYPE_NONE, 0);
  signals[ACTIVITY_RAISED] = g_signal_new ("activity-raised",
      G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL,
      NULL, G_TYPE_NONE, 0);
  signals[SELECTION_ABORTED] = g_signal_new ("selection-aborted",
      G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL,
      NULL, G_TYPE_NONE, 0);
  signals[ACTIVITY_CLOSED] = g_signal_new ("activity-closed",
      G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL,
      NULL, G_TYPE_NONE, 0);

  gtk_widget_class_set_css_name (widget_class, "phosh-overview");
}

static void
phosh_overview_init (PhoshOverview *self)
{
  PhoshOverviewPrivate *priv = phosh_overview_get_instance_private (self);
  gtk_widget_init_template (GTK_WIDGET (self));

    self->drag_start_x = 0;
    self->drag_start_y = 0;
    self->is_dragging = FALSE;
    self->initial_height = 0;
    self->current_height = 300;

  // Initialize phoshdesktop
  priv->phoshdesktop = phoshdesktop_new ();
  if (priv->phoshdesktop == NULL) {
    g_warning("Failed to create PhoshDesktop");
    return;
  }
  gtk_widget_show (GTK_WIDGET (priv->phoshdesktop));
  gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (priv->phoshdesktop));

  // Set the height of app_grid_positional_wrapper
  priv->app_grid_positional_wrapper = GTK_WIDGET(gtk_widget_get_template_child(GTK_WIDGET(self), PHOSH_TYPE_OVERVIEW, "app_grid_positional_wrapper"));
  if (priv->app_grid_positional_wrapper) {
    gtk_widget_set_size_request (priv->app_grid_positional_wrapper, -1, self->current_height);
  }

  // Ensure the phoshdesktop widget can receive all relevant events
  gtk_widget_add_events(priv->phoshdesktop, 
                        GDK_TOUCH_MASK |
                        GDK_SMOOTH_SCROLL_MASK |
                        GDK_TOUCHPAD_GESTURE_MASK |
                        GDK_POINTER_MOTION_MASK);  // Add this for motion events

  // Connect the event signals to the phoshdesktop widget
  g_signal_connect(priv->phoshdesktop, "touch-event", G_CALLBACK(on_touch_event), self);
  g_signal_connect(priv->phoshdesktop, "scroll-event", G_CALLBACK(on_scroll_event), self);
  g_signal_connect(priv->phoshdesktop, "motion-notify-event", G_CALLBACK(on_motion_notify_event), self);
  /*g_signal_connect(priv->phoshdesktop, "touchpad-swipe-event", G_CALLBACK(on_touchpad_swipe_event), self);
  g_signal_connect(priv->phoshdesktop, "touchpad-pinch-event", G_CALLBACK(on_touchpad_pinch_event), self);*/

  // Remove these events from the overview widget itself
  gtk_widget_add_events(GTK_WIDGET(self), 
                        gtk_widget_get_events(GTK_WIDGET(self)) & 
                        ~(GDK_TOUCH_MASK |
                          GDK_SMOOTH_SCROLL_MASK |
                          GDK_TOUCHPAD_GESTURE_MASK |
                          GDK_POINTER_MOTION_MASK));
}

GtkWidget *
phosh_overview_new (void)
{
  return g_object_new (PHOSH_TYPE_OVERVIEW, NULL);
}

static void
phosh_overview_dispose (GObject *object)
{
  PhoshOverview *self = PHOSH_OVERVIEW (object);
  PhoshOverviewPrivate *priv = phosh_overview_get_instance_private (self);

  if (priv->phoshdesktop) {
    gtk_widget_destroy (GTK_WIDGET (priv->phoshdesktop));
    priv->phoshdesktop = NULL;
  }

  G_OBJECT_CLASS (phosh_overview_parent_class)->dispose (object);
}


void
phosh_overview_reset (PhoshOverview *self)
{
  PhoshOverviewPrivate *priv;
  g_return_if_fail(PHOSH_IS_OVERVIEW (self));
  priv = phosh_overview_get_instance_private (self);
  phosh_app_grid_reset (PHOSH_APP_GRID (priv->app_grid));

  if (priv->activity) {
    gtk_widget_grab_focus (GTK_WIDGET (priv->activity));
    request_thumbnail (priv->activity, get_toplevel_from_activity (priv->activity));
  }
}

void
phosh_overview_focus_app_search (PhoshOverview *self)
{
  PhoshOverviewPrivate *priv;

  g_return_if_fail(PHOSH_IS_OVERVIEW (self));
  priv = phosh_overview_get_instance_private (self);
  phosh_app_grid_focus_search (PHOSH_APP_GRID (priv->app_grid));
}


gboolean
phosh_overview_handle_search (PhoshOverview *self, GdkEvent *event)
{
  PhoshOverviewPrivate *priv;

  g_return_val_if_fail(PHOSH_IS_OVERVIEW (self), GDK_EVENT_PROPAGATE);
  priv = phosh_overview_get_instance_private (self);
  return phosh_app_grid_handle_search (PHOSH_APP_GRID (priv->app_grid), event);
}


gboolean
phosh_overview_has_running_activities (PhoshOverview *self)
{
  PhoshOverviewPrivate *priv;

  g_return_val_if_fail (PHOSH_IS_OVERVIEW (self), FALSE);
  priv = phosh_overview_get_instance_private (self);

  return priv->has_activities;
}

/**
 * phosh_overview_get_app_grid:
 * @self: The overview
 *
 * Get the application grid
 *
 * Returns:(transfer none): The app grid widget
 */
PhoshAppGrid *
phosh_overview_get_app_grid (PhoshOverview *self)
{
  PhoshOverviewPrivate *priv;

  g_return_val_if_fail (PHOSH_IS_OVERVIEW (self), NULL);
  priv = phosh_overview_get_instance_private (self);

  return PHOSH_APP_GRID (priv->app_grid);
}

PhoshDesktop *
phosh_overview_get_phoshdesktop (PhoshOverview *self)
{
  PhoshOverviewPrivate *priv = phosh_overview_get_instance_private (self);
  if (!PHOSH_IS_DESKTOP (priv->phoshdesktop)) {
    g_warning ("Overview widget is not set or is not of type PhoshDesktop");
    return NULL;
  }
  return PHOSH_DESKTOP(priv->phoshdesktop);
}