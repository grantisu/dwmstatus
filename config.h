#include <sys/sysinfo.h>

const char *field_sep = "  \u2022  ";
const char *time_fmt  = "%a %b %e, %l:%M %p";

const int update_period = 15;

struct sysinfo info;

char *
loadavg(void)
{
	if (sysinfo(&info) < 0) {
		perror("sysinfo");
		exit(1);
	}

	return smprintf("%.2f %.2f %.2f",
		info.loads[0] / 65536.0f,
		info.loads[1] / 65536.0f,
		info.loads[2] / 65536.0f);
}

char *
memusage(void)
{
	unsigned long mtotal, mavail;
	FILE *mstat;

	mstat = fopen("/proc/meminfo", "r");
	if (mstat == NULL)
		return NULL;

	/* Fragile, but works for me: */
	fscanf(mstat, "MemTotal: %lu kB\nMemFree: %*u kB\nMemAvailable: %lu kB", &mtotal, &mavail);
	fclose(mstat);

	return smprintf("%.1f%%", 100.0f*(mtotal - mavail) /mtotal);
}

char *
prettytime(void)
{
	return mktimes(time_fmt, NULL);
}

static char *(*forder[])(void) = {
	loadavg,
	memusage,
	prettytime,
	NULL
};

