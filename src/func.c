#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "playlists.h"
#include <unistd.h>
#include <glib.h>

/* A ordem das músicas armazenadas nas playlists importa e deve ser modificável, organizável das
 seguinte formas: em ordem de inclusão na playlist, em ordem alfabética de nome, em ordem
 alfabética de artista ou em ordem crescente de duração. */

/*======================================================================================================
Files*/

// Playlist Controller
#define PLAYLISTS_DIR "../files/playlists/"
#define CONTROLLER_PATH PLAYLISTS_DIR "playlists_controller.bin"

/* Playlist files are named by ID, never by the name the user typed,
   so no user input ever becomes part of a file path. */
static void playlistPath(int id, char *path, size_t size) {
    snprintf(path, size, PLAYLISTS_DIR "playlist_%d.bin", id);
}

FILE * openPlaylistsController() { // Opens the playlist controller, creating it if it does not exist
    return fopen(CONTROLLER_PATH, "ab+");
}

int lengthPlaylistsController(FILE *pFile) { // Number of playlists registered in the controller
    fseek(pFile, 0, SEEK_END);
    return ftell(pFile) / sizeof(PlaylistData);
}

PlaylistData * readerPlaylistsController (FILE *pFile) { // Returns all registered playlists (NULL if there are none)
    int length = lengthPlaylistsController(pFile), count=0;
    if(length > 0) {
        PlaylistData * vector = malloc(sizeof(PlaylistData) * length);
        if(vector == NULL)
            return NULL;
        PlaylistData index;
        rewind(pFile);
        while(count < length && fread(&index, sizeof(PlaylistData), 1, pFile) == 1)
            vector[count++] = index;
        rewind(pFile);
        return vector;
    }
    return NULL;
}

int getPlaylistByIndex(FILE *controller, int index, PlaylistData *out) { // Copies the playlist at a list position; returns 1 if found
    int length = lengthPlaylistsController(controller);
    if(index < 0 || index >= length)
        return 0;
    fseek(controller, sizeof(PlaylistData) * index, SEEK_SET);
    int found = fread(out, sizeof(PlaylistData), 1, controller) == 1;
    rewind(controller);
    return found;
}

/* Returns a normalized copy of a playlist name (free with g_free), or NULL
   if the name is not accepted. Accepted: valid UTF-8, 1 to
   PLAYLIST_NAME_MAX_CHARS characters, only letters and digits of any
   language and spaces. Leading and trailing spaces are removed. */
