/*
 * Copyright (C) 2024 Bardia Moshiri
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Author: Bardia Moshiri <fakeshell@bardia.tech>
 */

#include "dark-mode-quick-setting.h"
#include "quick-setting.h"

#include <glib/gi18n.h>
#include <gio/gio.h>

/**
 * PhoshDarkModeQuickSetting:
 *
 * Enable and disable dark mode.
 */
struct _PhoshDarkModeQuickSetting {
  PhoshQuickSetting        parent;

  PhoshStatusIcon         *info;
  GSettings               *settings;
};

G_DEFINE_TYPE (PhoshDarkModeQuickSetting, phosh_dark_mode_quick_setting, PHOSH_TYPE_QUICK_SETTING);

static void
update_dark_mode_status (PhoshDarkModeQuickSetting *self)
{
  const gchar *color_scheme = g_settings_get_string (self->settings, "color-scheme");

  if (g_strcmp0 (color_scheme, "prefer-dark") == 0) {
    phosh_status_icon_set_icon_name (self->info, "weather-clear-night-symbolic");
    phosh_status_icon_set_info (self->info, _("Dark Mode"));
    phosh_quick_setting_set_active (PHOSH_QUICK_SETTING (self), TRUE);
  } else {
    phosh_status_icon_set_icon_name (self->info, "weather-clear-symbolic");
    phosh_status_icon_set_info (self->info, _("Light Mode"));
    phosh_quick_setting_set_active (PHOSH_QUICK_SETTING (self), FALSE);
  }
}

static void
on_clicked (PhoshDarkModeQuickSetting *self)
{
  gboolean active = phosh_quick_setting_get_active (PHOSH_QUICK_SETTING (self));

  if (!active)
    g_settings_set_string (self->settings, "color-scheme", "prefer-dark");
  else
    g_settings_set_string (self->settings, "color-scheme", "default");
}

static void
on_settings_changed (GSettings *settings, gchar *key, PhoshDarkModeQuickSetting *self)
{
  if (g_strcmp0 (key, "color-scheme") == 0)
    update_dark_mode_status (self);
}

static void
phosh_dark_mode_quick_setting_class_init (PhoshDarkModeQuickSettingClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/mobi/phosh/plugins/dark-mode-quick-setting/qs.ui");

  gtk_widget_class_bind_template_child (widget_class, PhoshDarkModeQuickSetting, info);

  gtk_widget_class_bind_template_callback (widget_class, on_clicked);
}

static void
phosh_dark_mode_quick_setting_init (PhoshDarkModeQuickSetting *self)
{
  self->settings = g_settings_new ("org.gnome.desktop.interface");
  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect (self->settings, "changed::color-scheme", G_CALLBACK (on_settings_changed), self);

  update_dark_mode_status (self);
}
