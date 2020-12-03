#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#define size_block 512

int record_file(char* fname, int arc_file, int depth) //записываем файл fname или директорию dirname глубиной depth в архив с дескриптором arc_file
{
  unsigned long size; //размер файла
  unsigned long int size_name; //размер имени файла
  struct stat inform_file; //структура, соддержащая информацию о файле
  char block[size_block]; //блоки данных файла
  unsigned short count; //количество прочитанных байт
  unsigned short check = 1;

  int in = open(fname, O_RDONLY);
  if(in == -1)
  {
    printf("Не удалось открыть файл %s", fname);
    return -1;
  }

  //записываем информацию о файле в структуру типа stat
  lstat(fname, &inform_file);

  //получаем размер файла
  size = inform_file.st_size;
  size_name = sizeof(fname);

  //записываем в архив данные о файле
  if(write(arc_file, &size_name, sizeof(unsigned long int)) < 0)
  {
    printf("Не удалось записать размера имени файла\n");
    return -2;
  }
  if(write(arc_file, fname, size_name) < 0)
  {
    printf("Не удалось записать имя файла\n");
    return -3;
  }
  if(write(arc_file, &check, sizeof(unsigned short)) < 0)
  {
    printf("Не удалось записать переменную проверки\n");
    return -4;
  }
  if(write(arc_file, &depth, sizeof(int)) < 0)
  {
    printf("Не удалось записать глубину\n");
    return -5;
  }
  if(write(arc_file, &size, sizeof(unsigned long)) < 0)
  {
    printf("Не удалось записать размер файла\n");
    return -6;
  }
  //считываем и записываем данные файла в блоках
  while(count=read(in, block, 1) > 0)
  {
    if(write(arc_file, block, count) < 0)
    {
      printf("Не удалось записать данные файла\n");
      return -7;
    }
  }

  return 1;
}

int record_dir(int arc_file, int depth, char* dirname) //записываем файл fname или директорию dirname глубиной depth в архив
{
  unsigned long int size_name; //размер имени директории
  unsigned short check = 0; //переменная проверки

  //получаем размер имени директории
  size_name = sizeof(dirname);

  //записываем в архив данные о директории
  if(write(arc_file, &size_name, sizeof(unsigned long int)) < 0)
  {
    printf("Не удалось записать размер имени директории\n");
    return -1;
  }
  if(write(arc_file, dirname, size_name) < 0)
  {
    printf("Не удалось записать имя директории\n");
    return -2;
  }
  if(write(arc_file, &check, sizeof(unsigned short)) < 0)
  {
    printf("Не удалось записать переменную проверки\n");
    return -3;
  }
  if(write(arc_file, &depth, sizeof(int)) < 0)
  {
    printf("Не удалось записать глубину\n");
    return -4;
  }

  return 1;
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
        //Записываем директорию в архив
        if(record_dir(arc_file, depth, informdir->d_name) < 0)
        {
          printf("Не удалось записать директорию\n");
          return -2;
        }
        archive(arc_file, informdir->d_name, depth+1); //опускаемся на уровень ниже
        rmdir(informdir->d_name);
      }
      else
      {
        printf(" - файл.\n");
        //записываем файл в архив
        if(record_file(informdir->d_name, arc_file, depth) < 0)
        {
          printf("Не удалось записать файл\n");
          return -3;
        }
        remove(informdir->d_name);
      }
    }
    informdir=readdir(directory);
  }

  chdir(".."); //поднимаемся на уровень выше
  closedir(directory);
  return 0;
}

int read_archive(char* aname, char* dirname)
{
  char* fname; //имя текущего файла
  char* dir_current; //имя последней считанной директории
  unsigned long size; //размер файла
  unsigned long int size_name; //размер имени файла
  int fnew; //дескриптор нового файла
  int depth; //глубина залегания файла
  int check_depth; //переменная проверки глубины
  int depth_current = 0; //глубина последнего считанного файла
  unsigned short check; //переменная проверки (файл или директория)
  char block[size_block]; //блоки данных файла

  int fa = open(aname, O_RDONLY);
  if (fa == -1)
  {
    printf("Не удалось открыть файл %s\n", aname);
    return -1;
  }

  DIR* directory = opendir(dirname);
  if(directory == NULL)
  {
    printf("Не удалось открыть директорию %s\n", dirname);
    return -2;
  }

  //переходим в директорию
  chdir(dirname);

  while(read(fa, &size_name, sizeof(unsigned long int)) > 0)
  {
    fname = malloc(size_name+1);
    if (read(fa, fname, size_name) < 0)
    {
      printf("Не удалось считать имя файла\n");
      close(fa);
      return -3;
    }

    if (read(fa, &check, sizeof(unsigned short)) < 0)
    {
      printf("Не удалось считать переменную проверки\n");
      close(fa);
      return -4;
    }

    if(read(fa, &depth, sizeof(int)) < 0)
    {
      printf("Не удалось считать глубину\n");
      close(fa);
      return -5;
    }

    if(depth_current < depth)
    {
       chdir(dir_current);
    }
    if(depth_current > depth)
    {
       check_depth = depth;
       while((depth_current - check_depth) > depth)
       {
         chdir("..");
         check_depth++;
       }
    }
    depth_current = depth;

    if(check == 0)
    {
      mkdir(fname, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IWOTH|S_IXOTH);
      dir_current = fname;
    }
    if(check == 1)
    {
      if(read(fa, &size, sizeof(unsigned long)) < 0)
      {
        printf("Не удалось считать размер файла\n");
        close(fa);
        return -6;
      }

      if(read(fa, block, size) < 0)
      {
        printf("Не удалось считать данные файла\n");
        close(fa);
        return -7;
      }

      fnew = open(fname, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IWOTH);

      if(write(fnew, block, size) < 0)
      {
        printf("Не удалось записать данные в файл\n");
        close(fa);
        return -8;
      }
    }
  }

  close(fa);
  return 1;
}

int main(int argc, char* argv[])
{
  char* fname = "arc"; //файл архива
  int rez = 0;  //переменная для проверки режима работы программы
  int check = 0;  //переменная для проверки режима работы программы

  //Выбираем режим работы
  while ((rez = getopt(argc, argv, "ar")) != -1)
  {
    switch(rez)
    {
      case 'a':
      {
        check = 1;
        break;
      }
      case 'r':
      {
        check = 2;
        break;
      }
    }
  }

  //Архивация файлов и директорий
  if(check == 1)
  {
    //Создаем файл-архив
    int a = open(fname, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IWOTH);

    //Архивирхивация файлов и директорий
    if(archive(a, argv[2], 0) < 0)
    {
      printf("Архивация прошла неудачно\n");
      close(a);
      return -1;
    }
    else
    {
      printf("Архивация прошла успешно\n");
      close(a);
      return 1;
    }
  }
  if(check == 2)
  {
    //Разархивация файлов и дирикторий
    if(read_archive(fname, argv[2]) < 0)
    {
      printf("Разархивация прошла неудачно\n");
      return -1;
    }
    else
    {
      printf("Разархивация прошла успешно\n");
      return 1;
    }
  }
}
