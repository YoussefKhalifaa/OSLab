#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
extern int errno;

int main()
{
	int fd = open("foo.txt", O_RDONLY | O_CREAT);
	if (fd == -1)
	{
		perror("PROGRAM\n");
	}
	return 0;
}
