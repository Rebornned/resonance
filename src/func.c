#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "playlists.h"
#include <unistd.h>
#include <locale.h>
#include <glib.h>

/* A ordem das músicas armazenadas nas playlists importa e deve ser modificável, organizável das
 seguinte formas: em ordem de inclusão na playlist, em ordem alfabética de nome, em ordem
 alfabética de artista ou em ordem crescente de duração. */

/* As estruturas não precisam ser declaradas neste arquivo, quando já foram declaradas no playlists.h
Isso causou erros, por isso está comentado.
*/
// ***********************************************************************************************
// Assinaturas

// Files
int musicsLength(FILE * pFile);
musica * readMusicsvector(FILE *pFile);
void reinsFile(FILE *pFile);
int addNewMusicInPlaylist(musica music, FILE *pFile);
int delNewMusicInPlaylist(musica music, FILE *pFile);
int createNewPlaylistFile(const char *name, FILE *controller);
FILE * openPlaylistsController();
FILE * acessPlaylistFile(int id);
int getPlaylistByIndex(FILE *controller, int index, PlaylistData *out);
PlaylistData * readerPlaylistsController (FILE *pFile);
int lengthPlaylistsController(FILE *pFile);
int removePlaylistsController(int id, FILE *controller);
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
// Strings
// ===============================================================================================
void ini_lista (playlist* nova) {

    nova->inicio = NULL;
    nova->fim = NULL;
    nova->tamanho = 0;

}

int segundos (int v[][2]) { // transformar uma matriz contendo minutos em segundos em segundos
    // chamar antes de atribuir o tempo à struct música

    return v[0][0] * 60 + v[0][1];

}

int add (int base, int adicionar) { // adiciona ao fim da variavel o numero desejado

    int digitos = (adicionar == 0) ? 1 : (int)log10(adicionar) + 1;

    return base * (int)pow(10, digitos) + adicionar;

}

int id (const char *nome, const char *artista, int tempo) { // aritmética para fabricar os ids das musicas

    int ascii_nome = 0, ascii_artista = 0, n = 0, tempo_bin = 0;

    for (int i = 0; nome[i] != '\0'; i++) ascii_nome += (int)nome[i];  // converte o caractere para ASCII e soma
    n = ascii_nome;

    while (tempo > 0) { // transforma o tempo em binário

        int resto = tempo % 2; // pega o próximo dígito binário
        tempo_bin = add(tempo_bin, resto); // adiciona ao final
        tempo /= 2; // divide por 2 para processar o próximo bit

    } n = add(n, tempo_bin);

    for (int i = 0; artista[i] != '\0'; i++) ascii_artista *= (int)artista[i]; // converte o caractere para ASCII e multiplica
    n = add(n, ascii_artista);
    return n;
}


musica *select_mostruario (playlist *pl) { // seleciona as musicas

    FILE *gravados = fopen("gravados.dat", "rb");
    if (!gravados) printf("Nao foi possivel abrir o arquivo de musicas.\n");

    musica aux;
    musica *selecionadas = malloc(sizeof(musica) * (pl->tamanho+1)); // vetor de struct pra armazenar as musicas selecionadas
    if (!selecionadas) {

        printf("Erro ao alocar memoria para as musicas selecionadas.\n");
        fclose(gravados);

        return NULL;
    }
    int escolha, encontrou, continuar = 1, contador = -1;

    int index = 1;
    while (fread(&aux, sizeof(musica), 1, gravados)) {

        printf("ID: %d\n", aux.id);
        printf("Nome: %s\n", aux.nome);
        printf("Artista: %s\n", aux.artista);
        printf("Tempo: %d segundos\n", aux.tempo);
        printf("----------------------------\n");

        index++;

    }

    printf("\nDigite os IDs das musicas que deseja selecionar (pressione 0 para finalizar):\n");

    while (continuar) {

        printf("ID: \n");
        scanf("%d", &escolha);

        if (escolha == 0) {

            printf("Finalizando a selecao.\n");
            continuar = 0;

        } else { // reinicia a busca no arquivo para localizar a música pelo id

            rewind(gravados);
            encontrou = 0;

            while (fread(&aux, sizeof(musica), 1, gravados)) {

                if (aux.id == escolha) {

                    selecionadas[contador++] = aux;
                    printf("Musica '%s' selecionada.\n", aux.nome);
                    encontrou = 1;

                    break;
                }

            } if (!encontrou) printf("ID %d nao encontrado. Tente novamente.\n", escolha);

        }

    }

    fclose(gravados);

    return selecionadas;
}

