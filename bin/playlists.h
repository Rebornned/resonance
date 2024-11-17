#ifndef FINAL_PROJECT_PROG2_PLAYLISTS_PLAYLISTS_H
#define FINAL_PROJECT_PROG2_PLAYLISTS_PLAYLISTS_H

#include <math.h>

/* A ordem das músicas armazenadas nas playlists importa e deve ser modificável, organizável das
 seguinte formas: em ordem de inclusão na playlist, em ordem alfabética de nome, em ordem
 alfabética de artista ou em ordem crescente de duração. */
typedef struct {

    char nome[200];
    char artista[200];
    int tempo;
    int id;

} musica;

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

void nova_playlist (void) {

    // a lista vai ser criada aq
    
    // colocar no final do id de cada musica na playlist um número
    // indicando a ordem de inserção

}

void ord_nome () { // vai receber a estrutura de lista como parametro

    // a ideia é que a lista já vai ta ordenada em ordem de inserção,
    // então vai ser reorganizada de acordo com oq o usuario quer

}

void ord_artista () { // vai receber a estrutura de lista como parametro



}

void ord_tempo () { // vai receber a estrutura de lista como parametro



}


#endif //FINAL_PROJECT_PROG2_PLAYLISTS_PLAYLISTS_H
