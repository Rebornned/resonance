#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib.h>
#include "library.h"

// Text shown in the sort labels, indexed by SortMode
static const gchar *SORT_LABELS[] = {
    [SORT_INSERTION] = "Inserção",
    [SORT_ID]        = "ID",
    [SORT_DURATION]  = "Duração",
    [SORT_TITLE]     = "Alfabética",
    [SORT_ARTIST]    = "Artista",
    [SORT_ALBUM]     = "Álbum",
};

/*
 * Resonance - graphical interface (GTK 3).
 * Build: see the Makefile in the project root (make / make run).
 * Paths are relative to bin/, so the program must run from that folder.
 */

// ================================================================================================
// Types
// A message that floats up and fades out (see show_toast)
typedef struct {
    GtkWidget *widget;
    GtkFixed *container;
    gfloat opacity;
    gfloat opacity_step;
    gint x;
    gint y;
    gint steps_left;
} ToastAnimation;


// +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=++=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=++=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
// Interface functions

// Setup and list filling
void register_signals(GtkBuilder *builder);
void on_button_clicked(GtkButton *btn, gpointer user_data);
void fill_song_list(gpointer data);
void fill_playlist_list(gpointer data);
void fill_playlist_songs(gpointer data);
void fill_song_picker(gpointer data);

// -----------------------------------------------------------------

// Editing
void add_selected_song_to_playlist(GtkButton *btn, gpointer user_data);
void change_label_text(GtkLabel *label, const gchar *text);
void show_playlist_song(GtkButton *btn, gpointer user_data);
void set_truncated_text(GtkWidget *button, const char *text);
void show_song(GtkButton *btn, gpointer user_data);
void select_playlist(GtkButton *btn, gpointer user_data);
void clear_dynamic_widgets(GtkFixed *fixed);

// -------------------------------------------------------------------
// Animations 
void show_toast(gchar *text, gchar *color, gint duration, gint height, gint width, gint x, gint y, gint steps, GtkFixed *fixed);
gboolean toast_step(gpointer data);

void button_set_hide_animation(GtkWidget *button, gint timer);
gboolean button_hide_animation(gpointer data);

void button_set_click_animation(GtkWidget *button); 
gboolean button_click_animation(gpointer data);

void turn_off_button_start(GtkWidget *widget);
gboolean turn_off_button(gpointer data);

static void set_custom_cursor(GtkWidget *widget, gpointer data);
static void on_song_sort_clicked(GtkButton *btn, gpointer data);
static void on_playlist_sort_clicked(GtkButton *btn, gpointer data);
// =======================================================================================================

// -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*--*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Global state

// Main window
GtkWidget *window; 
GtkBuilder *builder;
GtkStack *main_stack;

// Universal
FILE * songs_file;
Song * all_songs;

// Frame 2 - Songs
int fr2_main_stack_index;
GtkWidget **song_buttons;

// Frame 2 - Playlists
FILE * registry_file;
GtkWidget **playlist_buttons;
gint selected_playlist_index;

// Frame 2 - Access
FILE * open_playlist_file;
// =========================================================================================================

