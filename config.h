const char *field_sep = "  \u2022  ";
const char *time_fmt  = "%a %b %e, %l:%M %p";

const int update_period = 15;

char *
loadavg(void)
{
	double avgs[3];

	if (getloadavg(avgs, 3) < 0) {
		perror("getloadavg");
		exit(1);
	}

	return smprintf("%.2f %.2f %.2f", avgs[0], avgs[1], avgs[2]);
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

