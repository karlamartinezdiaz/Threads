#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <crypt.h>
#include <pthread.h>
#include <time.h>
#include "thread_hash.h"

static char *password_lines[BUF_SIZE];
static char *text_lines[BUF_SIZE];
//static char *crypt_password = NULL;
static int num_password_lines = 0;
static int num_text_lines = 0;

void parse_password(const char *filename);
void parse_text(const char *filename);
void *compare(void *arg);
void create_thread(int num_threads);
int get_next_row(void);
double elapse_time(struct timeval *, struct timeval *);
void loppy(char * password);

int main(int argc, char *argv[])
{
    int opt = -1;
    int num_threads = 1;
    char *input_file = NULL;
    char *dictionary_file = NULL;
    
    pthread_t * threads = NULL;
    //thread_stats_t *stats;
    //struct timeval start, end;
    //double total_elapsed;
    //char *output_file = NULL;

    while((opt = getopt(argc, argv, OPTIONS)) != -1)
    {
        switch(opt)
        {
            case 'h':
                fprintf(stderr, "help text\n");
                fprintf(stderr, "\t/u/rchaney/Classes/cs333/Labs.Lab4/./thread_hash ...\n");
                fprintf(stderr, "\tOptions: i:o:d:hvt:n\n");
                fprintf(stderr, "\t\t-i file\t\tinput file name (required)\n");
                fprintf(stderr, "\t\t-o file\t\toutput file name (default stdout)\n");
                fprintf(stderr, "\t\t-d file\t\tdictionary file name (default stdout)\n");
                fprintf(stderr, "\t\t-t #\t\tnumber of threads tp create (default 1)\n");
                fprintf(stderr, "\t\t-v\t\tenable verbose mode\n");
                fprintf(stderr, "\t\t-h\t\thelpful text\n");
                break;
            case 't':
                num_threads = atoi(optarg);
                if(num_threads <= 0){
                    num_threads = 1;
                }
                break;
            case 'i':
            {
                input_file = optarg;

            }
            break;
            case 'o':
                //output_file = optarg;
                break;
            case 'd':
                dictionary_file = optarg;
                break;

        }
    }

    parse_text(dictionary_file); 
    parse_password(input_file);

    //pthread_t threads[num_threads];
    //thread_stats_t stats[num_threads];
    //compare();

    threads = malloc(num_threads * sizeof(pthread_t));
    
    for (int i = 0; i < num_threads; i++)
    {
        pthread_create(&threads[i], NULL, compare, NULL);
    }

    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }
    //get_next_row();

    return EXIT_SUCCESS;
}


//parse in file(line by line)
void parse_password(const char *filename){
    FILE *file = fopen(filename, "r");
    int file_size = 0;
    char *file_stuff = NULL;
    char *line = NULL;
    if(!file){
        perror("Failed to open file");
        return;
    }
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    file_stuff = malloc(file_size + 1);
    if (!file_stuff) {
        perror("mehhh cant allocate memory");
        fclose(file);
        return;
    }

    fread(file_stuff, 1, file_size, file);
    file_stuff[file_size] = '\0';
    fclose(file);
    line = strtok(file_stuff, "\n");
    while (line) {
        password_lines[num_password_lines] = strdup(line);
        if (!password_lines[num_password_lines]) {
            perror("MEHHHH");
            break;
        }
        num_password_lines++;
        //printf("%s\n", line);
        if (num_password_lines >= BUF_SIZE) {
            fprintf(stderr, "Thats too much bro  (%d).", BUF_SIZE);
            break;
        }
        line = strtok(NULL, "\n");
    }

    free(file_stuff);
}

void parse_text(const char *filename){
    FILE *file = fopen(filename, "r");
    int file_size = 0;
    char *file_stuff = NULL;
    char *line = NULL;
    if(!file){
        perror("Failed to open file");
        return;
    }
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    file_stuff = malloc(file_size + 1);
    if (!file_stuff) {
        perror("mehhh cant allocate memory");
        fclose(file);
        return;
    }

    fread(file_stuff, 1, file_size, file);
    file_stuff[file_size] = '\0';
    fclose(file);

    line = strtok(file_stuff, "\n");
    while (line) {
        text_lines[num_text_lines] = strdup(line);
        if (!text_lines[num_text_lines]) {
            perror("MEHHHH while");
            break;
        }
        num_text_lines++;
        //printf("%s\n", line);
        if (num_text_lines >= BUF_SIZE) {
            fprintf(stderr, "Thats too much bro  (%d).", BUF_SIZE);
            break;
        }
        line = strtok(NULL, "\n");
    }

    free(file_stuff);
}

void loppy(char * password){
    struct crypt_data crypt_ob;
    char * crypt_password = NULL;
    memset(&crypt_ob, 0, sizeof(crypt_ob));
    strncpy(crypt_ob.setting, password, CRYPT_OUTPUT_SIZE);
    for(int j = 0; j < num_text_lines; j++){

        strncpy(crypt_ob.input, text_lines[j], CRYPT_MAX_PASSPHRASE_SIZE);
        crypt_password = crypt_rn(text_lines[j], password, &crypt_ob, sizeof(crypt_ob));
        if(crypt_password != NULL){
            if(strcmp(crypt_password, password) == 0){
                printf("cracked %s\t%s\n", text_lines[j], password);
                break;
            }
        }
    }
}

//compare password that crypt rn returns and the password I passed in 
void *compare(void *arg){
    int i = 0;
    
    do{
        i = get_next_row();
        if(i < num_password_lines){
            loppy(password_lines[i]);
        }

    } while(i < num_password_lines);



    
    /*
    while(get_next_row() < num_password_lines){
        memset(&crypt_ob, 0, sizeof(crypt_ob));

        if(password_lines[i] == NULL || text_lines[i] == NULL){
            fprintf(stderr, "Arrays empty\n");
            continue;
        }

        strncpy(crypt_ob.setting, password_lines[i], CRYPT_OUTPUT_SIZE);
        for(int j = 0; j < num_text_lines; j++){
            strncpy(crypt_ob.input, text_lines[j], CRYPT_MAX_PASSPHRASE_SIZE);
            crypt_password = crypt_rn(text_lines[j], password_lines[i], &crypt_ob, sizeof(crypt_ob));
            if(strcmp(crypt_password, password_lines[i]) == 0){
                printf("cracked %s\t%s\n", text_lines[j], password_lines[i]);
                break;
            }
        }

        ++i;
    }

        */
    pthread_exit(NULL);
    //return NULL;
}

int get_next_row(void){
    static int next_index = 0;
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    int cur_index = 0;

    pthread_mutex_lock(&lock);
    cur_index = next_index++;
    pthread_mutex_unlock(&lock);
    return cur_index;
}

double elapse_time(struct timeval *t0, struct timeval *t1){
    double et = (((double) (t1->tv_usec - t0->tv_usec))
                / MICROSECONDS_PER_SECOND)
        + ((double) (t1->tv_sec - t0->tv_sec));
    return et;

}


//Crypt_rn will take in 4 args 
//---1st is a string (Plain text), one Line 
//---2nd is string (password), one line at a time 
//---3rd an object of crypt stuff, make the struct 
//---4th is the size of stuff 
//Crypt_rn will return me another password 
//that password is generated by using 
//crypt_stuff.setting and crypt_stuff.input 