int main (int argc, char *argv[]) {
    // Window setup
    // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
    gtk_init(&argc, &argv);

    GtkCssProvider *css_provider;
    // Load the interface described in the .glade file
    builder = gtk_builder_new_from_file("../assets/ui_files/playlists.glade");
    gtk_builder_connect_signals(builder, NULL);

    // Main window
    window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    gtk_window_set_icon_from_file(GTK_WINDOW(window), "../assets/ui_images/playlists_ico.png", NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL); // Closing the window ends the program
    
    GdkGeometry size_hints;
    size_hints.max_width = 1000;  
    size_hints.max_height = 600; 
    gtk_window_set_geometry_hints(GTK_WINDOW(window), NULL, &size_hints, GDK_HINT_MAX_SIZE);

    // Load and apply the CSS file
    css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(css_provider, "../assets/css/style.css", NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(css_provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Main stack (home screen / songs and playlists screen)
    main_stack = GTK_STACK(gtk_builder_get_object(builder, "main_stack"));

    // ================================================================================================
    // Frame 2 - Songs
    fr2_main_stack_index = 0;
    songs_file = fopen("../files/musics_database.bin", "rb+");
    if(songs_file == NULL) {
        g_printerr("Could not open ../files/musics_database.bin (run the program from the bin/ folder).\n");
        return 1;
    }
    all_songs = read_songs(songs_file);

    // Buttons created at run time for each list item
    song_buttons =  malloc(sizeof(GtkWidget *) * song_count(songs_file));
    
    // First fill of the song list (sorted by ID)
    fill_song_list(GINT_TO_POINTER(0));

    // ======================================================================================
    // Frame 2 - Playlists
    registry_file = registry_open();
    if(registry_file == NULL) {
        g_printerr("Could not open the playlist registry in ../files/playlists/.\n");
        return 1;
    }

    // Buttons created at run time for each list item
    gint initial_slots = registry_count(registry_file);
    if(initial_slots < 4)
        initial_slots = 4;

    playlist_buttons = malloc(sizeof(GtkWidget *) * initial_slots);
    fill_playlist_list(GINT_TO_POINTER(0));
   
    // ************************************************************************************************

    // Connect button clicks to their handlers
    register_signals(builder);
    g_signal_connect(window, "map", G_CALLBACK(set_custom_cursor), NULL);

    gtk_widget_show_all(window);

    gtk_main();

    fclose(songs_file);
    fclose(registry_file);
    if(open_playlist_file != NULL)
        fclose(open_playlist_file);
    return 0;
}
/*
================================================================================================
Button handling */

void on_button_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    GtkStack *fr2_stack = GTK_STACK(gtk_builder_get_object(builder, "fr2_stack"));
    GtkStack *fr2_add_stack = GTK_STACK(gtk_builder_get_object(builder, "fr2_add_stack"));
    GtkStack *fr2_stack_access = GTK_STACK(gtk_builder_get_object(builder, "fr2_stack_access"));
    GtkLabel *fr2_label_page = GTK_LABEL(gtk_builder_get_object(builder, "fr2_label_page"));
    GtkImage *fr2_index_image = GTK_IMAGE(gtk_builder_get_object(builder, "fr2_index_image"));
    GtkEntry *fr2_entry_playlist_name = GTK_ENTRY(gtk_builder_get_object(builder, "fr2_entry_playlist_name"));

    const gchar *button_name = gtk_widget_get_name(GTK_WIDGET(btn));

    // Every button is identified by its widget name (set in the .glade file)
    // Frame 1
    if (g_strcmp0(button_name, "fr1_btn_music") == 0) {
        button_set_click_animation(GTK_WIDGET(btn));
        gtk_stack_set_visible_child_name(main_stack, "page_main");
    }

    if (g_strcmp0(button_name, "fr1_btn_playlist") == 0) {
        gtk_stack_set_transition_duration(fr2_stack, 0);
        gtk_stack_set_visible_child_name(fr2_stack, "page_playlist");
        gtk_stack_set_transition_duration(fr2_stack, 200);
        gtk_stack_set_visible_child_name(main_stack, "page_main");
        gtk_image_set_from_file(fr2_index_image, "../assets/ui_images/selector_page_2.png");
        change_label_text(fr2_label_page, "Playlists");
        fr2_main_stack_index++;
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
        gchar entry_text[100];
        button_set_click_animation(GTK_WIDGET(btn));
        GtkFixed *fixed = GTK_FIXED(gtk_builder_get_object(builder, "fr2_main"));
        GtkEntry *entry = GTK_ENTRY(gtk_builder_get_object(builder, "fr2_entry_playlist_name"));
        g_strlcpy(entry_text, gtk_entry_get_text(entry), sizeof(entry_text));
        g_snprintf(color, sizeof(color), "CB0000");
        if(gtk_entry_get_text_length(entry) == 0) 
            g_snprintf(message, sizeof(message), "Campo vazio, Preencha o nome para a playlist.");
        else {
            int status = playlist_create(entry_text, registry_file);
            if(status == -3)
                g_snprintf(message, sizeof(message), "Use apenas letras, números e espaços.");
            if(status == -2)
                g_snprintf(message, sizeof(message), "Ocorreu um erro na criação da playlist, tente novamente.");
            if(status == -1)
                g_snprintf(message, sizeof(message), "Já existe uma playlist com esse nome, tente outro.");
            if(status == 1) {
                g_snprintf(message, sizeof(message), "Nova playlist '%s' criada com sucesso!", entry_text);
                fill_playlist_list(GINT_TO_POINTER(1));
                gtk_stack_set_visible_child_name(fr2_add_stack, "new_playlist");
                g_snprintf(color, sizeof(color), "1FD660");
            }
        }
        gtk_entry_set_text(entry, "");
        show_toast(message, color, 1500, 21, 665, 163, 158, 20, fixed);
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
        gchar message[300], color[100];
        Playlist selected;
        gint status = -2;
        button_set_click_animation(GTK_WIDGET(btn));
        if(registry_get(registry_file, selected_playlist_index, &selected)) {
            // Close the last opened playlist first: Windows cannot delete an open file
            if(open_playlist_file != NULL) {
                fclose(open_playlist_file);
                open_playlist_file = NULL;
            }
            status = playlist_delete(selected.id, registry_file);
        }

        g_snprintf(color, sizeof(color), "CB0000");
        
        if(status == -1)
            g_snprintf(message, sizeof(message), "Ocorreu um erro na remoção da playlist, tente novamente.");
        if(status == -2)
            g_snprintf(message, sizeof(message), "A playlist não existe.");
        if(status == 1) {
            g_snprintf(message, sizeof(message), "A playlist '%s' foi apagada com sucesso!", selected.name);
            g_snprintf(color, sizeof(color), "1FD660");
            fill_playlist_list(GINT_TO_POINTER(1));
        }
        show_toast(message, color, 1500, 21, 665, 163, 158, 20, fixed);
    }
    
    if (g_strcmp0(button_name, "fr2_btn_access_playlist") == 0) {
        GtkFixed *fixed = GTK_FIXED(gtk_builder_get_object(builder, "fr2_main"));
        GtkLabel *label = GTK_LABEL(gtk_builder_get_object(builder, "fr2_playlist_view_name"));
        gchar message[300], color[100];
        Playlist selected;
        button_set_click_animation(GTK_WIDGET(btn));
        if(open_playlist_file != NULL) {
            fclose(open_playlist_file);
            open_playlist_file = NULL;
        }
        if(registry_get(registry_file, selected_playlist_index, &selected))
            open_playlist_file = playlist_open(selected.id);
        
        if(open_playlist_file == NULL) {
            g_snprintf(color, sizeof(color), "CB0000");
            g_snprintf(message, sizeof(message), "A playlist não existe.");
            show_toast(message, color, 1500, 21, 665, 163, 158, 20, fixed);
        }
        else {
            gtk_stack_set_transition_type(fr2_stack, GTK_STACK_TRANSITION_TYPE_CROSSFADE);
            fill_playlist_songs(GINT_TO_POINTER(0));
            change_label_text(label, selected.name);
            gtk_stack_set_visible_child_name(fr2_stack, "page_access");
            gtk_stack_set_visible_child_name(fr2_stack_access, "page_access_list");
        }
    }

    if (g_strcmp0(button_name, "fr2_btn_access_back") == 0) {
        if(open_playlist_file != NULL) {
            fclose(open_playlist_file);
            open_playlist_file = NULL;
        }
        button_set_click_animation(GTK_WIDGET(btn));
        gtk_stack_set_visible_child_name(fr2_stack, "page_playlist");
    }

     if (g_strcmp0(button_name, "fr2_btn_add_music") == 0) {
        button_set_click_animation(GTK_WIDGET(btn));
        gtk_stack_set_visible_child_name(fr2_stack_access, "page_access_add");
        fill_song_picker(NULL);
    }

    if (g_strcmp0(button_name, "fr2_btn_access_add_back") == 0) {
        button_set_click_animation(GTK_WIDGET(btn));
        gtk_stack_set_visible_child_name(fr2_stack_access, "page_access_list");
    }

    if (g_strcmp0(button_name, "fr2_btn_access_music_remove") == 0) {
        GtkLabel *id_label = GTK_LABEL(gtk_builder_get_object(builder, "fr2_access_label_view_id"));
        gchar message[300], color[100];
        GtkFixed *fixed = GTK_FIXED(gtk_builder_get_object(builder, "fr2_main"));
        gint length = song_count(open_playlist_file);
        Song song;
        gboolean found = FALSE;
        Song *vector = read_songs(open_playlist_file);
        for(int i=0; i < length; i++) {
            if(atoi(gtk_label_get_text(id_label)) == vector[i].id) {
                song = vector[i];
                found = TRUE;
            }
        }
        
        g_snprintf(color, sizeof(color), "CB0000");
        g_snprintf(message, sizeof(message), "Ocorreu um erro no arquivo.");
        button_set_click_animation(GTK_WIDGET(btn));
        gint status = found ? playlist_remove_song(song, open_playlist_file) : -3;

        if(status == 1) {
            g_snprintf(message, sizeof(message), "Música apagada com sucesso!");
            g_snprintf(color, sizeof(color), "1FD660");
            fill_playlist_songs(GINT_TO_POINTER(0));
            gtk_stack_set_visible_child_name(fr2_stack_access, "page_access_list");
        }
        if(status == -3)
            g_snprintf(message, sizeof(message), "A música não está na playlist.");
        
        show_toast(message, color, 1500, 21, 665, 163, 158, 20, fixed);
        g_free(vector);
    }

    if (g_strcmp0(button_name, "fr2_btn_access_list_back") == 0) {
        button_set_click_animation(GTK_WIDGET(btn));
        gtk_stack_set_visible_child_name(fr2_stack_access, "page_access_list");
    }

}

void register_signals(GtkBuilder *builder) {
    // Frame 1 buttons
    GObject *fr1_btn_music = gtk_builder_get_object(builder, "fr1_btn_music");
    g_signal_connect(fr1_btn_music, "clicked", G_CALLBACK(on_button_clicked), main_stack);

    GObject *fr1_btn_playlist = gtk_builder_get_object(builder, "fr1_btn_playlist");
    g_signal_connect(fr1_btn_playlist, "clicked", G_CALLBACK(on_button_clicked), main_stack);

    // Frame 2 buttons
    GObject * fr2_btn_musics_sort = gtk_builder_get_object(builder, "fr2_btn_musics_sort");
    g_signal_connect(fr2_btn_musics_sort, "clicked", G_CALLBACK(on_song_sort_clicked), NULL);

    GObject * fr2_arrow_return = gtk_builder_get_object(builder, "fr2_arrow_return");
    g_signal_connect(fr2_arrow_return, "clicked", G_CALLBACK(on_button_clicked), NULL);

    GObject * fr2_arrow_next = gtk_builder_get_object(builder, "fr2_arrow_next");
    g_signal_connect(fr2_arrow_next, "clicked", G_CALLBACK(on_button_clicked), NULL);

    // Frame 2 - Playlists
    GObject * fr2_btn_add_playlist = gtk_builder_get_object(builder, "fr2_btn_add_playlist");
    g_signal_connect(fr2_btn_add_playlist, "clicked", G_CALLBACK(on_button_clicked), NULL);

    GObject * fr2_btn_cancel_playlist = gtk_builder_get_object(builder, "fr2_btn_cancel_playlist");
    g_signal_connect(fr2_btn_cancel_playlist, "clicked", G_CALLBACK(on_button_clicked), NULL);

    GObject * fr2_btn_add_new_playlist = gtk_builder_get_object(builder, "fr2_btn_add_new_playlist");
    g_signal_connect(fr2_btn_add_new_playlist, "clicked", G_CALLBACK(on_button_clicked), NULL);

    GObject * fr2_btn_delete_playlist = gtk_builder_get_object(builder, "fr2_btn_delete_playlist");
    g_signal_connect(fr2_btn_delete_playlist, "clicked", G_CALLBACK(on_button_clicked), NULL);

    // Frame 2 - Access
    GObject * fr2_btn_access_playlist = gtk_builder_get_object(builder, "fr2_btn_access_playlist");
    g_signal_connect(fr2_btn_access_playlist, "clicked", G_CALLBACK(on_button_clicked), NULL);

    GObject * fr2_btn_access_back = gtk_builder_get_object(builder, "fr2_btn_access_back");
    g_signal_connect(fr2_btn_access_back, "clicked", G_CALLBACK(on_button_clicked), NULL);
    
    GObject * fr2_btn_playlist_music_sort = gtk_builder_get_object(builder, "fr2_btn_playlist_music_sort");
    g_signal_connect(fr2_btn_playlist_music_sort, "clicked", G_CALLBACK(on_playlist_sort_clicked), NULL);
    
    GObject * fr2_btn_access_add_back = gtk_builder_get_object(builder, "fr2_btn_access_add_back");
    g_signal_connect(fr2_btn_access_add_back, "clicked", G_CALLBACK(on_button_clicked), NULL);
    
    GObject * fr2_btn_add_music = gtk_builder_get_object(builder, "fr2_btn_add_music");
    g_signal_connect(fr2_btn_add_music, "clicked", G_CALLBACK(on_button_clicked), NULL);

    
    GObject * fr2_btn_access_music_remove = gtk_builder_get_object(builder, "fr2_btn_access_music_remove");
    g_signal_connect(fr2_btn_access_music_remove, "clicked", G_CALLBACK(on_button_clicked), NULL);

    GObject * fr2_btn_access_list_back = gtk_builder_get_object(builder, "fr2_btn_access_list_back");
    g_signal_connect(fr2_btn_access_list_back, "clicked", G_CALLBACK(on_button_clicked), NULL);
}

/* The sort buttons refresh the lists with the next criterion. These wrappers
   give the "clicked" signal a handler with the right signature. */
static void on_song_sort_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    fill_song_list(GINT_TO_POINTER(1));
}

