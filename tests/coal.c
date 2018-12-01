#include <stdlib.h>
#include <stdio.h>

int main()
{
  char * ptr1 =(char*)malloc( 100 );
  char * ptr2 =(char*)malloc( 100 );

  free( ptr1 );
  free( ptr2 );

  char * ptr3 = (char*)malloc( 200 );

  printf("%p == %p\n", ptr1, ptr3 );

  return 0;
}
