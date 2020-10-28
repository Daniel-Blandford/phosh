/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Author: Guido Günther <agx@sigxcpu.org>
 */
#pragma once

#include <gtk/gtk.h>
#include <wayland-client-protocol.h>

void phosh_cp_widget_destroy (void *widget);
char *phosh_fix_app_id (const char *app_id);
void phosh_clear_handler (gulong *handler, gpointer object);
void phosh_convert_buffer (void *data, enum wl_shm_format format, guint width, guint height, guint stride);