static void on_playlist_sort_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    fill_playlist_songs(GINT_TO_POINTER(1));
}

/**********************************************************************************************
List filling: setting 0 creates the widgets, any other value refreshes them
*/
void fill_song_list(gpointer data) {
    int setting = GPOINTER_TO_INT(data);

    GtkWidget *btn = GTK_WIDGET(gtk_builder_get_object(builder, "fr2_btn_musics_sort"));
    button_set_click_animation(btn);
    turn_off_button_start(GTK_WIDGET(btn));

    GtkLabel * fr2_music_current_sort = GTK_LABEL(gtk_builder_get_object(builder, "fr2_music_current_sort"));

    // Each click moves to the next criterion: ID, duration, title, artist, album, then ID again.
    // Starts at SORT_ALBUM to match the initial label in the .glade file, so the first call sorts by ID.
    static SortMode song_list_sort = SORT_ALBUM;
    song_list_sort = (song_list_sort == SORT_ALBUM) ? SORT_ID : song_list_sort + 1;
    change_label_text(fr2_music_current_sort, SORT_LABELS[song_list_sort]);

    int length = song_count(songs_file);
    GtkFixed *fixed = GTK_FIXED(gtk_builder_get_object(builder, "fr2_musics_list_fixed"));
    
    sort_songs(all_songs, length, song_list_sort);
    show_song(NULL, GINT_TO_POINTER(0));

    for(int i=0; i < length; i++) {
        if(setting == 0) {
            GtkWidget *button = gtk_button_new_with_label("");
            song_buttons[i] = button;
            set_truncated_text(button, all_songs[i].title);
            gtk_fixed_put(GTK_FIXED(fixed), button, 0, 63*i); 
            gtk_widget_set_size_request(button, 239, 63);     
            GtkStyleContext *btn_context = gtk_widget_get_style_context(button);
            gtk_style_context_add_class(btn_context, "universal_green_border");
            gtk_style_context_add_class(btn_context, "universal_font");
            gtk_style_context_add_class(btn_context, "color_FFFFFF");
            gtk_style_context_add_class(btn_context, "font_size_20");
            gtk_style_context_add_class(btn_context, "border_0_radius");
            gtk_style_context_add_class(btn_context, "fr2_btns_musics_list_border");

            g_signal_connect(button, "clicked", G_CALLBACK(show_song), GINT_TO_POINTER(i));
        }
        else {
            set_truncated_text(song_buttons[i], all_songs[i].title);
        }
    }
}

