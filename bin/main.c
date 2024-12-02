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
// gcc -o main.exe main.c func.c -mwindows `pkg-config --cflags --libs gtk+-3.0 glib-2.0 pango`

// ================================================================================================
// GtkStructures
typedef struct {
    GtkWidget *widget;
    GtkFixed *container;
    gfloat opacity;
    gfloat opacityDecrease;
    gint x;
    gint y;
    gint interactions;
} GtkUpAnimationData;

// +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=++=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=++=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
// Funções referentes a tela

// Inicializações
void registerSignals(GtkBuilder *builder);
void switchPage(GtkButton *btn, gpointer user_data);
void setting_musics_list(gpointer data);
void setting_playlist_list(gpointer data);
void setting_playlist_music_list(gpointer data);

// -----------------------------------------------------------------

// Animações
void button_set_click_animation(GtkWidget *button); 
gboolean button_click_animation(gpointer data);

// --------------------------------------------------------------

// Edição
void change_label_text(GtkLabel *label, gchar *text);
void set_actual_music_in_playlist(GtkButton *btn, gpointer user_data);
void set_button_text_with_limit(GtkWidget *button, const char *text);
void set_actual_music_in_list(GtkButton *btn, gpointer user_data);
void set_actual_playlist_in_list(GtkButton *btn, gpointer user_data);
void clear_buttons_from_fixed(GtkFixed *fixed);

// -------------------------------------------------------------------
// Animations 
void logStartAnimation(gchar *text, gchar *color, gint duration, gint height, gint width, gint x, gint y, gint yDirection, GtkFixed *fixed);
gboolean logAnimation(gpointer data);
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

// Frame 2 - Músicas
int fr2_main_stack_index;
GtkWidget **fr2_musics_btns_vector;

// Frame 2 - Playlists
FILE * pPlaylistController;
GtkWidget **fr2_playlists_btns_vector;
gint playlistIndex;

// Frame 2 - Access
FILE * pActualPlaylistOpened;
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

    // ======================================================================================
    // Frame 2 - Playlists
    // Inicializações globais
    pPlaylistController = openPlaylistsController();

    // Armazenador de botões temporários
    gint allocatedSizePlaylists = lengthPlaylistsController(pPlaylistController);
    if(allocatedSizePlaylists < 4)
        allocatedSizePlaylists = 4;

    fr2_playlists_btns_vector = malloc(sizeof(GtkWidget *) * allocatedSizePlaylists);
    setting_playlist_list(GINT_TO_POINTER(0));
    // Funções da tela
    //GtkLabel *label1 = GTK_LABEL(gtk_builder_get_object(builder, "label1"));
    //change_label_text(label1, "teste");
    
    // ************************************************************************************************

    // Registrando sinais de callback para botões executarem funções
    registerSignals(builder);
    //g_signal_connect(window, "map", G_CALLBACK(set_cursor_window), NULL);

    gtk_widget_show_all(window);

    gtk_main();

    fclose(pMusicsDatabase);
    fclose(pPlaylistController);
    fclose(pActualPlaylistOpened);
    return 0;
}
/*
================================================================================================ 
Inicializations */

