#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

// source: https://stackoverflow.com/questions/16620779/printing-tokenized-data-from-file-in-c


int main( int argc, char** argv ){
    int vsize;
    int hsize;
    int vsizeFound;
    int hsizeFound;
    int searched;
    const char *delimiter_characters = " ";
    const char *paramFile = "shconfig.txt";
    FILE *input_file = fopen( paramFile, "r+" );
    printf("%s", argv[0]);
    FILE *read_file = fopen(argv[0], "r");

    char buffer[ BUFFER_SIZE ];
    char *last_token;

    if( input_file == NULL ){
        fprintf( stderr, "Unable to open file %s\n", paramFile );
    }else{
        // Read each line into the buffer
        while( fgets(buffer, BUFFER_SIZE, input_file) != NULL ){
            // Write the line to stdout
            //fputs( buffer, stdout );
            char* line;
            char* vsizeParam = "VSIZE";
            char* hsizeParam = "HSIZE";
            line = buffer;
            // printf("line:%s\n", buffer);
            // Gets each token as a string and prints it
            last_token = strtok( buffer, delimiter_characters );
            int resultV = strcmp(last_token, vsizeParam);
            if (resultV == 0) {
              // printf("strcmp(str1, str2) = %d\n", resultV);
              vsizeFound = 1;
              if( last_token != NULL ){
                  last_token = strtok( NULL, delimiter_characters );
                  // printf( "%s\n", last_token );
                  int vsizeIn = atoi(last_token);
                  vsize = vsizeIn;              
              }
            }
            int resultH = strcmp(last_token, hsizeParam);
            if (resultH == 0) {
              // printf("strcmp(str1, str2) = %d\n", resultH);
              hsizeFound = 1;
              if( last_token != NULL ){
                  last_token = strtok( NULL, delimiter_characters );

                  // printf( "%s\n", last_token );
                  int hsizeIn = atoi(last_token);
                  hsize = hsizeIn;
              }
            }
        }
        if( ferror(input_file) ){
              perror( "The following error occurred" );
          }
        if (fgets(buffer, BUFFER_SIZE, input_file) == NULL ){
          if (vsizeFound && hsizeFound)  {
            printf("%s", "found both\n");  
          }
          if (vsizeFound && !hsizeFound)  {
            printf("%s", "only found vsize, adding HSIZE 75\n");
            fprintf(input_file, "HSIZE 75\n");
            hsize = 75;

          }
          if (!vsizeFound && hsizeFound)  {
            printf("%s", "only found hsize, adding VSIZE 40\n");
            fprintf(input_file, "VSIZE 40\n");
            vsize = 40;
          }
          if (!vsizeFound && !hsizeFound){
            printf("%s", "no params found\n");
            vsize = 40;
            hsize = 75;
            fprintf(input_file, "VSIZE  40\n");
            fprintf(input_file, "HSIZE 75\n");
          }
        }
        printf("VSIZE: %d\n", vsize);
        printf("HSIZE: %d\n", hsize);
        fclose( input_file );
    }

    if( input_file == NULL ){
        fprintf( stderr, "Unable to open file %s\n", paramFile );
    }else{
        // Read each line into the buffer
        while( fgets(buffer, BUFFER_SIZE, input_file) != NULL ){
            // Write the line to stdout
            //fputs( buffer, stdout );
            char* line;
            char* vsizeParam = "VSIZE";
            char* hsizeParam = "HSIZE";
            line = buffer;
            // printf("line:%s\n", buffer);
            // Gets each token as a string and prints it
            last_token = strtok( buffer, delimiter_characters );
            int resultV = strcmp(last_token, vsizeParam);
            if (resultV == 0) {
              // printf("strcmp(str1, str2) = %d\n", resultV);
              vsizeFound = 1;
              if( last_token != NULL ){
                  last_token = strtok( NULL, delimiter_characters );
                  // printf( "%s\n", last_token );
                  int vsizeIn = atoi(last_token);
                  vsize = vsizeIn;              
              }
            }
            int resultH = strcmp(last_token, hsizeParam);
            if (resultH == 0) {
              // printf("strcmp(str1, str2) = %d\n", resultH);
              hsizeFound = 1;
              if( last_token != NULL ){
                  last_token = strtok( NULL, delimiter_characters );

                  // printf( "%s\n", last_token );
                  int hsizeIn = atoi(last_token);
                  hsize = hsizeIn;
              }
            }
        }
        if( ferror(input_file) ){
              perror( "The following error occurred" );
          }
        if (fgets(buffer, BUFFER_SIZE, input_file) == NULL ){
          if (vsizeFound && hsizeFound)  {
            printf("%s", "found both\n");  
          }
          if (vsizeFound && !hsizeFound)  {
            printf("%s", "only found vsize, adding HSIZE 75\n");
            fprintf(input_file, "HSIZE 75\n");
            hsize = 75;

          }
          if (!vsizeFound && hsizeFound)  {
            printf("%s", "only found hsize, adding VSIZE 40\n");
            fprintf(input_file, "VSIZE 40\n");
            vsize = 40;
          }
          if (!vsizeFound && !hsizeFound){
            printf("%s", "no params found\n");
            vsize = 40;
            hsize = 75;
            fprintf(input_file, "VSIZE  40\n");
            fprintf(input_file, "HSIZE 75\n");
          }
        }
        printf("VSIZE: %d\n", vsize);
        printf("HSIZE: %d\n", hsize);
        fclose( input_file );
    }
    return 0;

}