void fill_song_picker(gpointer data) {
    (void)data;
    gint length = song_count(songs_file);
    GtkFixed * fixed = GTK_FIXED(gtk_builder_get_object(builder, "fr2_access_add_fixed"));
    gint item_count = length;

    if(length < 4)
        length = 4;
    
    Song * songs = read_songs(songs_file);

    gtk_widget_set_sensitive(GTK_WIDGET(fixed), FALSE);
    clear_dynamic_widgets(fixed);

    for(int i=0; i < length; i++) {
        GtkWidget *label = gtk_label_new("");
        GtkStyleContext *label_context = gtk_widget_get_style_context(label);

        gtk_fixed_put(GTK_FIXED(fixed), label, 0, 67+i*61);
        gtk_widget_set_size_request(label, 390, 61);  

        gtk_style_context_add_class(label_context, "universal_green_border");
        gtk_style_context_add_class(label_context, "universal_font");
        gtk_style_context_add_class(label_context, "color_FFFFFF");
        gtk_style_context_add_class(label_context, "font_size_20");
        gtk_style_context_add_class(label_context, "border_0_radius");
        gtk_style_context_add_class(label_context, "fr2_btns_musics_list_border");
        g_object_set_data(G_OBJECT(label), "is_button", "true");
        
        if(i < item_count) {
            GtkWidget *button = gtk_button_new_with_label("");
            gtk_fixed_put(GTK_FIXED(fixed), button, 336, 83+i*61);
            gtk_widget_set_size_request(button, 38, 38);
            set_truncated_text(label, songs[i].title);
            button_set_hide_animation(button, 200);
            GtkStyleContext *btn_context = gtk_widget_get_style_context(button);
            gtk_style_context_add_class(btn_context, "universal_imgs");
            gtk_style_context_add_class(btn_context, "fr2_playlist_add");
            g_object_set_data(G_OBJECT(button), "is_button", "true");
            g_signal_connect(button, "clicked", G_CALLBACK(add_selected_song_to_playlist), GINT_TO_POINTER(i));
        }
        else
            set_truncated_text(label, "---");
        
    }
    gtk_widget_show_all(GTK_WIDGET(fixed));
    gtk_widget_set_sensitive(GTK_WIDGET(fixed), TRUE);
    g_free(songs);
}

