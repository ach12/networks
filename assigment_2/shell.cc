#include <unistd.h>
#include <fcntl.h> // flags for open()
#include <stdio.h>
#include <stdlib.h>  // for atoi
#include <string.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "tcp-utils.h" // Bruda's TCP functions

#define MAXCHAR 255 // command can have at most 255 characters

char* tokenMaker(char* s); // parses string based on whitespaces, returns one token at a time
int getIntParam(char* line); // passed line, returns integer param
char* getStrParam(char* line); // passed line, returns string param
struct Param startup(struct Param m); // reads shconfig, searching for parameters
int more(char* argv[]);

struct Param{
int vsize;
int hsize; 
int rport;
char *rhost;
};

int main(void){
    int pipefd[2]; // pipe is used to communicate socket descriptor information from child to parent
  if(pipe(pipefd) == -1){
    perror("pipe");
    exit(EXIT_FAILURE);
  }
  char s[MAXCHAR+1];
  memset(s, '\0', sizeof(s));
  struct Param m = {-1,-1,-1,s};
  m = startup(m);

  int localFlag; // localFlag is set to 1 when command prefixed with '!' (local command)
  int bkgFlag; // bkgFlag set to 1 when command prefixed with '&' (background command)
  int constcxnFlag = 0; // constcxnFlag set to 1 when keepalive cmd has been entered
  int cxnDone = 0; // used in scenario of constcxnFlag equal to 1, is set after intial cxn has been made
  int sd; // file descriptor
  char line[MAXCHAR+1];
  char* cmd;
  while(1){
    localFlag = 0;
    bkgFlag = 0; // reset bkgFlag for each new command
    memset(line, '\0', sizeof(line)); // make sure line is cleared each time
    printf("@ "); 
    fgets(line, MAXCHAR, stdin); // normally returns not null
    char *endptr = strchr(line, '\n');
    if(endptr == NULL){ //fgets retains newline character. If there is none, it means cmd was longer than MAXCHAR
    printf("%s\n",line);
      printf("command too long\n");
      while(getchar() != '\n');
      break;
    }

    cmd = line;
    if((line[0] == '!') && (line[1] == ' ')){ // '!' cmd must be before '&' cmd
      localFlag = 1;
      cmd = cmd + 2;
    }
    if((*cmd == '&') && (*(cmd+1) == ' ')){
      bkgFlag = 1;
      cmd = cmd + 2;
    }

    if(localFlag == 1){ 

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
        tokenArr[i] = token; // i is the number of tokens
        i++;
      }
      int tokenNum = i; // i will be the number of tokens in the cmd line
      if((strcmp(tokenArr[0], "exit") == 0) && (tokenNum == 1)){
        if(constcxnFlag == 1){ // if we want to exit but we still have socket active
          shutdown(sd, SHUT_RDWR);
          close(sd);
        }
        return 0; // kills shell, but note that background cmd ran by child continues even after the shell is killed
      }

      else if((strcmp(tokenArr[0], "more") == 0) && (tokenNum == 2)){
        char vsizeStr[10];
        char hsizeStr[10];
        sprintf(vsizeStr, "%d", m.vsize); // convert int to string
        sprintf(hsizeStr, "%d", m.hsize);
        char *moreTokenArr[] = {tokenArr[0], tokenArr[1], vsizeStr, hsizeStr};
        if(more(moreTokenArr) == -1){
          printf("failure of internal more command\n");
        }
      }
      else if((strcmp(tokenArr[0], "keepalive") == 0) && (tokenNum == 1)){
        // all the subsequent remote commands will use the same connection to the server.
        constcxnFlag = 1;
        cxnDone = 0;
      }

      else if((strcmp(tokenArr[0], "close") == 0) && (tokenNum == 1)){
        // close connection
        constcxnFlag = 0; // reverts behavior of remote commands to close socket after request fufilled
        shutdown(sd, SHUT_RDWR); // Closes the connection to the server in case such a connection is open
        close(sd);
        sd = -1;
      }

      else{
        int child = fork();
        if(child == -1){
          perror("fork");
          exit(EXIT_FAILURE);
        }
        if (child == 0){ // if child process
          int status;
          int error;
          const char *bin = "/bin/";
          const char *usrbin = "/usr/bin/";
          char path[MAXCHAR+9];
          strcpy(path, bin);
          strcat(path, tokenArr[0]); // only want to copy command if we are not exceeding MAXCHAR
          if(bkgFlag == 0){ // child process executes command
            if((error = execve(path, tokenArr, 0)) == -1){ // check bin
              strcpy(path, usrbin);
              strcat(path, tokenArr[0]);
              if((error = execve(path, tokenArr, 0)) == -1){ // check usr bin path
                if((error = execve(cmd, tokenArr, 0)) == -1){ // check cmd
                  perror("perror: ");
                  exit(errno);
                }
              }
            }
          }
          else{   // background commands are executed by granchild child prints message when executable is finished
            int grandchild = fork();
            if(grandchild == -1){
              perror("fork");
              exit(EXIT_FAILURE);
            }
            if (grandchild == 0){ // if grandchild process
              if((error = execve(path, tokenArr, 0)) == -1){ // check if bin path
                strcpy(path, usrbin);
                strcat(path, tokenArr[0]);
                if((error = execve(path, tokenArr, 0)) == -1){ // check if usrbin path
                  if((error = execve(cmd, tokenArr, 0)) == -1){  // check if absolute path
                  perror("perror: ");
                  exit(errno);
                  }
                }
              }
            }
            else { // if child process AND a background command was requested
              waitpid(grandchild, &status, 0);
              printf("background command terminated\n");
              return 0;
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
// -----------------------------------------------------------------
    else{ // localFlag == 0
      if(m.rport == -1 || m.rhost[0] == '\0'){
      printf("Cannot find host or port number in shconfig\n");
      return 0;
      }
      if(constcxnFlag == 1){ // keepalive cmd
        if(cxnDone == 0){ // if this is the first cmd send to server after keepalive, we will need to allocate a socket
          sd = connectbyportint(m.rhost, m.rport);
          printf("Connected to %s on port %d.\n", m.rhost, m.rport);

          cxnDone = 1;
        }
        else{ // cxnDone == 1 
          read(pipefd[0], &sd, sizeof(sd));
        }

        if(sd < 0){ // sd < 0 means the socket was disconnected, allocate new socket
          sd = connectbyportint(m.rhost, m.rport);
          printf("Connected to %s on port %d.\n", m.rhost, m.rport);
        }
        else { // no need to allocate socket
        }
      }
      else{ // constcxnFlag == 0, always new connection
        sd = connectbyportint(m.rhost, m.rport);
        printf("Connected to %s on port %d.\n", m.rhost, m.rport);
      }

      int child = fork();
      if(child == -1){
        perror("fork");
        exit(EXIT_FAILURE);
      }
      if (child == 0){ // if child process
        int status;
        if(bkgFlag == 0){
          if (sd == err_host) {
            fprintf(stderr, "Cannot find host %s.\n", m.rhost);
            return 1;
          }
          if (sd < 0) {
            perror("connectbyport");
            return 1;
          }
          int n;
          fflush(stdout);
          send(sd,line,strlen(line),0);
          char ans[MAXCHAR+1];
          while ((n = recv_nonblock(sd,ans,MAXCHAR,2500)) != recv_nodata) { // while still recieving bits. recv_nonblock reads bits from sd and puts it into ans buffer
            if (n == 0) { // no more server response
              if(constcxnFlag == 1){ // do not close socket
                close(pipefd[0]); // process writes to pipe
                write(pipefd[1], &sd, sizeof(sd));
                close(pipefd[1]);
              } 
              else{
                shutdown(sd, SHUT_RDWR);
                close(sd);
                sd = -1;
                printf("Connection closed by %s.\n", m.rhost);
              }
              return 0;
            }
            if (n < 0) {
                perror("recv_nonblock");
                shutdown(sd, SHUT_WR);
                close(sd);
                sd = -1;
                if(constcxnFlag == 1){
                  close(pipefd[0]); // process writes to pipe
                  write(pipefd[1], &sd, sizeof(sd));
                  close(pipefd[1]);
                }
                break;
            }
            ans[n] = '\0';
            printf("%s", ans);
            fflush(stdout); // not sure why we fflush
          }
          // cannot assume a null terminated server response.
          if(constcxnFlag == 1){ // do not close socket
            close(pipefd[0]); // process writes to pipe
            write(pipefd[1], &sd, sizeof(sd));
            close(pipefd[1]);
          } 
          else{
            shutdown(sd, SHUT_RDWR);
            close(sd);
            sd = -1;
            printf("Connection closed by %s.\n", m.rhost);
          }
          return 0;
        }
        else{ // bkgFlag == 1, grandchild will connect to server, child will print termination message
          int grandchild = fork();
          if(grandchild == -1){
            perror("fork");
            exit(EXIT_FAILURE);
          }
          if (grandchild == 0){ // if grandchild process
          if (sd == err_host) {
            fprintf(stderr, "Cannot find host %s.\n", m.rhost);
            return 1;
          }
          if (sd < 0) {
            perror("connectbyport");
            return 1;
          }
          int n;
          char str[MAXCHAR+1];
          memset(str, '\0', MAXCHAR);
          strncpy(str, cmd, MAXCHAR);
          fflush(stdout);
          send(sd,str,strlen(str),0);
          char ans[MAXCHAR+1];
          while ((n = recv_nonblock(sd,ans,MAXCHAR,2500)) != recv_nodata) {
            if (n == 0) { 
              if(constcxnFlag == 1){
                close(pipefd[0]); 
                write(pipefd[1], &sd, sizeof(sd));
                close(pipefd[1]); 
              } 
              else{
                shutdown(sd, SHUT_RDWR);
                close(sd);
                sd = -1;
                printf("Connection closed by %s.\n", m.rhost);
              }
              return 0;
            }
            if (n < 0) {
                perror("recv_nonblock");
                shutdown(sd, SHUT_WR);
                close(sd);
                sd = -1;
                if(constcxnFlag == 1){
                  close(pipefd[0]); 
                  write(pipefd[1], &sd, sizeof(sd));
                  close(pipefd[1]);
                }
                break;
            }
            ans[n] = '\0';
            printf("%s", ans);
            fflush(stdout); 
          }
          if(constcxnFlag == 1){ // do not close sd
            close(pipefd[0]);
            write(pipefd[1], &sd, sizeof(sd));
            close(pipefd[1]); 
          } 
          else{
            shutdown(sd, SHUT_RDWR);
            close(sd);
            sd = -1;
            printf("Connection closed by %s.\n", m.rhost);
          }
          return 0;
          } else { // if child process AND a background command was requested
            waitpid(grandchild, &status, 0);
            printf("server answer complete\n");
            if(constcxnFlag == 0){
              shutdown(sd, SHUT_RDWR);
              close(sd);
            }
            return 0;
          }
        }
      }
      else{ // if parent
        int status;
        if(localFlag == 1){
          if(bkgFlag == 0){
            waitpid(child, &status, 0);
          }
        }
        else{ // localFlag == 0, sent to server
          if(bkgFlag == 0){ // not a background command
            waitpid(child, &status, 0); // wait
            if(constcxnFlag == 0){ // if not a const cxn
              shutdown(sd, SHUT_RDWR);
              close(sd);
            }
          }
          else{ // background cmmd
            if(constcxnFlag == 0){
              close(sd);
            }
          }
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

int getIntParam(char* bufChunk){
  bufChunk = bufChunk+5; // +5 because HSIZE, VSIZE, RPORT is 5 characters
  int param = atoi(bufChunk); // atoi ignores white spaces, returns numerical characters
  return param;
}

char* getStrParam(char* bufChunk){
  int size = 0;
  bufChunk = bufChunk+5; // +5 because RHOST is 5 characters
  while(*bufChunk == ' '){
    bufChunk++;
  }
  // buf points to start of hostname
  char* tmp = bufChunk;
  while(*tmp != '\n' && *tmp != '\0'){
    size++;
    tmp++;
  }

  char* s = (char*)malloc((size+1)*(sizeof(char)));

  int i = 0;
  for(char* t = bufChunk; *t != '\n' && *t != '\0'; t++){ 
    s[i] = *t;
    i++;
  }
  s[size+1] = '\0';
  if(size>MAXCHAR){
    printf("size of str parameter greater than MAXCHAR\n");
    s[0] = '\0';
  }
  return s;
}

struct Param startup(struct Param m){ // can only use systems call

  int fd;
  int character;
 
  char buffer[MAXCHAR+1]; // can hold a max of 255 characters
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
  character = read(fd, &buffer, sizeof(buffer)-1);  // read max of 254 characters from the file, 255th character is always '\0' for safety reasons
  if (character < 0){
    printf("error % d, read sys call\n", errno);
    perror("perror ");
  }
  //printf("bytes read: % d\n", character);
  char* bufChunk = buffer;
  char* tmp = buffer;
  if(character <= fileSize){ // read() will not need to be called again
    while(1){ 
      if(*tmp == '\n' || *tmp == '\0'){ // if we've gotten to the end of a line or EOF
        if (strncmp(bufChunk, "VSIZE", 5) == 0){
          // printf("vsize found\n");
          m.vsize = getIntParam(bufChunk);
        }
        else if (strncmp(bufChunk, "HSIZE", 5) == 0){
          // printf("hsize found\n");
          m.hsize = getIntParam(bufChunk);
        }
        else if (strncmp(bufChunk, "RPORT", 5) == 0){
          // printf("rport found\n");
          m.rport = getIntParam(bufChunk);
        }
        else if (strncmp(bufChunk, "RHOST", 5) == 0){
          // printf("rhost found\n");
          char* strParam = getStrParam(bufChunk);
          strncpy(m.rhost,strParam, MAXCHAR); 
          free(strParam);
          // copies MAXCHAR characters, m.rhost is initialized as having null terminator as the MAXCHAR+1 element
        }

        if(*tmp == '\0'){ 
          break;
        } 
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
    if((error = write(fd, "VSIZE 40\n", sizeBuf)) == -1){
      printf("error % d, VSIZE not appended to shconfig\n", errno);
    }
    m.vsize = 40;
    if((error = write(fd, "HSIZE 75", sizeBuf-1)) == -1){
      printf("error % d, HSIZE not appended to shconfig\n", errno);
    }
    m.hsize = 75;
  }

  if(m.vsize == -1 && m.hsize != 1){ // if vsize is NOT set
    printf("vsize not set\n");
    if((error = write(fd, "\nVSIZE 40", sizeBuf)) == -1){
      printf("error % d, VSIZE not appended to shconfig\n", errno);
    }
    m.vsize = 40;
  }
  if(m.hsize == -1 && m.vsize != 1){ // if hsize is NOT set
    printf("hsize not set\n");
    if((error = write(fd, "\nHSIZE 75", sizeBuf)) == -1){
      printf("error % d, HSIZE not appended to shconfig\n", errno);
    }
    m.hsize = 75;
  }
  if(m.rport == -1 || m.rhost[0] == '\0'){
    printf("RPORT and/or RHOST not found in shconfig. Nonlocal commands will not be possible");
  }
  close(fd);
  return m;
}

int more(char* argv[]){ // argv = {"more", "filename", vsizeStr, hsizeStr};

  const char *currentDir = "./";
  char path[MAXCHAR+2];
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
  //printf("vsize=%d, hsize=%d\n", vsize, hsize);

  int character;
  while((character = read(fd, &singleChar, 1)) == 1){ // read one character at a time
  
    if(v < vsize){
    
      if(h < hsize){
        if(singleChar != '\n'){ // if v does not need to be incremented    
          printf("%c", singleChar); // could be any character except '\0'
          h++; // remark that we have printed one character
          continue; // get the next character
        }
      }

      if(singleChar != '\n' && singleChar != '\0') { 
        int character;
        do {
          character = read(fd, &singleChar, 1);
          //printf("\ndiscarding %c\n", singleChar);
        }
        while(singleChar != '\n' && character == 1); // eat characters until newline is found or EOF

        if(character == 0){
          break;
        }
        else if(character == -1){
          perror("perror : ");
          exit(errno);
        }
      }

      // singleChar equals newline here
      printf("%c", singleChar); // print and increment newline
      v++;

      h=0; // EITHER we found a newline so we reset horizontal char count OR h==hsize, and we consumed chars until we got to newline, must reset count
      if(v < vsize){
        continue;
      }
      v++;
    }
    
    printf("\npress enter to continue");
    input = getchar();
    if(input != '\n'){
      break;
    }
    else{
      v=0; // reset line count and continue in while loop
      continue;
    }
  }

  if(character == -1){
    perror("perror : ");
    exit(errno);
  }  
  return 0;
}
