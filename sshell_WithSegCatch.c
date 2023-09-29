#include <unistd.h>
#include <fcntl.h> // flags for open()
#include <stdio.h>
#include <stdlib.h>  // for atoi
#include <string.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/stat.h>

#define MAXCHAR 12 // command can have at most 256 characters

char *tokenMaker(char *s); // parses string based on whitespaces, returns one token at a time
int getSizeParam(char* line); // passed the VSIZE and HSIZE lines in shconfig, returns value
struct More startup(struct More m); // reads shconfig, searching for VSIZE and HSIZE values

struct More{
int vsize;
int hsize; 
};

int main(void){

  struct More m = {-1,-1};

  m = startup(m);
  int bkgFlag; // bkgFlag set to 1 when command prefixed with '&' (background command)
  char line[MAXCHAR+1];
  char* cmd;
  int success = 0;
  int printed = 0;
  while(1){
    bkgFlag = 0; // reset bkgFlag for each new command
    printf("@ "); 

    while (success == 0){
      if (fgets(line, MAXCHAR + 1, stdin) != NULL) {
        size_t length = strlen(line);
        if (length && line[length - 1] != '\n' && !feof(stdin)) {
          if (printed == 0) {
            printf("Line too long. Allowed length = %d\n", MAXCHAR);        
            printed = 1;
          }
        }
        else success = 1;
      }
    }
    printed = 0;
    success = 0;

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
      int arrLength = wtspace+2; //  +1 because array will need to be NULL terminated, +1 since 1 whitespace implies 2 tokens
      char *tokenArr[arrLength];
      tokenArr[arrLength-1] = NULL;
      char *token;
      int i = 0;  
      while(i < arrLength-1){
        if(i==0){
          token = tokenMaker(cmd); 
        }
        else{
          token = tokenMaker(0);
          if(token == 0){ // no more tokens to parse 
            break;
          } 
        }
        tokenArr[i] = token;
        i++;
      }

      if(strcmp(line, "exit") == 0){
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
          /////// check for length (so if strlen(path) + str(tokenArr[0] > MAXCHAR ...))
          strcat(path, tokenArr[0]);

          if(bkgFlag == 0){ // child process executes command

            if(strcmp(tokenArr[0], "more") == 0){ // if more command
              char vsizeStr[10];
              char hsizeStr[10];
              sprintf(vsizeStr, "%d", m.vsize);
              sprintf(hsizeStr, "%d", m.hsize);
              char *moreTokenArr[] = {tokenArr[0], tokenArr[1], vsizeStr, hsizeStr, NULL};
              if((error = execve("./mymore", moreTokenArr, 0)) == -1){ // more executable is in current directory
                printf("error % d, internal more command cannot be executed\n", errno);
                perror("perror: ");
              }
            }

            if((error = execve(path, tokenArr, 0)) == -1){ // check if bin path
              strcpy(path, usrbin);
              strcat(path, tokenArr[0]);
              if((error = execve(path, tokenArr, 0)) == -1){ // check if usrbin path
                if((error = execve(cmd, tokenArr, 0)) == -1){ // check if absolute path
                  printf("error % d, command cannot be executed\n", errno); 
                  perror("perror: ");
                }
              }
            }

          }

          else{   // background commands are executed by granchild child prints message when executable is finished
            int grandchild = fork();
            if (grandchild == 0){ // if grandchild process
              if(strcmp(tokenArr[0], "more") == 0){ 
                if((error = execve("./mymore", tokenArr, 0)) == -1){
                  printf("error % d, internal more command cannot be executed\n", errno);
                  perror("perror: ");
                }
              }
              else if((error = execve(path, tokenArr, 0)) == -1){ // check if bin path
                strcpy(path, usrbin);
                strcat(path, tokenArr[0]);
                if((error = execve(path, tokenArr, 0)) == -1){ // check if usrbin path
                  if((error = execve(cmd, tokenArr, 0)) == -1){  // check if absolute path
                  printf("error % d, command cannot be executed\n", errno); 
                  perror("perror: ");

                  }
                }
              }
            }
            else { // if child process AND a background command was requested
              waitpid(grandchild, &status, 0);
              char* argv[] = {"bkg", NULL};
              if((error = execve("./bkg", argv, 0)) == -1){
                printf("error % d, background command termination message cannot be executed\n", errno);
              }
            }

          }
        }

        else { // if parent process
          int status;
          if(bkgFlag == 0){ // no background command requested
              waitpid(child, &status, 0); // wait for command to terminate before accepting new commands
          }
          else{ // accept new commands right away
          }
        }
      }
    }
  
}

