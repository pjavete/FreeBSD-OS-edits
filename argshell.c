#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#define SIZE 256

extern char **get_args();

//Command with 1 or more arguments
void defaultCommand(char **args)
{
	int pid = 0;
	int exeNum = 0;

	//creates a fork
	pid = fork();
	if (pid < 0)
	{
		//Error message if fork fails
		perror("Error: fork did not complete");
		exit(0);
	}
	else if (pid == 0)
	{
		exeNum = execvp(args[0], args);
		if (exeNum < 0)
		{
			//error message if execvp fails to execute
			perror("Error: execvp failed to execute");
			exit(0);
		}
	}
	else
	{
		//waits for child to finish
		wait();
	}
}

//input redirector
void inputRedirect(char **args, char *file)
{
	int fd = 0;
	int nfd = 0;
	int pid = 0;
	int exeNum = 0;

	if ((pid = fork()) < 0)
	{
		//Error message if fork fails
		perror("Error: fork did not complete");
		exit(0);
	}
	else if (pid == 0)
	{
		fd = open(file, O_RDONLY);
		nfd = dup2(fd, 0);
		if (nfd < 0)
		{
			//Error message if dup2 fails
			perror("Error: dup2 failed to execute");
			exit(0);
		}
		close(fd);
		exeNum = execvp(args[0], args);
		if (exeNum < 0)
		{
			//error message if execvp fails to execute
			perror("Error: execvp failed to execute");
			exit(0);
		}
	}
	else
	{
		//waits for child to finish
		wait();
	}
}

//output redirector
void outputRedirect(char **args, char *file)
{
	int fd = 0;
	int nfd = 0;
	int pid = 0;
	int exeNum = 0;

	pid = fork();
	if (pid < 0)
	{
		//Error message if fork fails
		perror("Error: fork failed to execute");
		return;
	}
	else if (pid == 0)
	{
		//referenced from Piazza Post
		fd = creat(file, 0666);
		nfd = dup2(fd, 1);
		if (nfd < 0)
		{
			//Error message if dup2 fails
			perror("Error: dup2 failed to execute");
			exit(0);
		}
		close(fd);
		exeNum = execvp(args[0], args);
		if (exeNum < 0)
		{
			//error message if execvp fails to execute
			perror("Error: execvp failed to execute");
			exit(0);
		}
	}
	else
	{
		//waits for child to finish
		wait();
	}
}

//output redirector append
void outputRedirectAppend(char **args, char *file)
{
	int fd = 0;
	int nfd = 0;
	int pid = 0;
	int exeNum = 0;

	pid = fork();
	if (pid < 0)
	{
		//Error message if fork fails
		perror("Error: fork failed to execute");
		return;
	}
	else if (pid == 0)
	{
		//referenced from https://stackoverflow.com/questions/28466715/using-open-to-create-a-file-in-c
		fd = open(file, O_RDWR | O_APPEND | O_CREAT, 0666);
		nfd = dup2(fd, 1);
		if (nfd < 0)
		{
			//Error message if dup2 fails
			perror("Error: dup2 failed to execute");
			exit(0);
		}
		close(fd);
		exeNum = execvp(args[0], args);
		if (exeNum < 0)
		{
			//error message if execvp fails to execute
			perror("Error: execvp failed to execute");
			exit(0);
		}
	}
	else
	{
		//waits for child to finish
		wait();
	}
}