void fill_playlist_list(gpointer data) {
    gint setting = GPOINTER_TO_INT(data);
    gint length = registry_count(registry_file);
    GtkFixed * fixed = GTK_FIXED(gtk_builder_get_object(builder, "fr2_playlist_list_fixed"));
    gint item_count = length;

    if(length < 4)
        length = 4;

    Playlist * playlists = registry_read_all(registry_file);

    // One button per playlist: grow the array when playlists are added
    GtkWidget **grown = realloc(playlist_buttons, sizeof(GtkWidget *) * length);
    if(grown == NULL) {
        g_free(playlists);
        return;
    }
    playlist_buttons = grown;
    gtk_widget_set_sensitive(GTK_WIDGET(fixed), FALSE);
    
    if(setting == 1) {
        clear_dynamic_widgets(fixed);
        setting = 0;
    }

    for(int i=0; i < length; i++) {
        if(setting == 0) {
            GtkWidget *button = gtk_button_new_with_label("");
            playlist_buttons[i] = button;
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
            if(i < item_count) {
                set_truncated_text(button, playlists[i].name);
                g_signal_connect(button, "clicked", G_CALLBACK(select_playlist), GINT_TO_POINTER(i));
            }
            else
                set_truncated_text(button, "---");
        }
    }
    gtk_widget_show_all(GTK_WIDGET(fixed));
    gtk_widget_set_sensitive(GTK_WIDGET(fixed), TRUE);

    select_playlist(GTK_BUTTON(playlist_buttons[0]), GINT_TO_POINTER(0));
    g_free(playlists);
}

