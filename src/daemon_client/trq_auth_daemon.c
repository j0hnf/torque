#include "license_pbs.h" /* See here for the software license */
#include "trq_auth_daemon.h"
#include <pbs_config.h>   /* the master config generated by configure */
#include <sys/stat.h> /* umask */

#include <stdlib.h> /* calloc, free */
#include <stdio.h> /* printf */
#include <string.h> /* strcat */
#include <pthread.h> /* threading functions */
#include <errno.h> /* errno */
#include <syslog.h> /* openlog and syslog */
#include <unistd.h> /* getgid, fork */
#include <grp.h> /* setgroups */
#include "pbs_error.h" /* PBSE_NONE */
#include "pbs_constants.h" /* AUTH_IP */
#include "pbs_ifl.h" /* pbs_default, PBS_BATCH_SERVICE_PORT, TRQ_AUTHD_SERVICE_PORT */
#include "net_connect.h" /* TRQAUTHD_SOCK_NAME */
#include "../lib/Libnet/lib_net.h" /* start_listener */
#include "../lib/Libifl/lib_ifl.h" /* process_svr_conn */
#include "../lib/Liblog/chk_file_sec.h" /* IamRoot */
#include "../lib/Liblog/pbs_log.h" /* logging stuff */
#include "../include/log.h"  /* log events and event classes */

#define MAX_BUF 1024
#define TRQ_LOGFILES "client_logs"

extern char *msg_daemonname;
extern pthread_mutex_t *log_mutex;
extern pthread_mutex_t *job_log_mutex;

extern int debug_mode;
static int changed_msg_daem = 0;

int load_config(
    char **ip,
    int *t_port,
    int *d_port)
  {
  int rc = PBSE_NONE;
  char *tmp_name = pbs_default();
  /* Assume TORQUE_HOME = /var/spool/torque */
  /* /var/spool/torque/server_name */
  if (tmp_name == NULL)
    rc = PBSE_BADHOST;
  else
    {
    /* Currently this only display's the port for the trq server
     * from the lib_ifl.h file or server_name file (The same way
     * the client utilities determine the pbs_server port)
     */
    printf("hostname: %s\n", tmp_name);
    *ip = tmp_name;
    PBS_get_server(tmp_name, (unsigned int *)t_port);
    if (*t_port == 0)
      *t_port = PBS_BATCH_SERVICE_PORT;
    *d_port = TRQ_AUTHD_SERVICE_PORT;
    }
  return rc;
  }

int load_ssh_key(
    char **ssh_key)
  {
  int rc = PBSE_NONE;
  return rc;
  }

int validate_server(
    char *t_server_ip,
    int t_server_port,
    char *ssh_key,
    char **sign_key)
  {
  int rc = PBSE_NONE;
  fprintf(stderr, "pbs_server port is: %d\n", t_server_port);
  return rc;
  }

void initialize_globals_for_log(void)
  {
  strcpy(pbs_current_user, "trqauthd");   
  if ((msg_daemonname = strdup(pbs_current_user)))
    changed_msg_daem = 1;
  }

void clean_log_init_mutex(void)
  {
  pthread_mutex_destroy(log_mutex);
  pthread_mutex_destroy(job_log_mutex);
  free(log_mutex);
  free(job_log_mutex);
  }

