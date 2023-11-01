#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int fd1, fd2;
char c, d;

int foo(int x, int y)
{
  write(fd2, &x, 1);
  return 0;
}

int boo(int x, int y)
{
  write(fd2, &y, 1);
}

int main()
{

  fd1 = open("foo.txt", O_RDONLY);
  fd2 = open("boo.txt", O_CREAT | O_WRONLY);

  read(fd1, &c, 1);
  d = 5;

  foo(c, d);
  boo(c, d);

  return 0;

}
