#include <gtk/gtk.h>

typedef struct {
    gchar *id;
    gchar *name;
    gchar *quantity;
    gchar *price;
} Product;

GtkListStore *list_store;
GHashTable *used_ids;

void show_error_dialog(GtkWidget *parent, const gchar *message) {
    GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(parent), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                                     GTK_BUTTONS_OK, "%s", message);
    gtk_dialog_run(GTK_DIALOG(error_dialog));
    gtk_widget_destroy(error_dialog);
}

void add_product(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Add Product", GTK_WINDOW(data), GTK_DIALOG_MODAL,
                                                   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                   GTK_STOCK_OK, GTK_RESPONSE_OK,
                                                   NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

    GtkWidget *id_label = gtk_label_new("Product ID:");
    GtkWidget *id_entry = gtk_entry_new();

    GtkWidget *name_label = gtk_label_new("Product Name:");
    GtkWidget *name_entry = gtk_entry_new();

    GtkWidget *quantity_label = gtk_label_new("Quantity:");
    GtkWidget *quantity_entry = gtk_entry_new();

    GtkWidget *price_label = gtk_label_new("Price:");
    GtkWidget *price_entry = gtk_entry_new();

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_box_pack_start(GTK_BOX(content_area), id_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content_area), id_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content_area), name_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content_area), name_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content_area), quantity_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content_area), quantity_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content_area), price_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content_area), price_entry, TRUE, TRUE, 0);

    gtk_widget_show_all(dialog);

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));

    if (response == GTK_RESPONSE_OK) {
        const gchar *id = gtk_entry_get_text(GTK_ENTRY(id_entry));
        const gchar *name = gtk_entry_get_text(GTK_ENTRY(name_entry));
        const gchar *quantity_str = gtk_entry_get_text(GTK_ENTRY(quantity_entry));
        const gchar *price_str = gtk_entry_get_text(GTK_ENTRY(price_entry));

        gchar *endptr;
        gdouble quantity = g_strtod(quantity_str, &endptr);
        if (*endptr != '\0' || quantity < 0) {
            show_error_dialog(GTK_WIDGET(data), "Invalid quantity. Please enter a valid number.");
            gtk_widget_destroy(dialog);
            return;
        }

        gdouble price = g_strtod(price_str, &endptr);
        if (*endptr != '\0' || price < 0) {
            show_error_dialog(GTK_WIDGET(data), "Invalid price. Please enter a valid number.");
            gtk_widget_destroy(dialog);
            return;
        }

        if (g_hash_table_contains(used_ids, id)) {
            show_error_dialog(GTK_WIDGET(data), g_strdup_printf("ID %s is already in use. Please enter a different ID.", id));
        } else {
            g_hash_table_add(used_ids, g_strdup(id));

            Product product = {g_strdup(id), g_strdup(name), g_strdup(quantity_str), g_strdup(price_str)};
            GtkTreeIter iter;
            gtk_list_store_append(list_store, &iter);
            gtk_list_store_set(list_store, &iter, 0, product.id, 1, product.name, 2, product.quantity, 3, product.price, -1);
        }
    }

    gtk_widget_destroy(dialog);
}

void delete_product(GtkWidget *widget, gpointer data) {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data));
    GList *selected_rows = gtk_tree_selection_get_selected_rows(selection, NULL);

    GList *l;
    for (l = selected_rows; l != NULL; l = l->next) {
        GtkTreePath *path = l->data;
        GtkTreeIter iter;

        if (gtk_tree_model_get_iter(GTK_TREE_MODEL(list_store), &iter, path)) {
            gchar *id;
            gtk_tree_model_get(GTK_TREE_MODEL(list_store), &iter, 0, &id, -1);
            g_hash_table_remove(used_ids, id);
            g_free(id);

            gtk_list_store_remove(list_store, &iter);
        }

        gtk_tree_path_free(path);
    }

    g_list_free(selected_rows);
}