void fill_playlist_songs(gpointer data) {
    gint setting = GPOINTER_TO_INT(data);
    gint length = song_count(open_playlist_file);
    GtkFixed * fixed = GTK_FIXED(gtk_builder_get_object(builder, "fr2_access_list_fixed"));
    gint item_count = length;
    GtkWidget *sort_button = GTK_WIDGET(gtk_builder_get_object(builder, "fr2_btn_playlist_music_sort"));
    turn_off_button_start(sort_button);
    button_set_click_animation(sort_button);

    if(length < 4)
        length = 4;
    
    Song * playlist_songs = read_songs(open_playlist_file);
    GtkLabel *fr2_access_sort = GTK_LABEL(gtk_builder_get_object(builder, "fr2_access_sort"));

    // setting 0 (playlist opened or changed) resets to insertion order;
    // a click on the sort button moves to the next criterion.
    static SortMode playlist_sort = SORT_INSERTION;
    if(setting == 0)
        playlist_sort = SORT_INSERTION;
    else {
        button_set_click_animation(GTK_WIDGET(fr2_access_sort));
        playlist_sort = (playlist_sort == SORT_ALBUM) ? SORT_INSERTION : playlist_sort + 1;
    }
    change_label_text(fr2_access_sort, SORT_LABELS[playlist_sort]);

    sort_songs(playlist_songs, item_count, playlist_sort);
    gtk_widget_set_sensitive(GTK_WIDGET(fixed), FALSE);
    clear_dynamic_widgets(fixed);

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

        if(i < item_count) {
            GtkWidget *button = gtk_button_new_with_label("");
            gtk_fixed_put(GTK_FIXED(fixed), button, 344, 136+i*61);
            gtk_widget_set_size_request(button, 29, 29);
            set_truncated_text(label, playlist_songs[i].title);
            button_set_hide_animation(button, 200);
            GtkStyleContext *btn_context = gtk_widget_get_style_context(button);
            gtk_style_context_add_class(btn_context, "universal_imgs");
            gtk_style_context_add_class(btn_context, "fr2_btn_buttons");
            g_object_set_data(G_OBJECT(button), "is_button", "true");

            g_signal_connect(button, "clicked", G_CALLBACK(show_playlist_song), GINT_TO_POINTER(playlist_songs[i].id));
        }
        else
            set_truncated_text(label, "---");
        
    }
    gtk_widget_show_all(GTK_WIDGET(fixed));
    gtk_widget_set_sensitive(GTK_WIDGET(fixed), TRUE);
    g_free(playlist_songs);
}

/*
================================================================================================
Animations */

void button_set_click_animation(GtkWidget *button) {
    if(button != NULL) {
        gtk_widget_set_opacity(button, 0.7);
        g_timeout_add(100, button_click_animation, button);
    }
}

gboolean button_click_animation(gpointer data) {
    GtkWidget *button = GTK_WIDGET(data);
    gtk_widget_set_opacity(button, 1.0);
    return FALSE;
}

void button_set_hide_animation(GtkWidget *button, gint timer) {
    if(button != NULL) {
        gtk_widget_set_opacity(button, 0.0);
        g_timeout_add(timer, button_click_animation, button);
    }
}