static gchar * normalizePlaylistName(const char *name) {
    if(name == NULL || !g_utf8_validate(name, -1, NULL))
        return NULL;

    /* NFC turns "e" + combining accent into a single "é", so the
       character check below sees letters, not loose accent marks. */
    gchar *clean = g_utf8_normalize(name, -1, G_NORMALIZE_NFC);
    if(clean == NULL)
        return NULL;
    g_strstrip(clean);

    glong chars = g_utf8_strlen(clean, -1);
    if(chars == 0 || chars > PLAYLIST_NAME_MAX_CHARS || strlen(clean) >= sizeof(((PlaylistData *)0)->name)) {
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
static int samePlaylistName(const char *a, const char *b) {
    gchar *foldA = g_utf8_casefold(a, -1);
    gchar *foldB = g_utf8_casefold(b, -1);
    int same = strcmp(foldA, foldB) == 0;
    g_free(foldA);
    g_free(foldB);
    return same;
}

int createNewPlaylistFile(const char *name, FILE *controller) { // Registers a new playlist and creates its file
    gchar *clean = normalizePlaylistName(name);
    if(clean == NULL)
        return -3; // Invalid name

    int length = lengthPlaylistsController(controller);
    PlaylistData *vector = readerPlaylistsController(controller);
    if(length > 0 && vector == NULL) {
        g_free(clean);
        return -2; // Read error
    }

    int nextId = 1;
    for(int i=0; i < length; i++) {
        if(samePlaylistName(vector[i].name, clean)) {
            free(vector);
            g_free(clean);
            return -1; // Playlist already exists
        }
        if(vector[i].id >= nextId)
            nextId = vector[i].id + 1;
    }
    free(vector);

    PlaylistData newPlaylist;
    memset(&newPlaylist, 0, sizeof(newPlaylist));
    newPlaylist.id = nextId;
    g_strlcpy(newPlaylist.name, clean, sizeof(newPlaylist.name));
    g_free(clean);

    char fileName[300];
    playlistPath(newPlaylist.id, fileName, sizeof(fileName));
    FILE *pFile = fopen(fileName, "wb");
    if(pFile == NULL)
        return -2; // Could not create the playlist file
    fclose(pFile);

    fseek(controller, 0, SEEK_END);
    if(fwrite(&newPlaylist, sizeof(PlaylistData), 1, controller) != 1) {
        remove(fileName);
        return -2; // Could not register the playlist
    }
    fflush(controller);
    return 1;
}

int removePlaylistsController(int id, FILE *controller) { // Removes a playlist: its record and its file
    int length = lengthPlaylistsController(controller);
    PlaylistData *vector = readerPlaylistsController(controller);
    if(vector == NULL)
        return -2; // No playlists

    int found = 0;
    for(int i=0; i < length; i++)
        if(vector[i].id == id)
            found = 1;
    if(!found) {
        free(vector);
        return -2; // Playlist does not exist
    }

    if(reinsFile(controller) != 0) {
        free(vector);
        return -1; // Error rewriting the controller
    }
    for(int i=0; i < length; i++) {
        if(vector[i].id != id && fwrite(&vector[i], sizeof(PlaylistData), 1, controller) != 1) {
            free(vector);
            return -1; // Error rewriting the controller
        }
    }
    fflush(controller);
    free(vector);

    char fileName[300];
    playlistPath(id, fileName, sizeof(fileName));
    remove(fileName);
    return 1; // Playlist removed
}

FILE * acessPlaylistFile(int id) { // Opens the file of a playlist (NULL if it does not exist)
    char fileName[300];
    playlistPath(id, fileName, sizeof(fileName));
    if(access(fileName, F_OK) != 0)
        return NULL;
    return fopen(fileName, "ab+");
}

// Playlists editors
int addNewMusicInPlaylist(musica music, FILE *pFile) { // Adiciona uma nova música ao arquivo da playlist
    int length = musicsLength(pFile);
    if(length > 0) {
        musica * vector = readMusicsvector(pFile);
        if(!vector)
                return -1; // Erro de alocação de memória
        
        for(int i=0; i < length; i++) {
            if(music.id == vector[i].id) {
                free(vector);
                return -2; // Música já existente na playlist
            }
        }
        free(vector);
    }

    fseek(pFile, length * sizeof(musica), SEEK_SET);
    if(fwrite(&music, sizeof(musica), 1, pFile) == 1)
        return 1; // Inserido com sucesso

    return 0; // Erro ao inserir música
}

int delNewMusicInPlaylist(musica music, FILE *pFile) { // Retira uma música do arquivo da playlist
    int lenght = musicsLength(pFile), finded = 0;
    if(lenght > 0) {
        musica *vector = readMusicsvector(pFile);
        if(!vector)
            return -1; // Erro de alocação de memória
        if(reinsFile(pFile) != 0) {
            free(vector);
            return -2; // Erro de escrita no arquivo
        }
        for(int i=0; i<lenght; i++) {
            if(music.id != vector[i].id) {
                if(fwrite(&vector[i], sizeof(musica), 1, pFile) != 1) {
                    free(vector);
                    return -2; // Erro de escrita no arquivo
                }
            }
            else {
                finded = 1;
            }
        }

        free(vector);
        if(finded == 0)
            return -3; // Música não existe na playlist
        return 1; // Música apagada com sucesso
    }
    return 0; // Playlist vazia
}
//********************************************************************************************************
// Musics functions
musica * readMusicsvector(FILE *pFile) { // Esta função lê o banco de músicas e retorna um vetor com as músicas 
    int length = musicsLength(pFile);
    rewind(pFile);

    if(length > 0) {
        musica * vector = (musica *) malloc (sizeof(musica) * length);
        musica music;
        if(!vector) {
            // Allocation error
            return NULL;
        }
        int count = 0;
        while(fread(&music, sizeof(musica), 1, pFile) == 1) {
            //printf("Artista: %s| ID: %d | Musica: %s | Seconds: %d | Album: %s\n", music.artista, music.id, music.nome, music.tempo, music.album);
            vector[count++] = music;
        }
        rewind(pFile);
        return vector;
    }
    return NULL;
}

int musicsLength(FILE *pFile) { // Esta função retorna a quantidade de músicas dentro do banco de músicas
    fseek(pFile, 0, SEEK_END);
    return ftell(pFile) / sizeof(musica);
}

int reinsFile(FILE *pFile) { // Reseta o arquivo binário; retorna 0 em caso de sucesso
    fflush(pFile);
    if(ftruncate(fileno(pFile), 0) != 0)
        return -1;
    rewind(pFile);
    return 0;
}

/*==========================================================================================================
Sorts
*/
static int compare_int(int a, int b) {
    return (a > b) - (a < b);
}

static int compare_by_id(const void *a, const void *b) {
    return compare_int(((const musica *)a)->id, ((const musica *)b)->id);
}

/* The other criteria use the ID as a tie-breaker, so the order is always the same. */
static int compare_by_duration(const void *a, const void *b) {
    int result = compare_int(((const musica *)a)->tempo, ((const musica *)b)->tempo);
    return result != 0 ? result : compare_by_id(a, b);
}

static int compare_by_title(const void *a, const void *b) {
    int result = g_utf8_collate(((const musica *)a)->nome, ((const musica *)b)->nome);
    return result != 0 ? result : compare_by_id(a, b);
}

static int compare_by_artist(const void *a, const void *b) {
    int result = g_utf8_collate(((const musica *)a)->artista, ((const musica *)b)->artista);
    return result != 0 ? result : compare_by_id(a, b);
}

static int compare_by_album(const void *a, const void *b) {
    int result = g_utf8_collate(((const musica *)a)->album, ((const musica *)b)->album);
    return result != 0 ? result : compare_by_id(a, b);
}

void sort_songs(musica *songs, int count, SortMode mode) { // SORT_INSERTION keeps the order stored in the file
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
        qsort(songs, count, sizeof(musica), compare);
}
