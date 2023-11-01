#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int fd1, fd2, fd3;
char c, d;

int main ()
{

  fd1 = open("foo.txt", O_RDONLY);
  fd2 = open("boo.txt", O_CREAT | O_WRONLY);

  read(fd1, &d, 1);
  c = d;
  write(fd2, &c, 1);


  return 0;

}

