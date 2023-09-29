#include <stdio.h>
#include <stdlib.h>
#include <string.h>   
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h> // open()
#include <sys/errno.h>

#define MAXCHAR 256

int main(int argc, char* argv[]){ // argv = {"more", "filename", vsizeStr, hsizeStr, NULL};

  const char *currentDir = "./";
  char path[MAXCHAR];
  strcpy(path, currentDir);
  strcat(path, argv[1]);

  int fd = open(path, O_RDONLY);
  char singleChar;
  char input;
  if(fd < 0) { // open() returns -1 on failure
    printf("error % d, open sys call\n", errno); 
    return -1;
  }

  int vsize = atoi(argv[2]);
  int hsize = atoi(argv[3]);
  int v = 0;
  int h = 0;

  int character;
  while((character = read(fd, &singleChar, 1)) == 1){ // read one character at a time
   if(v < vsize){

    if(h < hsize){
      printf("%c", singleChar); // could be any character except '\0'
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
      do{ character = read(fd, &singleChar, 1);}
      while(singleChar != '\n' && character == 1); // eat characters until newline is found or EOF
      if(character == 0){
        break;
      }
      else if(character == -1){
        perror("character error : ");
      }
      else{
        printf("%c", singleChar); // in the case where h=hsize, prints newline character
        v++; // remark that one line has been printed
      }
    }
    h=0; // EITHER we found a newline so we reset horizontal char count OR h==hsize, and we consumed chars until we got to newline, must reset count
    }
    input = getchar();
    if(input != '\n'){
      break;
    }
    else{
      v=0; // reset line count and continue in while loop
    }
  }
  if(character == -1){
    perror("character error : ");
  }  
  return 0;
}