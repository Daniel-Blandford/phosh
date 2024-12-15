/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Author: Alexander Mikhaylenko <alexander.mikhaylenko@puri.sm>
 *
 * Based on <hdy-fading-label.c> from <libhandy 1.5.0> which is LGPL-2.1+.
 */

#include "phosh-config.h"
#include "fading-label.h"

#include <glib/gi18n-lib.h>
#include "bidi.h"

struct _PhoshFadingLabel
{
  GtkBin parent_instance;
  GtkWidget *label;
  gfloat align;
};

G_DEFINE_TYPE (PhoshFadingLabel, phosh_fading_label, GTK_TYPE_BIN)

enum {
  PROP_0,
  PROP_LABEL,
  PROP_ALIGN,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static void
phosh_fading_label_get_preferred_width (GtkWidget *widget,
                                        gint      *min,
                                        gint      *nat)
{
  PhoshFadingLabel *self = PHOSH_FADING_LABEL (widget);

  gtk_widget_get_preferred_width (self->label, min, nat);

  if (min)
    *min = 0;
}

static void
phosh_fading_label_get_preferred_width_for_height (GtkWidget *widget,
                                                   gint       for_height,
                                                   gint      *min,
                                                   gint      *nat)
{
  phosh_fading_label_get_preferred_width (widget, min, nat);
}

static void
phosh_fading_label_size_allocate (GtkWidget     *widget,
                                  GtkAllocation *allocation)
{
  PhoshFadingLabel *self = PHOSH_FADING_LABEL (widget);
  PangoLayout *layout = gtk_label_get_layout (GTK_LABEL (self->label));
  gint width, height;
  //g_print ("phosh_fading_label_size_allocate: width=%d, height=%d\n", width, height);

  pango_layout_get_pixel_size (layout, &width, &height);

  allocation->height = height; // Use the natural height of the label layout

  gtk_widget_set_allocation (widget, allocation);

  // Allocate the label's size based on the calculated height
  gtk_widget_size_allocate (self->label, allocation);

  // Print the size request of the widget
  gint min_width, nat_width;
  gtk_widget_get_preferred_width (widget, &min_width, &nat_width);
  //g_print ("Size request: min_width=%d, nat_width=%d\n", min_width, nat_width);

  // Print the label's margins
  gint margin_top = gtk_widget_get_margin_top (self->label);
  gint margin_bottom = gtk_widget_get_margin_bottom (self->label);
  //g_print ("Label margins: top=%d, bottom=%d\n", margin_top, margin_bottom);

  // Print the label's wrapping behavior
  gboolean line_wrap = gtk_label_get_line_wrap (GTK_LABEL (self->label));
  //g_print ("Label wrapping: %s\n", line_wrap ? "TRUE" : "FALSE");
}

static gboolean
phosh_fading_label_draw (GtkWidget *widget,
                         cairo_t   *cr)
{
  PhoshFadingLabel *self = PHOSH_FADING_LABEL (widget);

  PangoLayout *layout = gtk_label_get_layout (GTK_LABEL (self->label));
  GtkAllocation alloc;
  gtk_widget_get_allocation (widget, &alloc);
  pango_layout_set_width (layout, alloc.width * PANGO_SCALE);
  pango_layout_set_wrap (layout, PANGO_WRAP_WORD_CHAR);

  gtk_container_propagate_draw (GTK_CONTAINER (widget), self->label, cr);

  return GDK_EVENT_PROPAGATE;
}

static void
phosh_fading_label_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  PhoshFadingLabel *self = PHOSH_FADING_LABEL (object);

