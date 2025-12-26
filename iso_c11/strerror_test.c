#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/errno.h>

// wrapper for strerror
int
sprintf_errno(int	errnum, char *buf, int sz)
{
	if (strerror_r(errnum, buf, sz) == ERANGE)
		fprintf(stderr, "Not enough space in buf (%d) for error message\n", sz);
	int		len = strnlen(buf, sz);
	return len;
}

int
main(int argc, char *argv[])
{
	int		BUFFER_SIZE = 50, cnt = 150;	//25

	if (argc > 1)
		cnt = atoi(argv[1]);
	if (argc > 2)
		BUFFER_SIZE = atoi(argv[2]);

	printf("\nExec with cnt %d, buffer %d\n\n", cnt, BUFFER_SIZE);

	char	buf[BUFFER_SIZE];

	for (int i = 0; i < cnt; i++)
	{
		int		len = sprintf_errno(i, buf, sizeof buf);
		printf("Errno(%d): [%s]\t", i, buf);
		printf("[%d]\n", len);
	}

	return 0;
}
