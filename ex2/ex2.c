/*
Operation Systems - Igor Rochlin
19/03/19
Ex2 - Compare 2 files
Bar Zrihan 203285770 בר זריהן
Ilay Pilosof 304961519 עילי פילוסוף
*/

#include <linux/limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define PATH_MAX 4096    /* # chars in a path name including null */
#define BUFFER_SIZE 1024
#define RESULT_FILE_NAME "results.csv"
#define STDOUT_FILE_NAME "stdout.txt"
#define GCC_COMMAND "/usr/bin/gcc"

int readFile(int fd, char * buffer)
{
	int readBytes;
	int totalBytes = 0;
	do
	{
		readBytes = read(fd, buffer, BUFFER_SIZE);
		if (readBytes < 0)
		{
			printf("error reading the file\n");
			_exit(readBytes);
		}
		totalBytes += readBytes;

	} while (readBytes == BUFFER_SIZE);
	return totalBytes;
}


int readline(int fd, char * buffer)
{
	int res;
	int count = 0;
	char * ptrLine = buffer;
	while ((res = read(fd, ptrLine, 1)) > 0)
	{
		// break if EOL (end of line)
		if (*ptrLine == '\n' || *ptrLine == '\0')
		{
			// null terminate the file
			*ptrLine = '\0';
			break;
		}
		++ptrLine;
		++count;
	}
	if (res < 0) /* means file read failed */
	{
		printf("error reading the line\n");
		_exit(res);
	}
	printf("done reading the line: %s\n", buffer);
	return count;
}

int executeAndWait(char* program, char * args[], int childFdIn, int childFdOut)
{
	pid_t pid;
	int status;

	if ((pid = fork()) < 0)
	{     /* fork a child process*/
		printf("forking child process failed\n");
		_exit(1);
	}
	else if (pid == 0)
	{
		// replace stdin/stdout
		if (0 != childFdIn) dup2(childFdIn, 0);
		if (0 != childFdOut) dup2(childFdOut, 1);
		/* for the child process: */
		/* execute the command  */
		if (execvp(program, args) < 0)
		{
			printf("exec failed: %d\n", errno);
			_exit(1);
		}
	}
	else
	{
		/* for the parent:      */
		/* wait for completion  */
		wait(&status);
		return status;
	}
}

int main(int argc, char* argv[])
{
	int fd1, fd2, fd3, fd4;   /* input file descriptor */
	int res; /* function result */
	int sizeOfInputFile;
	char studentsDirectoryPath[PATH_MAX]; /* input (output) buffer */
	char inputFilePath[PATH_MAX]; /* input (output) buffer */
	char outputFilePath[PATH_MAX]; /* input (output) buffer */
	char studentPath[PATH_MAX]; /* input (output) buffer */
	char pathToFile[PATH_MAX]; /* input (output) buffer */
	char resultLine[PATH_MAX]; /* input (output) buffer */
	char* execArgv[] = { NULL, NULL, NULL, NULL, NULL, NULL };
	DIR* dir, *studentDir;
	struct dirent* rootDirent, *studentDirent;
	char* dot;

	if (argc != 2) {
		printf("Usage: %s <config file>\n"\
			"config format:\n"\
			"LINE 1: path to user's direcorty\n"\
			"LINE 2: path to input file\n"\
			"LINE 3: path to output file\n"\
			, argv[0]);

		return 0;
	}

	// read config file
	fd1 = open(argv[1], O_RDONLY);
	if (fd1 < 0)
	{
		printf("error open config file %s\n", argv[1]);
		return 0;
	}
	readline(fd1, studentsDirectoryPath);
	readline(fd1, inputFilePath);
	readline(fd1, outputFilePath);
	close(fd1);

	// open input file
	fd2 = open(inputFilePath, O_RDONLY);
	if (fd2 < 0)
	{
		printf("error open input file %s\n", inputFilePath);
		return 0;
	}

	fd3 = open(STDOUT_FILE_NAME, O_CREAT | O_RDWR | O_TRUNC, S_IWUSR | S_IRUSR);
	if (fd3 < 0)
	{
		printf("error open stdout file %s\n", STDOUT_FILE_NAME);
		close(fd2);
		return 0;
	}

	fd4 = open(RESULT_FILE_NAME, O_CREAT | O_RDWR | O_TRUNC, S_IWUSR | S_IRUSR);
	if (fd4 < 0)
	{
		printf("error open results file %s\n", RESULT_FILE_NAME);
		close(fd2);
		close(fd3);
		return 0;
	}

	// dir the students directory
	dir = opendir(studentsDirectoryPath);
	if (NULL == dir)
	{
		printf("cannot open dir %s\n", studentsDirectoryPath);
		close(fd2);
		close(fd3);
		close(fd4);
		return 0;
	}
	while ((rootDirent = readdir(dir)) > 0)
	{
		if (rootDirent->d_type != DT_DIR ||
			0 == strcmp(rootDirent->d_name, ".") ||
			0 == strcmp(rootDirent->d_name, ".."))
		{
			// skip -> item is not a directory or it's ./..
			continue;
		}

		printf("student name: %s\n", rootDirent->d_name);
		sprintf(studentPath, "%s/%s", studentsDirectoryPath, rootDirent->d_name);
		studentDir = opendir(studentPath);
		if (NULL == studentDir)
		{
			printf("cannot open studentDir %s\n", rootDirent->d_name);
			close(fd2);
			close(fd3);
			close(fd4);
			closedir(dir);
			return 0;
		}
		while ((studentDirent = readdir(studentDir)) > 0)
		{
			// skip -> item is not a file
			if (studentDirent->d_type != DT_REG) continue;
			// check if it's a c file
			dot = strrchr(studentDirent->d_name, '.');
			if (!dot || 0 != strcmp(dot, ".c")) continue;

			sprintf(pathToFile, "%s/%s", studentPath, studentDirent->d_name);
			printf("about to run \"%s %s\"\n", GCC_COMMAND, pathToFile);
			// build args
			execArgv[0] = GCC_COMMAND;
			execArgv[1] = pathToFile;
			execArgv[2] = "-o";
			execArgv[3] = "compiled";
			execArgv[4] = NULL;

			res = executeAndWait(execArgv[0], execArgv, 0, 0);

			printf("%s, return code: %d\n", execArgv[0], res);
			if (res != 0) {
				// gcc failed to compile
				// put zero in results
				res = sprintf(resultLine, "%s,%d\n", rootDirent->d_name, 0);
				if (0 > write(fd4, resultLine, res))
				{
					close(fd2);
					close(fd3);
					close(fd4);
					closedir(dir);
					return 0;
				}
				break;
			}
			else // compiled success
			{
				// build args
				execArgv[0] = "/home/student/Desktop/Colman-Y03S02-Operation-Systems/ex2/compiled";
				execArgv[1] = NULL;
				res = executeAndWait(execArgv[0], execArgv, fd2, fd3);
				printf("%s, return code: %d\n", execArgv[0], res);
				res = sprintf(resultLine, "%s,%d\n", rootDirent->d_name, 100);
				if (0 > write(fd4, resultLine, res))
				{
					close(fd2);
					close(fd3);
					close(fd4);
					closedir(dir);
					return 0;
				}
				break;
			}
			// compile only one file
			break;
		} // student's file loop
		closedir(studentDir);
	} // root students loop

	close(fd2);
	close(fd3);
	closedir(dir);
	return 0;
}


