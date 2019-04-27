/*
Operation Systems - Igor Rochlin
19/03/19
Ex2 - Compare 2 files
Bar Zrihan 203285770 בר זריהן
Ilay Pilosof 304961519 עילי פילוסוף
*/

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>


// #####################  For MacOS - taken from linux/limits.h
#define NR_OPEN	        1024

#define NGROUPS_MAX    65536	/* supplemental group IDs are available */
#define ARG_MAX       131072	/* # bytes of args + environ for exec() */
#define LINK_MAX         127	/* # links a file may have */
#define MAX_CANON        255	/* size of the canonical input queue */
#define MAX_INPUT        255	/* size of the type-ahead buffer */
#define NAME_MAX         255	/* # chars in a file name */
#define PATH_MAX        4096	/* # chars in a path name including nul */
#define PIPE_BUF        4096	/* # bytes in atomic write to a pipe */
#define XATTR_NAME_MAX   255	/* # chars in an extended attribute name */
#define XATTR_SIZE_MAX 65536	/* size of an extended attribute value (64k) */
#define XATTR_LIST_MAX 65536	/* size of extended attribute namelist (64k) */

#define RTSIG_MAX	  32
// #####################  For MacOS - taken from linux/limits.h

#define BUFFER_SIZE 1024
#define RESULT_FILE_NAME "results.csv"
#define STDOUT_FILE_NAME "stdout.txt"
#define EXEC_FILE_NAME "main.exe"

