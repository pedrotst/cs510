#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int fd1, fd2;

char c, d, e;
char f, g;


int foo(int x, int y, int z)
{
  return (y+z);
}
int boo(int x, int y, int z)
{
  return (x+y);
}

int main()
{

  fd1 = open("foo.txt", O_RDONLY);
  fd2 = open("boo.txt", O_CREAT | O_WRONLY);

  read(fd1, &c, 1);

  f = boo(c, d, e);
  g = foo(c, d, e);

  write(fd2, &f, 1);
  write(fd2, &g, 1);

  return 0;
}
