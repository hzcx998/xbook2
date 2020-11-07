#include <stdio.h>

int main()
{
  int i;
  printf("hello, world\n");
  i = 0;
  while (i < 10) {
    printf("idx: %d\n", i);
    ++i;
  }
  return 0;
}
