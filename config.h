const char *field_sep = "  \u2022  ";
const char *time_fmt  = "%x %I:%M %p";

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

static char *(*forder[])(void) = {
	loadavg,
	prettytime,
	NULL
};

