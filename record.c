#include <unistd.h>
#include <fcntl.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#define size_block 512

void record_file(char* fname, int arc_file, int depth) //записываем файл fname или директорию dirname глубиной depth в архив
{
  unsigned long size; //размер файла
  unsigned long int size_name;
  struct stat inform_file; //структура, соддержащая информацию о файле
  char block[size_block]; //блоки данных файла
  unsigned short count; //количество прочитанных байт
  unsigned short check = 1;
  int p;
  
  int in = open(fname, O_RDONLY); 
  if(in == -1)
    printf("Не удалось открыть файл %s", fname);
  
  //записываем информацию о файле в структуру типа stat
  lstat(fname, &inform_file);
  
  //получаем размер файла
  size = inform_file.st_size;
  printf("Размер файла: %lu\n", size);
  
  size_name = sizeof(fname);
  
  printf("Size File = %ld", sizeof(fname));
  printf("\n");
  
  //записываем в архив данные о файле
  if(write(arc_file, &size_name, sizeof(unsigned long int)) > 0)
  {
    printf("Запись размера имени файла прошла успешно\n");
  }
  if(write(arc_file, fname, size_name) > 0) //записываем имя файла
  {
    printf("Запись имени файла прошла успешно\n");
  }
  if(write(arc_file, &check, sizeof(unsigned short))> 0)
  {
    printf("Запись типа прошла успешно\n");
  }
  if(write(arc_file, &depth, sizeof(int)) > 0) //записываем глубину файла
  {
    printf("Запись глубины файла прошла успешно\n");
  }
  if(write(arc_file, &size, sizeof(unsigned long)) > 0) //записываем  размер файла
  {
    printf("Запись размера файла прошла успешно\n");
  }
  //считываем и записываем данные файла в блоках
  while(count=read(in, block, 1) > 0)
  {
    write(arc_file, block, count);
    printf("Запись данных файла прошла успешно\n");
    p++;
  }
  
  printf("количество = %d\n", p);
}

void record_dir(int arc_file, int depth, char* dirname) //записываем файл fname или директорию dirname глубиной depth в архив
{
  unsigned long int size_name;
  unsigned short check = 0;
  
  size_name = sizeof(dirname);
  
  printf("Size Dir = %ld", sizeof(dirname));
  printf("\n");
  
  //записываем в архив данные о файле
  write(arc_file, &size_name, sizeof(unsigned long int));
  if(write(arc_file, dirname, size_name) > 0) //записываем имя директории
  {
    printf("Запись имени дириктории прошла успешно\n");
  } 
  if(write(arc_file, &check, sizeof(unsigned short))> 0)
  {
    printf("Запись типа прошла успешно\n");
  }
  if(write(arc_file, &depth, sizeof(int)) > 0) //записываем глубину файла
  {
    printf("Запись глубины файла прошла успешно\n");
  }
  
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
        record_dir(arc_file, depth, informdir->d_name); //записываем директорию в архив
        archive(arc_file, informdir->d_name, depth+1); //опускаемся на уровень ниже
        //rmdir(informdir->d_name);
      }
      else
      { 
        printf(" - файл.\n");
        record_file(informdir->d_name, arc_file, depth); //записываем файл в архив
        //remove(informdir->d_name);
      }
    }   
    
    informdir=readdir(directory);
  }
   
  chdir(".."); //поднимаемся на уровень выше
  closedir(directory);
  return 0;
}

int write_archive(char* aname, char* bname)
{
  int cheak = 0;
  int fa = open(aname, O_WRONLY | O_APPEND); 
  if (fa == -1)
  {
    printf("Не удалось открыть файл %s", aname);
    return 1;
  }
  
  cheak = archive(fa, bname, 0);
  
  close(fa);
  
  return 2;
}

int read_archive(char* aname, char* dirname)
{
  char* fname;
  char* dir_current;
  unsigned long size; //размер файла
  unsigned long int size_name;
  int fnew;
  int p = 0;
  int depth;
  int count;
  int check_depth;
  int depth_current = 0;
  unsigned short check;
  long position;
  char block[size_block]; //блоки данных файла
  struct stat inform_file; //структура, соддержащая информацию о файле
  struct dirent* informdir; //структура, соддержащая информацию о директории

  int fa = open(aname, O_RDONLY); 
  if (fa == -1)
  {
    printf("Не удалось открыть файл %s", aname);
    return -1;
  }
  
  position = lseek(fa, 0L, SEEK_CUR);
  
  printf("Смешение = %ld\n", position);
  
  DIR* directory = opendir(dirname);
  if(directory == NULL)
  {
    printf("Не удалось открыть директорию %s", dirname);
    return -2;
  }
  
  //переходим в директорию
  chdir(dirname);
  
    while(read(fa, &size_name, sizeof(unsigned long int)) > 0)
    {
     
      fname = malloc(size_name+1);
      if (read(fa, fname, size_name) > 0)
      {
        printf("fname = %s\n", fname); 
      } 
      
      
      if (read(fa, &check, sizeof(unsigned short)) > 0)
      {
        printf("сheck = %hu\n", check);
      }
      if(check == 0)
      {
        printf("dir\n");
        
        if(read(fa, &depth, sizeof(int)) > 0)
        { 
          printf("depth = %d\n", depth);
        }
        
        if(depth_current < depth)
        {
          chdir(dir_current);
          printf("Опустились на уровень ниже\n");
        }
        if(depth_current == depth)
          printf("Остаемся на том же уровне\n");
        if(depth_current > depth)
        {
           check_depth = depth;
           while((depth_current - check_depth) > depth)
          {
            chdir("..");
            check_depth++;
            printf("Поднялись на уровень выше\n");
          }
        }
       
        depth_current = depth;
        
        mkdir(fname, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IWOTH|S_IXOTH);
        dir_current = fname;
        printf("dir_name = %s\n", fname);  
      }
      if(check == 1)
      {
        printf("file\n");
        
        if(read(fa, &depth, sizeof(int)) > 0)
        { 
          printf("depth = %d\n", depth);
        }
        
        if(depth_current < depth)
        {
          chdir(dir_current);
          printf("Опустились на уровень ниже\n");
        }
        if(depth_current > depth)
        {
          check_depth = depth;
          while((depth_current - check_depth) > depth)
          {
            chdir("..");
            check_depth++;
            printf("Поднялись на уровень выше\n");
          }
          
        }
       if(depth_current == depth)
          printf("Остаемся на том же уровне\n");
        depth_current = depth;
        
        if(read(fa, &size, sizeof(unsigned long)) > 0);
        
        if(read(fa, block, size)> 0);
        fnew = open(fname, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IWOTH);
        if(write(fnew, block, size) > 0);
        printf("\n");
      }    
      
      printf("\n");
      printf("depth_current = %d\n", depth_current );
    }
   

  
  close(fa);
  
  return 3;
}

int main()
{
  char* fname = "arc";
  char* b = "abc";
  char* c = "readarc";
  
  int a = open(fname, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IWOTH);
  
  close(a);

  //Архивация файлов и директорий
  write_archive(fname, b); 

  //Разархивация файлов и дирикторий
  read_archive(fname, c);

  
  
  return 0;
}
























