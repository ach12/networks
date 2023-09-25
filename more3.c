
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

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
// https://gist.github.com/mpontillo/4405788
int count_lines(char* filename)
{
    int fd;
    int character;
    int i;
    int lines = 0;
    static char buf[32768];

    fd = open(filename, O_RDWR|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR);

    if(0 > fd)
    {
        perror(filename);
        return -1;
    }

    while(0 < (character = read(fd, &buf, sizeof(buf))))
    {
        for(i = 0 ; i < character ; i++)
        {
            if(buf[i] == '\n') lines++;
        }
    }

    close(fd);
    
    printf("%8d %s\n", lines, filename);
    return lines;
}


void print_lines(char* filename, int hsize, int vsize)
{
    int fd3;
    int character2;
    
    int stringsIndexFile = 0;

    int linesFile = 0;
    static char bufFile[32768];
    char tempLineFile[500];
    linesFile = count_lines(filename);
    char allLinesFile[linesFile][500];

    fd3 = open(filename, O_RDONLY|O_CREAT|S_IWUSR);

    if(0 > fd3)
    {
        perror(filename);
        return;
    }

    while(0 < (character2 = read(fd3, &bufFile, sizeof(bufFile))))
    {
         int lineIndexFile = 0;
            for(int i = 0 ; i < character2 ; i++)
            {
                tempLineFile[lineIndexFile] = bufFile[i];   //using tempFile to be able to allocate right size in array of lines later
                lineIndexFile++; // to be able to keep track of the index of the line and not just the giant buffer
                if(bufFile[i] == '\n') {     //end of line
                  char lineFile[lineIndexFile+1];
                  // memset(lineFile, 0, sizeof(lineFile));

                  for (int j = 0; j < lineIndexFile; j++){
                    lineFile[j] = tempLineFile[j];
                  }
                  lineFile[lineIndexFile+1] = '\0';      //terminating the line

                  // printf("its a real line: %s\n", line);

                  int len = strlen(lineFile);
                  // printf("length : %i\n",len);
                  // printf("its a line: %s\n", lineFile);
                  strcpy(allLinesFile[stringsIndexFile],lineFile);   // storing in the array of lines

                  // memset(lineFile, 0, sizeof(lineFile));

                  lineIndexFile = 0;
                  stringsIndexFile++;
                  
                }
            }
            // printf("%i\n", vsize);
            // printf("%i\n", hsize);
            int persistHeight = 0;    // to keep track of height in the file
            char c;
            int readAll = 0;
            int pageNum = 1;
            while((c=getchar())=='\n'&& persistHeight < linesFile){    // wait for Enter key and safewatch for segmentation
              if (vsize > linesFile) vsize = linesFile;   //safewatch for segmentation
              for (int height = 0; height < vsize; height++){
                    printf("%.*s\n", hsize, allLinesFile[height]); 
                    persistHeight ++;
                  }
              printf("________________________end of page %i\n", pageNum);  //pretty
              pageNum++;   
            }

            
            printf("File is done reading\n");
            
    }

    close(fd3);
    
    
  // printf("%8d %s\n", linesFile, filename);
                                       
}

