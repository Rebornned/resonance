#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include "library.h"

/*
 * Songs stored in a playlist keep the order in which they were added.
 * The interface can also show them sorted by ID, duration, title,
 * artist or album (see sort_songs).
 */

// ================================================================================================
// Songs

int song_count(FILE *file) { // Number of song records in a database or playlist file
    fseek(file, 0, SEEK_END);
    return ftell(file) / sizeof(Song);
}

Song * read_songs(FILE *file) { // Reads every song of the file (NULL if it is empty); free the result
    int count = song_count(file);
    rewind(file);

    if(count > 0) {
        Song *songs = malloc(sizeof(Song) * count);
        if(songs == NULL)
            return NULL;
        Song song;
        int read = 0;
        while(read < count && fread(&song, sizeof(Song), 1, file) == 1)
            songs[read++] = song;
        rewind(file);
        return songs;
    }
    return NULL;
}

int truncate_file(FILE *file) { // Empties the file; returns 0 on success
    fflush(file);
    if(ftruncate(fileno(file), 0) != 0)
        return -1;
    rewind(file);
    return 0;
}

int playlist_add_song(Song song, FILE *playlist_file) { // Appends a song to a playlist
    int count = song_count(playlist_file);
    if(count > 0) {
        Song *songs = read_songs(playlist_file);
        if(songs == NULL)
            return -1; // Out of memory

        for(int i=0; i < count; i++) {
            if(song.id == songs[i].id) {
                free(songs);
                return -2; // Song is already in the playlist
            }
        }
        free(songs);
    }

    fseek(playlist_file, count * sizeof(Song), SEEK_SET);
    if(fwrite(&song, sizeof(Song), 1, playlist_file) == 1)
        return 1; // Added

    return 0; // Write error
}

int playlist_remove_song(Song song, FILE *playlist_file) { // Removes a song, keeping the order of the others
    int count = song_count(playlist_file), found = 0;
    if(count > 0) {
        Song *songs = read_songs(playlist_file);
        if(songs == NULL)
            return -1; // Out of memory
        if(truncate_file(playlist_file) != 0) {
            free(songs);
            return -2; // Write error
        }
        for(int i=0; i < count; i++) {
            if(song.id != songs[i].id) {
                if(fwrite(&songs[i], sizeof(Song), 1, playlist_file) != 1) {
                    free(songs);
                    return -2; // Write error
                }
            }
            else {
                found = 1;
            }
        }

        free(songs);
        if(found == 0)
            return -3; // Song is not in the playlist
        return 1; // Removed
    }
    return 0; // Empty playlist
}

// ================================================================================================
// Playlist registry: one Playlist record per playlist, in creation order

#define PLAYLISTS_DIR "../files/playlists/"
#define REGISTRY_PATH PLAYLISTS_DIR "playlist_registry.bin"

/* Playlist files are named by ID, never by the name the user typed,
   so no user input ever becomes part of a file path. */
static void playlist_path(int id, char *path, size_t size) {
    snprintf(path, size, PLAYLISTS_DIR "playlist_%d.bin", id);
}

FILE * registry_open(void) { // Opens the registry, creating it if it does not exist
    return fopen(REGISTRY_PATH, "ab+");
}

int registry_count(FILE *registry) { // Number of registered playlists
    fseek(registry, 0, SEEK_END);
    return ftell(registry) / sizeof(Playlist);
}

Playlist * registry_read_all(FILE *registry) { // Returns all registered playlists (NULL if there are none); free the result
    int count = registry_count(registry), read = 0;
    if(count > 0) {
        Playlist *playlists = malloc(sizeof(Playlist) * count);
        if(playlists == NULL)
            return NULL;
        Playlist playlist;
        rewind(registry);
        while(read < count && fread(&playlist, sizeof(Playlist), 1, registry) == 1)
            playlists[read++] = playlist;
        rewind(registry);
        return playlists;
    }
    return NULL;
}

int registry_get(FILE *registry, int index, Playlist *out) { // Copies the playlist at a list position; returns 1 if found
    int count = registry_count(registry);
    if(index < 0 || index >= count)
        return 0;
    fseek(registry, sizeof(Playlist) * index, SEEK_SET);
    int found = fread(out, sizeof(Playlist), 1, registry) == 1;
    rewind(registry);
    return found;
}

/* Returns a normalized copy of a playlist name (free with g_free), or NULL
   if the name is not accepted. Accepted: valid UTF-8, 1 to
   PLAYLIST_NAME_MAX_CHARS characters, only letters and digits of any
   language and spaces. Leading and trailing spaces are removed. */
static gchar * normalize_playlist_name(const char *name) {
    if(name == NULL || !g_utf8_validate(name, -1, NULL))
        return NULL;

    /* NFC turns "e" + combining accent into a single "é", so the
       character check below sees letters, not loose accent marks. */
    gchar *clean = g_utf8_normalize(name, -1, G_NORMALIZE_NFC);
    if(clean == NULL)
        return NULL;
    g_strstrip(clean);

    glong chars = g_utf8_strlen(clean, -1);
    if(chars == 0 || chars > PLAYLIST_NAME_MAX_CHARS || strlen(clean) >= sizeof(((Playlist *)0)->name)) {
        g_free(clean);
        return NULL;
    }

    for(const gchar *p = clean; *p != '\0'; p = g_utf8_next_char(p)) {
        gunichar c = g_utf8_get_char(p);
        if(!g_unichar_isalnum(c) && c != ' ') {
            g_free(clean);
            return NULL;
        }
    }
    return clean;
}

