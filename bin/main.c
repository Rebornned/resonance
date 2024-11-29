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

// Inicializações
void registerSignals(GtkBuilder *builder);
void switchPage(GtkButton *btn, gpointer user_data);
void setting_musics_list(gpointer data);

// -----------------------------------------------------------------

// Animações
void button_set_click_animation(GtkWidget *button); 
gboolean button_click_animation(gpointer data);

// --------------------------------------------------------------

// Edição
void change_label_text(GtkLabel *label, gchar *text);
void set_button_text_with_limit(GtkWidget *button, const char *text);
void set_actual_music_in_list(GtkButton *btn, gpointer user_data);

// -------------------------------------------------------------------

// =======================================================================================================

// -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*--*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Ponteiros globais

// Main window
GtkWidget *window; 
GtkBuilder *builder;
GtkStack *main_stack;

// Universal
FILE * pMusicsDatabase;
musica * pMusicsDatabaseVector;

// Frame 2
int fr2_main_stack_index;
GtkWidget **fr2_musics_btns_vector;

// =========================================================================================================

int main (int argc, char *argv[]) {
    // Inicialização da tela
    // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
    setlocale(LC_ALL, "en_US.utf8");
    gtk_init(&argc, &argv); // Init gtk

    // Inicialização dos ponteiros GTK
    GtkCssProvider *css_provider;
    // Iniciando interface XML para C
    builder = gtk_builder_new_from_file("../assets/ui_files/playlists.glade");
    gtk_builder_connect_signals(builder, NULL);

    // Inicialização de objetos principais da interface
    window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    gtk_window_set_icon_from_file(GTK_WINDOW(window), "../assets/ui_images/playlists_ico.png", NULL);
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

    // Main stack principal
    main_stack = GTK_STACK(gtk_builder_get_object(builder, "main_stack"));

    // ================================================================================================
    // Frame 2 - Principal
    // Inicializações globais
    fr2_main_stack_index = 0;
    pMusicsDatabase = fopen("../files/musics_database.bin", "rb+");
    pMusicsDatabaseVector = readMusicsvector(pMusicsDatabase);

    // Armazenador de botões temporários
    fr2_musics_btns_vector =  malloc(sizeof(GtkWidget *) * musicsLength(pMusicsDatabase));
    
    // Set inicial da organização das músicas
    setting_musics_list(GINT_TO_POINTER(0));
    
    // Funções da tela
    //GtkLabel *label1 = GTK_LABEL(gtk_builder_get_object(builder, "label1"));
    //change_label_text(label1, "teste");
    
    // ************************************************************************************************

    // Registrando sinais de callback para botões executarem funções
    registerSignals(builder);
    //g_signal_connect(window, "map", G_CALLBACK(set_cursor_window), NULL);

    gtk_widget_show_all(window);

    gtk_main();


    return 0;
}
/*
================================================================================================ 
Inicializations */

void switchPage(GtkButton *btn, gpointer user_data) {
    GtkStack *fr2_stack = GTK_STACK(gtk_builder_get_object(builder, "fr2_stack"));
    GtkLabel *fr2_label_page = GTK_LABEL(gtk_builder_get_object(builder, "fr2_label_page"));
    GtkImage *fr2_index_image = GTK_IMAGE(gtk_builder_get_object(builder, "fr2_index_image"));

    // Obtém o nome do botão clicado
    const gchar *button_name = gtk_widget_get_name(GTK_WIDGET(btn));

    /*// Frame 2 Elementos para limpar
    GtkLabel *fr2_label_elements[] = {NULL};
    GtkEntry *fr2_input_elements[] = {fr2_inp_user, fr2_inp_pass, NULL};

    // Frame 3 Elementos para limpar
    GtkLabel *fr3_label_elements[] = {NULL};
    GtkEntry *fr3_input_elements[] = {NULL};

    // Frame 5 Elementos para analisar e limpar
    GtkEntry *fr5_cave_inp_name = GTK_ENTRY(gtk_builder_get_object(builder, "fr5_cave_inp_name"));
    GtkLabel *fr5_cave_dragon_name_error = GTK_LABEL(gtk_builder_get_object(builder, "fr5_cave_dragon_name_error"));
    */

    // Altera a página visível da GtkStack com base no nome do botão
    // Frame 1
    if (g_strcmp0(button_name, "fr1_btn_music") == 0) {
        button_set_click_animation(GTK_WIDGET(btn));
        gtk_stack_set_visible_child_name(main_stack, "page_main");
    }

    if (g_strcmp0(button_name, "fr1_btn_playlist") == 0) {
        gtk_stack_set_visible_child_name(main_stack, "page_main");
    }

    // Frame 2
    if (g_strcmp0(button_name, "fr2_arrow_return") == 0) {
        button_set_click_animation(GTK_WIDGET(btn));
        gtk_stack_set_transition_type(fr2_stack, GTK_STACK_TRANSITION_TYPE_SLIDE_RIGHT);
        if(fr2_main_stack_index == 1) {
            gtk_image_set_from_file(fr2_index_image, "../assets/ui_images/selector_page_1.png");
            change_label_text(fr2_label_page, "Músicas");
            gtk_stack_set_visible_child_name(fr2_stack, "page_music");
            fr2_main_stack_index--;
        }
    }

    if (g_strcmp0(button_name, "fr2_arrow_next") == 0) {
        button_set_click_animation(GTK_WIDGET(btn));
        gtk_stack_set_transition_type(fr2_stack, GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT);
        if(fr2_main_stack_index == 0) {
            gtk_image_set_from_file(fr2_index_image, "../assets/ui_images/selector_page_2.png");
            change_label_text(fr2_label_page, "Playlists");
            gtk_stack_set_visible_child_name(fr2_stack, "page_playlist");
            fr2_main_stack_index++;
        }
    }
    
}

