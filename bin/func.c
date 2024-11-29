#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "playlists.h"
#include <unistd.h>
#include <locale.h>

/* A ordem das músicas armazenadas nas playlists importa e deve ser modificável, organizável das
 seguinte formas: em ordem de inclusão na playlist, em ordem alfabética de nome, em ordem
 alfabética de artista ou em ordem crescente de duração. */

/* As estruturas não precisam ser declaradas neste arquivo, quando já foram declaradas no playlists.h
Isso causou erros, por isso está comentado.

typedef struct {

    char nome[200];
    char artista[200];
    char album[400];
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
*/

// ***********************************************************************************************
// Assinaturas
int musicsLength(FILE * pFile);
musica * readMusicsvector(FILE *pFile);
void reinsFile(FILE *pFile);
void bubbleTypeSort(musica *vector, int type, int size);
int sortCompName(const void *a, const void *b);
int sortCompArtist(const void *a, const void *b);
int sortCompAlbum(const void *a, const void *b);


/*
int main() {
    FILE *resetDatabase = fopen("../files/musics_database.bin", "rb+");
    reinsFile(resetDatabase);

    for(int i=0; i < 11; i++) {
        gravador(i);
    }
    printf("\n");
    FILE *resetDatabaseNew = fopen("../files/musics_database.bin", "rb+");
    musica * musicsVector = readMusicsvector(resetDatabaseNew);
    
    return 0;
}
*/
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

void gravador (int index_num) { // função para gravar as músicas no arq binário
    musica nova;
    int v[1][2]; // tempo da musica antes de transformar em segundos

    char inputStr[400];

    FILE *gravados = fopen("../files/musics_database.bin", "ab+");
    if (gravados == NULL) printf("Nao foi possivel abrir o arquivo de musicas.");

    scanf(" %[^\n]" ,inputStr);
    strcpy(nova.nome, inputStr);
    getchar();

    scanf(" %[^\n]" ,inputStr);
    strcpy(nova.album, inputStr);
    getchar();

    scanf(" %[^\n]" ,inputStr);
    strcpy(nova.artista, inputStr);
    getchar();
    
    //printf("Insira o tempo da musica no formato [mm:ss]: \n");
    scanf("%d:%d", &v[0][0], &v[0][1]);
    getchar();

    //printf("Insira o nome da musica: \n");
    //fgets(nova.nome, sizeof(nova.nome), stdin);
    //nova.nome[strcspn(nova.nome, "\n")] = 0;

    //printf("Insira o nome do álbum: \n");
    //fgets(nova.album, sizeof(nova.album), stdin);
    //nova.album[strcspn(nova.album, "\n")] = 0;

    //printf("Insira o nome do artista: \n");
    //fgets(nova.artista, sizeof(nova.artista), stdin);
    //nova.artista[strcspn(nova.artista, "\n")] = 0;


    nova.tempo = segundos(v);
    nova.id = index_num;
    //nova.id = id(nova.nome, nova.artista, nova.tempo);
    //printf("%s | %s | %d | %d | %s\n", nova.nome, nova.artista, nova.id, nova.tempo, nova.album);
    if(fwrite(&nova, sizeof(musica), 1, gravados) == 1)
        printf("Dados gravados com sucesso!\n");
    
    fclose(gravados);

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

    /* while (fread(&excluir, sizeof(musica), 1, gravados)) {

        if (excluir.id == id_remove) {

            encontrou = 1;
            printf("Musica com ID %d excluida.\n", id_remove);

        } else fwrite(&excluir, sizeof(musica), 1, aux);

    } if (!encontrou) printf("ID %d não encontrado. Tente novamente.\n", escolha); */

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

void bubbleTypeSort(musica *vector, int type, int size) { // Um bubble sort capaz de sortear por diferentes condições
    int ordened = 0, sortByValue1, sortByValue2;
    musica aux, comp1, comp2;

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
    return strcmp(musicaA->nome, musicaB->nome);
}

int sortCompArtist(const void *a, const void *b) { // auxiliar de qsort para campo artista
    const musica *musicaA = (const musica *)a;
    const musica *musicaB = (const musica *)b;
    return strcmp(musicaA->artista, musicaB->artista);
}

int sortCompAlbum(const void *a, const void *b) { // auxiliar de qsort para campo album
    const musica *musicaA = (const musica *)a;
    const musica *musicaB = (const musica *)b;
    return strcmp(musicaA->album, musicaB->album);
}