void apagar_musica_bin (void) { // excluir música do arquivo binario

    musica excluir;
    musica *musicas_selecionadas = select_mostruario(NULL);  // select_mostruario é chamada aqui
    if (musicas_selecionadas == NULL) {

        printf("Nenhuma música selecionada para exclusão.\n");

        return;
    }

    FILE *gravados = fopen("gravados.dat", "rb");
    if (gravados == NULL) {

        printf("Nao foi possivel abrir o arquivo de musicas.");

        return;
    }
    FILE *aux = fopen("aux.dat", "wb");
    if (aux == NULL) {

        printf("Nao foi possivel criar o arquivo auxiliar.\n");
        fclose(gravados);

        return;
    }

    for (int i = 0; musicas_selecionadas[i].id != 0; i++) { // percorre as músicas selecionadas

        int encontrou = 0;
        rewind(gravados);

        while (fread(&aux, sizeof(musica), 1, gravados)) {

            if (excluir.id == musicas_selecionadas[i].id) {

                printf("Musica '%s' excluida.\n", excluir.nome);
                encontrou = 1;

                break; // se encontrou a música, ignora a copia dela para o arquivo temporário

            } else fwrite(&aux, sizeof(musica), 1, aux); // caso a música não tenha sido selecionada para exclusão, copia para o arquivo temporário

        } if (!encontrou) printf("Musica com ID %d noo encontrada para exclusao.\n", musicas_selecionadas[i].id);

    }

    fclose(gravados);
    fclose(aux);

    // substituir o arquivo original pelo temporário
    if (remove("gravados.dat") != 0) {

        printf("Erro ao excluir o arquivo original.\n");
        free(musicas_selecionadas);

        return;
    }
    if (rename("aux.dat", "gravados.dat") != 0) {

        printf("Erro ao renomear o arquivo auxiliar.\n");
        free(musicas_selecionadas);

        return;
    }

    free(musicas_selecionadas);

}

void add_playlist (playlist *pl) { // a lista vai ser criada aq
    // colocar no final do id de cada musica na playlist um número indicando a ordem de inserção

    musica *musicas_selecionadas = select_mostruario(pl); // seleciona as músicas a serem adicionadas
    if (!musicas_selecionadas) {

        printf("Nenhuma musica selecionada para adicionar a playlist.\n");
        return;

    }

    for (int i = 0; musicas_selecionadas[i].id != 0; i++) {  // adiciona cada música selecionada

        nodo *dado = (nodo*)malloc(sizeof(nodo));
        if (!dado) {

            printf("Erro ao alocar memoria.\n");
            return;

        }

        musicas_selecionadas[i].id = add(musicas_selecionadas[i].id, i);

        dado->dados = musicas_selecionadas[i]; // preenche com a música
        dado->prox = NULL;

        if (pl->inicio == NULL) {

            pl->inicio = dado;
            pl->fim = dado;

        } else {

            pl->fim->prox = dado;
            pl->fim = dado;

        }

        pl->tamanho++;

    }

    free(musicas_selecionadas);

}

void remover_musica_playlist (playlist* pl) {

    if (pl->inicio == NULL) {

        printf("A playlist esta vazia.\n");

        return;
    }

    nodo *atual = pl->inicio;
    nodo *anterior = NULL;

    musica *selecionadas = select_mostruario(pl);
    if (!selecionadas) {

        printf("Nenhuma musica selecionada.\n");

        return;
    }

    while (atual != NULL) { // itera sobre a lista encadeada de músicas

        int encontrado = 0;

        for (int i = 0; selecionadas[i].id != 0; i++) {

            if (atual->dados.id == selecionadas[i].id) {

                encontrado = 1;

                if (anterior == NULL) pl->inicio = atual->prox; // caso a música a ser removida seja o primeiro nodo altera a cabeça da lista pro próximo nodo
                else anterior->prox = atual->prox; // senão pula o nodo a ser removido

                free(atual);
                printf("Musica com ID %d removida da playlist.\n", atual->dados.id);

                break;
            }

        }

        if (encontrado) atual = anterior ? anterior->prox : pl->inicio;
        else { // avança o nodo porque n foi encontrado o id

            anterior = atual;
            atual = atual->prox;

        }

    }

    free(selecionadas);

}

