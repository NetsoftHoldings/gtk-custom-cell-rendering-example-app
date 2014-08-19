
#include "tablemodel.h"

static void
table_model_class_init(TableModelClass *)
{
}

static void
table_model_init(TableModel *)
{
}

TableModel *table_model_new(NSC::NSTableViewDataSource *source,
                            NSC::NSTableView *view,
                            NSC::NSTableViewDelegate *del)
{
    TableModel *self = TABLE_MODEL(g_object_new(TYPE_TABLE_MODEL, NULL));

    if (!self)
        return NULL;

    self->src = source;
    self->view = view;
    self->del = del;

    return self;
}

void table_model_set_source(TableModel *table_model, NSC::NSTableViewDataSource *source)
{
    table_model->src = source;
}

void table_model_set_delegate(TableModel *table_model, NSC::NSTableViewDelegate *del)
{
    table_model->del = del;
}

void table_model_notify_row_changed(TableModel *table_model, gint row)
{
    GtkTreePath *path = gtk_tree_path_new_from_indices(row, -1);
    GtkTreeIter iter;
    iter.user_data = GINT_TO_POINTER(row);

    g_signal_emit_by_name(table_model, "row-changed", path, &iter);

    g_object_unref(path);
}

#define GET_SRC NSC::NSTableViewDataSource *src = TABLE_MODEL(tree_model)->src
#define GET_ROW gint row = GPOINTER_TO_INT(iter->user_data)
#define GET_DEL NSC::NSTableViewDelegate *del = TABLE_MODEL(tree_model)->del;
#define GET_VIEW TABLE_MODEL(tree_model)->view

static GtkTreeModelFlags
table_model_get_flags(GtkTreeModel *)
{
    return GTK_TREE_MODEL_LIST_ONLY;
}

static gint
table_model_get_n_columns(GtkTreeModel *tree_model)
{
    GET_SRC;

    if (!src)
        return 0;

    return src->numberOfColumns(GET_VIEW) + 1;
}

static GType
table_model_get_column_type(GtkTreeModel *, gint index)
{
    switch (index)
    {
    case TABLE_MODEL_ROW_HEIGHT :
        return G_TYPE_INT;
    case TABLE_MODEL_GROUP_ROW :
        return G_TYPE_BOOLEAN;
    case TABLE_MODEL_ACTIVE :
        return G_TYPE_BOOLEAN;
    default:
        return G_TYPE_STRING;
    }
}

static gboolean
table_model_get_iter(GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreePath *path)
{
    GET_SRC;

    if (!src)
        return FALSE;

    if (gtk_tree_path_get_depth(path) != 1)
        return FALSE;

    gint row = *gtk_tree_path_get_indices(path);

    if (row < 0 || row > src->numberOfRows(GET_VIEW))
        return FALSE;

    iter->user_data = GINT_TO_POINTER(row);
    return TRUE;
}

static GtkTreePath*
table_model_get_path(GtkTreeModel *tree_model, GtkTreeIter *iter)
{
    GET_SRC;
    GET_ROW;

    GtkTreePath *path = gtk_tree_path_new();

    if (!src)
        return path;

    if (row < 0 || row >= src->numberOfRows(GET_VIEW))
        return path;

    gtk_tree_path_append_index(path, row);
    return path;
}

static void
table_model_get_value(GtkTreeModel *tree_model, GtkTreeIter *iter, gint column, GValue *value)
{
    GET_SRC;
    GET_ROW;
    GET_DEL;

    if (!src)
        return;

    g_value_init(value, table_model_get_column_type(tree_model, column));

    switch (column)
    {
    case TABLE_MODEL_ROW_HEIGHT :
        g_value_set_int(value, del ? del->heightOfRow(GET_VIEW, row) : 0);
        return;
    case TABLE_MODEL_GROUP_ROW :
        g_value_set_boolean(value, src->isGroupRow(GET_VIEW, row));
        return;
    case TABLE_MODEL_ACTIVE :
        g_value_set_boolean(value, src->activeRow() == row);
        return;
    default:
        const char *str = src->valueForColumnAndRow
            (GET_VIEW, column-TABLE_MODEL_DATA_OFFSET, row).c_str();
        g_value_set_string(value, str);
    }
}

static gboolean
table_model_iter_next(GtkTreeModel *tree_model, GtkTreeIter *iter)
{
    GET_SRC;
    GET_ROW;

    if (!src)
        return FALSE;

    if (row < 0 || row >= src->numberOfRows(GET_VIEW)-1)
        return FALSE;

    iter->user_data = GINT_TO_POINTER(row+1);
    return TRUE;
}

static gboolean
table_model_iter_previous(GtkTreeModel *tree_model, GtkTreeIter *iter)
{
    GET_SRC;
    GET_ROW;

    if (!src)
        return FALSE;

    if (row <= 0 || row >= src->numberOfRows(GET_VIEW))
        return FALSE;

    iter->user_data = GINT_TO_POINTER(row-1);
    return TRUE;
}

static gboolean
table_model_iter_children(GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreeIter *parent)
{
    GET_SRC;

    if (!src)
        return FALSE;

    // no children
    if (parent)
        return FALSE;

    if (src->numberOfRows(GET_VIEW) == 0)
        return FALSE;

    // first row
    iter->user_data = GINT_TO_POINTER(0);
    return TRUE;
}

static gboolean table_model_iter_has_child(GtkTreeModel *, GtkTreeIter *)
{
    return FALSE;
}

static gint
table_model_iter_n_children(GtkTreeModel *tree_model, GtkTreeIter *iter)
{
    GET_SRC;

    if (!src)
        return 0;

    if (iter == NULL)
        return src->numberOfRows(GET_VIEW);
    else
        return 0;
}

static gboolean
table_model_iter_nth_child(GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreeIter *parent, gint n)
{
    GET_SRC;

    if (!src)
        return FALSE;

    if (parent)
        return FALSE;

    if (n < 0 || n >= src->numberOfRows(GET_VIEW))
        return FALSE;

    iter->user_data = GINT_TO_POINTER(n);
    return TRUE;
}

static gboolean
table_model_iter_parent(GtkTreeModel *, GtkTreeIter *, GtkTreeIter *)
{
    return FALSE;
}

static void
table_model_tree_model_init(GtkTreeModelIface *iface)
{
    iface->get_flags = table_model_get_flags;
    iface->get_n_columns = table_model_get_n_columns;
    iface->get_column_type = table_model_get_column_type;
    iface->get_iter = table_model_get_iter;
    iface->get_path = table_model_get_path;
    iface->get_value = table_model_get_value;
    iface->iter_next = table_model_iter_next;
    iface->iter_previous = table_model_iter_previous;
    iface->iter_children = table_model_iter_children;
    iface->iter_has_child = table_model_iter_has_child;
    iface->iter_n_children = table_model_iter_n_children;
    iface->iter_nth_child = table_model_iter_nth_child;
    iface->iter_parent = table_model_iter_parent;
}

G_DEFINE_TYPE_WITH_CODE (TableModel, table_model, G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE (GTK_TYPE_TREE_MODEL,
						table_model_tree_model_init))
