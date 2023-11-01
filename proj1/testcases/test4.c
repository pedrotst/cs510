#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int fd1, fd2;

char c, d;
char e;

int main()
{

  fd1 = open("foo.txt", O_RDONLY);
  fd2 = open("boo.txt", O_CREAT | O_WRONLY);

  read(fd1, &c, 1);

  d = '\0';
  d = c + d;
  e = c + d + e;

  write(fd2, &e, 1);

  return 0;

}
