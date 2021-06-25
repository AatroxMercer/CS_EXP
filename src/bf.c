#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#define EOF_BF 0
#define SIZE 32768

char cells[SIZE];
FILE* fin;

int past(FILE* fin)
{
    int cnt = 1;
    while (cnt) {
        switch (fgetc(fin)) {
        case '[':
            ++cnt;
            break;
        case ']':
            --cnt;
            break;
        case EOF:
            return 1;
        }
    }
    return 0;
}

int back(FILE* fin)
{
    int cnt = 1;
    while (cnt) {
        fseek(fin, -2, SEEK_CUR);
        switch (fgetc(fin)) {
        case '[':
            --cnt;
            break;
        case ']':
            ++cnt;
            break;
        case EOF:
            return 1;
        }
    }
    return 0;
}

void bf(FILE* fin)
{
    char* pointer = cells;

    for (int cmd; (cmd = fgetc(fin)) != EOF;) {
        switch (cmd) {
        case '>':
            ++pointer;
            break;
        case '<':
            --pointer;
            break;
        case '+':
            ++*pointer;
            break;
        case '-':
            --*pointer;
            break;
        case '.':
            putchar(*pointer);
            break;
        case ',':
            if ((*pointer = getchar()) == EOF) {
                *pointer = EOF_BF;
            }
			if (*pointer == '!') {
				exit(0);
            }
            break;
        case '[':
            if (*pointer == 0 && past(fin)) {
                return;
            }
            break;
        case ']':
            if (*pointer != 0 && back(fin)) {
                return;
            }
            break;
        case EOF:
            return;
        }
    }
}

void destruct(int sig)
{
    fclose(fin);
    system("/bin/stty icanon isig");
    puts("");
    exit(0);
}

int main(int argc, char* argv[])
{
    signal(SIGINT, destruct);
    system("/bin/stty -icanon");
    if (argc != 2) {
        fputs("Usage: bf <src_path>\n", stderr);
        return 1;
    }
    fin = fopen(argv[1], "r");
    if (fin == NULL) {
        perror("fopen() failed");
        return 1;
    }
    bf(fin);
    destruct(SIGINT);
}
