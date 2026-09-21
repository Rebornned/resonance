/*
 * build_database - builds the song database used by Resonance.
 *
 * Reads a text file with four lines per song:
 *     title
 *     album
 *     artist
 *     duration (m:ss)
 * and writes one fixed-size binary record per song (struct musica),
 * using the song's position in the file as its ID.
 *
 * Usage: build_database <input.txt> <output.bin>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/library.h"

#define LINE_SIZE 512

/* Reads one line, strips the trailing "\n" or "\r\n". Returns 0 at end of file. */
static int read_line(FILE *in, char *line, int *line_number) {
    if (fgets(line, LINE_SIZE, in) == NULL)
        return 0;
    (*line_number)++;
    line[strcspn(line, "\r\n")] = '\0';
    return 1;
}

/* Copies a text field, rejecting empty values and values that do not fit. */
static int copy_field(char *dest, size_t dest_size, const char *src,
                      const char *field, int line_number) {
    size_t length = strlen(src);
    if (length == 0 || length >= dest_size) {
        fprintf(stderr, "line %d: %s must have 1 to %zu characters\n",
                line_number, field, dest_size - 1);
        return 0;
    }
    memcpy(dest, src, length + 1);
    return 1;
}

/* Parses "m:ss" into seconds. Returns -1 if the format is invalid. */
static int parse_duration(const char *text) {
    int minutes, seconds;
    char extra;
    if (sscanf(text, "%d:%d%c", &minutes, &seconds, &extra) != 2)
        return -1;
    if (minutes < 0 || seconds < 0 || seconds > 59)
        return -1;
    return minutes * 60 + seconds;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <input.txt> <output.bin>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *in = fopen(argv[1], "r");
    if (in == NULL) {
        perror(argv[1]);
        return EXIT_FAILURE;
    }

    /* Songs are validated and kept in memory first, so an invalid
       input never leaves a half-written database behind. */
    size_t capacity = 128, count = 0;
    musica *songs = malloc(capacity * sizeof(musica));
    if (songs == NULL) {
        fclose(in);
        fprintf(stderr, "out of memory\n");
        return EXIT_FAILURE;
    }

    char title[LINE_SIZE], album[LINE_SIZE], artist[LINE_SIZE], duration[LINE_SIZE];
    int line_number = 0, ok = 1;

    while (read_line(in, title, &line_number)) {
        if (title[0] == '\0')
            continue; /* ignore blank lines between or after records */

        if (!read_line(in, album, &line_number) ||
            !read_line(in, artist, &line_number) ||
            !read_line(in, duration, &line_number)) {
            fprintf(stderr, "line %d: incomplete record (expected 4 lines)\n", line_number);
            ok = 0;
            break;
        }

        if (count == capacity) {
            capacity *= 2;
            musica *grown = realloc(songs, capacity * sizeof(musica));
            if (grown == NULL) {
                fprintf(stderr, "out of memory\n");
                ok = 0;
                break;
            }
            songs = grown;
        }

        musica *song = &songs[count];
        memset(song, 0, sizeof(*song)); /* no uninitialized bytes in the file */

        if (!copy_field(song->nome, sizeof(song->nome), title, "title", line_number - 3) ||
            !copy_field(song->album, sizeof(song->album), album, "album", line_number - 2) ||
            !copy_field(song->artista, sizeof(song->artista), artist, "artist", line_number - 1)) {
            ok = 0;
            break;
        }

        song->tempo = parse_duration(duration);
        if (song->tempo < 0) {
            fprintf(stderr, "line %d: invalid duration \"%s\" (expected m:ss)\n",
                    line_number, duration);
            ok = 0;
            break;
        }

        song->id = (int)count;
        count++;
    }
    fclose(in);

    if (!ok || count == 0) {
        if (ok)
            fprintf(stderr, "%s: no songs found\n", argv[1]);
        free(songs);
        return EXIT_FAILURE;
    }

    FILE *out = fopen(argv[2], "wb");
    if (out == NULL) {
        perror(argv[2]);
        free(songs);
        return EXIT_FAILURE;
    }
    size_t written = fwrite(songs, sizeof(musica), count, out);
    int close_failed = fclose(out) != 0;
    free(songs);

    if (written != count || close_failed) {
        fprintf(stderr, "%s: write error\n", argv[2]);
        return EXIT_FAILURE;
    }

    printf("%zu songs written to %s\n", count, argv[2]);
    return EXIT_SUCCESS;
}
