#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int fd1, fd2;
char c;
char d;
char e;

int main()
{

  fd1 = open("foo.txt", O_RDONLY);
  fd2 = open("boo.txt", O_CREAT | O_WRONLY);

  read(fd1, &c, 1);

  e = 3 *c + d - 3;

  write(fd2, &e,1);

  return 0;

}