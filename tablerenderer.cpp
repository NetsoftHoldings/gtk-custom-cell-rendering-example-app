
#include "tablerenderer.h"
#include <stdio.h>

static GtkCellRendererClass *parent_class;

enum
{
    PROP_0,
    PROP_GROUP_ROW,
    PROP_PLAYING
};

static void
table_renderer_set_property(GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    TableRenderer *self = TABLE_RENDERER(object);

    switch (property_id)
    {
    case PROP_GROUP_ROW:
        self->group_row = g_value_get_boolean(value);
        break;
    case PROP_PLAYING:
        self->active = g_value_get_boolean(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
table_renderer_render(GtkCellRenderer      *cell,
                      cairo_t              *cr,
                      GtkWidget            *widget,
                      const GdkRectangle   *background_area,
                      const GdkRectangle   *cell_area,
                      GtkCellRendererState  flags)
{
    TableRenderer *self = TABLE_RENDERER(cell);

    if (self->have_groups)
    {
        GValue xpad = G_VALUE_INIT;
        g_value_init(&xpad, G_TYPE_INT);

        GValue weight = G_VALUE_INIT;
        g_value_init(&weight, G_TYPE_INT);

        if (self->group_row)
            g_value_set_int(&weight, PANGO_WEIGHT_BOLD);
        else
            g_value_set_int(&weight, PANGO_WEIGHT_NORMAL);

        int xpad_val = 2;

        if (self->icon.play && !self->group_row)
            xpad_val = cairo_image_surface_get_width(self->icon.play);

        g_value_set_int(&xpad, xpad_val);

        GValue width = G_VALUE_INIT;
        g_value_init(&width, G_TYPE_INT);
        g_object_get_property(G_OBJECT(cell), "width", &width);
        g_value_set_int(&width, g_value_get_int(&width) + xpad_val);

        g_object_set_property(G_OBJECT(cell), "xpad", &xpad);
        g_object_set_property(G_OBJECT(cell), "weight", &weight);
        g_object_set_property(G_OBJECT(cell), "width", &width);

        g_value_unset(&xpad);
        g_value_unset(&weight);
    }

    if (self->group_row)
    {
        /* Overpaint background with "unselected" color */
        GtkStyleContext *style = gtk_widget_get_style_context(widget);
        GdkRGBA bg_color;
        gtk_style_context_get_background_color
            (style, GTK_STATE_FLAG_NORMAL, &bg_color);

        cairo_save(cr);
        gdk_cairo_rectangle(cr, background_area);
        gdk_cairo_set_source_rgba(cr, &bg_color);
        cairo_fill(cr);
        cairo_restore(cr);
    }

    gboolean selected_prelit = flags & (GTK_CELL_RENDERER_SELECTED
                                       | GTK_CELL_RENDERER_PRELIT);

    if (self->icon.play && !self->group_row && (selected_prelit || self->active))
    {
        cairo_surface_t *surf = self->active ? self->icon.play : self->icon.stop;
        cairo_save(cr);
        gint yOff = (background_area->height - cairo_image_surface_get_height(surf)) / 2;
        cairo_set_source_surface(cr, surf, background_area->x, background_area->y + yOff);
        gdk_cairo_rectangle(cr, background_area);
        cairo_paint(cr);
        cairo_restore(cr);
    }

    parent_class->render(cell, cr, widget, background_area, cell_area, flags);
}

static void
table_renderer_class_init(TableRendererClass *klass)
{
    GObjectClass *gobject_class = (GObjectClass*) klass;
    GtkCellRendererClass *cell = (GtkCellRendererClass*) klass;

    parent_class = (GtkCellRendererClass*) g_type_class_peek_parent(klass);

    gobject_class->set_property = table_renderer_set_property;

    cell->render = table_renderer_render;

    g_object_class_install_property
        (gobject_class, PROP_GROUP_ROW,
         g_param_spec_boolean("group-row", "", "", FALSE, (GParamFlags) G_PARAM_WRITABLE));

    g_object_class_install_property
        (gobject_class, PROP_PLAYING,
         g_param_spec_boolean("playing", "", "", FALSE, (GParamFlags) G_PARAM_WRITABLE));
}

static void
table_renderer_init(TableRenderer *self)
{
    self->group_row = FALSE;
    self->have_groups = FALSE;
}

GtkCellRenderer *
table_renderer_new(gboolean has_groups, cairo_surface_t *icon_play, cairo_surface_t *icon_stop)
{
    TableRenderer *self = TABLE_RENDERER(g_object_new(TYPE_TABLE_RENDERER, NULL));
    self->have_groups = has_groups;
    self->icon.play = icon_play;
    self->icon.stop = icon_stop;

    return GTK_CELL_RENDERER(self);
}

G_DEFINE_TYPE(TableRenderer, table_renderer, GTK_TYPE_CELL_RENDERER_TEXT)
