#define _DEFAULT_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <X11/Xlib.h>

static Display *dpy;
int fd = -1;

char * smprintf(char *fmt, ...);
char * readfile(char *base, char *file);
char * mktimes(const char *fmt, char *tzname);
void setstatus(char *str);
char * joinstrings(char **astr);

#include "config.h"

char *
smprintf(char *fmt, ...)
{
	va_list fmtargs;
	char *ret;
	int len;

	va_start(fmtargs, fmt);
	len = vsnprintf(NULL, 0, fmt, fmtargs);
	va_end(fmtargs);

	ret = malloc(++len);
	if (ret == NULL) {
		perror("malloc");
		exit(1);
	}

	va_start(fmtargs, fmt);
	vsnprintf(ret, len, fmt, fmtargs);
	va_end(fmtargs);

	return ret;
}

char *
readfile(char *base, char *file)
{
	char *path, line[513];
	FILE *fd;

	memset(line, 0, sizeof(line));

	path = smprintf("%s/%s", base, file);
	fd = fopen(path, "r");
	free(path);
	if (fd == NULL)
		return NULL;

	if (fgets(line, sizeof(line)-1, fd) == NULL)
		return NULL;
	fclose(fd);

	return smprintf("%s", line);
}

char *
mktimes(const char *fmt, char *tzname)
{
	char buf[129];
	char *tzorig = NULL;
	time_t tim;
	struct tm *timtm;

	memset(buf, 0, sizeof(buf));
	if (tzname) {
		tzorig = getenv("TZ");
		if (tzorig)
			tzorig = smprintf("%s", tzorig);
		setenv("TZ", tzname, 1);
	}

	tim = time(NULL);
	timtm = localtime(&tim);
	if (timtm == NULL) {
		perror("localtime");
		exit(1);
	}

	if (!strftime(buf, sizeof(buf)-1, fmt, timtm)) {
		fprintf(stderr, "strftime == 0\n");
		exit(1);
	}

	if (tzorig) {
		setenv("TZ", tzorig, 1);
		free(tzorig);
	} else {
		unsetenv("TZ");
	}

	return smprintf("%s", buf);
}

void
setstatus(char *str)
{
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}

char *
joinstrings(char **astr)
{
	char *ret;
	int sz = 127;

	ret = malloc(sz+1);
	if (ret == NULL) {
		perror("malloc");
		exit(1);
	}
	ret[0] = '\0';

	do {
		strncat(ret, *astr, sz);
		sz -= strlen(*astr);
		if (astr[1]) {
			strncat(ret, field_sep, sz);
			sz -= strlen(field_sep);
		}
	} while(sz > 0 && *++astr);

	return ret;
}

char **
applyfuncmap(char *(*fmap[])(void))
{
	int i;
	int sz = 0;
	char **ret;

	while (fmap[sz] != NULL) ++sz;

	ret = malloc((sz+1) * sizeof(char *));
	if (ret == NULL) {
		perror("malloc");
		exit(1);
	}

	for (i=0; i < sz; i++)
		ret[i] = fmap[i]();
	ret[i] = 0;

	return ret;
}

void
updatestatus(int read_fd)
{
	int bytes_read;
	char *status;
	char **astr;
	char *more_str[3];
	char read_buf[128];

	astr = applyfuncmap(forder);
	status = joinstrings(astr);

	if (read_fd) {
		bytes_read = read(fd, read_buf, 127);
		read_buf[bytes_read] = '\0';
		for (int i=0; read_buf[i]; i++) {
			if (read_buf[i] == '\n') {
				read_buf[i] = '\0';
				break;
			}
		}
		more_str[0] = read_buf;
		more_str[1] = status;
		more_str[2] = NULL;

		status = joinstrings(more_str);

		free(more_str[1]);
	}

	setstatus(status);

	free(status);
	for (char **s=astr; *s; s++)
		free(*s);
	free(astr);
}

int
main(void)
{
	struct pollfd poll_event;

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "dwmstatus: cannot open display.\n");
		return 1;
	}

	mkfifo(msg_pipe, 0660);
	fd = open(msg_pipe, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		fprintf(stderr, "dwmstatus: cannot open fifo.\n");
		return 1;
	}

	poll_event.revents = 0;

	do {
		updatestatus(poll_event.revents & POLLIN);
		if (poll_event.revents | POLLHUP) {
			close(fd);
			fd = open(msg_pipe, O_RDONLY | O_NONBLOCK);
			if (fd < 0) {
				fprintf(stderr, "dwmstatus: cannot open fifo.\n");
				return 1;
			}
		}
		poll_event.fd = fd;
		poll_event.events = POLLIN;
		poll_event.revents = 0;
	} while (poll(&poll_event, 1, update_period*1000) >= 0);

	close(fd);
	XCloseDisplay(dpy);

	return 0;
}

