#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>


/* Below is a macro definition */
#define SHW_VAR_ADR(ID, I) (printf("ID %s \t is at virtual address: %16p\n", ID, (void*)&I))
#define SHW_FUNC_ADR(ID, I) (printf("ID %s \t is at virtual address: %16p\n", ID, (void*)(__SIZE_TYPE__)I))


extern int etext, edata, end; /* Global variables for process memory */


char *cptr = "This message is output by the function showit()\n"; /* Static */
char buffer1[25];
int showit(char *p); /* Function prototype */


int main() {
  int i = 0; /* Automatic variable */


  /* Printing addressing information */
  printf("\nAddress etext: %16p \n", (void*)&etext);
  printf("Address edata: %16p \n", (void*)&edata);
  printf("Address end  : %16p \n", (void*)&end);


  SHW_FUNC_ADR("main", main);
  SHW_FUNC_ADR("showit", showit);
  SHW_VAR_ADR("cptr", cptr);
  SHW_VAR_ADR("buffer1", buffer1);
  SHW_VAR_ADR("i", i);
 
  strcpy(buffer1, "A demonstration\n");   /* Library function */
  write(1, buffer1, strlen(buffer1) + 1); /* System call */
  showit(cptr);


  return 0;
} /* end of main function */


/* A function follows */
int showit(char *p) {
  char *buffer2;
  SHW_VAR_ADR("buffer2", buffer2);
 
  if ((buffer2 = (char *)malloc((unsigned)(strlen(p) + 1))) != NULL) {
    printf("Allocated memory at %p\n", (void*)buffer2);
    strcpy(buffer2, p);    /* copy the string */
    printf("%s", buffer2); /* Display the string */
    free(buffer2);         /* Release location */
  } else {
    printf("Allocation error\n");
    exit(1);
  }
 
  return 0;
}
