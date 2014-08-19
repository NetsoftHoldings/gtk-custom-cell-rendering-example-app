#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TYPE_TABLE_RENDERER            (table_renderer_get_type ())
#define TABLE_RENDERER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_TABLE_RENDERER, TableRenderer))
#define TABLE_RENDERER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_TABLE_RENDERER, TableRendererClass))
#define IS_TABLE_RENDERER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_TABLE_RENDERER))
#define IS_TABLE_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_TABLE_RENDERER))
#define TABLE_RENDERER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_TABLE_RENDERER, TableRendererClass))

typedef struct _TableRenderer      TableRenderer;
typedef struct _TableRendererClass TableRendererClass;

struct _TableRenderer
{
    GtkCellRendererText parent;

    /* private */
    gboolean group_row;
    gboolean have_groups;
    gboolean active;
    struct {
        cairo_surface_t *play;
        cairo_surface_t *stop;
    } icon;
};

struct _TableRendererClass
{
    GtkCellRendererTextClass parent_class;
};

GType table_renderer_get_type() G_GNUC_CONST;

GtkCellRenderer *table_renderer_new(gboolean has_groups, cairo_surface_t *icon_play, cairo_surface_t *icon_stop);

G_END_DECLS
