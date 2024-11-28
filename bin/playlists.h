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

void ini_lista (playlist* nova);

int segundos (int v[][2]);

int add (int base, int adicionar);

int id (const char *nome, const char *artista, int tempo);

void gravador (void);

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

musica * readMusicsvector(FILE *pFile);
int musicsLength(FILE *pFile);