void modify_quantity(GtkWidget *widget, gpointer data) {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data));
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar *current_quantity;
        gtk_tree_model_get(model, &iter, 2, &current_quantity, -1);

        GtkWidget *dialog = gtk_dialog_new_with_buttons("Modify Quantity", GTK_WINDOW(gtk_widget_get_toplevel(data)),
                                                       GTK_DIALOG_MODAL,
                                                       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                       GTK_STOCK_OK, GTK_RESPONSE_OK,
                                                       NULL);
        gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

        GtkWidget *quantity_label = gtk_label_new("New Quantity:");
        GtkWidget *quantity_entry = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(quantity_entry), current_quantity);

        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        gtk_box_pack_start(GTK_BOX(content_area), quantity_label, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(content_area), quantity_entry, TRUE, TRUE, 0);

        gtk_widget_show_all(dialog);

        gint response = gtk_dialog_run(GTK_DIALOG(dialog));

        if (response == GTK_RESPONSE_OK) {
            const gchar *new_quantity_str = gtk_entry_get_text(GTK_ENTRY(quantity_entry));

            gchar *endptr;
            gdouble new_quantity = g_strtod(new_quantity_str, &endptr);
            if (*endptr != '\0' || new_quantity < 0) {
                show_error_dialog(GTK_WIDGET(data), "Invalid quantity. Please enter a valid number.");
                gtk_widget_destroy(dialog);
                g_free(current_quantity);
                return;
            }

            gtk_list_store_set(list_store, &iter, 2, new_quantity_str, -1);
        }

        g_free(current_quantity);
        gtk_widget_destroy(dialog);
    }
}
void modify_price(GtkWidget *widget, gpointer data) {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data));
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar *current_price;
        gtk_tree_model_get(model, &iter, 3, &current_price, -1);

        GtkWidget *dialog = gtk_dialog_new_with_buttons("Modify Price", GTK_WINDOW(gtk_widget_get_toplevel(data)),
                                                       GTK_DIALOG_MODAL,
                                                       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                       GTK_STOCK_OK, GTK_RESPONSE_OK,
                                                       NULL);
        gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

        GtkWidget *price_label = gtk_label_new("New Price:");
        GtkWidget *price_entry = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(price_entry), current_price);

        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        gtk_box_pack_start(GTK_BOX(content_area), price_label, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(content_area), price_entry, TRUE, TRUE, 0);

        gtk_widget_show_all(dialog);

        gint response = gtk_dialog_run(GTK_DIALOG(dialog));

        if (response == GTK_RESPONSE_OK) {
            const gchar *new_price_str = gtk_entry_get_text(GTK_ENTRY(price_entry));

            gchar *endptr;
            gdouble new_price = g_strtod(new_price_str, &endptr);
            if (*endptr != '\0' || new_price < 0) {
                show_error_dialog(GTK_WIDGET(data), "Invalid price. Please enter a valid number.");
                gtk_widget_destroy(dialog);
                g_free(current_price);
                return;
            }

            gtk_list_store_set(list_store, &iter, 3, new_price_str, -1);
        }

        g_free(current_price);
        gtk_widget_destroy(dialog);
    }
}
void sell_product(GtkWidget *widget, gpointer data) {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data));
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar *product_name, *quantity_str, *price_str, *id;
        gtk_tree_model_get(model, &iter, 0, &id, 1, &product_name, 2, &quantity_str, 3, &price_str, -1);

        GtkWidget *dialog = gtk_dialog_new_with_buttons("Sell Product", GTK_WINDOW(gtk_widget_get_toplevel(data)),
                                                       GTK_DIALOG_MODAL,
                                                       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                       GTK_STOCK_OK, GTK_RESPONSE_OK,
                                                       NULL);
        gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

        GtkWidget *quantity_label = gtk_label_new("Quantity to Sell:");
        GtkWidget *quantity_entry = gtk_entry_new();

        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        gtk_box_pack_start(GTK_BOX(content_area), quantity_label, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(content_area), quantity_entry, TRUE, TRUE, 0);

        gtk_widget_show_all(dialog);

        gint response = gtk_dialog_run(GTK_DIALOG(dialog));

        if (response == GTK_RESPONSE_OK) {
            const gchar *sell_quantity_str = gtk_entry_get_text(GTK_ENTRY(quantity_entry));

            gchar *endptr;
            gdouble sell_quantity = g_strtod(sell_quantity_str, &endptr);
            if (*endptr != '\0' || sell_quantity < 0 || sell_quantity > g_ascii_strtod(quantity_str, NULL)) {
                show_error_dialog(GTK_WIDGET(data), "Invalid sell quantity. Please enter a valid number within available stock.");
                gtk_widget_destroy(dialog);
                g_free(product_name);
                g_free(quantity_str);
                g_free(price_str);
                g_free(id);
                return;
            }

            gdouble price = g_strtod(price_str, NULL);
            gdouble total_price = sell_quantity * price;

            gdouble remaining_quantity = g_ascii_strtod(quantity_str, NULL) - sell_quantity;
            gchar *remaining_quantity_str = g_strdup_printf("%.2f", remaining_quantity);
            gtk_list_store_set(list_store, &iter, 2, remaining_quantity_str, -1);

            gchar *receipt_message = g_strdup_printf("Product: %s\nQuantity Sold: %.2f\nTotal Price: %.2f", product_name, sell_quantity, total_price);
            GtkWidget *receipt_dialog = gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
                                                               GTK_BUTTONS_OK, "%s", receipt_message);
            gtk_dialog_run(GTK_DIALOG(receipt_dialog));
            gtk_widget_destroy(receipt_dialog);

            g_free(receipt_message);
            g_free(remaining_quantity_str);
        }

        g_free(product_name);
        g_free(quantity_str);
        g_free(price_str);
        g_free(id);
        gtk_widget_destroy(dialog);
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *main_window = window;
    gtk_window_set_title(GTK_WINDOW(window), "Products");
    gtk_widget_set_size_request(window, 400, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    list_store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    GtkWidget *treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview), TRUE);

    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    column = gtk_tree_view_column_new_with_attributes("Product", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    column = gtk_tree_view_column_new_with_attributes("Quantity", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    column = gtk_tree_view_column_new_with_attributes("Price", renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    GtkWidget *add_button = gtk_button_new_with_label("Add");
    GtkWidget *delete_button = gtk_button_new_with_label("Delete");
    GtkWidget *modify_button = gtk_button_new_with_label("Modify Quantity");
    GtkWidget *modify_price_button = gtk_button_new_with_label("Modify Price");
    GtkWidget *sell_button = gtk_button_new_with_label("Sell");

    g_signal_connect(add_button, "clicked", G_CALLBACK(add_product), treeview);
    g_signal_connect(delete_button, "clicked", G_CALLBACK(delete_product), treeview);
    g_signal_connect(modify_button, "clicked", G_CALLBACK(modify_quantity), treeview);
    g_signal_connect(modify_price_button, "clicked", G_CALLBACK(modify_price), treeview);
    g_signal_connect(sell_button, "clicked", G_CALLBACK(sell_product), treeview);

    GtkWidget *button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(button_box), GTK_BUTTONBOX_SPREAD);
    gtk_box_pack_start(GTK_BOX(button_box), add_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), delete_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), modify_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), modify_price_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), sell_button, TRUE, TRUE, 0);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window), treeview);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(window), vbox);

    GdkRGBA color;
    gdk_rgba_parse(&color, "#b10000");
    gtk_widget_override_background_color(main_window, GTK_STATE_FLAG_NORMAL, &color);

    gtk_widget_show_all(window);

    used_ids = g_hash_table_new(g_str_hash, g_str_equal);

    gtk_main();

    GtkTreeModel *model = GTK_TREE_MODEL(list_store);
    GtkTreeIter iter;
    gboolean valid = gtk_tree_model_get_iter_first(model, &iter);

    while (valid) {
        gchar *id, *name, *quantity, *price;
        gtk_tree_model_get(model, &iter, 0, &id, 1, &name, 2, &quantity, 3, &price, -1);
        g_free(id);
        g_free(name);
        g_free(quantity);
        g_free(price);

        valid = gtk_tree_model_iter_next(model, &iter);
    }

    g_object_unref(list_store);

    g_hash_table_destroy(used_ids);

    return 0;
}
