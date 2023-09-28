// not finished, needs to print lines based on inherited vsize and hsize
// vsize and hsize should probably be arguments...
#include <stdio.h>
#include <stdlib.h>
#include <string.h>   
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h> // open()
#include <sys/errno.h>

#define MAXCHAR 256 // can have at most 255 characters as your input line



int main(int argc, char* argv[]){ 
  // argv = {"more", "filename", vsizeStr, hsizeStr, NULL};

  // printf("argc = %d\n", argc);

  int vsize = atoi(argv[2]);
  int hsize = atoi(argv[3]);

  int fd;

  const char *currentDir = "./";
  char path[MAXCHAR];

  strcpy(path, currentDir);
  strcat(path, argv[1]);
  printf("path %s\n", path);
  fd = open(path, O_RDONLY); // create file if it does not exist, append on each write, 
  char singleChar;
  char input;
  // int stopMoreFlag = 0;
  if(fd < 0) { // open() returns -1 on failure
    printf("error % d, open sys call\n", errno); 
    return -1;
  }
  int v = 0;
  int h = 0;
  int character;
  while((character = read(fd, &singleChar, 1)) == 1){
   if(v < vsize){
    if(h < hsize){
      printf("%c", singleChar); // could be anything but EOF
      if(singleChar != '\n'){ // if v does not need to be incremented
        h++; // remark that we have printed one character
        continue; // get the next character
      }
      else{ // if we have reached a newline
        v++; // remark that one line has been printed
      }
    }
      if(singleChar != '\n' && singleChar != '\0') {
        int character;
        do{
          character = read(fd, &singleChar, 1);
        }while(singleChar != '\n' && character == 1); // eat characters until newline is found or EOF
        if(character == 0){
          // printf("EOF found, goodbye.\n");
          break;
        }
        else if(character == -1){
          perror("character error : ");
        }
        else{
          printf("%c", singleChar); // case where h=hsize, we have not yet printed newline
          v++; // remark that one line has been printed
        }
      }
      h=0; // either we found a newline, so we reset horizontal char count. OR h==hsize, and we consumed chars until we got to newline (newline printed in both cases)
   }
    input = getchar();
    if(input != '\n'){
      break;
    }
    else{
      // printf("reset v for new page\n");
      v=0;
    }
  }
  if(character == -1){
    perror("character error : ");
  }  
return 0;
}