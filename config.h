const char *field_sep = "  \u2022  ";
const char *time_fmt  = "%a %b %e, %l:%M %p";
const char *deg_sym   = "\u00B0";

char *batloc = "/sys/class/power_supply/BAT0";
char *temploc = "/sys/class/hwmon/hwmon0";

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

char *
gettemperature(void)
{
	char *co;
	char *ret;

	co = readfile(temploc, "temp1_input");
	if (co == NULL)
		return smprintf("");

	ret = smprintf("%02.0f%sC", atof(co) / 1001, deg_sym);
	free(co);
	return ret;
}

static char *(*forder[])(void) = {
	loadavg,
	gettemperature,
	getbattery,
	prettytime,
	NULL
};