void registerSignals(GtkBuilder *builder) {

    // Registrar sinais de callback

    // Frame 1 Botões
    GObject *fr1_btn_music = gtk_builder_get_object(builder, "fr1_btn_music");
    g_signal_connect(fr1_btn_music, "clicked", G_CALLBACK(switchPage), main_stack);

    // Frame 2 Botões
    GObject * fr2_btn_musics_sort = gtk_builder_get_object(builder, "fr2_btn_musics_sort");
    g_signal_connect(fr2_btn_musics_sort, "clicked", G_CALLBACK(setting_musics_list), NULL);

    GObject * fr2_arrow_return = gtk_builder_get_object(builder, "fr2_arrow_return");
    g_signal_connect(fr2_arrow_return, "clicked", G_CALLBACK(switchPage), NULL);

    GObject * fr2_arrow_next = gtk_builder_get_object(builder, "fr2_arrow_next");
    g_signal_connect(fr2_arrow_next, "clicked", G_CALLBACK(switchPage), NULL);

}

/**********************************************************************************************
Settings 
*/
void setting_musics_list(gpointer data) {
    int setting = GPOINTER_TO_INT(data);

    GtkWidget *btn = GTK_WIDGET(gtk_builder_get_object(builder, "fr2_btn_musics_sort"));
    button_set_click_animation(btn);

    GtkLabel * fr2_music_actual_sort = GTK_LABEL(gtk_builder_get_object(builder, "fr2_music_actual_sort"));
    int typeSort = 1;
    const gchar *sortText = gtk_label_get_text(fr2_music_actual_sort);

    if(strcmp(sortText, "ID") == 0) {
        typeSort = 2;
        change_label_text(fr2_music_actual_sort, "Duração");
    }
    else if(strcmp(sortText, "Duração") == 0) {
        typeSort = 3;
        change_label_text(fr2_music_actual_sort, "Alfabética");
    }
    else if(strcmp(sortText, "Alfabética") == 0) {
        typeSort = 4;
        change_label_text(fr2_music_actual_sort, "Artista");
    }
    else if(strcmp(sortText, "Artista") == 0) {
        typeSort = 5;
        change_label_text(fr2_music_actual_sort, "Álbum");
    }
    else if(strcmp(sortText, "Álbum") == 0) {
        typeSort = 1;
        change_label_text(fr2_music_actual_sort, "ID");
    }

    int length = musicsLength(pMusicsDatabase);
    GtkFixed *fixed = GTK_FIXED(gtk_builder_get_object(builder, "fr2_musics_list_fixed"));
    
    bubbleTypeSort(pMusicsDatabaseVector, typeSort, length);
    set_actual_music_in_list(NULL, GINT_TO_POINTER(0));

    for(int i=0; i < length; i++) {
        if(setting == 0) {
            GtkWidget *button = gtk_button_new_with_label("");
            fr2_musics_btns_vector[i] = button;
            set_button_text_with_limit(button, pMusicsDatabaseVector[i].nome);
            gtk_fixed_put(GTK_FIXED(fixed), button, 0, 63*i); 
            gtk_widget_set_size_request(button, 239, 63);     
            GtkStyleContext *btn_context = gtk_widget_get_style_context(button);
            gtk_style_context_add_class(btn_context, "universal_green_border");
            gtk_style_context_add_class(btn_context, "universal_font");
            gtk_style_context_add_class(btn_context, "color_FFFFFF");
            gtk_style_context_add_class(btn_context, "font_size_20");
            gtk_style_context_add_class(btn_context, "border_0_radius");
            gtk_style_context_add_class(btn_context, "fr2_btns_musics_list_border");

            g_signal_connect(button, "clicked", G_CALLBACK(set_actual_music_in_list), GINT_TO_POINTER(i));
        }
        else {
            set_button_text_with_limit(fr2_musics_btns_vector[i], pMusicsDatabaseVector[i].nome);
        }
    }
}


