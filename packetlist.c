#include <stdio.h>
#include <stdlib.h>

int main () {
   char *str;

   /* Initial memory allocation */
   str = (char *) malloc(15);
   strcpy(str, "tutorialspoint");
   printf("String = %d,  Address = %u\n", strlen(str), str);

   /* Reallocating memory */
   str = (char *) realloc(str, 25);
   strcat(str, ".com");
   printf("String = %d,  Address = %u\n", sizeof(str), str);

   free(str);
   
   return(0);
}