int main(int argc, char* argv[])
{
    char* vsizeParam = "VSIZE";
    char* hsizeParam = "HSIZE";
    int vsize;
    int hsize;
    char* paramFileName = "shutil.txt";
    int vsizeFound;
    int hsizeFound;
    int i = 1;
    int lines = 0;
    char* line;
    int closed;
    if(argc < 2)
    {
        return 1;
    }

    for(; i < argc ; i++)
    {
        //////////////dealing with config file. had a hard time passing an array of strings so i decided to do it here
        vsize = 0;
        hsize = 0;
        lines = count_lines(paramFileName);
        int fd2;
        int characters;
        // int i;
        int stringsIndex = 0;
        static char buf[32768];
        char tempLine[500];
        char allLines[lines][500];

        fd2 = open(paramFileName, O_RDWR|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR);

        if(0 > fd2)
        {
            perror(paramFileName);
        }

        while(0 < (characters = read(fd2, &buf, sizeof(buf))))
        {
            int lineIndex = 0;
            for(int i = 0 ; i < characters ; i++)
            {
                tempLine[lineIndex] = buf[i];
                lineIndex++;
                if(buf[i] == '\n') {
                  char line[lineIndex+1];
                  memset(line, 0, sizeof(line));

                  for (int j = 0; j < lineIndex; j++){
                    line[j] = tempLine[j];
                  }
                  line[lineIndex+1] = '\0';

                  // printf("its a real line: %s\n", line);

                  int len = strlen(line);
                  // printf("length : %i\n",len);
                  // printf("its a line: %s\n", line);
                  strcpy(allLines[stringsIndex],line);
                  memset(line, 0, sizeof(line));
                  lineIndex = 0;
                  stringsIndex++;

                }


            }
            // printf("%s\n", line);
            
        }
        // for (int k = 0 ; k<lines; k++){
        //   if (allLines[k] != NULL)
        //   printf("%i, %s\n", k, allLines[k]);
        // }
        // printf("lines : %i\n", lines);

        for (int k = 0 ; k<lines; k++){
          if (allLines[k] != NULL){
            
            // printf("%i, %s\n", k, allLines[k]);
            line = allLines[k];

            int wtspace=0;
            for(char* tmp = line; *tmp != '\n'; tmp++){
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
                token = token_maker(line);
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

            // printf("%s ", tokenArr[0]);
            // printf("%s\n", tokenArr[1]);

            if (tokenArr[1] != NULL) {
              // printf("%s\n", tokenArr[1]);
              int resultV = strcmp(tokenArr[0], vsizeParam);
              // printf("resutlV :%i\n", resultV);

              if (resultV == 0) {
                vsizeFound = 1;
                int vsizeIn = atoi(tokenArr[1]);
                vsize = vsizeIn;
              }
              int resultH = strcmp(tokenArr[0], hsizeParam);
              // printf("resutlH :%i\n", resultH);
              if (resultH == 0) {
                hsizeFound = 1;
                int hsizeIn = atoi(tokenArr[1]);
                hsize = hsizeIn;
              }
            }
          }
        }
        // printf("vsizeFound :%i\n", vsizeFound);
        // printf("hsizeFound :%i\n", hsizeFound);

        if (vsizeFound == 1 && hsizeFound == 1)  {
            closed = close(fd2);
            // printf("%s", "found both\n");  
        }
        if (vsizeFound == 1 && hsizeFound != 1)  { 
          char* hNew = "HSIZE 75\n";
          write(fd2, hNew, strlen(hNew));
          hsize = 75;
          char* end = "\0";
          write(fd2, end, strlen(end));
          closed = close(fd2);

          // printf("%s", "only found vsize, adding HSIZE 75\n");

        }
        if (vsizeFound != 1 && hsizeFound == 1)  {
          // 
          char* vNew = "VSIZE 40\n";
          write(fd2, vNew, strlen(vNew));
          vsize = 40;
          char* end = "\0";
          write(fd2, end, strlen(end));
          closed = close(fd2);

          // printf("%s", "only found hsize, adding VSIZE 40\n");
        }
        if (vsizeFound != 1 && hsizeFound != 1){
          // 
          vsize = 40;
          hsize = 75;
          char* vNew = "VSIZE 40\n";
          write(fd2, vNew, strlen(vNew));
          char* hNew = "HSIZE 75\n";
          write(fd2, hNew, strlen(hNew));
          char* end = "\0";
          write(fd2, end, strlen(end));
          closed = close(fd2);
          // printf("%s", "no params found\n");
        }

        printf("VSIZE: %d\n", vsize);
        printf("HSIZE: %d\n", hsize);
        printf("%i\n", closed);

        ///////////// now lets print the other file
        print_lines(argv[i],hsize, vsize);


      }
  }
    


