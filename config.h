const char *field_sep = "  \u2022  ";
const char *time_fmt  = "%a %b %e, %l:%M %p";

char *batloc = "/sys/class/power_supply/BAT0";

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
prettytime(void)
{
	return mktimes(time_fmt, NULL);
}

char *
getbattery(void)
{
	char *co, *ret;
	int cap;

	cap = -1;

	co = readfile(batloc, "capacity");
	if (co == NULL) {
		return smprintf("not present");
	}
	sscanf(co, "%d", &cap);
	free(co);

	co = readfile(batloc, "status");
	if (co == NULL) {
		return smprintf("no status");
	}

	co[4] = '\0';

	ret = smprintf("% 3d%% %s", cap, co);

	free(co);

	return ret;
}

static char *(*forder[])(void) = {
	loadavg,
	getbattery,
	prettytime,
	NULL
};