void switchPage(GtkButton *btn, gpointer user_data) {
    GtkStack *fr2_stack = GTK_STACK(gtk_builder_get_object(builder, "fr2_stack"));
    GtkStack *fr2_add_stack = GTK_STACK(gtk_builder_get_object(builder, "fr2_add_stack"));
    GtkStack *fr2_stack_access = GTK_STACK(gtk_builder_get_object(builder, "fr2_stack_access"));
    GtkLabel *fr2_label_page = GTK_LABEL(gtk_builder_get_object(builder, "fr2_label_page"));
    GtkImage *fr2_index_image = GTK_IMAGE(gtk_builder_get_object(builder, "fr2_index_image"));
    GtkEntry *fr2_entry_playlist_name = GTK_ENTRY(gtk_builder_get_object(builder, "fr2_entry_playlist_name"));

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
    
    // ***************************************************************************************************
    // Frame 2 - Universal
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

    // =================================================================================================
    // Frame 2 - Playlists
    if (g_strcmp0(button_name, "fr2_btn_add_playlist") == 0) {
        button_set_click_animation(GTK_WIDGET(btn));
        gtk_stack_set_visible_child_name(fr2_add_stack, "add_playlist");
    }

    if (g_strcmp0(button_name, "fr2_btn_add_new_playlist") == 0) {
        gchar message[300], color[100];
        char entryText[50];
        button_set_click_animation(GTK_WIDGET(btn));
        GtkFixed *fixed = GTK_FIXED(gtk_builder_get_object(builder, "fr2_main"));
        GtkEntry *entry = GTK_ENTRY(gtk_builder_get_object(builder, "fr2_entry_playlist_name"));
        strcpy(entryText, gtk_entry_get_text(entry));
        g_snprintf(color, sizeof(color), "CB0000");
        if(gtk_entry_get_text_length(entry) == 0) 
            g_snprintf(message, sizeof(message), "Campo vazio, Preencha o nome para a playlist.");
        else {
            int status = createNewPlaylistFile(entryText, pPlaylistController);
            if(status == -3)
                g_snprintf(message, sizeof(message), "Não são permitidos caracteres especiais.");
            if(status == -2)
                g_snprintf(message, sizeof(message), "Ocorreu um erro na criação da playlist, tente novamente.");
            if(status == -1)
                g_snprintf(message, sizeof(message), "Já existe uma playlist com esse nome, tente outro.");
            if(status == 1) {
                g_snprintf(message, sizeof(message), "Nova playlist '%s' criada com sucesso!", entryText);
                setting_playlist_list(GINT_TO_POINTER(1));
                gtk_stack_set_visible_child_name(fr2_add_stack, "new_playlist");
                g_snprintf(color, sizeof(color), "1FD660");
            }
        }
        gtk_entry_set_text(entry, "");
        logStartAnimation(message, color, 1500, 21, 665, 163, 158, 20, fixed);
    }
    
    if (g_strcmp0(button_name, "fr2_btn_cancel_playlist") == 0) {
        button_set_click_animation(GTK_WIDGET(btn));
        gtk_stack_set_visible_child_name(fr2_add_stack, "new_playlist");
        gtk_entry_set_text(fr2_entry_playlist_name, "");
    }
    
    // ***************************************************************************************************
    // Playlist Access and Delete
    if (g_strcmp0(button_name, "fr2_btn_delete_playlist") == 0) {
        GtkFixed *fixed = GTK_FIXED(gtk_builder_get_object(builder, "fr2_main"));
        gchar message[300], color[100], nameFile[100];
        button_set_click_animation(GTK_WIDGET(btn));
        strcpy(nameFile, gtk_button_get_label(GTK_BUTTON(fr2_playlists_btns_vector[playlistIndex])));
        FILE *newPlaylistFile = acessPlaylistFile(nameFile);
        gint status = removePlaylistsController(nameFile, pPlaylistController, newPlaylistFile);

        g_snprintf(color, sizeof(color), "CB0000");
        
        if(status == -1)
            g_snprintf(message, sizeof(message), "Ocorreu um erro na remoção da playlist, tente novamente.");
        if(status == -2)
            g_snprintf(message, sizeof(message), "A playlist não existe.");
        if(status == 1) {
            g_snprintf(message, sizeof(message), "A playlist '%s' foi apagada com sucesso!", nameFile);
            g_snprintf(color, sizeof(color), "1FD660");
            setting_playlist_list(GINT_TO_POINTER(1));
        }
        logStartAnimation(message, color, 1500, 21, 665, 163, 158, 20, fixed);
    }
    
    if (g_strcmp0(button_name, "fr2_btn_access_playlist") == 0) {
        GtkFixed *fixed = GTK_FIXED(gtk_builder_get_object(builder, "fr2_main"));
        GtkLabel *label = GTK_LABEL(gtk_builder_get_object(builder, "fr2_playlist_view_name"));
        gchar message[300], color[100], nameFile[100];
        strcpy(nameFile, gtk_button_get_label(GTK_BUTTON(fr2_playlists_btns_vector[playlistIndex])));
        button_set_click_animation(GTK_WIDGET(btn));
        pActualPlaylistOpened = acessPlaylistFile(nameFile);
        
        if(pActualPlaylistOpened == NULL) {
            g_snprintf(color, sizeof(color), "CB0000");
            g_snprintf(message, sizeof(message), "A playlist não existe.");
            logStartAnimation(message, color, 1500, 21, 665, 163, 158, 20, fixed);
        }
        else {
            gtk_stack_set_transition_type(fr2_stack, GTK_STACK_TRANSITION_TYPE_CROSSFADE);
            setting_playlist_music_list(GINT_TO_POINTER(0));
            change_label_text(label, nameFile);
            gtk_stack_set_visible_child_name(fr2_stack, "page_access");
            gtk_stack_set_visible_child_name(fr2_stack_access, "page_access_list");
        }
    }

    if (g_strcmp0(button_name, "fr2_btn_access_back") == 0) {
        fclose(pActualPlaylistOpened);
        button_set_click_animation(GTK_WIDGET(btn));
        gtk_stack_set_visible_child_name(fr2_stack, "page_playlist");
    }

     if (g_strcmp0(button_name, "fr2_btn_add_music") == 0) {
        button_set_click_animation(GTK_WIDGET(btn));
        gtk_stack_set_visible_child_name(fr2_stack_access, "page_access_add");
    }

    if (g_strcmp0(button_name, "fr2_btn_access_add_back") == 0) {
        button_set_click_animation(GTK_WIDGET(btn));
        gtk_stack_set_visible_child_name(fr2_stack_access, "page_access_list");
    }

    if (g_strcmp0(button_name, "fr2_btn_access_music_remove") == 0) {
        GtkLabel *labelId = GTK_LABEL(gtk_builder_get_object(builder, "fr2_access_label_view_id"));
        gchar message[300], color[100];
        GtkFixed *fixed = GTK_FIXED(gtk_builder_get_object(builder, "fr2_main"));
        gint length = musicsLength(pActualPlaylistOpened);
        musica delMusic;
        musica *vector = readMusicsvector(pActualPlaylistOpened);
        for(int i=0; i < length; i++) {
            if(atoi(gtk_label_get_text(labelId)) == vector[i].id) {
                delMusic = vector[i];
            }
        }
        
        g_snprintf(color, sizeof(color), "CB0000");
        button_set_click_animation(GTK_WIDGET(btn));
        gint status = delNewMusicInPlaylist(delMusic, pActualPlaylistOpened);
        g_print("status: %d\n", status);

        if(status == 1) {
            g_snprintf(message, sizeof(message), "Música apagada com sucesso!");
            g_snprintf(color, sizeof(color), "1FD660");
            setting_playlist_music_list(GINT_TO_POINTER(0));
            gtk_stack_set_visible_child_name(fr2_stack_access, "page_access_list");
        }
        if(status == -2)
            g_snprintf(message, sizeof(message), "Ocorreu um erro no arquivo.");
        
        logStartAnimation(message, color, 1500, 21, 665, 163, 158, 20, fixed);
        g_free(vector);
    }

    if (g_strcmp0(button_name, "fr2_btn_access_list_back") == 0) {
        button_set_click_animation(GTK_WIDGET(btn));
        gtk_stack_set_visible_child_name(fr2_stack_access, "page_access_list");
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

    // Frame 2 - Playlists
    GObject * fr2_btn_add_playlist = gtk_builder_get_object(builder, "fr2_btn_add_playlist");
    g_signal_connect(fr2_btn_add_playlist, "clicked", G_CALLBACK(switchPage), NULL);

    GObject * fr2_btn_cancel_playlist = gtk_builder_get_object(builder, "fr2_btn_cancel_playlist");
    g_signal_connect(fr2_btn_cancel_playlist, "clicked", G_CALLBACK(switchPage), NULL);

    GObject * fr2_btn_add_new_playlist = gtk_builder_get_object(builder, "fr2_btn_add_new_playlist");
    g_signal_connect(fr2_btn_add_new_playlist, "clicked", G_CALLBACK(switchPage), NULL);

    GObject * fr2_btn_delete_playlist = gtk_builder_get_object(builder, "fr2_btn_delete_playlist");
    g_signal_connect(fr2_btn_delete_playlist, "clicked", G_CALLBACK(switchPage), NULL);

    // Frame 2 - Access
    GObject * fr2_btn_access_playlist = gtk_builder_get_object(builder, "fr2_btn_access_playlist");
    g_signal_connect(fr2_btn_access_playlist, "clicked", G_CALLBACK(switchPage), NULL);

    GObject * fr2_btn_access_back = gtk_builder_get_object(builder, "fr2_btn_access_back");
    g_signal_connect(fr2_btn_access_back, "clicked", G_CALLBACK(switchPage), NULL);
    
    GObject * fr2_btn_playlist_music_sort = gtk_builder_get_object(builder, "fr2_btn_playlist_music_sort");
    g_signal_connect(fr2_btn_playlist_music_sort, "clicked", G_CALLBACK(setting_playlist_music_list), GINT_TO_POINTER(1));
    
    GObject * fr2_btn_access_add_back = gtk_builder_get_object(builder, "fr2_btn_access_add_back");
    g_signal_connect(fr2_btn_access_add_back, "clicked", G_CALLBACK(switchPage), NULL);
    
    GObject * fr2_btn_add_music = gtk_builder_get_object(builder, "fr2_btn_add_music");
    g_signal_connect(fr2_btn_add_music, "clicked", G_CALLBACK(switchPage), NULL);

    
    GObject * fr2_btn_access_music_remove = gtk_builder_get_object(builder, "fr2_btn_access_music_remove");
    g_signal_connect(fr2_btn_access_music_remove, "clicked", G_CALLBACK(switchPage), NULL);

    GObject * fr2_btn_access_list_back = gtk_builder_get_object(builder, "fr2_btn_access_list_back");
    g_signal_connect(fr2_btn_access_list_back, "clicked", G_CALLBACK(switchPage), NULL);

}

/**********************************************************************************************
Settings 
*/
void setting_musics_list(gpointer data) {
    int setting = GPOINTER_TO_INT(data);

    GtkWidget *btn = GTK_WIDGET(gtk_builder_get_object(builder, "fr2_btn_musics_sort"));
    button_set_click_animation(btn);

    GtkLabel * fr2_music_actual_sort = GTK_LABEL(gtk_builder_get_object(builder, "fr2_music_actual_sort"));
    gint typeSort = 1;
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

void setting_playlist_list(gpointer data) {
    gint setting = GPOINTER_TO_INT(data);
    gint length = lengthPlaylistsController(pPlaylistController);
    GtkFixed * fixed = GTK_FIXED(gtk_builder_get_object(builder, "fr2_playlist_list_fixed"));
    gint actualLength = length;

    if(length < 4)
        length = 4;

    PlaylistData * playlistVector = readerPlaylistsController(pPlaylistController);
    gtk_widget_set_sensitive(GTK_WIDGET(fixed), FALSE);
    
    if(setting == 1) {
        clear_buttons_from_fixed(fixed);
        setting = 0;
    }

    for(int i=0; i < length; i++) {
        if(setting == 0) {
            GtkWidget *button = gtk_button_new_with_label("");
            fr2_playlists_btns_vector[i] = button;
            gtk_fixed_put(GTK_FIXED(fixed), button, 0, 62*i+52); 
            gtk_widget_set_size_request(button, 671, 62);     
            GtkStyleContext *btn_context = gtk_widget_get_style_context(button);
            gtk_style_context_add_class(btn_context, "universal_green_border");
            gtk_style_context_add_class(btn_context, "universal_font");
            gtk_style_context_add_class(btn_context, "color_FFFFFF");
            gtk_style_context_add_class(btn_context, "font_size_20");
            gtk_style_context_add_class(btn_context, "border_0_radius");
            gtk_style_context_add_class(btn_context, "fr2_btns_musics_list_border");
            g_object_set_data(G_OBJECT(button), "is_button", "true");
            if(i < actualLength) {
                set_button_text_with_limit(button, playlistVector[i].name);
                g_signal_connect(button, "clicked", G_CALLBACK(set_actual_playlist_in_list), GINT_TO_POINTER(i));
            }
            else
                set_button_text_with_limit(button, "---");
        }
    }
    gtk_widget_show_all(GTK_WIDGET(fixed));
    gtk_widget_set_sensitive(GTK_WIDGET(fixed), TRUE);

    set_actual_playlist_in_list(GTK_BUTTON(fr2_playlists_btns_vector[0]), GINT_TO_POINTER(0));
    g_free(playlistVector);
}

void setting_playlist_music_list(gpointer data) {
    gint setting = GPOINTER_TO_INT(data);
    gint length = musicsLength(pActualPlaylistOpened);
    GtkFixed * fixed = GTK_FIXED(gtk_builder_get_object(builder, "fr2_access_list_fixed"));
    gint actualLength = length;
    gint typeSort = 0;

    if(length < 4)
        length = 4;
    
    musica * playlistMusicsVector = readMusicsvector(pActualPlaylistOpened);
    GtkLabel *fr2_access_sort = GTK_LABEL(gtk_builder_get_object(builder, "fr2_access_sort"));
    const gchar *sortText = gtk_label_get_text(fr2_access_sort);
    
    if(setting == 0) {
        typeSort = 0;
        change_label_text(fr2_access_sort, "Inserção");
    }
    if(setting != 0) {
        button_set_click_animation(GTK_WIDGET(fr2_access_sort));
        if(strcmp(sortText, "Inserção") == 0) {
            typeSort = 1;
            change_label_text(fr2_access_sort, "ID");
        }
        else if(strcmp(sortText, "ID") == 0) {
            typeSort = 2;
            change_label_text(fr2_access_sort, "Duração");
        }
        else if(strcmp(sortText, "Duração") == 0) {
            typeSort = 3;
            change_label_text(fr2_access_sort, "Alfabética");
        }
        else if(strcmp(sortText, "Alfabética") == 0) {
            typeSort = 4;
            change_label_text(fr2_access_sort, "Artista");
        }
        else if(strcmp(sortText, "Artista") == 0) {
            typeSort = 5;
            change_label_text(fr2_access_sort, "Álbum");
        }
        else if(strcmp(sortText, "Álbum") == 0) {
            typeSort = 0;
            change_label_text(fr2_access_sort, "Inserção");
        }
    }

    bubbleTypeSort(playlistMusicsVector, typeSort, actualLength);
    gtk_widget_set_sensitive(GTK_WIDGET(fixed), FALSE);
    clear_buttons_from_fixed(fixed);

    for(int i=0; i < length; i++) {
        GtkWidget *label = gtk_label_new("");
        GtkStyleContext *label_context = gtk_widget_get_style_context(label);

        gtk_fixed_put(GTK_FIXED(fixed), label, 0, 121+i*60);
        gtk_widget_set_size_request(label, 390, 60);  

        gtk_style_context_add_class(label_context, "universal_green_border");
        gtk_style_context_add_class(label_context, "universal_font");
        gtk_style_context_add_class(label_context, "color_FFFFFF");
        gtk_style_context_add_class(label_context, "font_size_20");
        gtk_style_context_add_class(label_context, "border_0_radius");
        gtk_style_context_add_class(label_context, "fr2_btns_musics_list_border");
        g_object_set_data(G_OBJECT(label), "is_button", "true");

        if(i < actualLength) {
            GtkWidget *button = gtk_button_new_with_label("");
            gtk_fixed_put(GTK_FIXED(fixed), button, 344, 136+i*61);
            gtk_widget_set_size_request(button, 29, 29);
            set_button_text_with_limit(label, playlistMusicsVector[i].nome);

            GtkStyleContext *btn_context = gtk_widget_get_style_context(button);
            gtk_style_context_add_class(btn_context, "universal_imgs");
            gtk_style_context_add_class(btn_context, "fr2_btn_buttons");
            g_object_set_data(G_OBJECT(button), "is_button", "true");
            g_signal_connect(button, "clicked", G_CALLBACK(set_actual_music_in_playlist), GINT_TO_POINTER(i));
        }
        else
            set_button_text_with_limit(label, "---");
        
    }
    gtk_widget_show_all(GTK_WIDGET(fixed));
    gtk_widget_set_sensitive(GTK_WIDGET(fixed), TRUE);
    g_free(playlistMusicsVector);
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
void set_actual_music_in_playlist(GtkButton *btn, gpointer user_data) {
    gint index = GPOINTER_TO_INT(user_data);
    gchar sprintText[200];
    gint minutes, seconds;

    GtkStack *fr2_stack_access = GTK_STACK(gtk_builder_get_object(builder, "fr2_stack_access"));
    GtkLabel *fr2_access_label_view_name = GTK_LABEL(gtk_builder_get_object(builder, "fr2_access_label_view_name"));
    GtkLabel *fr2_access_label_view_artist = GTK_LABEL(gtk_builder_get_object(builder, "fr2_access_label_view_artist"));
    GtkLabel *fr2_access_label_view_album = GTK_LABEL(gtk_builder_get_object(builder, "fr2_access_label_view_album"));
    GtkLabel *fr2_access_label_view_minutes = GTK_LABEL(gtk_builder_get_object(builder, "fr2_access_label_view_minutes"));
    GtkLabel *fr2_access_label_view_id = GTK_LABEL(gtk_builder_get_object(builder, "fr2_access_label_view_id"));
    
    if(btn != NULL)
        button_set_click_animation(GTK_WIDGET(btn));

    gtk_stack_set_visible_child_name(fr2_stack_access, "page_access_view");
    
    musica *vector = readMusicsvector(pActualPlaylistOpened);
    minutes = vector[index].tempo / 60;
    seconds = vector[index].tempo - minutes*60;
    sprintf(sprintText, "%d:%d minutos", minutes, seconds);

    if(seconds < 10)
        sprintf(sprintText, "%d:0%d minutos", minutes, seconds);
    
    change_label_text(fr2_access_label_view_name, vector[index].nome);
    change_label_text(fr2_access_label_view_artist, vector[index].artista);
    change_label_text(fr2_access_label_view_album, vector[index].album);
    change_label_text(fr2_access_label_view_minutes, sprintText);
    
    sprintf(sprintText, "%d", vector[index].id);
    change_label_text(fr2_access_label_view_id, sprintText);

    g_free(vector);
}

void set_actual_music_in_list(GtkButton *btn, gpointer user_data) {
    int index = GPOINTER_TO_INT(user_data);
    gchar sprintText[200];
    gint minutes, seconds;

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

void set_actual_playlist_in_list(GtkButton *btn, gpointer user_data) {
    int index = GPOINTER_TO_INT(user_data);
    playlistIndex = index;
    gchar sprintText[200];
    GtkLabel *selected = GTK_LABEL(gtk_builder_get_object(builder, "fr2_selected_playlist"));
    if(btn != NULL)
        button_set_click_animation(GTK_WIDGET(btn));
    
    g_snprintf(sprintText, sizeof(sprintText), "Playlist selecionada: %s", gtk_button_get_label(btn));
    change_label_text(selected, sprintText);
    GtkFixed *fixed = GTK_FIXED(gtk_builder_get_object(builder, "fr2_playlist_list_fixed"));
    GtkWidget *fr2_btn_playlist_marker = GTK_WIDGET(gtk_builder_get_object(builder, "fr2_btn_playlist_marker"));
    gtk_fixed_move(GTK_FIXED(fixed), fr2_btn_playlist_marker, 8, 62*index+60);
    
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

    if(GTK_IS_BUTTON(button)) {
        gtk_button_set_label(GTK_BUTTON(button), truncated_text);
        GtkWidget *label = gtk_bin_get_child(GTK_BIN(button));
        gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
    }
    else if(GTK_IS_LABEL(button)) {
        change_label_text(GTK_LABEL(button), truncated_text);
        gtk_label_set_ellipsize(GTK_LABEL(button), PANGO_ELLIPSIZE_END);
    }
}

void clear_buttons_from_fixed(GtkFixed *fixed) {
    GList *children = gtk_container_get_children(GTK_CONTAINER(fixed));
    for (GList *iter = children; iter != NULL; iter = iter->next) {
        GtkWidget *child = GTK_WIDGET(iter->data);

        const char *tag = g_object_get_data(G_OBJECT(child), "is_button");
        if (tag && g_strcmp0(tag, "true") == 0) {
            gtk_widget_destroy(child); 
        }
    }
    g_list_free(children);
}

// --------------------------------------------------------------------------------------------------
// Animations
void logStartAnimation(gchar *text, gchar *color, gint duration, gint height, gint width, gint x, gint y, gint yDirection, GtkFixed *fixed) {
    gchar colorText[100];
    GtkWidget *label = gtk_label_new(text);
    GtkStyleContext *label_context = gtk_widget_get_style_context(label);
    g_snprintf(colorText, sizeof(colorText), "color_%s", color);
    
    gtk_fixed_put(GTK_FIXED(fixed), label, x, y); 
    gtk_widget_set_size_request(label, width, height);     
    gtk_style_context_add_class(label_context, "universal_no_border");
    gtk_style_context_add_class(label_context, "universal_font");
    gtk_style_context_add_class(label_context, colorText);
    gtk_style_context_add_class(label_context, "font_size_16");
    gtk_style_context_add_class(label_context, "universal_no_background");
    gtk_widget_show(label);
    
    GtkUpAnimationData *data = g_malloc(sizeof(GtkUpAnimationData) * 1);
    data->interactions = yDirection;
    data->container = fixed;
    data->widget = label;
    data->x = x;
    data->y = y;
    data->opacity = 1.0;
    data->opacityDecrease = 1.0 / (gfloat) yDirection;
    g_timeout_add(duration / yDirection , logAnimation, data);
}

gboolean logAnimation(gpointer data) {
    GtkUpAnimationData *animData = (GtkUpAnimationData*) data;
    if(animData->interactions > 0) {
        animData->y -= 1;
        animData->opacity -= animData->opacityDecrease;
        gtk_fixed_move(animData->container, GTK_WIDGET(animData->widget), animData->x, animData->y);
        gtk_widget_set_opacity(animData->widget, animData->opacity);
        animData->interactions -= 1;
        return TRUE;
    }

    gtk_widget_destroy(animData->widget);
    g_free(animData);
    return FALSE;
}
/*=============================================================================================*/