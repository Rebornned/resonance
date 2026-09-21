#ifndef RESONANCE_LIBRARY_H
#define RESONANCE_LIBRARY_H

/*
 * Data layer of Resonance: the song database, playlist files and sorting.
 * Everything here is plain file I/O; the interface lives in main.c.
 */

#include <stdio.h>

/* One record of files/musics_database.bin and of every playlist file.
   The layout (sizes and order of the fields) is the on-disk format:
   changing it makes existing files unreadable. */
typedef struct {
    char title[200];
    char album[400];
    char artist[200];
    int duration;    /* seconds */
    int id;          /* position in data/musics.txt */
} Song;

#define PLAYLIST_NAME_MAX_CHARS 18 /* same limit as the name field in the interface */

/* One record of files/playlists/playlist_registry.bin. */
typedef struct {
    int id;          /* also names the file: files/playlists/playlist_<id>.bin */
    char name[100];  /* UTF-8, as typed by the user (after trimming) */
} Playlist;

// ================================================================================================
// Songs (database and playlist files share the same record format)

int song_count(FILE *file);
Song * read_songs(FILE *file);
int truncate_file(FILE *file);
int playlist_add_song(Song song, FILE *playlist_file);
int playlist_remove_song(Song song, FILE *playlist_file);

// ================================================================================================
// Playlist registry

FILE * registry_open(void);
int registry_count(FILE *registry);
Playlist * registry_read_all(FILE *registry);
int registry_get(FILE *registry, int index, Playlist *out);
int playlist_create(const char *name, FILE *registry);
int playlist_delete(int id, FILE *registry);
FILE * playlist_open(int id);

// ================================================================================================
// Sorting

typedef enum {
    SORT_INSERTION, /* order in which songs were added (playlists only) */
    SORT_ID,
    SORT_DURATION,
    SORT_TITLE,
    SORT_ARTIST,
    SORT_ALBUM
} SortMode;

void sort_songs(Song *songs, int count, SortMode mode);

#endif // RESONANCE_LIBRARY_H
