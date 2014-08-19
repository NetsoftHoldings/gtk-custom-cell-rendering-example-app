
#include <gtk/gtk.h>

#include "tablemodel.h"
#include "tablerenderer.h"

#include <vector>
#include <algorithm>
#include <assert.h>

typedef std::vector<std::string> StringVec;

const char *strings[] = {
    "String 1",
    "String 2",
    "String 3",
    "String 4",
    "String 5",
    "String 6",
    "String 7",
    "String 8",
    "String 9",
    "String 10",
    "String 11",
    "String 12",
    "String 13",
    "String 14"
};

const size_t stringN = sizeof(strings) / sizeof(strings[0]);

class Source : public NSC::NSTableViewDataSource
{
public:
    Source()
        : _activeRow(-1)
    {
        groups.resize(stringN);
        groups[0] = true;
        groups[2] = true;
        groups[5] = true;
        groups[9] = true;
        groups[11] = true;

        data.resize(stringN);
        for (size_t i = 0; i < data.size(); ++i)
        {
            StringVec &v = data[i];
            v.resize(2);

            if (groups[i])
            {
                v[0] = std::string("Group ") + strings[i];
            }
            else
            {
                v[0] = strings[i];
                v[1] = "0:00";
            }
        }

        headers.push_back("Column 1");
        headers.push_back("Column 2");
    }

    int numberOfColumns(NSC::NSTableView *) { return data[0].size(); }
    int numberOfRows(NSC::NSTableView *) { return data.size(); }
    std::string valueForColumnAndRow(NSC::NSTableView *, int C, int R) { return data[R][C]; }
    std::string valueForColumnHeader(NSC::NSTableView *, int C) { return headers[C]; }
    bool isGroupRow(NSC::NSTableView *, int R) { return groups[R]; }

    int activeRow() { return _activeRow; }

    int _activeRow;

private:
    std::vector<StringVec> data;
    StringVec headers;
    std::vector<bool> groups;
};

class Delegate : public NSC::NSTableViewDelegate
{
    Source &src;
public:
    Delegate(Source &src)
        : src(src)
    {}

    int heightOfRow(NSC::NSTableView *v, int R) { return src.isGroupRow(v, R) ? 32 : 38; }
};

struct Icons
{
    cairo_surface_t *play, *stop;
};

void updateView(NSC::NSTableViewDataSource *src, GtkTreeView *_view, Icons &icons, GtkCellRenderer **gr_rend)
{
    int _lastColumnCount = src->numberOfColumns(0);
    int current = gtk_tree_view_get_n_columns(_view);
    for (int i = 0; i < current; ++i)
    {
        GtkTreeViewColumn *c = gtk_tree_view_get_column(_view, 0);
        gtk_tree_view_remove_column(_view, c);
    }

    bool haveGroups = false;

    for (int i = 0; i < _lastColumnCount; ++i)
    if (src->isGroupRow(0, i))
    {
        haveGroups = true;
        break;
    }

    GtkCellRenderer *rend;
    GtkTreeViewColumn *c;
    gint rowI;

    /* Row 0 */
    rowI = 0;
    rend = table_renderer_new(haveGroups,icons.play, icons.stop);
    c = gtk_tree_view_column_new_with_attributes
        (src->valueForColumnHeader(0, rowI).c_str(), rend,
         "group-row", TABLE_MODEL_GROUP_ROW,
         "playing", TABLE_MODEL_ACTIVE,
         "text", rowI+TABLE_MODEL_DATA_OFFSET, NULL);

    /* One column enforcing the height is enough */
    gtk_tree_view_column_add_attribute
        (c, rend, "height", TABLE_MODEL_ROW_HEIGHT);

    gtk_tree_view_insert_column(_view, c, -1);
    gtk_tree_view_column_set_fixed_width(c, 128);

    *gr_rend = rend;

    /* Row 1 */
    rowI = 1;
    rend = table_renderer_new(haveGroups, 0, 0);
    c = gtk_tree_view_column_new_with_attributes
        (src->valueForColumnHeader(0, rowI).c_str(), rend,
         "group-row", TABLE_MODEL_GROUP_ROW,
         "playing", TABLE_MODEL_ACTIVE,
         "text", rowI+TABLE_MODEL_DATA_OFFSET, NULL);

    gtk_tree_view_insert_column(_view, c, -1);

    gtk_tree_view_set_headers_visible(_view, false);
}

static int getColumnIndex(GtkTreeView *view, GtkTreeViewColumn *column)
{
    int index = -1;
    int count = gtk_tree_view_get_n_columns(view);
    for (int i = 0; i < count; ++i)
        if (gtk_tree_view_get_column(view, i) == column)
        {
            index = i;
            break;
        }

    return index;
}

struct CBData
{
    GtkTreeView *view;
    Source *src;
    Delegate *del;
    gint icon_width;
    GtkAdjustment *vadj;
    GtkCellRenderer *gr_rend;
};

