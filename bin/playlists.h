#ifndef FINAL_PROJECT_PROG2_PLAYLISTS_PLAYLISTS_H
#define FINAL_PROJECT_PROG2_PLAYLISTS_PLAYLISTS_H

typedef struct {

    char nome[200];
    char album[400];
    char artista[200];
    int tempo;
    int id;

} musica;

typedef struct nodo {

    musica dados;
    struct nodo* prox;

} nodo;

typedef struct {

    nodo* inicio;
    nodo* fim;
    int tamanho;

} playlist;

typedef struct {
    char name[100];
} PlaylistData;


void ini_lista (playlist* nova);

int segundos (int v[][2]);

int add (int base, int adicionar);

int id (const char *nome, const char *artista, int tempo);

void gravador (int index_num);

musica *select_mostruario (playlist* pl); // seleciona as músicas que o usuário
// pedir num vetor (eu chamei tanto pra apagar quanto pra excluir as musicas selecionadas em outras funções)

void apagar_musica_bin (void);

void add_playlist (playlist *nova);

void apagar_playlist (playlist *pl);

void ord_nome ();

void ord_artista ();

void ord_tempo ();

#endif //FINAL_PROJECT_PROG2_PLAYLISTS_PLAYLISTS_H

// ===========================================================================================
// Funções de tratamento de arquivo

// Files
int musicsLength(FILE * pFile);
musica * readMusicsvector(FILE *pFile);
void reinsFile(FILE *pFile);
int addNewMusicInPlaylist(musica music, FILE *pFile);
int delNewMusicInPlaylist(musica music, FILE *pFile);
int playlistFileExists(char *name);
int createNewPlaylistFile(char *name, FILE *controller);
FILE * openPlaylistsController();
FILE * acessPlaylistFile(char *name);
int addPlaylistsController(char *name, FILE *controller);
PlaylistData * readerPlaylistsController (FILE *pFile);
int lengthPlaylistsController(FILE *pFile);
int removePlaylistsController(char *name, FILE *controller, FILE *removeFile);
int printMusicsInPlaylist(FILE *pFile);


//=================================================================================================
// Sort

void bubbleTypeSort(musica *vector, int type, int size);
int sortCompName(const void *a, const void *b);
int sortCompArtist(const void *a, const void *b);
int sortCompAlbum(const void *a, const void *b);
//================================================================================================
// Searchs
int sequencialSearch(int num, int vector[], int length);
int isMusicInVector(musica music, musica *vector, int length);

//================================================================================================