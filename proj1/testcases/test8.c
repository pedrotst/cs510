#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int fd, fd2;
char c, d, e;

int main()
{

  fd = open("foo.txt", O_RDONLY);
  fd2 = open("boo.txt", O_CREAT | O_WRONLY);
  d = '\0';
  e = '\0'; 
  read(fd, &c, 1);

  for (int i = 0; i < 4; i++)
  {
      if (i%2){
        d = c;
        c = e;
      }

      else{
        e = d;
      }
  }
  write(fd2, &e, 1);

  return 0;

}


