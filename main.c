/*
https://stackoverflow.com/questions/7656549/understanding-requirements-for-execve-and-setting-environment-vars
token_maker based on strtok function, but altered since we know already that our delimiter will be a white space, and strings will include '\n' : https://www.youtube.com/watch?v=Eu4pXgvQnK8

Questions

2) assumes that the user puts one space between tokens. so ' cmd' or 'cmd   arg' are not acceptable (should have printout statment for this)

*/
#include <stdio.h>
#include <stdlib.h>  // for atoi
#include <string.h> // only specifies we must use system calls for file access 
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <unistd.h>
#include <signal.h> // kills process
#include <fcntl.h> // flags for open()
#include <sys/stat.h>

#define MAXCHAR 256 // can have at most 255 characters as your input line

char *token_maker(char *s); // parses line based on whitespaces
int getSizeParam(char* line);
struct More startup(struct More m); // read shconfig, searching for VSIZE and HSIZE values

struct More{
int vsize;
int hsize; 
};

int main(void){

struct More m = {-1,-1};

 m = startup(m);
  printf("VSIZE set to %d\nHSIZE set to %d\n", m.vsize, m.hsize);
  int bkgFlag; // bkgFlag set to 1 when command prefixed with '&'
  char line[MAXCHAR];
  char* cmd;

  while(1){
    bkgFlag = 0; // reset bkgFlag for each new command
    printf("\n@ "); 
    fgets(line, MAXCHAR, stdin);
    cmd = line;
    if((line[0] == '&') && (line[1] == ' ')){
      bkgFlag = 1;
      cmd = line + 2;
    }

    int wtspace=0; // count number of white spaces
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
      return 0; // kills shell, but background cmd ran by child continues even after the shell is killed
    }

    else{
      int child = fork();
      if (child == 0){ // if child process
          int status;
          int error;
          const char *bin = "/bin/";
          const char *usrbin = "/usr/bin/";
          char path[MAXCHAR];

          strcpy(path, bin);
          strcat(path, tokenArr[0]);

        if(bkgFlag == 0){ // replace child process with cmd program
      // --------------------------------
          if((error = execve(path, tokenArr, 0)) == -1){ // try bin path

            strcpy(path, usrbin);
            strcat(path, tokenArr[0]);
            // printf("path %s\n", path);
            if(strcmp(tokenArr[0], "more") == 0){ // if more cmd
              char vsizeStr[10];
              char hsizeStr[10];
              sprintf(vsizeStr, "%d", m.vsize);
              sprintf(hsizeStr, "%d", m.hsize);

              char *moreTokenArr[] = {tokenArr[0], tokenArr[1], vsizeStr, hsizeStr, NULL};
              if((error = execve("./mymore", moreTokenArr, 0)) == -1){ // more cmd is in current directory
                printf("error % d, internal more command cannot be executed\n", errno);
                perror("perror: ");
              }
            }
            else if((error = execve(path, tokenArr, 0)) == -1){ // try usrbin path
              if((error = execve(cmd, tokenArr, 0)) == -1){
                printf("error % d, command cannot be executed\n", errno); // execve() fill 
              }
            }
          }

        }
        else{   // grandchild runs cmd program, child prints bkg termination message
          int grandchild = fork();
          if (grandchild == 0){ // if grandchild process
            if(strcmp(tokenArr[0], "more") == 0){ // if more cmd
              if((error = execve("./mymore", tokenArr, 0)) == -1){ // more cmd is in current directory
                printf("error % d, internal more command cannot be executed\n", errno);
                perror("perror: ");
              }
            }
            else if((error = execve(path, tokenArr, 0)) == -1){ // try bin path
              strcpy(path, usrbin);
              strcat(path, tokenArr[0]);
             // printf("path %s\n", path);
              if((error = execve(path, tokenArr, 0)) == -1){ // try usrbin path
                if((error = execve(cmd, tokenArr, 0)) == -1){
                 printf("error % d, command cannot be executed\n", errno); // execve() fill 
                }
              }
            }
          }
          else { // if child process AND we did a bkg command
            waitpid(grandchild, &status, 0);
            char* argv[] = {"bkg", NULL};
            if((error = execve("./bkg", argv, 0)) == -1){
              printf("error % d, bkg cmd termination message cannot be found\n", errno); // execve() fill 
            }
          }
        }
      // --------------------------------
      }

      else { // if parent process
        int status;
        if(bkgFlag == 0){ // no ampersand cmd
            waitpid(child, &status, 0); // wait for ampersand command to finish
        }
        else{ // ampersand cmd
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


int getSizeParam(char* bufChunk){
  bufChunk = bufChunk+5;
  int size = atoi(bufChunk); // atoi ignores white spaces
  return size;
}


struct More startup(struct More m){ // can only use systems call

  int fd;
  int character;
 
  char buffer[MAXCHAR]; // can hold a max of 256 characters
  memset(buffer, '\0', MAXCHAR); 

  fd = open("./shconfig", O_RDWR|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR);

  if(fd < 0) { // open() returns -1 on failure
    printf("error % d, open sys call\n", errno); 
  }
  //https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c
  struct stat st;
  stat("./shconfig", &st);
  int fileSize = st.st_size;
  printf("size of file: %d\n", fileSize);
  character = read(fd, &buffer, sizeof(buffer)-1);  // read max of 255 characters from the file, 256th character is always '\0
  if (character < 0){
    printf("error % d, read sys call\n", errno);
  }
  printf("bytes read: % d\n", character);
  char* bufChunk = buffer;
  char* tmp = buffer;
  if(character <= fileSize){
    while(m.vsize == -1 || m.hsize ==-1){ // while loop breaks if *tmp points to 0. This will always happen since buffer will always have 0 at the end. Loop ends early is sizes are found
      if(*tmp == '\n' || *tmp == '\0'){
        if (strncmp(bufChunk, "VSIZE", 5) == 0){
          m.vsize = getSizeParam(bufChunk);
          printf("atoi m.vsize=%d\n", m.vsize);
        }
        else if (strncmp(bufChunk, "HSIZE", 5) == 0){
          m.hsize = getSizeParam(bufChunk);
          printf("atoi m.hsize=%d\n", m.hsize);
        }
        if(*tmp == '\0'){ 
          break;
        } // dont want tmp to point to unknown mem
        tmp++;
        bufChunk = tmp;
      }
      else{
      tmp++;
      }
    }
  }
  if(character > fileSize){
    // while (character > fileSize){
    //   int i = 0;
    //     for(i=0; buffer[i] != '\n'; i++){}
    //       character = pread(fd, &buffer, sizeof(buffer)-1, 0);  // read max of 255 characters from the file, 256th character is always '\0
    printf("oh no, can't handle when shconfig size bigger than buffer\n");
    } 
  int error;
  const int sizeBuf = 9;
  if (m.vsize == -1){ // m.vsize NOT set
    if((error = write(fd, "\nVSIZE 40", sizeBuf)) == -1){
      printf("error % d, VSIZE not appended to shconfig\n", errno);
    }
    m.vsize = 40;
  }
  if (m.hsize == -1){ // m.hsize NOT set
    if((error = write(fd, "\nHSIZE 75", sizeBuf)) == -1){
      printf("error % d, HSIZE not appended to shconfig\n", errno);
    }
    m.hsize = 75;
  }

  close(fd);
  return m;
}




