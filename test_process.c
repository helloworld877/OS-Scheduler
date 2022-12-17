#include "headers.h"




int main()

{
	int pid;
	char*argv[2];
	char buff1[5];
	int x = 70;
	int arr[1];
	sprintf(buff1, "%d", x);
	argv[1] = buff1;
	pid = fork();
	if (pid!=0)
		arr[0] = pid;
	if (pid == 0)
		execv("./process.out", argv);
	else
	{
		sleep(5);
		kill(arr[0], SIGUSR2);
	}


}
