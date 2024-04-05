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

#define MAX_CHILDREN 10 //МАКСИМАЛЬНОЕ КОЛ-ВО ДОЧЕРНИХ ПРОЦЕССОВ

pid_t children[MAX_CHILDREN] = { 0 }; //ХРАНИТ PID ДОЧЕРНИХ ПРОЦЕССОВ

bool flag = true;
void send_all_child_STOP();
void send_all_child_CONT();

void clear_stdin();//очищает stdin от \n после scanf

void send_all_child_processes_SIGUSR1();
void send_all_child_processes_SIGUSR2();

int delete_last_child_process(); //возвращает кол-во оставшихся процессов

void output_list_processes(pid_t mypid);//выводит все процессы (PID и порядковый номер)

void handler_sigusr2(int signum, siginfo_t *info, void *context);
void handler_sigusr1(int signum, siginfo_t *info, void *context);

void handler_SIGALRM(int sig){
    flag = false;
    printf("5 секунд прошло и вы не ввели g\n");
    //send_all_child_processes_SIGUSR1(); //разрешает всем процессам выводить статистику
    //send_all_child_CONT(); //и продолжая их работу они завершаются и выводят статистику если уже подошли к стопу перед выводом статистики
}

void void_handler(int sig){

}

int permission_one_number_output(int number); //возвращает 0 если нет процесса с таким номером. 1 если всё ок

void create_child_process();
void delete_all_child_process();



//SIGUSR1 - Разрешает вывод при посылании его от Родительского к дочернему прорцессу | Родитель по
//SIGUSR2 - Запрещает вывод при посылании его от Родительского к дочернему прорцессу | 
int main(int argc, char *argv[], char** envp){
    setlocale(LC_ALL, "Rus");

    struct sigaction sa2;
    sa2.sa_sigaction = handler_sigusr2;
    sa2.sa_flags = SA_SIGINFO;
    sigemptyset(&sa2.sa_mask);
    sigaction(SIGUSR2, &sa2, NULL);

    struct sigaction sa1;
    sa1.sa_sigaction = handler_sigusr1;
    sa1.sa_flags = SA_SIGINFO;
    sigemptyset(&sa1.sa_mask);
    sigaction(SIGUSR1, &sa1, NULL);

    pid_t mypid = getpid(); //Не 0, а PID главного(родительского процесса)

    while(true){
        //rewind(stdin);
        clear_stdin();
        sleep(1); 
        printf("ввод: ");
        //char input[10];
        char* input = (char*)malloc(10);
        scanf("%s", input);
        clear_stdin(); //чтобы c = getchar(); не считывал \n
        if(strlen(input) == 1)
        {   
            if(input[0] == '+'){
            // По нажатию клавиши «+» pодительский процесс (P) порождает дочерний процесс (C_k) и
            // сообщает об этом. 
                create_child_process();
            
            }else if(input[0] == '-'){
                //По нажатию клавиши «-» P удаляет последний порожденный C_k, сообщает об этом и о
                //количестве оставшихся.
                printf("Удалён последний порожденный дочерний процесс\nОстанолось %d процессов", delete_last_child_process());

            }else if(input[0] == 'l'){
                //При вводе символа «l» выводится перечень родительских и дочерних процессов
                output_list_processes(mypid);

            }else if(input[0] == 'k'){
                //При вводе символа «k» P удаляет все C_k и сообщает об этом
                delete_all_child_process();
                printf("Все дочерние процессы удалены\n");
            }else if(input[0] == 's'){
                //При вводе символа «s» P запрещает всем C_k выводить статистику
                //kill(-mypid, SIGUSR2);
                 send_all_child_processes_SIGUSR2();

            }else if(input[0] == 'g'){
                //При вводе символа «g» P разрешает всем C_k выводить статистику
                //kill(-mypid, SIGUSR1);
                send_all_child_processes_SIGUSR1();

            }else if(input[0] == 'q'){ 
                //По нажатию клавиши «q» P удаляет все C_k, сообщает об этом и завершается.
                delete_all_child_process();
                //kill(mypid, SIGKILL);
                exit(0);
            }else{
                printf("Неизвестная команда\n");
                continue;
            }
        }
        else if (strlen(input) == 4 && (input[2] >= '0' && input[2] <= '9')){        //_<num>

            if(input[0] == 's'){
                //При вводе символов «s<num>» P запрещает C_<num> выводить статистику.
                if(children[input[2] - 48] != 0){
                    kill(children[input[2] - 48], SIGUSR2);
                    printf("Был отправлен запрет на вывод процессу под идексом %d\n", input[2] - 48);
                }else{
                    printf("Нет такого процесса процессы пока приостановлены, скип - enter и будут возобновлены процессы)\n");
                    //continue;
                }
            }else if(input[0] == 'g'){
                //При вводе символов «g<num>» P разрешает C_<num> выводить статистику.
                 if(children[input[2] - 48] != 0){
                        if(kill(children[input[2] - 48], SIGUSR1) == 0){
                            //kill(children[input[2] - 48], SIGCONT);
                            printf("Было отправлено разрешение на вывод процессу под идексом %d\n", input[2] - 48);
                        }else{
                            printf("Не отправлен сигнал процессу под индексом %c с PID = %d", input[2], children[input[2] - 48]);
                        }
                        //continue;
                   }else{
                       printf("Нет такого процесса\n");
                       //continue;
                   }
            }else if(input[0] == 'p'){
                /*При вводе символов «p<num>» P запрещает всем C_k вывод и запрашивает C_<num> вы-
                вести свою статистику. По истечению заданного времени (5 с, например), если не введен символ
                «g», разрешает всем C_k снова выводить статистику*/
                if(!permission_one_number_output(input[2] - 48)){
                    printf("Нет процессора \n\n\n\n\n\n с таким порядковым номером\n");
                }
                //continue;//////////////////////
            }else{
                printf("Неизвестная команда\n");
                //continue;
            }

        }else{
            printf("Неизвестная команда\n");
            //continue;
        }
        free(input);
    }

    return 0;
}

