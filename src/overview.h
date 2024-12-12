/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <gtk/gtk.h>
#include <glib.h>

#include "app-grid.h"
#include "phoshdesktop.h"

G_BEGIN_DECLS

#define PHOSH_TYPE_OVERVIEW (phosh_overview_get_type())

G_DECLARE_FINAL_TYPE (PhoshOverview, phosh_overview, PHOSH, OVERVIEW, GtkBox)


GtkWidget *phosh_overview_new (void);
void       phosh_overview_reset (PhoshOverview *self);
void       phosh_overview_focus_app_search (PhoshOverview *self);
gboolean   phosh_overview_has_running_activities (PhoshOverview *self);
gboolean   phosh_overview_handle_search (PhoshOverview *self, GdkEvent *event);
PhoshAppGrid *phosh_overview_get_app_grid (PhoshOverview *self);
PhoshDesktop *phosh_overview_get_phoshdesktop (PhoshOverview *self);

G_END_DECLS
