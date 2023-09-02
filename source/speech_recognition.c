#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>

#define DEBUG 0

#if DEBUG
    #define TRACE(text) printf("TRACE: File: %s, Line: %d, msg = \"%s\"\n", __FILE__, __LINE__, text)
#else
    #define  TRACE(text)
#endif

#define DURATION 120

double duration;

void cut_file(char *name);
void invoke_python();
void merge_results();

int main(int argc, char *argv[])
{
    if(argc < 2) {
        fprintf(stderr, "invoke with \"speech_recognition <file_name>\"\n");
        exit(1);
    }
    char buffer[500];
    FILE *info;
    sprintf(buffer, "ffprobe -i \"%s\" -show_entries format=duration -v quiet -of csv=\"p=0\" > duration.txt", argv[1]);
    TRACE("");
    system(buffer);
    info = fopen("duration.txt", "r");
    fscanf(info, "%lf", &duration);
    fclose(info);
    system("rm duration.txt");
    TRACE("");
    printf("cutting file...\n");
    cut_file(argv[1]);
    printf("cut finished\n");
    TRACE("");
    invoke_python();
    TRACE("");
    printf("merging results...\n");
    merge_results();
    printf("** FINISH **\n");
    return 0;
}

void cut_file(char *name)
{
    char name_tmp[10];
    char command[500];
    int i;
    int n_cut = duration / DURATION;
    TRACE("");
    printf("cutted: 0/%d\n", n_cut); 
    for(i = 0; i < n_cut; i++) {
        int start = i * DURATION;
        int dur = DURATION + 1;
        if(i == n_cut - 1)
            dur = DURATION * 2;
        sprintf(command, "ffmpeg -i \"%s\" -ss %d -t %d %d.wav -y -hide_banner -loglevel error", name, start, dur, i);
        //printf("%s\n", command);
        TRACE("");
        system(command);
        printf("cutted: %d/%d\n", i + 1, n_cut); 
    } 
}

void invoke_python() 
{
    int i;
    int n_cut = duration / DURATION;
    for(i = 0; i < n_cut; i ++) {
        int pid = fork();
        if(pid == 0) {
            char command[500];
            sprintf(command, "python source/recognition.py %d %d", i, DURATION);
            //printf("%s", command);
            system(command);
            exit(0);
        }
    }
    printf("recognised: 0/%d\n", n_cut);
    for(i = 0; i < n_cut; i ++) {
        wait(NULL);
        printf("recognised: %d/%d\n", i+1, n_cut);
    }
    //printf("hanno finito tutti\n");
}

void merge_results()
{
    int i;
    int n_cut = duration / DURATION;
    FILE *out = fopen("out.txt", "w");
    char buffer[1000000];
    fprintf(out, "Testo riconosciuto:\n");
    for(i = 0; i < n_cut; i++) {
        char name_in[50];
        FILE *in;
        sprintf(name_in, "%d.txt", i);
        in = fopen(name_in, "r");
        while(fgets(buffer, 1000000, in) != NULL) {
            fprintf(out, "%s", buffer);
        }
        fclose(in);
        sprintf(buffer, "rm %s", name_in);
        system(buffer);
    }
    fprintf(out, "\n\nTesto non riconosciuto:\n");
    for(i = 0; i < n_cut; i++) {
        char name_in[50];
        FILE *in;
        sprintf(name_in, "%d_failed.txt", i);
        in = fopen(name_in, "r");
        while(fgets(buffer, 1000000, in) != NULL) {
            fprintf(out, "%s", buffer);
        }
        fclose(in);
        sprintf(buffer, "rm %s", name_in);
        system(buffer);
    }
}