/*
================================================================================================
Animations */

void button_set_click_animation(GtkWidget *button) {
    gtk_widget_set_opacity(button, 0.7);
    g_timeout_add(100, button_click_animation, button);
}

gboolean button_click_animation(gpointer data) {
    GtkWidget *button = GTK_WIDGET(data);
    gtk_widget_set_opacity(button, 1.0);
    return FALSE;
}

/***********************************************************************************************/
/* Editions */
void set_actual_music_in_list(GtkButton *btn, gpointer user_data) {
    int index = GPOINTER_TO_INT(user_data);
    gchar sprintText[200];
    int minutes, seconds;

    if(btn != NULL)
        button_set_click_animation(GTK_WIDGET(btn));

    GtkLabel *fr2_label_view_name = GTK_LABEL(gtk_builder_get_object(builder, "fr2_label_view_name"));
    GtkLabel *fr2_label_view_artist = GTK_LABEL(gtk_builder_get_object(builder, "fr2_label_view_artist"));
    GtkLabel *fr2_label_view_album = GTK_LABEL(gtk_builder_get_object(builder, "fr2_label_view_album"));
    GtkLabel *fr2_label_view_minutes = GTK_LABEL(gtk_builder_get_object(builder, "fr2_label_view_minutes"));
    GtkLabel *fr2_label_view_id = GTK_LABEL(gtk_builder_get_object(builder, "fr2_label_view_id"));

    GtkFixed *fixed = GTK_FIXED(gtk_builder_get_object(builder, "fr2_musics_list_fixed"));
    GtkWidget *fr2_btn_music_marker = GTK_WIDGET(gtk_builder_get_object(builder, "fr2_btn_music_marker"));
    gtk_fixed_move(GTK_FIXED(fixed), fr2_btn_music_marker, 8, 63*index+9);
    
    minutes = pMusicsDatabaseVector[index].tempo / 60;
    seconds = pMusicsDatabaseVector[index].tempo - minutes*60;
    sprintf(sprintText, "%d:%d minutos", minutes, seconds);

    if(seconds < 10)
        sprintf(sprintText, "%d:0%d minutos", minutes, seconds);
    
    change_label_text(fr2_label_view_name, pMusicsDatabaseVector[index].nome);
    change_label_text(fr2_label_view_artist, pMusicsDatabaseVector[index].artista);
    change_label_text(fr2_label_view_album, pMusicsDatabaseVector[index].album);
    change_label_text(fr2_label_view_minutes, sprintText);
    
    sprintf(sprintText, "%d", pMusicsDatabaseVector[index].id);
    change_label_text(fr2_label_view_id, sprintText);


}

void change_label_text(GtkLabel *label, gchar *text) {
    gtk_label_set_text(label, text);
}

void set_button_text_with_limit(GtkWidget *button, const char *text) {
    const int max_length = 18;

    char truncated_text[max_length + 4]; 
    if (g_utf8_strlen(text, -1) > max_length) {
        g_utf8_strncpy(truncated_text, text, max_length); 
        strcat(truncated_text, "...");
    } else {
        strcpy(truncated_text, text);
    }

    // Define o texto no botão
    gtk_button_set_label(GTK_BUTTON(button), truncated_text);
    // Configura o GtkLabel do botão para cortar o texto com reticências
    GtkWidget *label = gtk_bin_get_child(GTK_BIN(button));
    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
}

/*=============================================================================================*/