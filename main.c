#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAXCHAR 256 // can have at most 255 characters as your input line

int main(void){
  // int token, status;
  int bkgFlag = 0; // bkgFlag set to 1 when command prefixed with '&'
  // char c;
  char line[MAXCHAR];
  char* cmd = line;
	printf("@ "); 
  fgets(line, MAXCHAR, stdin);
  puts(line);
    if((line[0] == '&') && (line[1] == ' ')){
      bkgFlag = 1;
      cmd = line + 2;
    }
  puts(line);
    for(int i = 0; i<MAXCHAR; i++){
      if (line[i] == '\n'){
        line[i] = '\0';
      }
    }
  printf("cmd is: %s", cmd);
  return 0;
   
}


