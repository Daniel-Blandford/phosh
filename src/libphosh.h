/*
 * Copyright (C) 2024 The Phosh Developers
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Author: Guido Günther <agx@sigxcpu.org>
 */

/*
 * The list of headers here must match what is exposed in Phosh-0.gir
 * for Rust binding generation.
 */

#pragma once

#include <glib.h>

G_BEGIN_DECLS

#define __LIBPHOSH_H_INSIDE__

#include "app-auth-prompt.h"
#include "activity.h"
#include "ambient.h"
#include "animation.h"
#include "app-grid.h"
#include "app-grid-base-button.h"
#include "app-grid-button.h"
#include "app-grid-folder-button.h"
#include "app-list-model.h"
#include "auth-prompt-option.h"
#include "background-cache.h"
#include "background-image.h"
#include "background.h"
#include "bidi.h"
#include "call.h"
#include "calls-manager.h"
#include "call-notification.h"
#include "clamp.h"
#include "connectivity-info.h"
#include "drag-surface.h"
#include "end-session-dialog.h"
#include "emergency-contact-row.h"
#include "emergency-contact.h"
#include "emergency-menu.h"
#include "folder-info.h"
#include "hks-info.h"
#include "hks-manager.h"
#include "docked-info.h"
#include "docked-manager.h"
#include "fading-label.h"
#include "favorite-list-model.h"
#include "feedback-manager.h"
#include "gnome-shell-manager.h"
#include "gtk-mount-manager.h"
#include "gtk-mount-prompt.h"
#include "keypad.h"
#include "launcher-entry-manager.h"
#include "lockshield.h"
#include "log.h"
#include "manager.h"
#include "media-player.h"
#include "mode-manager.h"
#include "mount-manager.h"
#include "mount-operation.h"
#include "overview.h"
#include "password-entry.h"
#include "osd-window.h"
#include "plugin-loader.h"
#include "power-menu.h"
#include "power-menu-manager.h"
#include "revealer.h"
#include "status-icon.h"
#include "splash.h"
#include "splash-manager.h"
#include "style-manager.h"
#include "suspend-manager.h"
#include "system-modal.h"
#include "system-modal-dialog.h"
#include "vpn-info.h"
#include "vpn-manager.h"
#include "quick-setting.h"
#include "quick-settings.h"
#include "quick-settings-box.h"
#include "phosh-wayland.h"
#include "swipe-away-bin.h"
#include "util.h"
#include "wall-clock.h"
#include "widget-box.h"
#include "wl-buffer.h"

#include "arrow.h"
#include "app-tracker.h"
#include "auth.h"
#include "background-manager.h"
#include "batteryinfo.h"
#include "bt-info.h"
#include "bt-manager.h"
#include "emergency-calls-manager.h"
#include "fader.h"
#include "feedbackinfo.h"
#include "home.h"
#include "keyboard-events.h"
#include "idle-manager.h"
#include "layout-manager.h"
#include "location-info.h"
#include "location-manager.h"
#include "lockscreen-manager.h"
#include "lockscreen.h"
#include "monitor-manager.h"
#include "network-auth-manager.h"
#include "osk-button.h"
#include "osk-manager.h"
#include "top-panel.h"
#include "plugin-shell.h"
#include "polkit-auth-agent.h"
#include "polkit-auth-prompt.h"
#include "portal-access-manager.h"
#include "portal-request.h"
#include "proximity.h"
#include "rotation-manager.h"
#include "run-command-dialog.h"
#include "run-command-manager.h"
#include "sensor-proxy-manager.h"
#include "rotateinfo.h"
#include "screen-saver-manager.h"
#include "screenshot-manager.h"
#include "session-presence.h"
#include "session-manager.h"
#include "shell.h"
#include "status-page.h"
#include "status-page-placeholder.h"
#include "system-prompt.h"
#include "system-prompter.h"
#include "thumbnail.h"
#include "toplevel-manager.h"
#include "toplevel-thumbnail.h"
#include "torch-info.h"
#include "torch-manager.h"
#include "toplevel.h"
#include "wifi-status-page.h"
#include "wifi-network-row.h"
#include "wifi-network.h"
#include "wifi-info.h"
#include "wifi-manager.h"
#include "wwan-info.h"

#include "dbus/calls-dbus.h"
#include "dbus/calls-emergency-dbus.h"
#include "dbus/geoclue-agent-dbus.h"
#include "dbus/geoclue-manager-dbus.h"
#include "dbus/gnome-session-client-private-dbus.h"
#include "dbus/gnome-session-dbus.h"
#include "dbus/gnome-session-presence-dbus.h"
#include "dbus/gsd-color-dbus.h"
#include "dbus/gsd-rfkill-dbus.h"
#include "dbus/hostname1-dbus.h"
#include "dbus/iio-sensor-proxy-dbus.h"
#include "dbus/login1-manager-dbus.h"
#include "dbus/login1-session-dbus.h"
#include "dbus/mpris-dbus.h"
#include "dbus/notify-dbus.h"
#include "dbus/phosh-display-dbus.h"
#include "dbus/phosh-end-session-dialog-dbus.h"
#include "dbus/phosh-gnome-shell-dbus.h"
#include "dbus/phosh-gtk-mountoperation-dbus.h"
#include "dbus/phosh-idle-dbus.h"
#include "dbus/phosh-osk0-dbus.h"
#include "dbus/phosh-screen-saver-dbus.h"
#include "dbus/phosh-screenshot-dbus.h"
#include "dbus/phosh-wwan-ofono-dbus.h"
#include "dbus/portal-dbus.h"

#include "monitor/gamma-table.h"
#include "monitor/head-priv.h"
#include "monitor/monitor.h"

#include "notifications/dbus-notification.h"
#include "notifications/mount-notification.h"
#include "notifications/notification-banner.h"
#include "notifications/notification-content.h"
#include "notifications/notification-frame.h"
#include "notifications/notification.h"
#include "notifications/notification-list.h"
#include "notifications/notification-source.h"
#include "notifications/notify-feedback.h"
#include "notifications/notify-manager.h"
#include "notifications/timestamp-label.h"

#include "settings/audio-device.h"
#include "settings/audio-device-row.h"
#include "settings/audio-settings.h"

#include "wwan/phosh-wwan-ofono.h"
#include "wwan/phosh-wwan-mm.h"
#include "wwan/phosh-wwan-iface.h"
#include "wwan/wwan-manager.h"

G_END_DECLS