gboolean button_hide_animation(gpointer data) {
    GtkWidget *button = GTK_WIDGET(data);
    gtk_widget_set_opacity(button, 1.0);
    return FALSE;
}

void turn_off_button_start(GtkWidget *widget) {
    if(widget != NULL) {
        gtk_widget_set_sensitive(widget, FALSE);
        g_timeout_add(500, turn_off_button, widget);
    }
}

gboolean turn_off_button(gpointer data) {
    GtkWidget *widget = (GtkWidget *) data;
    gtk_widget_set_sensitive(widget, TRUE);
    return FALSE;
}
/***********************************************************************************************/
/* Editing */
void add_selected_song_to_playlist(GtkButton *btn, gpointer user_data) {
    gint index = GPOINTER_TO_INT(user_data);
    gchar message[300], color[100];
    GtkFixed *fixed = GTK_FIXED(gtk_builder_get_object(builder, "fr2_main"));
    GtkStack *fr2_stack_access = GTK_STACK(gtk_builder_get_object(builder, "fr2_stack_access"));
    Song *vector = read_songs(songs_file);
    g_snprintf(color, sizeof(color), "CB0000");
    g_snprintf(message, sizeof(message), "Ocorreu um erro ao inserir a música.");
    button_set_click_animation(GTK_WIDGET(btn));
    if(vector == NULL || index < 0 || index >= song_count(songs_file)) {
        show_toast(message, color, 1500, 21, 665, 163, 158, 20, fixed);
        g_free(vector);
        return;
    }
    Song song = vector[index];
    gint status = playlist_add_song(song, open_playlist_file);

    if(status == 0) {
        g_snprintf(message, sizeof(message), "Ocorreu um erro ao inserir a música.");
    }
    if(status == 1) {
        g_snprintf(message, sizeof(message), "Música %s adicionada com sucesso!", vector[index].title);
        g_snprintf(color, sizeof(color), "1FD660");
        fill_playlist_songs(GINT_TO_POINTER(0));
        gtk_stack_set_visible_child_name(fr2_stack_access, "page_access_list");
    }
    if(status == -2)
        g_snprintf(message, sizeof(message), "A música já está presente na playlist.");
    
    show_toast(message, color, 1500, 21, 665, 163, 158, 20, fixed);
    g_free(vector);
}

void show_playlist_song(GtkButton *btn, gpointer user_data) {
    gint id = GPOINTER_TO_INT(user_data), index = -1;
    gchar text_buffer[200];
    gint minutes, seconds;
    Song *vector = read_songs(open_playlist_file);

    GtkStack *fr2_stack_access = GTK_STACK(gtk_builder_get_object(builder, "fr2_stack_access"));
    GtkLabel *fr2_access_label_view_name = GTK_LABEL(gtk_builder_get_object(builder, "fr2_access_label_view_name"));
    GtkLabel *fr2_access_label_view_artist = GTK_LABEL(gtk_builder_get_object(builder, "fr2_access_label_view_artist"));
    GtkLabel *fr2_access_label_view_album = GTK_LABEL(gtk_builder_get_object(builder, "fr2_access_label_view_album"));
    GtkLabel *fr2_access_label_view_minutes = GTK_LABEL(gtk_builder_get_object(builder, "fr2_access_label_view_minutes"));
    GtkLabel *fr2_access_label_view_id = GTK_LABEL(gtk_builder_get_object(builder, "fr2_access_label_view_id"));
    

    if(btn != NULL)
        button_set_click_animation(GTK_WIDGET(btn));

    gtk_stack_set_visible_child_name(fr2_stack_access, "page_access_view");
    
    for(int i=0; i < song_count(open_playlist_file); i++)
        if(vector[i].id == id)
            index = i;

    if(index < 0) {
        g_free(vector);
        return;
    }

    minutes = vector[index].duration / 60;
    seconds = vector[index].duration - minutes*60;
    sprintf(text_buffer, "%d:%d minutos", minutes, seconds);

    if(seconds < 10)
        sprintf(text_buffer, "%d:0%d minutos", minutes, seconds);
    
    change_label_text(fr2_access_label_view_name, vector[index].title);
    change_label_text(fr2_access_label_view_artist, vector[index].artist);
    change_label_text(fr2_access_label_view_album, vector[index].album);
    change_label_text(fr2_access_label_view_minutes, text_buffer);
    
    sprintf(text_buffer, "%d", vector[index].id);
    change_label_text(fr2_access_label_view_id, text_buffer);
    g_free(vector);
}

