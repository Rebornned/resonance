#ifndef FINAL_PROJECT_PROG2_PLAYLISTS_PLAYLISTS_H
#define FINAL_PROJECT_PROG2_PLAYLISTS_PLAYLISTS_H

typedef struct {

    char nome[200];
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

void select_mostruario (playlist* pl);

void nova_playlist (playlist *nova);

void ord_nome ();

void ord_artista ();

void ord_tempo ();

#endif //FINAL_PROJECT_PROG2_PLAYLISTS_PLAYLISTS_H