//pipe Command
void pipeCommand(char **args, int i)
{
	int pipefd[2];
	int pid = 0;
	int pid2 = 0;
	int fd = 0;
	int nfd = 0;
	int pipeid = 0;
	int exeNum = 0;
	int pipeCount = 0;
	int numArgs = 0;
	char *pipeArgs[SIZE];
	char *pipeArgs2[SIZE];
	args[i] = NULL;

	for (int j = 0; args[j] != NULL; j++)
	{
		pipeArgs[j] = args[j];
	}

	for (int k = i; args[k] != NULL; k++)
	{
		pipeArgs2[k - i] = args[k + 1];
	}

	//create the pipe
	pipeid = pipe(pipefd);
	if (pipeid == -1)
	{
		//Error message if pipe fails
		perror("Error: pipe did not complete");
		exit(0);
	}
	//create the fork
	pid = fork();
	if (pid < 0)
	{
		//Error message if fork fails
		perror("Error: fork failed to execute");
		exit(0);
	}
	else if (pid == 0)
	{
		pid2 = fork();
		if (pid2 < 0)
		{
			perror("Error: fork failed to execute");
			exit(0);
		}
		else if (pid2 == 0)
		{
			close(pipefd[1]);
			nfd = dup2(pipefd[0], 0);
			if (nfd < 0)
			{
				//Error message if dup2 fails
				perror("Error: dup2 failed to execute");
				exit(0);
			}
			exeNum = execvp(pipeArgs[0], pipeArgs);
			if (exeNum < 0)
			{
				//error message if execvp fails to execute
				perror("Error: execvp failed to execute");
				exit(0);
			}
			close(pipefd[0]);
		}
		else
		{
			close(pipefd[0]);
			nfd = dup2(pipefd[1], 1);
			if (nfd < 0)
			{
				//Error message if dup2 fails
				perror("Error: dup2 failed to execute");
				exit(0);
			}
			exeNum = execvp(pipeArgs2[0], pipeArgs2);
			if (exeNum < 0)
			{
				//error message if execvp fails to execute
				perror("Error: execvp failed to execute");
				exit(0);
			}
			close(pipefd[1]);
			wait();
		}
	}
	else
	{
		wait();
	}
}

/*
void standardError(char **args, char *file)
{
	int pipefd[2];
	int fd = 0;
	int nfd = 0;
	int val = 0;
	int pid = 0;
	int exeNum = 0;

	pid = fork();
	if (pid < 0)
	{
		//Error message if fork fails
		perror("Error: fork failed to execute");
		exit(0);
	}
	else if (pid == 0)
	{
		//Referenced from Piazza
		fd = creat(file, 0666);
		nfd = dup2(fd, 1);
		if (nfd < 0)
		{
			//Error message if dup2 fails
			perror("Error: dup2 failed to execute");
			exit(0);
		}
		close(fd);
		exeNum = execvp(args[0], args);
		if (exeNum < 0)
		{
			//error message if execvp fails to execute
			perror("Error: execvp failed to execute");
			exit(0);
		}
	}
	else
	{
		//waits for child to finish
		wait();
	}
}
*/

void changeDirectory(char **args)
{
	int pid = 0;
	int pathChange = 0;
	char *path;
	char *buf[SIZE];
	long size;

	pid = fork();
	if (pid < 0)
	{
		perror("Error: fork failed to execute");
		exit(0);
	}
	else if (pid == 0)
	{
		path = getcwd(buf, SIZE);
		pathChange = chdir(path);
		if (pathChange < 0)
		{
			perror("Error: cd failed to execute");
			exit(0);
		}
	}
	else
	{
		wait();
	}
}

int main()
{
	int i;
	char **args;
	char *file;

	while (1)
	{
		printf("Command ('exit' to quit): ");
		args = get_args();
		for (i = 0; args[i] != NULL; i++)
		{
			printf("Argument %d: %s\n", i, args[i]);
		}
		if (args[0] == NULL)
		{
			printf("No arguments on line!\n");
		}
		else if (!strcmp(args[0], "cd"))
		{
			changeDirectory(args);
		}
		else if (!strcmp(args[0], "exit"))
		{
			printf("Exiting...\n");
			exit(0);
			break;
		}
		else
		{
			for (i = 0; args[i] != NULL; i++)
			{
				if (!strcmp(args[i], "<"))
				{
					args[i] = NULL;
					file = args[i + 1];
					inputRedirect(args, file);
				}
				else if (!strcmp(args[i], ">"))
				{
					args[i] = NULL;
					file = args[i + 1];
					outputRedirect(args, file);
				}
				else if (!strcmp(args[i], ">>"))
				{
					args[i] = NULL;
					file = args[i + 1];
					outputRedirectAppend(args, file);
				}
				else if (!strcmp(args[i], "|"))
				{
					pipeCommand(args, i);
				}
				else
				{
					defaultCommand(args);
				}
			}
		}
	}
}