void show_song(GtkButton *btn, gpointer user_data) {
    int index = GPOINTER_TO_INT(user_data);
    gchar text_buffer[200];
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
    
    minutes = all_songs[index].duration / 60;
    seconds = all_songs[index].duration - minutes*60;
    sprintf(text_buffer, "%d:%d minutos", minutes, seconds);

    if(seconds < 10)
        sprintf(text_buffer, "%d:0%d minutos", minutes, seconds);
    
    change_label_text(fr2_label_view_name, all_songs[index].title);
    change_label_text(fr2_label_view_artist, all_songs[index].artist);
    change_label_text(fr2_label_view_album, all_songs[index].album);
    change_label_text(fr2_label_view_minutes, text_buffer);
    
    sprintf(text_buffer, "%d", all_songs[index].id);
    change_label_text(fr2_label_view_id, text_buffer);
}

void select_playlist(GtkButton *btn, gpointer user_data) {
    int index = GPOINTER_TO_INT(user_data);
    selected_playlist_index = index;
    gchar text_buffer[200];
    GtkLabel *selected = GTK_LABEL(gtk_builder_get_object(builder, "fr2_selected_playlist"));
    if(btn != NULL)
        button_set_click_animation(GTK_WIDGET(btn));
    
    g_snprintf(text_buffer, sizeof(text_buffer), "Playlist selecionada: %s", gtk_button_get_label(btn));
    change_label_text(selected, text_buffer);
    GtkFixed *fixed = GTK_FIXED(gtk_builder_get_object(builder, "fr2_playlist_list_fixed"));
    GtkWidget *fr2_btn_playlist_marker = GTK_WIDGET(gtk_builder_get_object(builder, "fr2_btn_playlist_marker"));
    gtk_fixed_move(GTK_FIXED(fixed), fr2_btn_playlist_marker, 8, 62*index+60);
    
}

void change_label_text(GtkLabel *label, const gchar *text) {
    gtk_label_set_text(label, text);
}

void set_truncated_text(GtkWidget *button, const char *text) {
    const int max_length = 18;

    char truncated_text[max_length * 4 + 4]; // up to 4 bytes per UTF-8 character, plus "..."
    if (g_utf8_strlen(text, -1) > max_length) {
        g_utf8_strncpy(truncated_text, text, max_length); 
        strcat(truncated_text, "...");
    } else {
        g_strlcpy(truncated_text, text, sizeof(truncated_text));
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

void clear_dynamic_widgets(GtkFixed *fixed) {
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
void show_toast(gchar *text, gchar *color, gint duration, gint height, gint width, gint x, gint y, gint steps, GtkFixed *fixed) {
    gchar css_class[100];
    GtkWidget *label = gtk_label_new(text);
    GtkStyleContext *label_context = gtk_widget_get_style_context(label);
    g_snprintf(css_class, sizeof(css_class), "color_%s", color);
    
    gtk_fixed_put(GTK_FIXED(fixed), label, x, y); 
    gtk_widget_set_size_request(label, width, height);     
    gtk_style_context_add_class(label_context, "universal_no_border");
    gtk_style_context_add_class(label_context, "universal_font");
    gtk_style_context_add_class(label_context, css_class);
    gtk_style_context_add_class(label_context, "font_size_16");
    gtk_style_context_add_class(label_context, "universal_no_background");
    gtk_widget_show(label);
    
    ToastAnimation *data = g_malloc(sizeof(ToastAnimation) * 1);
    data->steps_left = steps;
    data->container = fixed;
    data->widget = label;
    data->x = x;
    data->y = y;
    data->opacity = 1.0;
    data->opacity_step = 1.0 / (gfloat) steps;
    g_timeout_add(duration / steps , toast_step, data);
}

gboolean toast_step(gpointer data) {
    ToastAnimation *anim = (ToastAnimation*) data;
    if(anim->steps_left > 0) {
        anim->y -= 1;
        anim->opacity -= anim->opacity_step;
        gtk_fixed_move(anim->container, GTK_WIDGET(anim->widget), anim->x, anim->y);
        if(anim->widget != NULL)
            gtk_widget_set_opacity(anim->widget, anim->opacity);
        anim->steps_left -= 1;
        return TRUE;
    }

    gtk_widget_destroy(anim->widget);
    g_free(anim);
    return FALSE;
}

static void set_custom_cursor(GtkWidget *widget, gpointer data) {
    (void)widget;
    (void)data;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file("../assets/ui_images/cursor.png", NULL);
    if(pixbuf == NULL)
        return; // keep the default cursor if the image is missing
    GdkCursor *cursor = gdk_cursor_new_from_pixbuf(gdk_display_get_default(), pixbuf, 0, 0);

    gdk_window_set_cursor(gtk_widget_get_window(window), cursor);
    g_object_unref(cursor);
    g_object_unref(pixbuf);
}

/*=============================================================================================*/