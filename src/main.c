/*******************************************************************************
MIT License

Copyright (c) 2021 Zhihong Li <humble_zh@163.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <event.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <mosquitto.h>
#include "pidfile.h"
#include "ver.h"
#include "l.h"
#include "zhbox.h"

#define PID_FILE    "/var/run/%s.pid"
#define OUT_FILE    "/tmp/%s.log"

static int need_start = 0,
           need_stop = 0,
           need_daemon = 1;

static struct event_base *base = NULL;
static char *configfile;
static char *includedir;
static char pid_file[PATH_MAX];

static int get_opt(int argc, char **argv);
static void termination(int sig);
static int daemonize(void);
static void usage(void);

int main (int argc, char **argv)
{
    int old_pid;

    if (get_opt(argc, argv)) { return (-1); }

    sprintf(pid_file, PID_FILE, PROG);
    if (check_pid(pid_file)) {
        old_pid = read_pid(pid_file);
        if (need_stop) {
            if(kill(old_pid, SIGTERM) < 0){ fprintf(stderr, "kill():%s\n", strerror(errno)); }
            else { printf("zhbox (%d) Stop.\n", old_pid); }
        }
        else {
            fprintf(stderr, "zhbox (%d) was already run.\n", old_pid);
            return (-1);
        }
    }
    else {
        remove_pid(pid_file);
    }

    if (need_stop) { return (0); }
    if (need_daemon) {
        if (!daemonize()) { printf("Daemonizing succeeds.\n"); }
        else { fprintf(stderr, "Deamonizing failed.\n"); }
    }
    if(0 == write_pid(pid_file)){ return (-1); }
    signal(SIGTERM, termination);

    base = event_base_new();
    if(base == NULL) { l_e("event_base_new() failed"); return -1; }

    mosquitto_lib_init();

    if(zhbox_init(base, configfile, includedir) < 0){ return -1; }

    event_base_dispatch(base);

    l_d("zhbox exiting");
    zhbox_destory();
    mosquitto_lib_cleanup();
    event_base_free(base);
    remove_pid(pid_file);
    return (0);
}

static int get_opt(int argc, char **argv)
{
    int opt;
    struct option long_opts[] = {
        {"help",       0,  NULL,  'h'},
        {"stop",       0,  NULL,  'S'},
        {"start",      0,  NULL,  's'},
        {"configfile", 1,  NULL,  'c'},
        {"includedir", 1,  NULL,  'i'},
        {"foreground", 0,  NULL,  'f'},
        {"Version",    0,  NULL,  'V'},
        {0,            0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "hSsc:i:fV", long_opts, NULL)) != -1) {
        switch (opt) {
            case 'S': need_stop = 1; break;
            case 's': need_start = 1; break;
            case 'c': configfile = optarg; break;
            case 'i': includedir = optarg; break;
            case 'f': need_daemon = 0; break;
            case 'V': printf(PROG " - %s\n", ver); return 0;
            case '?':
            case 'h':
            default:  usage(); return -1;
        }
    }

    if (!need_start && !need_stop) { usage(); return (-1); }

    if (need_start && configfile == NULL) { usage(); return (-1); }

    if (includedir == NULL) { includedir = "/etc/zhbox/"; }

    return (0);
}

static void termination(int sig) { l_i("stoping"); event_base_loopbreak(base); }

static int daemonize(void)
{
    char out_file[PATH_MAX];
    int pid;

    pid = fork();
    if (pid == -1) { return (-1); }
    else if (pid != 0) { exit(0); }
    setsid();

    pid = fork();
    if (pid == -1) { return (-1); }
    else if (pid != 0) { exit(0); }

    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

    close(0);
    close(1);
    close(2);

    open("/dev/null", O_RDWR);
    sprintf(out_file, OUT_FILE, PROG);
    open(out_file, O_WRONLY|O_CREAT|O_TRUNC, 0600);
    open(out_file, O_WRONLY|O_CREAT|O_TRUNC, 0600);
    chdir("/");
    umask(0);
    return (0);
}

static void usage(void)
{
#define USAGE "\
zhbox - %s\n\
Usage: \n\
  zhbox -<hsSc:> -[f]\n\
    -h,        --help,                   print this manual\n\
    -s,        --start,                  start the process\n\
    -S,        --stop,                   stop the process\n\
    -c <file>, --configfile <file>,      start the process with <file>\n\
    -i <dir>,  --includedir <directory>, change the include dir, default: /etc/zhbox/\n\
    -f,        --foreground,             start without daemonized\n\
e.g.\n\
  # start with the /etc/file.cfg\n\
    zhbox -sc /etc/file.cfg\n\n\
  # Stop the process\n\
    zhbox -S\n"

    printf(USAGE, ver);
}