int daemonize_trqauthd(const char *server_ip, int server_port, void *(*process_meth)(void *))
  {
  int gid;
  pid_t pid;
  int   rc;
  char  error_buf[MAX_BUF];
  char msg_trqauthddown[MAX_BUF];
  char path_log[MAXPATHLEN + 1];
  char unix_socket_name[MAXPATHLEN + 1];
  char *log_file=NULL;
  int eventclass = PBS_EVENTCLASS_TRQAUTHD;
  const char *path_home = PBS_SERVER_HOME;

  umask(022);

  gid = getgid();
  /* secure supplemental groups */
  if(setgroups(1, (gid_t *)&gid) != 0)
    {
    fprintf(stderr, "Unable to drop secondary groups. Some MAC framework is active?\n");
    snprintf(error_buf, sizeof(error_buf),
                     "setgroups(group = %lu) failed: %s\n",
                     (unsigned long)gid, strerror(errno));
    fprintf(stderr, "%s\n", error_buf);
    return(1);
    }

  if (getenv("PBSDEBUG") != NULL)
    debug_mode = TRUE;
  if (debug_mode == FALSE)
    {
    pid = fork();
    if(pid > 0)
      {
      /* parent. We are done */
      return(0);
      }
    else if (pid < 0)
      {
      /* something went wrong */
      fprintf(stderr, "fork failed. errno = %d\n", errno);
      return(PBSE_RMSYSTEM);
      }
    else
      {
      fprintf(stderr, "trqauthd daemonized - port %d\n", server_port);
      /* If I made it here I am the child */
      fclose(stdin);
      fclose(stdout);
      fclose(stderr);
      /* We closed 0 (stdin), 1 (stdout), and 2 (stderr). fopen should give us
         0, 1 and 2 in that order. this is a UNIX practice */
      if (fopen("/dev/null", "r") == NULL)
        perror(__func__);

      if (fopen("/dev/null", "r") == NULL)
        perror(__func__);

      if (fopen("/dev/null", "r") == NULL)
        perror(__func__);
      }
    }
  else
    {
    fprintf(stderr, "trqauthd port: %d\n", server_port);
    }

    log_init(NULL, NULL);
    log_get_set_eventclass(&eventclass, SETV);
    initialize_globals_for_log();
    sprintf(path_log, "%s/%s", path_home, TRQ_LOGFILES);
    if ((mkdir(path_log, 0755) == -1) && (errno != EEXIST))
      {
         openlog("daemonize_trqauthd", LOG_PID | LOG_NOWAIT, LOG_DAEMON);
         syslog(LOG_ALERT, "Failed to create client_logs directory: errno: %d", errno);
         log_err(errno,"daemonize_trqauthd", "Failed to create client_logs directory");
         closelog();
      }
    pthread_mutex_lock(log_mutex);
    log_open(log_file, path_log);
    pthread_mutex_unlock(log_mutex);

    /* start the listener */
    snprintf(unix_socket_name, sizeof(unix_socket_name), "%s/%s", path_home, TRQAUTHD_SOCK_NAME);
    rc = start_domainsocket_listener(unix_socket_name, process_meth);
    if(rc != PBSE_NONE)
      {
      openlog("daemonize_trqauthd", LOG_PID | LOG_NOWAIT, LOG_DAEMON);
      syslog(LOG_ALERT, "trqauthd could not start: %d\n", rc);
      log_err(rc, "daemonize_trqauthd", (char *)"trqauthd could not start");
      pthread_mutex_lock(log_mutex);
      log_close(1);
      pthread_mutex_unlock(log_mutex);
      if (changed_msg_daem && msg_daemonname) {
          free(msg_daemonname);
      }
      clean_log_init_mutex();
      exit(-1);
      }
    snprintf(msg_trqauthddown, sizeof(msg_trqauthddown),
      "TORQUE authd daemon shut down and no longer listening on IP:port %s:%d",
      server_ip, server_port);
    log_event(PBSEVENT_SYSTEM | PBSEVENT_FORCE, PBS_EVENTCLASS_TRQAUTHD,
      msg_daemonname, msg_trqauthddown);
    pthread_mutex_lock(log_mutex);
    log_close(1);
    pthread_mutex_unlock(log_mutex);
    if (changed_msg_daem && msg_daemonname)
      {
      free(msg_daemonname);
      }
    clean_log_init_mutex();
    exit(0);
  }

void parse_command_line(int argc, char **argv)
  {
  int c;

  while ((c = getopt(argc, argv, "D")) != -1)
    {
    switch (c)
      {
      case 'D':
        debug_mode = TRUE;
        break;

      default:
        fprintf(stderr, "Only the -D flag  is currently supported\n");
        exit(1);
        break;
      }
    }
  }


extern "C"
{
int trq_main(

  int    argc,
  char **argv,
  char **envp)

  {
  int rc = PBSE_NONE;
  char *trq_server_ip = NULL;
  char *the_key = NULL;
  char *sign_key = NULL;
  int trq_server_port = 0;
  int daemon_port = 0;
  void *(*process_method)(void *) = process_svr_conn;

  if (IamRoot() == 0)
    {
    printf("This program must be run as root!!!\n");
    return(PBSE_IVALREQ);
    }


  parse_command_line(argc, argv);
  if ((rc = load_config(&trq_server_ip, &trq_server_port, &daemon_port)) != PBSE_NONE)
    {
    }
  else if ((rc = load_ssh_key(&the_key)) != PBSE_NONE)
    {
    }
  else if ((validate_server(trq_server_ip, trq_server_port, the_key, &sign_key)) != PBSE_NONE)
    {
    }
  else if ((rc = daemonize_trqauthd(AUTH_IP, daemon_port, process_method)) == PBSE_NONE)
    {
    }
  else
    {
    printf("Daemon exit requested\n");
    }
  if (the_key != NULL)
    free(the_key);
  return rc;
  }
}
