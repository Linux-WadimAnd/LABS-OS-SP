#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#include <signal.h>
#include <time.h>

volatile struct two_words {
    int a;
    int b;
} two_words;

struct statistic{
    int two_0;
    int two_1;
    int para_0_1;
    int para_1_0;
}statistic;

struct statistic latest_stat = {0, 0, 0, 0};


struct two_words m;

//SIGUSR1 - Разрешает вывод
//SIGUSR2 - Запрещает вывод

bool flag_the_end_loop = true;

bool permission_to_display_statistics = false; //Разрешение на вывод статистики   | false - нелья 

void handler_sigusr1(int sig){
    //printf("Я в child обработчике handler_sigusr1(разрешаю вывод) процессора \n", getpid());
    permission_to_display_statistics = true;
}

void handler_sigusr2(int sig){
    //printf("Я в child обработчике handler_sigusr2(запрещаю вывод) проессора \n", getpid());
    permission_to_display_statistics = false;
}

void handler_alarm(int signum) {
    flag_the_end_loop = false;
            if(m.a == 0 && m.b == 0){
                latest_stat.two_0++;
            }else if(m.a == 1 && m.b == 1){
                latest_stat.two_1++;
            }else if(m.a == 0 && m.b == 1){
                latest_stat.para_0_1++;
            }else{
                latest_stat.para_1_0++;
            }
}

void display_statistic(struct statistic);

union sigval value_pid;

int main (int argc, char *argv[], char* envp[]) {
    struct two_words zeros = { 0, 0 }, ones = { 1, 1 };
    signal(SIGALRM, handler_alarm); 
    signal(SIGUSR1, handler_sigusr1);
    signal(SIGUSR2, handler_sigusr2);
    
    int j = 0;
    m = zeros;
    struct timespec time;
    time.tv_sec = 0;
    time.tv_nsec = 0;
  
    while(j != 20){
        alarm(1);
        while (flag_the_end_loop == true) {
            m = zeros;
            //nanosleep(&time, NULL);
            m = ones;
        }
        j++;
        flag_the_end_loop = true;
    }
    if(permission_to_display_statistics == false){ //Не было разрешения дано из родительского процесса
        printf("Дочерний процесс с PID = %d запрашивает разрешение на вывод\n", getpid());
        //raise(SIGSTOP);
        while(1)
        {
                if(permission_to_display_statistics == true){
                display_statistic(latest_stat);
                //printf("Дочерний процесс %d завершён: PPID = %d\n (0, 0) = %d\n (1,1) = %d\n (0,1) = %d\n (1, 0) = %d\n", getpid(), getppid(), 
                //latest_stat.two_0, latest_stat.two_1, latest_stat.para_0_1, latest_stat.para_1_0);
                break;
                }
        }
    }else{
        display_statistic(latest_stat);
    }
    value_pid.sival_int =  getpid();
    sigqueue(getppid(), SIGUSR2, value_pid);
    //sleep(30); //ЗА ЭТО ВРЕМЯ МОЖНО ОТПРАВИТЬ SIGKILL ЭТОМУ ПРОЦЕССУ
    raise(SIGSTOP);
    return 1;
}

void display_statistic(struct statistic stat){

    char statistic_str[256];
    sprintf(statistic_str, "Дочерний процесс PID = %d, PPID = %d\n (0, 0) = %d\n (1,1) = %d\n (0,1) = %d\n (1, 0) = %d\n", (int)getpid(), (int)getppid(), 
        latest_stat.two_0, latest_stat.two_1, latest_stat.para_0_1, latest_stat.para_1_0);

    for(int i = 0; i < strlen(statistic_str); i++){
        fputc(statistic_str[i], stdout);
    }

}