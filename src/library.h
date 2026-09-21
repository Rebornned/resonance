#ifndef FINAL_PROJECT_PROG2_PLAYLISTS_PLAYLISTS_H
#define FINAL_PROJECT_PROG2_PLAYLISTS_PLAYLISTS_H

typedef struct {

    char nome[200];
    char album[400];
    char artista[200];
    int tempo;
    int id;

} musica;

#define PLAYLIST_NAME_MAX_CHARS 18 /* same limit as the name field in the interface */

typedef struct {
    int id;          /* also names the file: files/playlists/playlist_<id>.bin */
    char name[100];  /* UTF-8, as typed by the user (after trimming) */
} PlaylistData;


// ===========================================================================================
// Funções de tratamento de arquivo

// Files
int musicsLength(FILE * pFile);
musica * readMusicsvector(FILE *pFile);
int reinsFile(FILE *pFile);
int addNewMusicInPlaylist(musica music, FILE *pFile);
int delNewMusicInPlaylist(musica music, FILE *pFile);
int createNewPlaylistFile(const char *name, FILE *controller);
FILE * openPlaylistsController();
FILE * acessPlaylistFile(int id);
int getPlaylistByIndex(FILE *controller, int index, PlaylistData *out);
PlaylistData * readerPlaylistsController (FILE *pFile);
int lengthPlaylistsController(FILE *pFile);
int removePlaylistsController(int id, FILE *controller);

//=================================================================================================
// Sort

typedef enum {
    SORT_INSERTION, /* order in which songs were added (playlists only) */
    SORT_ID,
    SORT_DURATION,
    SORT_TITLE,
    SORT_ARTIST,
    SORT_ALBUM
} SortMode;

void sort_songs(musica *songs, int count, SortMode mode);

#endif // FINAL_PROJECT_PROG2_PLAYLISTS_PLAYLISTS_H
