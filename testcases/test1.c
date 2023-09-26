#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int fd1, fd2;
char c;

int main ()
{

  fd1 = open("foo.txt", O_RDONLY);
  fd2 = open("boo.txt", O_CREAT | O_WRONLY);

  read(fd1, &c, 1);
  write(fd2, &c, 1);

  return 0;

}