/* Names are compared ignoring case, so "Rock" and "rock" are the same playlist. */
static int same_playlist_name(const char *a, const char *b) {
    gchar *fold_a = g_utf8_casefold(a, -1);
    gchar *fold_b = g_utf8_casefold(b, -1);
    int same = strcmp(fold_a, fold_b) == 0;
    g_free(fold_a);
    g_free(fold_b);
    return same;
}

int playlist_create(const char *name, FILE *registry) { // Registers a new playlist and creates its file
    gchar *clean = normalize_playlist_name(name);
    if(clean == NULL)
        return -3; // Invalid name

    int count = registry_count(registry);
    Playlist *playlists = registry_read_all(registry);
    if(count > 0 && playlists == NULL) {
        g_free(clean);
        return -2; // Read error
    }

    int next_id = 1;
    for(int i=0; i < count; i++) {
        if(same_playlist_name(playlists[i].name, clean)) {
            free(playlists);
            g_free(clean);
            return -1; // Playlist already exists
        }
        if(playlists[i].id >= next_id)
            next_id = playlists[i].id + 1;
    }
    free(playlists);

    Playlist playlist;
    memset(&playlist, 0, sizeof(playlist));
    playlist.id = next_id;
    g_strlcpy(playlist.name, clean, sizeof(playlist.name));
    g_free(clean);

    char path[300];
    playlist_path(playlist.id, path, sizeof(path));
    FILE *file = fopen(path, "wb");
    if(file == NULL)
        return -2; // Could not create the playlist file
    fclose(file);

    fseek(registry, 0, SEEK_END);
    if(fwrite(&playlist, sizeof(Playlist), 1, registry) != 1) {
        remove(path);
        return -2; // Could not register the playlist
    }
    fflush(registry);
    return 1;
}

int playlist_delete(int id, FILE *registry) { // Removes a playlist: its record and its file
    int count = registry_count(registry);
    Playlist *playlists = registry_read_all(registry);
    if(playlists == NULL)
        return -2; // No playlists

    int found = 0;
    for(int i=0; i < count; i++)
        if(playlists[i].id == id)
            found = 1;
    if(!found) {
        free(playlists);
        return -2; // Playlist does not exist
    }

    if(truncate_file(registry) != 0) {
        free(playlists);
        return -1; // Error rewriting the registry
    }
    for(int i=0; i < count; i++) {
        if(playlists[i].id != id && fwrite(&playlists[i], sizeof(Playlist), 1, registry) != 1) {
            free(playlists);
            return -1; // Error rewriting the registry
        }
    }
    fflush(registry);
    free(playlists);

    char path[300];
    playlist_path(id, path, sizeof(path));
    remove(path);
    return 1; // Playlist removed
}

FILE * playlist_open(int id) { // Opens the file of a playlist (NULL if it does not exist)
    char path[300];
    playlist_path(id, path, sizeof(path));
    if(access(path, F_OK) != 0)
        return NULL;
    return fopen(path, "ab+");
}

// ================================================================================================
// Sorting

static int compare_int(int a, int b) {
    return (a > b) - (a < b);
}

static int compare_by_id(const void *a, const void *b) {
    return compare_int(((const Song *)a)->id, ((const Song *)b)->id);
}

/* The other criteria use the ID as a tie-breaker, so the order is always the same. */
static int compare_by_duration(const void *a, const void *b) {
    int result = compare_int(((const Song *)a)->duration, ((const Song *)b)->duration);
    return result != 0 ? result : compare_by_id(a, b);
}

static int compare_by_title(const void *a, const void *b) {
    int result = g_utf8_collate(((const Song *)a)->title, ((const Song *)b)->title);
    return result != 0 ? result : compare_by_id(a, b);
}

static int compare_by_artist(const void *a, const void *b) {
    int result = g_utf8_collate(((const Song *)a)->artist, ((const Song *)b)->artist);
    return result != 0 ? result : compare_by_id(a, b);
}

static int compare_by_album(const void *a, const void *b) {
    int result = g_utf8_collate(((const Song *)a)->album, ((const Song *)b)->album);
    return result != 0 ? result : compare_by_id(a, b);
}

void sort_songs(Song *songs, int count, SortMode mode) { // SORT_INSERTION keeps the order stored in the file
    int (*compare)(const void *, const void *) = NULL;
    switch(mode) {
        case SORT_ID:       compare = compare_by_id;       break;
        case SORT_DURATION: compare = compare_by_duration; break;
        case SORT_TITLE:    compare = compare_by_title;    break;
        case SORT_ARTIST:   compare = compare_by_artist;   break;
        case SORT_ALBUM:    compare = compare_by_album;    break;
        case SORT_INSERTION:
        default:            return;
    }
    if(songs != NULL && count > 1)
        qsort(songs, count, sizeof(Song), compare);
}
