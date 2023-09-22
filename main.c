/*
https://stackoverflow.com/questions/7656549/understanding-requirements-for-execve-and-setting-environment-vars

*/
#include <stdio.h>
#include <string.h> // only specifies we must use system calls for file access 
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h> // kills process

#define MAXCHAR 256 // can have at most 255 characters as your input line

char *token_maker(char *s); // parses line based on whitespaces

int main(void){
  int pid;
  int bkgFlag = 0; // bkgFlag set to 1 when command prefixed with '&'
  char line[MAXCHAR];
  char* cmd;

  while(1){
    printf("@ "); 
    fgets(line, MAXCHAR, stdin);
    cmd = line;
    if((line[0] == '&') && (line[1] == ' ')){
      bkgFlag = 1;
      cmd = line + 2;
    }

    int wtspace=0;
    for(char* tmp = cmd; *tmp != '\n'; tmp++){
      if(*tmp == ' '){
        wtspace++;
      }
    }
    // printf("wtspace=%d\n", wtspace);
    int arrLength = wtspace+2;
    char *tokenArr[arrLength]; // need extra element for NULL, if there is 1 whitespace there is 2 tokens
    tokenArr[arrLength-1] = NULL;
    char *token;
    int i = 0;  
    while(i < arrLength-1){
      // printf("i=%d\n", i);
      if(i==0){
        token = token_maker(cmd);
        for(char* tmp = token; *tmp != '\0'; tmp++){
          }
      }
      else{
      token = token_maker(0);
      if(token == 0){break;} // no more tokens to parse, after breaking i = num of toks 
      }
      //puts(token);
      tokenArr[i] = token;
      i++;
    }

    if(strcmp(line, "exit") == 0){
      printf("internal exiting cmd\n");
      return 0;
    }

    else{
      pid = fork();
      if (pid == 0){ // if child process
        int error;
        const char *bin = "/bin/";
        const char *usrbin = "/usr/bin/";
        char path[MAXCHAR];

        strcpy(path, bin);
        strcat(path, tokenArr[0]);
        // printf("path %s\n", path);


        if((error = execve(path, tokenArr, 0)) == -1){ // try bin path

          strcpy(path, usrbin);
          strcat(path, tokenArr[0]);
          printf("path %s\n", path);

          if((error = execve(path, tokenArr, 0)) == -1){ // try usrbin path
            if((error = execve(cmd, tokenArr, 0)) == -1){
              printf("command not found\n");
              return 0;
            }
          }
        }

      //   else{
      //    if(bkgFlag == 1){
      //     printf("background cmd\n");
      //   }
      // }
      }

      else { // if parent process
        if(bkgFlag == 1){}
        else{
          wait(0);
        }
      }
    }
  }
}

char *token_maker(char *s){ // first time token_maker called, s is the raw input from get
	static char *p = 0; 
	if (s == 0){ // after the token_maker has already been called once, s = input - last token made
		s = p;
	}
	p = strchr(s, ' '); // find next white space
	if(p==0){ // if there are no more whitespaces
	    if (s[strlen(s)-1] == '\n'){
	    	s[strlen(s)-1] = '\0';
	     }
	    return s;
	}
	else {
	  *p = '\0'; //make whitespaces terminating zeros so you can return one token at a time
	  p++;
	}
	return s;
}

