#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <stdbool.h>
#include <sys/stat.h>
//int stat(const char *pathname, struct stat *statbuf); - получить информацию о файле

///////////////////////////
typedef struct{
bool d;
bool f;
bool l;
bool s;
}Marker;

void with_options(int argc, char *argv[]); //ФУНКЦИЯ ОБРАБОТАЕТ ОПЦИИ И ПУТЬ К КАТАЛОГУ
void without_options(char* file_path); //ФУНКЦИЯ ОБРАБОТАЕТ КАТАЛОГ СО ВСЕМИ ОПЦИЯМИ СРАЗУ ПОТОМУ ЧТО ИХ НЕ УКАЗАЛИ
Marker create_marker(char* optargument);
void directory_search_with_marker(char* file_path, Marker marker, char** filenames, int *filenames_count);
void sort_file_and_output(char** filenames, int filenames_count);

int main(int argc, char *argv[]) {
   setlocale(LC_ALL, "Rus");

    if(argc > 4){
        printf("Превышено количество аргументов командной строки\n");
    }
    else if(argc == 4){ //У нас и путь к каталогу есть и опции
        with_options(argc, argv);
    }
    else if(argc == 2){ //У НАС ТОЛЬКО ПУТЬ ЗНАЧИТ ВЫВЕСТИ ВСЁ И ВСЯ С КАТАЛОГОВ
      char* file_path = (char*)malloc(255);
      strcpy(file_path, argv[1]);
      without_options(file_path);
      free(file_path);
    }
    else{
        if(argc == 3)
        printf("Неверный формат ввода, вы ввели 3 аргумента\n");
        else
        printf("Вы ничего не передали в командную строку\n");
    }
       
    return 0;
}

Marker create_marker(char* optargument){
    Marker buf = {false, false, false, false};
    for(int i = 0; i < strlen(optargument); i++)
    {
        //printf("\n\ni = %d -type = %c\n\n", i, optargument[i]);
        if(optargument[i] == 'f') buf.f = true;
        else if(optargument[i] == 'd') buf.d = true;
        else if(optargument[i] == 'l') buf.l = true;
        else if(optargument[i] == 's') buf.s = true;
        else{
            printf("Неверный режим -type %c\n", optargument[i]);
            exit(0);
        }
    }
    return buf;
}


void sort_file_and_output(char** filenames, int filenames_count){
     //setlocale(LC_COLLATE, "en_US.UTF-8");
     setlocale(LC_COLLATE, "");
     //qsort(filenames, filenames_count, sizeof(char *), (int (*)(const void *, const void *)) strcoll);
     qsort(filenames, filenames_count, sizeof(char *), (int (*)(const void *, const void *)) strcmp);
     setlocale(LC_COLLATE, "C");

     for(int i = 0; i < filenames_count; i++){
        printf("%s\n", filenames[i]);
     }
}


void with_options(int argc, char *argv[]){ //ФУНКЦИЯ ОБРАБОТАЕТ ОПЦИИ И ПУТЬ К КАТАЛОГУ
    char* type = "-type\0";
   char* file_path = (char*)malloc(255);
   Marker marker_search;
  if(argv[1][0] != '-') //СНАЧАЛА ИДЁТ ПУТЬ К ФАЙЛУ А ПОТОМ ОПЦИИ
  {
       if(strcmp(argv[2], type) != 0)
        {
         printf("Неверный формат\n");
         exit(0);
        }
        marker_search = create_marker(argv[3]);
        strcpy(file_path, argv[1]);
              
  }else{ //СНАЧАЛА ИДЁТ ОПЦИЯ А ПОТОМ ПУТЬ К ФАЙЛУ
       if(strcmp(argv[1], type) != 0)
        {
         printf("Неверный формат\n");
         exit(0);
        }
        marker_search = create_marker(argv[2]);
        strcpy(file_path, argv[3]);
  }
   char *filenames[255]; 
   int filenames_count = 0;
  //ВСЁ, ТЕПЕРБ ЕСТЬ МАРКЕР И ПУТЬ К ФАЙЛУ, ОСТАЛОСЬ ВЫВЕСТИ
  directory_search_with_marker(file_path, marker_search, filenames, &filenames_count);
   
  if(marker_search.s == true){
     sort_file_and_output(filenames, filenames_count);
  }
  for(int i = 0; i < filenames_count; i++){
      free(filenames[i]);
  }
  //free(filenames);
  free(file_path);
  
}

void directory_search_with_marker(char* file_path, Marker marker, char** filenames, int *filenames_count){
    
    DIR* dir = opendir(file_path);
   if(dir == NULL){
    printf("Неправильный путь [%s] к каталогу\n", file_path);
    exit(0);
   }

    struct dirent *dirfile;
   while(NULL != (dirfile = readdir(dir))){
 // Проверяем, не является ли имя записи текущим или родительским каталогом
    if (strcmp(dirfile->d_name, ".") == 0 || strcmp(dirfile->d_name, "..") == 0) {
        // Пропускаем обработку этих записей
        continue;
    }
    if(dirfile->d_type == DT_DIR){
        char path[255];
        snprintf(path, sizeof(path), "%s/%s", file_path, dirfile->d_name);
        if(marker.d == true){
            if(marker.s != true)
          printf("%s\n", path);
             else{
               filenames[*filenames_count] = strdup(path);
               (*filenames_count)++;
             }
          }
        directory_search_with_marker(path, marker, filenames, filenames_count);
     }
     if(dirfile->d_type == DT_REG && marker.f == true){
        if(marker.s != true)
          printf("%s/%s\n", file_path, dirfile->d_name);
        else{
            char path[255];
            snprintf(path, sizeof(path), "%s/%s", file_path, dirfile->d_name);
            filenames[*filenames_count] = strdup(path);
           (*filenames_count)++;
        }
     }
     if(dirfile->d_type == DT_LNK && marker.l == true){
        if(marker.s != true)
          printf("%s/%s\n", file_path, dirfile->d_name);
        else{
            char path[255];
            snprintf(path, sizeof(path), "%s/%s", file_path, dirfile->d_name);
            filenames[*filenames_count] = strdup(path);
            (*filenames_count)++;
        }
     }
     
   }
   if(closedir(dir) == -1){
    printf("Ошибка при закрытии каталога [%s]\n", file_path);
   }
}
/*

*/
void without_options(char* file_path){ //ФУНКЦИЯ ОБРАБОТАЕТ КАТАЛОГ СО ВСЕМИ ОПЦИЯМИ СРАЗУ ПОТОМУ ЧТО ИХ НЕ УКАЗАЛИ
   DIR* dir = opendir(file_path);
   if(dir == NULL){
    printf("Неправильный путь [%s] к каталогу\n", file_path);
    exit(0);
   }
   struct dirent *dirfile;
   while(NULL != (dirfile = readdir(dir))){
     // Проверяем, не является ли имя записи текущим или родительским каталогом
    if (strcmp(dirfile->d_name, ".") == 0 || strcmp(dirfile->d_name, "..") == 0) {
        // Пропускаем обработку этих записей
        continue;
    }
    printf("%s/%s\n", file_path, dirfile->d_name);

    if(dirfile->d_type == DT_DIR){
        char path[255];
        snprintf(path, sizeof(path), "%s/%s", file_path, dirfile->d_name);
        without_options(path);
    }
   }
}