int readFile(int fd, char * buffer)
{
	int readBytes;
	int totalBytes = 0;
	do
	{
		readBytes = read(fd, buffer, BUFFER_SIZE);
		if (readBytes < 0)
		{
			// printf("error reading the file\n");
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
		// printf("error reading the line\n");
		_exit(res);
	}
	// printf("done reading the line: %s\n", buffer);
	return count;
}

int executeAndWait(char* program, char * args[], int childFdIn, int childFdOut)
{
	pid_t pid;
	int status;

	if ((pid = fork()) < 0)
	{     /* fork a child process*/
		// printf("forking child process failed\n");
		_exit(1);
	}
	else if (pid == 0)
	{
		// replace stdin/stdout
		if (0 != childFdIn) dup2(childFdIn, STDIN_FILENO);
		if (0 != childFdOut) dup2(childFdOut, STDOUT_FILENO);

		/* for the child process: */
		/* execute the command  */
		if (execvp(program, args) < 0)
		{
			// printf("exec failed: %d\n", errno);
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
	return status;
}


int main(int argc, char* argv[])
{
	/* 
	input file descriptors:
		fd1 - Program's config
		fd2 - Inputs
		fd3 - stdout 
		fd4 - CSV results
	 */
	int fd1, fd2, fd3, fd4;
	int res;  /* function return code result */
	int studentScore;
	char studentsDirectoryPath[PATH_MAX]; /* input (output) buffer */
	char inputFilePath[PATH_MAX]; /* input (output) buffer */
	char outputFilePath[PATH_MAX]; /* input (output) buffer */
	char studentPath[PATH_MAX]; /* input (output) buffer */
	char pathToFile[PATH_MAX]; /* input (output) buffer */
	char resultLine[PATH_MAX]; /* input (output) buffer */

	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd));
	char * _file = "/comp.out";
	strcat(cwd, _file);

	char* execArgv[] = { NULL, NULL, NULL, NULL };
	DIR* dir, *studentDir;
	struct dirent* studentsDirent, *studentDirent;

	// Validate program usage before continue
	if (argc != 2) {
// 		printf("Usage: %s <config file>\n"\
// 			"config format:\n"\
// 			"LINE 1: path to user's directory\n"\
// 			"LINE 2: path to input file\n"\
// 			"LINE 3: path to output file\n"\
// 			, argv[0]);

		return 0;
	}

	// read config file
	fd1 = open(argv[1], O_RDONLY);
	if (fd1 < 0)
	{
		// printf("error open config file %s\n", argv[1]);
		return 0;
	}
	// read the first 3 lines, each line indicates different path
	readline(fd1, studentsDirectoryPath);
	readline(fd1, inputFilePath);
	readline(fd1, outputFilePath);
	close(fd1);  // config file is not needed anymore, closing it

	// open input file
	fd2 = open(inputFilePath, O_RDONLY);
	if (fd2 < 0)
	{
		// printf("error open input file %s\n", inputFilePath);
		return 0;
	}

	// open stdout file
	fd3 = open(STDOUT_FILE_NAME, O_CREAT | O_RDWR | O_TRUNC, S_IWUSR | S_IRUSR);
	if (fd3 < 0)
	{
		// printf("error open stdout file %s\n", STDOUT_FILE_NAME);
		close(fd2);
		return 0;
	}

	// open results file
	fd4 = open(RESULT_FILE_NAME, O_CREAT | O_RDWR | O_TRUNC, S_IWUSR | S_IRUSR);
	if (fd4 < 0)
	{
		// printf("error open results file %s\n", RESULT_FILE_NAME);
		close(fd2);
		close(fd3);
		return 0;
	}

	// dir the students directory
	dir = opendir(studentsDirectoryPath);
	if (NULL == dir)
	{
		// printf("cannot open dir %s\n", studentsDirectoryPath);
		close(fd2);
		close(fd3);
		close(fd4);
		return 0;
	}

	// start the main loop over students directory
	while ((studentsDirent = readdir(dir)))
	{
		// enters only to students directories
		if (studentsDirent->d_type != DT_DIR ||
			0 == strcmp(studentsDirent->d_name, ".") ||
			0 == strcmp(studentsDirent->d_name, ".."))
		{
			// skip -> item is not a directory or it's ./..
			continue;
		}

		// printf("student name: %s\n", studentsDirent->d_name);
		sprintf(studentPath, "%s%s", studentsDirectoryPath, studentsDirent->d_name);
		studentDir = opendir(studentPath);
		if (NULL == studentDir)
		{
			// printf("cannot open studentDir %s\n", studentsDirent->d_name);
			close(fd2);
			close(fd3);
			close(fd4);
			closedir(dir);
			return 0;
		}

		// go over the files inside specific student
		while ((studentDirent = readdir(studentDir)))
		{
			studentScore = sprintf(resultLine, "%s,%d\n", studentsDirent->d_name, 0);
			// skip -> item is not a file
			if (studentDirent->d_type != DT_REG) continue;
			// check if it's a main.exe file
			if (0 != strcmp(studentDirent->d_name, EXEC_FILE_NAME)) continue;

			sprintf(pathToFile, "%s/%s", studentPath, studentDirent->d_name);
			// set inputs and output file to the beggining of the file
			if (lseek(fd2, 0, SEEK_SET) < 0 || lseek(fd3, 0, SEEK_SET) < 0)
			{
				// printf("cannot seek file back to 0");
                		close(fd2);
                		close(fd3);
                		close(fd4);
				closedir(dir);
				closedir(studentDir);
                		return 0;
			}

			// build args
			execArgv[0] = pathToFile;
			execArgv[1] = NULL;
			res = executeAndWait(execArgv[0], execArgv, fd2, fd3);
			studentScore = sprintf(resultLine, "%s,%d\n", studentsDirent->d_name, 0);
			if (res != 0) {
				break;
			}
			// compare program output to the expected output
			execArgv[0] = cwd;
			execArgv[1] = outputFilePath;
			execArgv[2] = STDOUT_FILE_NAME;
			execArgv[3] = NULL;
			res = executeAndWait(execArgv[0], execArgv, 0, 0);
			if (WEXITSTATUS(res) == 2) {
				// printf("YAY 100!\n");
				studentScore = sprintf(resultLine, "%s,%d\n", studentsDirent->d_name, 100);
			}
			break;
		} 
		// write student results to file
		if (0 > write(fd4, resultLine, studentScore))
                {
			close(fd2);
                        close(fd3);
                        close(fd4);
                        closedir(dir);
			closedir(studentDir);
                        return 0;
                }	
		closedir(studentDir);
	} // root students loop

	close(fd2);
	close(fd3);
	closedir(dir);
	return 0;
}


