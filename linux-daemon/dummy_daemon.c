#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

#define UID 500
#define GID 500
#define PIDFILE "/var/run/dummy"

int fd0, fd1, fd2;    /* file descriptors for standard files */
struct sigaction sa;  /* for signal handler */


/* generate PID file */
void make_pidfile(char *pidfilename)
  {
  FILE    *pf;         /* We use a FILE to use fscanf */
  int      fd;         /* File descriptor for pid file */
  int      fpid;       /* PID found in existing pidfile */
  int      opid;       /* Our PID */

  pf = fopen(pidfilename, "r");
  if (pf)
    {
    if (fscanf(pf, "%d", &fpid))
      {
      /* We've gotten a PID out of the file.  Is it running? */
      if (!(kill(fpid, 0) == -1 && errno == ESRCH))
        {
        /* Looks like another daemon is running.  Exit. */
        fclose(pf);
        /* Log the failure */
        exit(EXIT_FAILURE);
        }
      }
    /* stale PID file.  remove it */
    fclose(pf);
    if (unlink(pidfilename) != 0)
      {
      /* Could not remove PID file.  Exit.  */
      /* Log the failure */
      exit(EXIT_FAILURE);
      }
    }

  /* Write a new  PID file. */
  fd = creat(pidfilename, 0644);
  if (fd < 0)
    {
    /* Log the failure */
    exit(EXIT_FAILURE);
    }
  if (flock(fd, LOCK_EX | LOCK_NB) < 0)
    {
    /* Log the failure */
    exit(EXIT_FAILURE);
    }
  opid = getpid();     /* get our pid */
  /* Get a FILE pointer so we can use fprintf */
  pf = fdopen(fd, "w");
  if (!pf)
    {
    /* Log the failure */
    exit(EXIT_FAILURE);
    }
  fprintf(pf, "%d\n", opid);
  fflush(pf);
  flock(fd, LOCK_UN);
  close(fd);
  }


/* signal handler -- shoud be more sophisticated */
void handle_sighup(int recv_sig)
  {
  /* do something */

  }

void handle_sigquit(int recv_sig)
  {
  /* do something */

  }

int main(void)
  {
  pid_t pid, sid;     /* process ID and Session ID */

  /* Fork off the parent process */
  pid = fork();
  if (pid < 0)
    {
    exit(EXIT_FAILURE);
    }
    /* exit the parent process. */
    if (pid > 0)
      {
      exit(EXIT_SUCCESS);
      }

    /* Open any logs here */

    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0)
      {
      /* Log the failure */
      exit(EXIT_FAILURE);
      }

    /* Change the current working directory */
    if ((chdir("/")) < 0)
      {
      /* Log the failure */
      exit(EXIT_FAILURE);
      }

    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    /* reopen them pointing to /dev/null */
    fd0 = open ("/dev/null", (O_RDONLY | O_NOCTTY | O_NOFOLLOW));
    fd1 = open ("/dev/null", (O_WRONLY | O_NOCTTY | O_NOFOLLOW));
    fd2 = open ("/dev/null", (O_WRONLY | O_NOCTTY | O_NOFOLLOW));
    if (fd0 != 0 || fd1 != 1 || fd2 != 2 )
      {
      /* Log the failure */
      exit(EXIT_FAILURE);
      }

    /* change user id */
    if (setuid(UID) != 0)
      {
      /* Log the failure */
      exit(EXIT_FAILURE);
      }

    /* change group id */
    if (setgid(GID) != 0)
      {
      /* Log the failure */
      exit(EXIT_FAILURE);
      }

    /* make PID file */
    make_pidfile(PIDFILE);

    /* Change the file mode mask */
    umask(0);

    /* invoke signal handler */
    sa.sa_handler = handle_sighup;
    sa.sa_flags = 0;
    if (sigaction(SIGHUP,&sa,NULL))
      {
      /* Log the failure */
      exit(EXIT_FAILURE);
      }

    sa.sa_handler = handle_sigquit;
    sa.sa_flags = 0;
    if (sigaction(SIGQUIT,&sa,NULL))
      {
      /* Log the failure */
      exit(EXIT_FAILURE);
      }


    /* Daemon-specific initialization goes here */


   /* The Big Loop */
   while (1)
     {
     /* Do some task here ... */

          sleep(30); /* wait 30 seconds */
     }
  exit(EXIT_SUCCESS);
  }



/*
Es geht noch einfacher:

BEZEICHNUNG
       daemon - im Hintergrund ausführen

UEBERSICHT
       #include <unistd.h>

       int daemon(int nochdir, int noclose);

BESCHREIBUNG
       Die  Funktion daemon() gibt einem Programm die Möglichkeit, sich selbst
       vom Terminal in den Hintergrund zu setzen und dort weiter  als  System-
       Daemon zu laufen.

       daemon()  wechselt  das  aktuelle  Arbeitsverzeichnis  nach root ("/"),
       sofern nochdir nicht 0 ist.

       daemon() leitet die Standardausgabe, Standardeingabe und die Fehleraus-
       gabe nach /dev/null um, sofern noclose nicht 0 ist.

RÜCKGABEWERT
       (Diese  Funktion  teilt  den  Prozess  und falls fork() erfolgreich ist,
       führt der Vater ein _exit(0) aus, so dass zukünftige Fehlermeldungen nur
       vom  Kind  gesehen werden.)  Bei Erfolg wird  Null zurückgegeben.  Falls
       ein Fehler auftritt, liefert daemon() -1 zurück und  setzt  die  globale
       Variable  errno  zu einem der Fehler, die für die  Bibliotheksfunktionen
       fork(2) und setsid(2) beschrieben werden.
*/