int permission_one_number_output(int number){  //возвращает 0 если нет процесса с таким номером. 1 если всё ок

                /*При вводе символов «p<num>» P запрещает всем C_k вывод и запрашивает C_<num> вы-
                вести свою статистику. По истечению заданного времени (5 с, например), если не введен символ
                «g», разрешает всем C_k снова выводить статистику*/
    printf("\n================\nnumber = %d, pid = %d\n================\n",number, children[number]);
    if(children[number] == 0)
        return 0;
    
    struct sigaction sa3;
    sa3.sa_handler = handler_SIGALRM;
    sigaction(SIGALRM, &sa3, NULL);

    //send_all_child_STOP();
    send_all_child_processes_SIGUSR2(); //ЗАПРЕТ ВСЕМ НА ВЫВОД

    //
    //if(kill(children[number], SIGSTOP) != 0)
      //  printf("Ошибка отправки сигналла SIGSTOP процессу под индексом %d с PID = %d", number, children[number]);

    fputs("Введите g чтобы сохранить запрет всем остальным процессам на вывод статистики\n(время на нажатие 5 с): ", stdout);
    alarm(5);
    char c = 'a'; 
    while(flag == true)
    {
        c = getchar();
        if(c == 'g'){
            sa3.sa_handler = void_handler; 
            sigaction(SIGALRM, &sa3, NULL);
            //sigaction(SIGALRM, &sa2, NULL); //отключили обработку будильника 5 секунд.  flag остался true
            break;
        }
    }
   
    if(flag == true) //если в условие зашли, то значит не сработал будильник 
    {
        kill(children[number], SIGUSR1); //запрашивает C_<num> вывести свою статистику
    }else{ //сработал будильник
        send_all_child_processes_SIGUSR1(); //разрешает всем процессам выводить статистику
    }
    clear_stdin();
    flag = true;
    return 1;
}


void create_child_process(){
    char* name = (char*)malloc(9); 
    int i = 0;
    for(i = 0; i < MAX_CHILDREN; i++){
        printf("[%d] = %d\n", i, children[i]);
        if(children[i] == 0){
            sprintf(name, "child_%02d", i);
            break;
        }
    }
    char* argv[] = {name, NULL};
    char* envp[] = { NULL };
    pid_t pid = fork();
    if(pid == 0){
        execve("/home/wadimandrianov/working_dir/lab03/child", argv, envp);
        printf("Ошибка с execve\n");
        exit(0);
    }else if(pid < 0){
        printf("Ошибка fork() процесс не создался\n");
    }else{
        printf("%s создан с PID %d\n", name, pid);
        children[i] = pid;
    }
    free(name);
}


