#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[], char **envp) {

    pid_t pid, ppid;
    pid = getpid();
    ppid = getppid();

    printf("CHILD PROCESS\nname: %s\nPID: %d\nPPID: %d\n", argv[0], pid, ppid);

    FILE* file;
    if((file = fopen(argv[1], "r")) == 0){
        printf("File opening error\n");
        exit(1);
    }else{
        char buf[255];
         while(fgets(buf, sizeof(buf), file) != NULL){
            buf[strcspn(buf, "\n")] = '\0'; // Убираем символ новой строки
            char* env_value = getenv(buf);
            if (env_value != NULL) {
                printf("%s=%s\n", buf, env_value);
            } else {
                printf("%s is not set\n", buf);
            }
        }
        fclose(file);
    }

    return 0;
}