char *tokenMaker(char *s){ // first time tokenMaker called, s is the raw input from fgets()
	static char *p = 0; // being a static char
	if (s == 0){ // if tokenMaker has already been called once, s = input - last token made
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
	  p++; // p points to the first letter of the next token for the next time we call it
	}
	return s;
}

int getSizeParam(char* bufChunk){
  bufChunk = bufChunk+5; // +5 because HSIZE or VSIZE is 5 characters
  int size = atoi(bufChunk); // atoi ignores white spaces, returns numerical characters
  return size;
}
struct More startup(struct More m){ // can only use systems call

  int fd;
  int character;
 
  char buffer[MAXCHAR]; // can hold a max of 256 characters
  memset(buffer, '\0', MAXCHAR); 

  fd = open("./shconfig", O_RDWR|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR);
  if(fd < 0) {
    printf("error % d, open sys call\n", errno); 
    perror("perror ");
  }

  struct stat st;
  stat("./shconfig", &st);
  int fileSize = st.st_size;
  //printf("fileSize: % d\n", fileSize);
  character = read(fd, &buffer, sizeof(buffer)-1);  // read max of 255 characters from the file, 256th character is always '\0' for safety reasons
  if (character < 0){
    printf("error % d, read sys call\n", errno);
    perror("perror ");
  }
  //printf("bytes read: % d\n", character);
  char* bufChunk = buffer;
  char* tmp = buffer;
  if(character <= fileSize){ // read() will not need to be called again
    while(m.vsize == -1 || m.hsize ==-1){ // does not keep searching shconfig if both parameters set
      if(*tmp == '\n' || *tmp == '\0'){ // if we've gotten to the end of a line or EOF
        if (strncmp(bufChunk, "VSIZE", 5) == 0){
          m.vsize = getSizeParam(bufChunk);
        }
        else if (strncmp(bufChunk, "HSIZE", 5) == 0){
          m.hsize = getSizeParam(bufChunk);
        }
        if(*tmp == '\0'){ 
          break;
        } // dont want pointer pointing at unknown memory
        tmp++; // tmp points to first character in new line
        bufChunk = tmp; 
      }
      else{
      tmp++;
      }
    }
  }
  if(character > fileSize){ // could try to handle this implementation later
    printf("oh no, can't handle when shconfig size bigger than buffer\n");
  }

  int error;
  const int sizeBuf = 9; // "VSIZE 40\n" as 9 characters
  if(m.vsize == -1 && m.hsize == -1){ // if both NOT set
    printf("both not set\n");
    if((error = write(fd, "VSIZE 40\n", sizeBuf)) == -1){
      printf("error % d, VSIZE not appended to shconfig\n", errno);
    }
    m.vsize = 40;
    if((error = write(fd, "HSIZE 75", sizeBuf-1)) == -1){
      printf("error % d, HSIZE not appended to shconfig\n", errno);
    }
    m.hsize = 75;
  }

  if (m.vsize == -1 && m.hsize != 1){ // if vsize is NOT set
    printf("vsize not set\n");
    if((error = write(fd, "\nVSIZE 40", sizeBuf)) == -1){
      printf("error % d, VSIZE not appended to shconfig\n", errno);
    }
    m.vsize = 40;
  }
  if (m.hsize == -1 && m.vsize != 1){ // if hsize is NOT set
    printf("hsize not set\n");
    if((error = write(fd, "\nHSIZE 75", sizeBuf)) == -1){
      printf("error % d, HSIZE not appended to shconfig\n", errno);
    }
    m.hsize = 75;
  }
  printf("vsize=%d, hsize=%d\n", m.vsize, m.hsize);
  close(fd);
  return m;
}