void apagar_playlist (playlist *pl) {

    if (pl == NULL) {

        printf("A playlist já está vazia ou inexistente.\n");

        return;
    }

    nodo* atual = pl->inicio;
    nodo* proximo;

    while (atual != NULL) { // percorre toda a lista encadeada

        proximo = atual->prox; // armazena o próximo nodo
        free(atual);              // libera o nodo atual
        atual = proximo;          // avança para o próximo nodo

    }

    // após liberar todos os nodos, limpa a estrutura principal
    pl->inicio = NULL;  // a cabeça da lista é redefinida como vazia
    pl->tamanho = 0;    // redefine o tamanho da lista

    printf("A playlist foi apagada com sucesso.\n");
}


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

    reinsFile(controller);
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
int printMusicsInPlaylist(FILE *pFile) { // Printa todas as músicas existentes na playlist
    int length = musicsLength(pFile);
    printf("=====================================================================\n");
    if(length > 0) {
        musica *vector = readMusicsvector(pFile);
        for(int i=0; i < length; i++) {
            printf("Name: %s | ID: %d\n", vector[i].nome, vector[i].id);
        }
        printf("=====================================================================\n");
        return 1; // Playlist percorrida corretamente
    }

    return 0; // Playlist vazia
}

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
        reinsFile(pFile);
        rewind(pFile);
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

void reinsFile(FILE *pFile) { // Reseta o arquivo binário
    int fd = fileno(pFile);
    ftruncate(fd, 0);
    rewind(pFile);
}

/* =========================================================================================================
Searchs */
int sequencialSearch(int num, int vector[], int length) { // Busca sequencial de um número em um vetor
    for(int i=0; i < length; i++) 
        if(num == vector[i])
            return 1;
    return 0;
}

int isMusicInVector(musica music, musica *vector, int length) { // Verifica se uma música especifica está no vetor de músicas
    for(int i=0; i < length; i++) {
        if(music.id == vector[i].id)
            return i;
    }
    return 0;
}
/*==========================================================================================================
Sorts
*/
void bubbleTypeSort(musica *vector, int type, int size) { // Um bubble sort capaz de sortear por diferentes condições
    int ordened = 0, sortByValue1 = 0, sortByValue2 = 0;
    musica aux, comp1, comp2;
    int value = size;
    if(size < 1)
        return;

    while(ordened == 0) {
        ordened = 1;
    
        for(int i=0; i < size-1; i++) {
            comp1 = vector[i];
            comp2 = vector[i+1];

            switch (type)
            {
            case 1:
                sortByValue1 = comp1.id;
                sortByValue2 = comp2.id;
                break;
            case 2:
                sortByValue1 = comp1.tempo;
                sortByValue2 = comp2.tempo;
                break;
            case 3:
                qsort(vector, size, sizeof(musica), sortCompName);
                return;
                break;
            case 4:
                qsort(vector, size, sizeof(musica), sortCompArtist);
                return;
                break;
            case 5:
                qsort(vector, size, sizeof(musica), sortCompAlbum);
                return;
                break;

            default:
                break;
            }
            if(sortByValue1 > sortByValue2) {
                ordened = 0;
                aux = vector[i];
                vector[i] = vector[i+1];
                vector[i+1] = aux;
            }
        }
        size--;
    }
}

int sortCompName(const void *a, const void *b) { // auxiliar de qsort para campo nome
    const musica *musicaA = (const musica *)a;
    const musica *musicaB = (const musica *)b;
    return g_utf8_collate(musicaA->nome, musicaB->nome);
}

int sortCompArtist(const void *a, const void *b) { // auxiliar de qsort para campo artista
    const musica *musicaA = (const musica *)a;
    const musica *musicaB = (const musica *)b;
    return g_utf8_collate(musicaA->artista, musicaB->artista);
}

int sortCompAlbum(const void *a, const void *b) { // auxiliar de qsort para campo album
    const musica *musicaA = (const musica *)a;
    const musica *musicaB = (const musica *)b;
    return g_utf8_collate(musicaA->album, musicaB->album);
}

/*========================================================================================================
Strings
*/
