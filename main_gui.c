#include <gtk/gtk.h>
#include <stdio.h>
#include <time.h>
#include "image_io.h"

#define MODE_SEQUENTIAL 0
#define MODE_PARALLEL 1

typedef struct {
    GtkWidget *window;
    GtkWidget *image;
    char *input_filename;
    char *output_filename;
    int mode;
} AppWidgets;

static void process_image_sequential(const char *input_filename, const char *output_filename) {
    clock_t start_time = clock();

    int width, height, num_components;
    unsigned char *image_data = read_jpeg_file(input_filename, &width, &height, &num_components);
    if (!image_data) {
        g_print("Erreur lors de la lecture du fichier JPEG.\n");
        return;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = (y * width + x) * num_components;
            unsigned char gray = 0.299 * image_data[index] + 0.587 * image_data[index + 1] + 0.114 * image_data[index + 2];
            image_data[index] = gray;
            image_data[index + 1] = gray;
            image_data[index + 2] = gray;
        }
    }

    write_jpeg_file(output_filename, image_data, width, height, num_components);
    free(image_data);

    clock_t end_time = clock();
    double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    g_print("Image processed sequentially in %.4f seconds.\n", execution_time);
}

static void process_image_parallel(const char *input_filename, const char *output_filename) {
    clock_t start_time = clock();

    process_image_sequential(input_filename, output_filename);

    clock_t end_time = clock();
    double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    g_print("Image processed in parallel in %.4f seconds.\n", execution_time);
}

static void on_file_set(GtkFileChooserButton *button, AppWidgets *widgets) {
    if (widgets->input_filename) {
        g_free(widgets->input_filename);
    }
    widgets->input_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(button));
}

static void on_mode_changed(GtkComboBoxText *combo, AppWidgets *widgets) {
    const char *mode_text = gtk_combo_box_text_get_active_text(combo);
    if (g_strcmp0(mode_text, "Séquentiel") == 0) {
        widgets->mode = MODE_SEQUENTIAL;
    } else if (g_strcmp0(mode_text, "Parallèle") == 0) {
        widgets->mode = MODE_PARALLEL;
    }
}

static void on_process_image_clicked(GtkWidget *widget, AppWidgets *widgets) {
    if (!widgets->input_filename) {
        g_print("Aucun fichier sélectionné.\n");
        return;
    }

    widgets->output_filename = g_strdup_printf("output_%s.jpg",
                                               widgets->mode == MODE_SEQUENTIAL ? "sequential" : "parallel");

    if (widgets->mode == MODE_SEQUENTIAL) {
        process_image_sequential(widgets->input_filename, widgets->output_filename);
    } else if (widgets->mode == MODE_PARALLEL) {
        process_image_parallel(widgets->input_filename, widgets->output_filename);
    }

    gtk_image_set_from_file(GTK_IMAGE(widgets->image), widgets->output_filename);
    g_print("Image processed and saved to %s\n", widgets->output_filename);
}

static void on_start_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *file_chooser;
    GtkWidget *combo;
    GtkWidget *button;
    GtkWidget *image;
    GtkWidget *title;
    GtkWidget *authors;
    AppWidgets *widgets = (AppWidgets *)data;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Image Processing Application - Eya Rahmeni & Ameni Rahmeni");

    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file("2.png", NULL);
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    gtk_window_set_default_size(GTK_WINDOW(window), width, height);
    g_object_unref(pixbuf);

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    title = gtk_label_new("Application de Traitement d'Images");
    gtk_grid_attach(GTK_GRID(grid), title, 0, 0, 2, 1);

    authors = gtk_label_new("Réalisé par : Eya Rahmeni & Ameni Rahmeni");
    gtk_grid_attach(GTK_GRID(grid), authors, 0, 1, 2, 1);

    file_chooser = gtk_file_chooser_button_new("Choisissez une image", GTK_FILE_CHOOSER_ACTION_OPEN);
    g_signal_connect(file_chooser, "file-set", G_CALLBACK(on_file_set), widgets);
    gtk_grid_attach(GTK_GRID(grid), file_chooser, 0, 2, 2, 1);

    combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "Séquentiel");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "Parallèle");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    g_signal_connect(combo, "changed", G_CALLBACK(on_mode_changed), widgets);
    gtk_grid_attach(GTK_GRID(grid), combo, 0, 3, 2, 1);

    button = gtk_button_new_with_label("Process Image");
    gtk_widget_set_name(button, "custom-button");
    g_signal_connect(button, "clicked", G_CALLBACK(on_process_image_clicked), widgets);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 4, 2, 1);

    image = gtk_image_new_from_file("2.png");
    gtk_widget_set_size_request(image, width, height);
    gtk_widget_set_name(image, "image-container");
    gtk_grid_attach(GTK_GRID(grid), image, 0, 5, 2, 1);
    widgets->image = image;

    gtk_widget_show_all(window);
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *start_button;
    GtkWidget *background_image;
    AppWidgets widgets = {0};

    gtk_init(&argc, &argv);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "style.css", NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_USER);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Welcome to Image Processing Application");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    background_image = gtk_image_new_from_file("1.png");
    gtk_widget_set_size_request(background_image, 800, 600);
    gtk_widget_set_name(background_image, "image-container");
    gtk_grid_attach(GTK_GRID(grid), background_image, 0, 0, 1, 1);

    start_button = gtk_button_new_with_label("Start");
    gtk_widget_set_name(start_button, "start_button");
    g_signal_connect(start_button, "clicked", G_CALLBACK(on_start_button_clicked), &widgets);
    gtk_grid_attach(GTK_GRID(grid), start_button, 0, 1, 1, 1);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
