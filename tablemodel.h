#pragma once

#include <gtk/gtk.h>

#include "tablesource.h"

G_BEGIN_DECLS


#define TYPE_TABLE_MODEL            (table_model_get_type ())
#define TABLE_MODEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_TABLE_MODEL, TableModel))
#define TABLE_MODEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_TABLE_MODEL, TableModelClass))
#define IS_TABLE_MODEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_TABLE_MODEL))
#define IS_TABLE_MODEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_TABLE_MODEL))
#define TABLE_MODEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_TABLE_MODEL, TableModelClass))

typedef struct _TableModel              TableModel;
typedef struct _TableModelClass         TableModelClass;

struct _TableModel
{
  GObject parent;

  /*< private >*/
  NSC::NSTableViewDataSource *src;
  NSC::NSTableView *view;
  NSC::NSTableViewDelegate *del;
};

struct _TableModelClass
{
  GObjectClass parent_class;
};

/* The TableViewDataSource contents are accessed by querying
 * at (row, column+TABLE_MODEL_DATA_OFFSET).
 * The first columns hold meta data (row height etc.) */

enum
{
    TABLE_MODEL_ROW_HEIGHT = 0, /* gint */
    TABLE_MODEL_GROUP_ROW,      /* gboolean */
    TABLE_MODEL_ACTIVE,         /* gboolean */

    TABLE_MODEL_DATA_OFFSET
};

GType table_model_get_type(void) G_GNUC_CONST;

TableModel *table_model_new(NSC::NSTableViewDataSource *source,
                            NSC::NSTableView *view,
                            NSC::NSTableViewDelegate *del);

void table_model_set_source(TableModel *table_model, NSC::NSTableViewDataSource *source);
void table_model_set_delegate(TableModel *table_model, NSC::NSTableViewDelegate *del);
void table_model_notify_row_changed(TableModel *table_model, gint row);


G_END_DECLS
