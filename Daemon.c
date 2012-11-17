


int Daemon()
{
	if(fork()!=0) exit(1);	
	 
	/* ����һ���µĻ����� */  
	if(setsid()<0)exit(1);	
	 
	/* �����ź�SIGHUP */  
	act.sa_handler=SIG_IGN;  
	sigemptyset(&act.sa_mask);	
	act.sa_flags=0;  
	if(sigaction(SIGHUP,&act,NULL)==-1)exit(1);  
	 
	/* �ӽ����˳�,�����û�п����ն��� */  
	if(fork()!=0) exit(1);	
	
	if(chdir("/")==-1)exit(1);	
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









































