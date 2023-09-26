#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int fd1, fd2;

char c, d;
char e;

char x, y;

int foo(int x, int y, int z)
{
  return 0;
}

int boo(int x, int y, int z)
{
  return (x + y + z);
}

int main()
{

  fd1 = open("foo.txt", O_RDONLY);
  fd2 = open("boo.txt", O_CREAT | O_WRONLY);

  read(fd1, &c, 1);
  d = 5;
  e = 4;

  x = foo(c, d, e);
  y = boo(c, d, e);

  write(fd2, &x, 1);
  write(fd2, &y, 1);

  return 0;
}