void delete_all_child_process(){
    for(int i = 0; i < MAX_CHILDREN; i++){
        if(children[i] != 0){
            if(kill(children[i], SIGKILL) != 0){
                printf("Ошибка отправки SIGKILL процессу под индексом %d с PID = %d", i, children[i]);
            }else{
                printf("Дочерний процесс[%d] с PID %d удалён\n", i, children[i]);
            }
        }
    }
}


void output_list_processes(pid_t mypid){
    printf("Родительский процесс PID %d\n", mypid);
    for(int i = 0; i < MAX_CHILDREN; i++){
        if(children[i] != 0){
            printf("Дочерний процесс[%d]: PID = %d\n", i, children[i]);
        }else{
            break;
        }
    }

}

int delete_last_child_process(){
    if(children[0] == 0){
        return 0;
    }else{
        int i = 0;
        for(i = 0; i < MAX_CHILDREN; i++){
            if(children[i] == 0){
                kill(children[i - 1], SIGKILL);
                children[i - 1] = 0;
                break;
            }
        }
        return (i - 1);
    }
}

void send_all_child_processes_SIGUSR1(){
    pid_t children_lat[MAX_CHILDREN] = { 0 };
    for(int i = 0; i < MAX_CHILDREN; i++)
        children_lat[i] = children[i];

    int i = 0;
    while(children_lat[i] != 0){
        kill(children_lat[i], SIGUSR1);
        //kill(children[i], SIGCONT);
        sleep(1);
        i++;
    }
}

void send_all_child_processes_SIGUSR2(){
     pid_t children_lat[MAX_CHILDREN] = { 0 };
    for(int i = 0; i < MAX_CHILDREN; i++)
        children_lat[i] = children[i];

    int i = 0;
    while(children[i] != 0){
        kill(children[i], SIGUSR2);
        i++;
    }
}

void handler_sigusr2(int signum, siginfo_t *info, void *context){//запрос на разрешение вывода статистики дочернего процесса
                    //info->si_value.sival_int  -  pid дочернего процесса
        if(signum == SIGUSR2){   //УДАЛЯЕМ PID ДОЧЕРНЕГО ПРОЦЕССА ИЗ МАССИВА
 
            for(int i = 0; i < MAX_CHILDREN; i++){
                
                if(children[i] == info->si_value.sival_int){
                
                    while(i < MAX_CHILDREN - 1){
                        children[i] = children[i + 1];
                        i++;                    
                    }
                    children[MAX_CHILDREN - 1] = 0;
                    break;
                }
            }
            if(kill(info->si_value.sival_int, SIGKILL) != 0){
                printf("Ошибка с kill при завершении процесса %d\n", info->si_value.sival_int);
            }
            printf("Дочерний процесс %d завершён и удалён\n", info->si_value.sival_int);
    }else{
        printf("Словили не тот сигнал, ожидался SIGUSR1\n");
    }
}

//вообще не используем
void handler_sigusr1(int signum, siginfo_t *info, void *context){//Дочерний процесс отправил этот сигнал потому что собирается выводить 
                    //info->si_value.sival_int  -  pid дочернего процесса
  if(signum == SIGUSR1){
    if(info->si_value.sival_int != 0){ //НАЧИНАЕМ ВЫВОД, ЗАБЛОКИРОВАТЬ ОСТАЛЬНЫЕ ПРОЦЕССЫ info->si_value.sival_int = PID
        //не используем, зарезервировано

    }else{ //ЗАКОНЧИЛИ ВЫВОД
        send_all_child_CONT();
    }
  }else{
    printf("Словили не тот сигнал, ожидался SIGUSR1\n");
  }
}

void send_all_child_STOP(){
    for(int i = 0; i < MAX_CHILDREN; i++){
        if(children[i] != 0){
            kill(children[i], SIGSTOP);
        }
    }
}

void send_all_child_CONT(){
    
    for(int i = 0; i < MAX_CHILDREN; i++){
        if(children[i] != 0){
            kill(children[i], SIGCONT);
        }
    }
}

void clear_stdin() //очищает stdin от \n после scanf
{
    char c;
    while ((c = getchar()) != '\n' && c != EOF);
}