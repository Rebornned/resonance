#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <direct.h>
#include <math.h>
#include "playlists.h"

// ************************************************************************************************
// Compilação necessária
// cd C:/Users/Amage/Desktop/Programacao/Playlists_Final_Prog2/Final_Project_Prog2_Playlists/bin/
// gcc -o main.exe main.c -mwindows `pkg-config --cflags --libs gtk+-3.0 glib-2.0 pango`

// ================================================================================================

// +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=++=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=++=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
// Funções referentes a tela

void registerSignals(GtkBuilder *builder);
// =======================================================================================================

// -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*--*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Ponteiros globais

// Main window
GtkWidget *window; 
GtkBuilder *builder;
GtkStack *main_stack;

// =========================================================================================================

int main (int argc, char *argv[]) {
    setlocale(LC_ALL, "en_US.utf8");
    gtk_init(&argc, &argv); // Init gtk

    // Inicialização dos ponteiros GTK
    GtkCssProvider *css_provider;
    // Iniciando interface XML para C
    builder = gtk_builder_new_from_file("../assets/ui_files/playlists.glade");
    gtk_builder_connect_signals(builder, NULL);

    // Inicialização de objetos principais da interface
    window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    gtk_window_set_icon_from_file(GTK_WINDOW(window), "../assets/ui_images/playlists.ico", NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL); // Fecha a tela
    
    GdkGeometry hints_window;
    hints_window.max_width = 1000;  
    hints_window.max_height = 600; 
    gtk_window_set_geometry_hints(GTK_WINDOW(window), NULL, &hints_window, GDK_HINT_MAX_SIZE);

    // Carregar e aplicar o arquivo CSS
    css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(css_provider, "../assets/css/style.css", NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(css_provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_USER);

    
    // Registrando sinais de callback para botões executarem funções
    registerSignals(builder);

    //g_signal_connect(window, "map", G_CALLBACK(set_cursor_window), NULL);

    gtk_widget_show_all(window);

    gtk_main();


    return 0;
}

void registerSignals(GtkBuilder *builder) {
    // Registrar sinais de callback
    // Passa a main_stack como user_data

    // Frame 1 Botões
    //GObject *fr1_btn_iniciar = gtk_builder_get_object(builder, "fr1_btn_start");
    //g_signal_connect(fr1_btn_iniciar, "clicked", G_CALLBACK(switchPage), main_stack);
}