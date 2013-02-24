#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mount.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h> 
#include <stddef.h> 
#include <sys/types.h> 
#include <signal.h> 
#include <assert.h>
#include <errno.h>

#define LOCKFILE "/var/run/lnvrDaemon.pid"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IRGRP) //can read|can write|can group read|can other read

int lockfile(int fd)
{
	struct flock fl;
	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;
	return (fcntl(fd, F_SETLK, &fl));
}

int already_runing(void)
{
	int fd;
	char buf[16];

	fd = open(LOCKFILE, O_RDWR|O_CREAT, LOCKMODE);
	if(fd<0)
	{
		printf("can't open %s, %s\n", LOCKFILE, strerror(errno));
		exit(1);
	}
	
	if(lockfile(fd)<0)
	{
		if(errno==EACCES || errno==EAGAIN)
		{
			printf("2222222222lockfile %s, %s\n", LOCKFILE, strerror(errno));
			close(fd);
			return 1;
		}
		
		printf("lockfile %s, %s\n", LOCKFILE, strerror(errno));
		exit(1);
	}

	printf("1111111111111111lockfile %s, %s\n", LOCKFILE, strerror(errno));
	ftruncate(fd, 0);
	sprintf(buf, "%ld", (long)getpid());
	write(fd, buf, strlen(buf)+1);

	return 0;
}

int Daemon2()
{
	struct sigaction act;

	/* 父进程退出 */  
	if(fork()!=0) exit(1);	

	/* 创建一个新的会议组 */  
	if(setsid()<0)exit(1);	

	/* 忽略信号SIGHUP */  
	act.sa_handler=SIG_IGN;  
	sigemptyset(&act.sa_mask);	
	act.sa_flags=0;  
	if(sigaction(SIGHUP,&act,NULL)==-1)exit(1);  

	/* 子进程退出,孙进程没有控制终端了 */  
	if(fork()!=0) exit(1);	

	if(chdir("/")==-1)exit(1);	

	if(already_runing())
	{
		printf("already run\n");
		exit(1);
	}
	
	return 0;
}


int Daemon()
{
	struct sigaction act;

	if(fork()!=0) exit(1);	
	 
	/* 创建一个新的会议组 */  
	if(setsid()<0)exit(1);	
	 
	/* 忽略信号SIGHUP */  
	act.sa_handler=SIG_IGN;  
	sigemptyset(&act.sa_mask);	
	act.sa_flags=0;  
	if(sigaction(SIGHUP,&act,NULL)==-1)exit(1);  
	 
	/* 子进程退出,孙进程没有控制终端了 */  
	if(fork()!=0) exit(1);	
	
	if(chdir("/")==-1)exit(1);

	return 0;
}

int daemonize(int nochdir, int noclose)
{
    int fd;

    switch (fork()) {
    case -1:
        return (-1);
    case 0:
        break;
    default:
        _exit(EXIT_SUCCESS);
    }

    if (setsid() == -1)
        return (-1);

    if (nochdir == 0) {
        if(chdir("/") != 0) {
            perror("chdir");
            return (-1);
        }
    }

    if (noclose == 0 && (fd = open("/dev/null", O_RDWR, 0)) != -1) {
        if(dup2(fd, STDIN_FILENO) < 0) {
            perror("dup2 stdin");
            return (-1);
        }
        if(dup2(fd, STDOUT_FILENO) < 0) {
            perror("dup2 stdout");
            return (-1);
        }
        if(dup2(fd, STDERR_FILENO) < 0) {
            perror("dup2 stderr");
            return (-1);
        }

        if (fd > STDERR_FILENO) {
            if(close(fd) < 0) {
                perror("close");
                return (-1);
            }
        }
    }
    return (0);
}


/*
   //daemonize if requested 
    //if we want to ensure our ability to dump core, don't chdir to / 
    if (do_daemonize) {
        if (sigignore(SIGHUP) == -1) {
            perror("Failed to ignore SIGHUP");
        }
        if (daemonize(maxcore, settings.verbose) == -1) {
            fprintf(stderr, "failed to daemon() in order to daemonize\n");
            exit(EXIT_FAILURE);
        }
    }
*/









































