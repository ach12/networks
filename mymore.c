// not finished, needs to print lines based on inherited vsize and hsize
// vsize and hsize should probably be arguments...
#include <stdio.h>
#include <fcntl.h>

#define MAXCHAR 256 // can have at most 255 characters as your input line

int main(int argc, char* argv[]){ // argv = {"more", "filename", NULL}

  // printf("argc = %d\n", argc);

  int fd;

  const char *currentDir = "./";
  char path[MAXCHAR];

  strcpy(path, currentDir);
  strcat(path, argv[1]);
  // if(argc != 2){
  //   printf("more command requires a single file name as an argument\n");
  //   return 0;
  // }



  fd = open(path, O_RDWR|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR); // create file if it does not exist, append on each write, 


return 0;
}