// For license details see file LICENSE

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <libmnl/libmnl.h>

void eprintf(const char *format, ...)
{
	va_list ap;

	fputs("ttpch: ", stderr);

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);

	if (format[0] != '\0' && format[strlen(format)-1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}
}

void die(const char *msg)
{
	eprintf(msg);
	exit(1);
}

int main(int argc, char* argv[])
{
	struct mnl_socket *nl;

	if ((struct mnl_socket*)-1 == (nl = mnl_socket_open(NETLINK_ROUTE)))
		die("could not open netlink socket:");

	if (-1 == mnl_socket_bind(nl, RTNLGRP_IPV4_ROUTE | RTNLGRP_IPV6_ROUTE, MNL_SOCKET_AUTOPID)) {
		eprintf("could not bind netlink socket:");
		goto exit;
	}

exit:
	if (-1 == mnl_socket_close(nl))
		die("could not close netlink socket:");
	return 0;
}
