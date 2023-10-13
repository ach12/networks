#include <stdio.h>
#include <unistd.h>

int main(void){
  for(int i=0; i<6; i++){
  sleep(2);
  printf("%d/5\n", i);
  }
return 0;
}