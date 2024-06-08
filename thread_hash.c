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
//int algo_count_per_thread[ALGORITHM_MAX] = {0};
static int algo_count[ALGORITHM_MAX] = {0};
static int total_count = 0;
//static char *crypt_password = NULL;
static int num_password_lines = 0;
static int num_text_lines = 0;

void parse_password(const char *filename);
void parse_text(const char *filename);
void *compare(void *arg);
void create_thread(int num_threads);
int get_next_row(void);
double elapse_time(struct timeval *, struct timeval *);
void loppy(char * password, FILE *out_file, int algo_count_per_thread[]);

int main(int argc, char *argv[])
{
    int opt = -1;
    int n = 0;
    int num_threads = 1;
    int output_trigger = 0;
    char *input_file = NULL;
    char *dictionary_file = NULL;
    char *output_file = NULL;
    FILE * out_file = NULL;
    pthread_t * threads = NULL;

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
            case 'n':
            {
                n = 1;
            }
            break;
            case 'o':
            {
                output_trigger = 1;
                output_file = optarg;
            }
                break;
            case 'd':
                dictionary_file = optarg;
                break;

        }
    }
     if(n == 1){
        nice(10);
    }
    if(output_trigger == 1){
        out_file = fopen(output_file, "a");
        if(out_file == NULL){
            perror("error opening output file");
            exit(EXIT_FAILURE);
        }
    }
    parse_text(dictionary_file); 
    parse_password(input_file);

    threads = malloc(num_threads * sizeof(pthread_t));

    for (int i = 0; i < num_threads; i++)
    {
        pthread_create(&threads[i], NULL, compare, (void *)out_file);
    }

    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
        fprintf(stderr, "thread: %lu", threads[i]);
   }
    for(hash_algorithm_t i = DES; i < ALGORITHM_MAX; i++)
    {
        fprintf(stderr, "%15s: %5d  ", algorithm_string[i], algo_count[i]);
    }

    if(out_file){
        fclose(out_file);
    }

    //get_next_row();
    free(threads);
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

void loppy(char * password, FILE *out_file, int algo_count_per_thread[]){
    struct crypt_data crypt_ob;
    char * crypt_password = NULL;
    static pthread_mutex_t loppy_lock = PTHREAD_MUTEX_INITIALIZER;
    memset(&crypt_ob, 0, sizeof(crypt_ob));
    strncpy(crypt_ob.setting, password, CRYPT_OUTPUT_SIZE);
    for(int j = 0; j < num_text_lines; j++){
        strncpy(crypt_ob.input, text_lines[j], CRYPT_MAX_PASSPHRASE_SIZE);
        crypt_password = crypt_rn(text_lines[j], password, &crypt_ob, sizeof(crypt_ob));
        if(crypt_password != NULL){
            if(strcmp(crypt_password, password) == 0){
                pthread_mutex_lock(&loppy_lock);
                if(password[DES] != '$'){
                    algo_count[DES]+=1;
                    algo_count_per_thread[DES]+=1;
                }
                else if(password[1] == '3'){
                    algo_count[NT]+=1;
                    algo_count_per_thread[NT]+=1;
                }
                else if(password[1] == '1'){
                    algo_count[MD5]+=1;
                    algo_count_per_thread[MD5]+=1;
                }
                else if(password[1] == '5'){
                    algo_count[SHA256]+=1;
                    algo_count_per_thread[SHA256]+=1;
                }
                else if(password[1] == '6'){
                    algo_count[SHA512]+=1;
                    algo_count_per_thread[SHA512]+=1;
                }
                else if(password[1] == 'y'){
                    algo_count[YESCRYPT]+=1;
                    algo_count_per_thread[YESCRYPT]+=1;
                }
                else if(password[1] == 'g'){
                    algo_count[GOST_YESCRYPT]+=1;
                    algo_count_per_thread[GOST_YESCRYPT]+=1;
                }
                else if(password[2] == 'b'){
                    algo_count[BCRYPT]+=1;
                    algo_count_per_thread[BCRYPT]+=1;
                }

                if(out_file){
                    fprintf(out_file, "cracked  %s  %s\n", text_lines[j], password);
                }
                else{
                    fprintf(stdout, "cracked  %s  %s\n", text_lines[j], password);
                }
                ++total_count;
                pthread_mutex_unlock(&loppy_lock);
                //Might need a mutex lock for now 
                break;
            }
        }
    }
}

//compare password that crypt rn returns and the password I passed in 
void *compare(void *arg){
    int i = 0;
    int tot_count_per_thread = 0;
    int algo_count_per_thread[ALGORITHM_MAX] = {0};
    static pthread_mutex_t compare_lock = PTHREAD_MUTEX_INITIALIZER;
    FILE *out_file = (FILE *)arg; 
    do{
        i = get_next_row();
        if(i < num_password_lines){
            loppy(password_lines[i], out_file, algo_count_per_thread);
        }

    } while(i < num_password_lines);
    pthread_mutex_lock(&compare_lock);
    tot_count_per_thread = 0;
    for(hash_algorithm_t j = DES; j < ALGORITHM_MAX; j++)
    {
        fprintf(stderr, "%15s: %5d  ", algorithm_string[j], algo_count_per_thread[j]);
        tot_count_per_thread+= algo_count_per_thread[j];
    }
    fprintf(stderr, "total: %8d\n", tot_count_per_thread);
    pthread_mutex_unlock(&compare_lock);
    pthread_exit(NULL);
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