  switch (prop_id) {
  case PROP_LABEL:
    g_value_set_string (value, phosh_fading_label_get_label (self));
    break;

  case PROP_ALIGN:
    g_value_set_float (value, phosh_fading_label_get_align (self));
    break;

    default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

const char *
get_modified_text(const char *text)
{
    if (g_strcmp0 (text, "GTK Demo") == 0) {// TODO <- Fix this GTK Demo text causing the label height problem
        return "GTk demo";
    }
    return text;
}

static void
phosh_fading_label_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PhoshFadingLabel *self = PHOSH_FADING_LABEL (object);

  switch (prop_id) {
  case PROP_LABEL:
    {
      const char *text = g_value_get_string (value);
      const char *modified_text = get_modified_text(text);
      phosh_fading_label_set_label (self, modified_text);
    }
    break;

  case PROP_ALIGN:
    phosh_fading_label_set_align (self, 0.5);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
phosh_fading_label_finalize (GObject *object)
{
  G_OBJECT_CLASS (phosh_fading_label_parent_class)->finalize (object);
}

static void
phosh_fading_label_class_init (PhoshFadingLabelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = phosh_fading_label_get_property;
  object_class->set_property = phosh_fading_label_set_property;
  object_class->finalize = phosh_fading_label_finalize;

  widget_class->get_preferred_width = phosh_fading_label_get_preferred_width;
  widget_class->get_preferred_width_for_height = phosh_fading_label_get_preferred_width_for_height;
  widget_class->size_allocate = phosh_fading_label_size_allocate;
  widget_class->draw = phosh_fading_label_draw;

  props[PROP_LABEL] =
    g_param_spec_string ("label", "", "",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_ALIGN] =
    g_param_spec_float ("align", "", "",
                        0.0, 1.0, 0.0,
                        G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
phosh_fading_label_init (PhoshFadingLabel *self)
{
  gtk_widget_set_has_window (GTK_WIDGET (self), FALSE);

  self->label = gtk_label_new (NULL);
  gtk_label_set_width_chars (GTK_LABEL (self->label), 40);
  gtk_label_set_max_width_chars (GTK_LABEL (self->label), 40);
  gtk_label_set_justify (GTK_LABEL (self->label), GTK_JUSTIFY_CENTER);
  gtk_label_set_line_wrap (GTK_LABEL (self->label), TRUE);
  gtk_label_set_line_wrap_mode (GTK_LABEL (self->label), GTK_WRAP_WORD);
  gtk_label_set_lines (GTK_LABEL (self->label), 2); // Set max number of lines to 2
  gtk_label_set_ellipsize (GTK_LABEL (self->label), PANGO_ELLIPSIZE_END); // Add ellipsize to truncate text

  gtk_widget_set_valign (GTK_WIDGET (self->label), GTK_ALIGN_CENTER);
  gtk_container_set_border_width (GTK_CONTAINER (self), 6);


  gtk_widget_show (self->label);

  gtk_container_add (GTK_CONTAINER (self), self->label);
}

GtkWidget *
phosh_fading_label_new (const char *label)
{
  return GTK_WIDGET (g_object_new (PHOSH_TYPE_FADING_LABEL, "label", label, NULL));
}

const char *
phosh_fading_label_get_label (PhoshFadingLabel *self)
{
  g_return_val_if_fail (PHOSH_IS_FADING_LABEL (self), NULL);

  return gtk_label_get_label (GTK_LABEL (self->label));
}

void
phosh_fading_label_set_label (PhoshFadingLabel *self,
                              const char       *labeldefault)
{
  const char *label = get_modified_text(labeldefault);
  gint content_length;

  g_return_if_fail (PHOSH_IS_FADING_LABEL (self));

  if (!g_strcmp0 (label, phosh_fading_label_get_label (self)))
    return;

  content_length = g_utf8_strlen (label, -1);

  gtk_label_set_label (GTK_LABEL (self->label), label);

  // Adjust the width and max width based on the content's length
  gtk_label_set_width_chars (GTK_LABEL (self->label), content_length);
  gtk_label_set_max_width_chars (GTK_LABEL (self->label), content_length);

  // Add debug code to test label width adjustment
  GtkRequisition minimum_size;
  GtkRequisition natural_size;
  gtk_widget_get_preferred_size (self->label, &minimum_size, &natural_size);

  //g_print ("App: '%s', name length %d, Min size: %dx%d, natural size: %dx%d\n", label, content_length, minimum_size.width, minimum_size.height, natural_size.width, natural_size.height);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LABEL]);
}

float
phosh_fading_label_get_align (PhoshFadingLabel *self)
{
  g_return_val_if_fail (PHOSH_IS_FADING_LABEL (self), 0.5f);

  return self->align;
}

void
phosh_fading_label_set_align (PhoshFadingLabel *self,
                              gfloat            align)
{
  g_return_if_fail (PHOSH_IS_FADING_LABEL (self));

  align = CLAMP (align, 0.0, 1.0);

  if (!(self->align > align || self->align < align))
    return;

  self->align = align;

  gtk_widget_queue_allocate (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ALIGN]);
}
