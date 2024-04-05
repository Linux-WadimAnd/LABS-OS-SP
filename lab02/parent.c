#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>


//export CHILD_PATH=/home/wadimandrianov/working_dir/lab02/papka/child    -   Создать переменную окружения перед запуском программы родительской

//env_var_only_name.txt   -   ХРАНИТ НУЖНЫЕ ПЕРЕМЕННЫЕ ОКРУЖЕНИЯ

//argv[1] - путь к файлу с нужными env    /home/wadimandrianov/working_dir/lab02/env_var_only_name.txt

extern char **environ; 

char* create_name_child(int* count);
void function(char* child_file_path, int* count, char* env[], char *argv[]);

int main(int argc, char *argv[], char** envp) {
    setlocale(LC_ALL, "Rus");
   
    if(argc != 2){
        printf("Неверное колличество аргументов командной строки(их %d)\n", argc);
        exit(1);
    }

    //ВЫВОДИМ ПЕРЕМЕННЫЕ ОКРУЖЕНИЯ В КОТОРЫЕ ВНЕСЛИ ЕЩЁ И CHILD_PATH
    char** cur = envp;
    while((*cur) != NULL){
        printf("%s\n", *cur);
        cur++;
    }
    printf("\n\n=============================================\n\n");
    char* env[] = {
        "SHELL=/bin/bash",
        "HOSTNAME=fedora",
        "PWD=/home/wadimandrianov/working_dir/lab02",
        "LOGNAME=wadimandrianov",
        "HOME=/home/wadimandrianov",
        "LANG=ru_RU.UTF-8",
        "TERM=xterm-256color",
        "USER=wadimandrianov",
        "PATH=/home/wadimandrianov/.local/bin:/home/wadimandrianov/bin:/usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin",
        NULL
        };
        
    int count = 0;  
    while (1)
    {
        char c = getchar();
        if(c == '\n')
            continue;
        else if(c == '+'){
        
           char* child_file_path = (char*)malloc(255);
           char* buf = getenv("CHILD_PATH");
           if(buf == NULL){
            printf("Invalid CHILD_PATH\n");
            exit(1);
           }

           strcpy(child_file_path, buf);

           function(child_file_path, &count, env, argv);

            free(child_file_path);
            printf("\n*********************************\n");
        }else if(c == '*'){
            
            char* child_file_path = (char*)malloc(255);
            char** cur = envp;
            while((*cur) != NULL && strcmp((*cur), "CHILD_PATH=/home/wadimandrianov/working_dir/lab02/papka/child") != 0){
             cur++;
            }

            if((*cur) == NULL){
                printf("CHILD_PATH=/home/wadimandrianov/working_dir/lab02/papka/child NOT FOUND");
                exit(1);
            }
            for(int i = 11, j = 0; i < strlen(*cur); i++, j++){
                child_file_path[j] = (*cur)[i]; ////////////////////////////
                child_file_path[j+1] = '\0';
            }

            function(child_file_path, &count, env, argv);

            free(child_file_path);
            printf("\n*********************************\n");
        }else if(c == '&'){
           
            char* child_file_path = (char*)malloc(255);
            char** cur = environ;
            while(strcmp((*cur), "CHILD_PATH=/home/wadimandrianov/working_dir/lab02/papka/child") != 0 && (*cur) != NULL){
             cur++;
            }

            if((*cur) == NULL){
                printf("CHILD_PATH=/home/wadimandrianov/working_dir/lab02/papka/child NOT FOUND");
                exit(1);
            }
           for(int i = 11, j = 0; i < strlen(*cur); i++, j++){
                child_file_path[j] = (*cur)[i]; ////////////////////////////
                child_file_path[j+1] = '\0';
            }

            function(child_file_path, &count, env, argv);
            free(child_file_path);
            printf("\n*********************************\n");

        }else if(c == 'q'){
            //Символ «q» завершает выполнение родительского процесса.

            break;
        }else{
            printf("Unknown command\n");
        }
    }
   
    exit(0);
    return 0;
}

char* create_name_child(int* count){
    char* name = (char*)malloc(9);
    sprintf(name, "child_%02d", *count);
    (*count)++;
    return name;
}

void function(char* child_file_path, int* count, char* env[], char* argv[]){
    
    char* name_child = create_name_child(count);
                  
    char* path_file_with_env = (char*)malloc(255);
    strcpy(path_file_with_env, argv[1]);
    char* child_argv[] = {name_child, path_file_with_env, NULL};

    pid_t pid;
    pid = fork();
    if(pid == -1){
    printf("Не удалось создать дочерний процесс\n");
        exit(1);
    }else if(pid == 0){
        //дочерний процесс
        execve(child_file_path, child_argv, env);
    }else{
        //родительский процесс
        pid_t childwait;
        wait(&childwait);
    }
    free(path_file_with_env);
    free(name_child);
}