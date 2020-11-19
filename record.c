#include <unistd.h>
#include <fcntl.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#define size_block 512

void record_file(char* fname, int arc_file, int depth) //записываем файл fname или директорию dirname глубиной depth в архив
{
  unsigned long size; //размер файла
  struct stat inform_file; //структура, соддержащая информацию о файле
  char block[size_block]; //блоки данных файла
  unsigned short count; //количество прочитанных байт
  
  int in = open(fname, O_RDONLY); 
  if(in == -1)
    printf("Не удалось открыть файл %s", fname);
  
  //записываем информацию о файле в структуру типа stat
  lstat(fname, &inform_file);
  
  //получаем размер файла
  size = inform_file.st_size;
  printf("Размер файла: %lu\n", size);
  
  //записываем в архив данные о файле
  write(arc_file, fname, 100); //записываем имя файла
  write(arc_file, &size, 1); //записываем  размер файла
  write(arc_file, &depth, 1); //записываем глубину файла
  
  //считываем и записываем данные файла в блоках
  while(count=read(in, block, size_block) > 0)
  {
    write(arc_file, block, count);
  }
  
}

void record_dir(int arc_file, int depth, char* dirname) //записываем файл fname или директорию dirname глубиной depth в архив
{
  unsigned long size; //размер файла
  struct stat inform_file; //структура, соддержащая информацию о файле
  char block[size_block]; //блоки данных файла
  unsigned short count; //количество прочитанных байт
  
  //записываем в архив данные о файле
  write(arc_file, dirname, 100); //записываем имя директории
  write(arc_file, &depth, 1); //записываем глубину файла
  
}

int archive(int arc_file, char* dirname, int depth) //архивируем файлы из папки dir, лежащие на глубине depth, в архив fname
{ 
  struct dirent* informdir; //структура, соддержащая информацию о директории
  struct stat inform_file; //структура, соддержащая информацию о файле

  DIR* directory = opendir(dirname);
  if(directory == NULL)
  {
    printf("Не удалось открыть директорию %s", dirname);
    return 2;
  }

  //переходим в директорию
  chdir(dirname);
  
  //получаем указатель на очередной файл директории
  informdir = readdir(directory);
  
  //пока не закончились файлы в директории
  while(informdir != NULL)
  {
    printf("%s", informdir->d_name); 
    if(strcmp(".", informdir->d_name) != 0 && strcmp("..", informdir->d_name) != 0)
    {
      lstat(informdir->d_name, &inform_file);
      if(S_ISDIR(inform_file.st_mode))
      {
        printf(" - директория.\n");
        archive(arc_file, informdir->d_name, depth+1); //опускаемся на уровень ниже
        record_dir(arc_file, depth+1, informdir->d_name); //записываем директорию в архив 
      }
      else
      { 
        printf(" - файл.\n");
        record_file(informdir->d_name, arc_file, depth); //записываем файл в архив
      }
    }   
    
    informdir=readdir(directory);
  }
    
  chdir(".."); //поднимаемся на уровень выше
  closedir(directory);
  return 0;
}

int main()
{
  char* a = "arc";
  char* b = "abc";
  int cheak = 0;
  
  int fa = open(a, O_WRONLY | O_APPEND); 
  if (fa == -1)
  {
    printf("Не удалось открыть файл %s", a);
    return 1;
  }

 
  cheak = archive(fa, b, 0);
  
  close(fa);
  
  if(cheak == 0)
    printf("Архивация прошла успешно!\n");
  else
    printf("Архивация не удалась!\n");

  return 0;
}
























