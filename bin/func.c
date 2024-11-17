#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* A ordem das músicas armazenadas nas playlists importa e deve ser modificável, organizável das
 seguinte formas: em ordem de inclusão na playlist, em ordem alfabética de nome, em ordem
 alfabética de artista ou em ordem crescente de duração. */
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

void gravador (void) { // função para gravar as músicas no arq binário

    musica nova;
    int v[1][2]; // tempo da musica antes de transformar em segundos

    FILE *gravados = fopen("gravados.dat", "w");
    if (gravados == NULL) printf("Nao foi possivel abrir o arquivo de musicas.");

    printf("Insira o nome da musica: \n");
    fgets(nova.nome, sizeof(nova.nome), stdin);
    nova.nome[strcspn(nova.nome, "\n")] = 0;

    printf("Insira o nome do artista: \n");
    fgets(nova.artista, sizeof(nova.artista), stdin);
    nova.artista[strcspn(nova.artista, "\n")] = 0;

    printf("Insira o tempo da musica no formato [mm:ss]: \n");
    scanf("%d:%d", &v[0][0], &v[0][1]);

    nova.tempo = segundos(v);
    nova.id = id(nova.nome, nova.artista, nova.tempo);

    fwrite(&nova, sizeof(musica), 1, gravados);

    fclose(gravados);

}

musica select_mostruario (playlist *pl) { // seleciona as musicas pra adicionar na playlist

    FILE *gravados = fopen("gravados.dat", "rb");
    if (!gravados) printf("Nao foi possível abrir o arquivo de musicas.\n");

    musica aux;
    musica *selecionadas = malloc(sizeof(musica) * (pl->tamanho+1)); // vetor de struct pra armazenar as musicas selecionadas
    if (!selecionadas) {

        printf("Erro ao alocar memoria para as músicas selecionadas.\n");
        fclose(gravados);

        return NULL;
    }
    int escolha, encontrou, continuar = 1, contador = -1;

    printf("\nDigite os IDs das musicas que deseja adicionar a playlist (pressione 0 para finalizar):\n");

    while (continuar) {

        printf("ID: ");
        scanf("%d", &escolha);

        if (escolha == 0) {

            printf("Finalizando a seleção.\n");
            continuar = 0;

        } else { // reinicia a busca no arquivo para localizar a música pelo id

            rewind(gravados);
            encontrou = 0;

            while (fread(&aux, sizeof(musica), 1, gravados)) {

                if (aux.id == escolha) {

                    selecionadas[contador++] = aux;
                    printf("Música '%s' adicionada à playlist.\n", aux.nome);
                    encontrou = 1;

                    break;
                }

            } if (!encontrou) printf("ID %d não encontrado. Tente novamente.\n", escolha);

        }

    }

    fclose(gravados);

    return *selecionadas;

}

void nova_playlist (playlist *nova) { // a lista vai ser criada aq

    // colocar no final do id de cada musica na playlist um número indicando a ordem de inserção
    //gravada.id = add(gravada.id, nova->tamanho);

    musica *musicas_selecionadas = select_mostruario(nova); // Seleciona as músicas a serem adicionadas
    if (!musicas_selecionadas) {

        printf("Nenhuma música selecionada para adicionar à playlist.\n");
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

        if (nova->inicio == NULL) {

            nova->inicio = dado;
            nova->fim = dado;

        } else {

            nova->fim->prox = dado;
            nova->fim = dado;

        }

        nova->tamanho++;

    }

    free(musicas_selecionadas);

}

void ord_nome (playlist *pl) { // vai receber a estrutura de lista como parametro

    // a ideia é que a lista já vai ta ordenada em ordem de inserção,
    // então vai ser reorganizada de acordo com oq o usuario quer

}

void ord_artista (playlist *pl) { // vai receber a estrutura de lista como parametro



}

void ord_tempo () { // vai receber a estrutura de lista como parametro



}