gboolean onViewButtonPress(GtkWidget *, GdkEventButton *be, gpointer data)
{
    CBData *cbdata = static_cast<CBData*>(data);

    GtkTreePath *path;
    GtkTreeViewColumn *column;
    gint cellX, cellY;
    gtk_tree_view_get_path_at_pos(cbdata->view, be->x, be->y, &path, &column, &cellX, &cellY);

    gint rowI, columnI;
    rowI = path ? *gtk_tree_path_get_indices(path) : -1;
    columnI = getColumnIndex(cbdata->view, column);

    gtk_tree_path_free(path);

    bool groupRow = cbdata->src->isGroupRow(0, rowI);

    if (be->type == GDK_DOUBLE_BUTTON_PRESS ||
        be->type == GDK_TRIPLE_BUTTON_PRESS || groupRow || columnI > 0)
        return groupRow;

    if (cellX < cbdata->icon_width)
    {
        if (cbdata->src->_activeRow != rowI)
        {
            cbdata->src->_activeRow = rowI;
            printf("Row %d started\n", rowI);
            fflush(stdout);
        }
        else
        {
            cbdata->src->_activeRow = -1;
            printf("Row %d stopped\n", rowI);
            fflush(stdout);
        }

        gtk_widget_queue_draw(GTK_WIDGET(cbdata->view));
    }

    return FALSE;
}

gboolean onViewDraw(GtkWidget *view, cairo_t *cr, gpointer data)
{
    CBData *d = static_cast<CBData*>(data);

    /* y coordinate of the top most visible pixel row */
    gdouble top = gtk_adjustment_get_value(d->vadj);
    gint stickyI = -1;

    // Let's hope this stays constant
    const gint ypad = 2;
    const gint ypad2 = 4;

    gint v = 0;
    int i;

    /* Find the sticky row */
    for (i = 0; i < d->src->numberOfRows(0); ++i)
    {
        if (!d->src->isGroupRow(0, i))
        {
            v += d->del->heightOfRow(0, i) + ypad2;
            continue;
        }

        if (v >= top)
            break;

        stickyI = i;
        v += d->del->heightOfRow(0, i) + ypad2;
    }

    /* Find the y coor of the group row below it that we're aligning against */
    for (; i < d->src->numberOfRows(0); ++i)
    {
        if (d->src->isGroupRow(0, i))
            break;

        v += d->del->heightOfRow(0, i) + ypad2;
        continue;
    }

    if (stickyI == -1)
        return FALSE;

    gint stickyH = d->del->heightOfRow(0, stickyI) + ypad2;
    gint offset = std::max(0.0, (stickyH - v) + top);

    GtkCellRenderer *rend = d->gr_rend;

    g_object_set(rend, "text", d->src->valueForColumnAndRow(0, 0, stickyI).c_str(), NULL);
    g_object_set(rend, "group-row", TRUE, NULL);
    g_object_set(rend, "height", stickyH, NULL);

    gint allocW = gtk_widget_get_allocated_width(view);
    GdkRectangle background = { 0, -offset, allocW, stickyH };
    GdkRectangle cellarea = { ypad, -offset+ypad, allocW-ypad2, stickyH - ypad2 };

    /* Render sticky row */
    gtk_cell_renderer_render(rend, cr, view, &background, &cellarea, (GtkCellRendererState) 0);

    return FALSE;
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *view = gtk_tree_view_new();
    GtkAdjustment *vadj = gtk_adjustment_new(0, 0, 1.0, 0, 0, 0);
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, vadj);

    gtk_window_set_default_size(GTK_WINDOW(win), 200, 200);

    g_signal_connect(G_OBJECT(win), "delete-event", G_CALLBACK(gtk_main_quit), NULL);

    gtk_container_add(GTK_CONTAINER(scroll), view);
    gtk_container_add(GTK_CONTAINER(win), scroll);

    Icons icons;
    icons.play = cairo_image_surface_create_from_png("play.png");
    icons.stop = cairo_image_surface_create_from_png("stop.png");
    assert(cairo_surface_status(icons.play) == CAIRO_STATUS_SUCCESS);
    assert(cairo_surface_status(icons.stop) == CAIRO_STATUS_SUCCESS);

    Source src;
    Delegate del(src);
    CBData cbdata = { GTK_TREE_VIEW(view), &src, &del, cairo_image_surface_get_width(icons.play), vadj, 0 };
    g_signal_connect(G_OBJECT(view), "button-press-event", G_CALLBACK(onViewButtonPress), &cbdata);
    g_signal_connect_after(G_OBJECT(view), "draw", G_CALLBACK(onViewDraw), &cbdata);

    /* Need to redraw the view every time it is scrolled (so sticky row is properly drawn),
     * can probably be optimized to only queue the area of the sticky */
    g_signal_connect_swapped(G_OBJECT(vadj), "value-changed", G_CALLBACK(gtk_widget_queue_draw), view);

    TableModel *model = table_model_new(&src, 0, &del);
    gtk_tree_view_set_model(GTK_TREE_VIEW(view), GTK_TREE_MODEL(model));

    updateView(&src, GTK_TREE_VIEW(view), icons, &cbdata.gr_rend);

    gtk_widget_show_all(win);
    gtk_main();

    cairo_surface_destroy(icons.play);
    cairo_surface_destroy(icons.stop);

    return 0;
}
