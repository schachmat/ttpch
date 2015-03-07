/* For license details see file LICENSE */

#include <errno.h>
#include <libmnl/libmnl.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "config.h"

#define LENGTH(X)               (sizeof X / sizeof X[0])

/* replace with define from libmnl.h after next libmnl release */
#define SOCK_BUF_SIZ (sysconf(_SC_PAGESIZE) < 8192L ? sysconf(_SC_PAGESIZE) : 8192L)

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

int cb(const struct nlmsghdr *nlh, void *data)
{
	const size_t lencalls[2] = {LENGTH(delcalls), LENGTH(addcalls)};
	struct rtmsg *rthdr;
	const struct nlattr *attr;
	const char *d;
	const size_t dstlen = 19;
	char dst[dstlen];
	pid_t child;
	int add;
	int i, j;
	char **const *calls;

	if (nlh->nlmsg_type != RTM_NEWROUTE && nlh->nlmsg_type != RTM_DELROUTE)
		return MNL_CB_OK;

	rthdr = mnl_nlmsg_get_payload(nlh);
	if (rthdr->rtm_dst_len == 0) /* We don't want to handle the default route */
		return MNL_CB_OK;
	if (rthdr->rtm_family != AF_INET) /* Tor only works with IPv4 atm */
		return MNL_CB_OK;
	if (rthdr->rtm_scope != RT_SCOPE_LINK)
		return MNL_CB_OK;
/*	if (rthdr->rtm_table != RT_TABLE_MAIN)
		return MNL_CB_OK; */
	if (rthdr->rtm_type != RTN_UNICAST)
		return MNL_CB_OK;

/*	eprintf("ipv4 route-%s: %u %u %u %u   %x",
			nlh->nlmsg_type == RTM_NEWROUTE ? "add" : "del",
			rthdr->rtm_table, rthdr->rtm_protocol, rthdr->rtm_scope, rthdr->rtm_type, rthdr->rtm_flags);
	mnl_nlmsg_fprintf(stdout, nlh, nlh->nlmsg_len, sizeof(struct rtmsg)); */

	mnl_attr_for_each(attr, nlh, sizeof(struct rtmsg)) {
		if (attr->nla_type != RTA_DST)
			continue;

		add = nlh->nlmsg_type == RTM_NEWROUTE;
		calls = add ? addcalls : delcalls;
		d = mnl_attr_get_payload(attr);

		/* unfortunately iptables provides no stable API. The devs recommend
		 * calling the command in their FAQ. */

		snprintf(dst, dstlen, "%hhu.%hhu.%hhu.%hhu/%hhu",
		         d[0], d[1], d[2], d[3], rthdr->rtm_dst_len);
		eprintf("%sed route to %s", add ? "add" : "dropp", dst);

		for (i = 0; i < lencalls[add]; i++) {
			for (j = 0; calls[i][j] != NULL; j++) {
				if (0 == strcmp(calls[i][j], "-d") && calls[i][j+1] != NULL)
					calls[i][j+1] = dst;
			}
			if (0 == j)
				continue;

			if (0 == (child = fork())) { /* child */
				execvp(calls[i][0], calls[i]);
				eprintf("failed to execvp() %s:", calls[i][0]);
			} else if (-1 != child) { /* parent */
				if (wait(NULL) != child)
					eprintf("failed to wait() for %s:", calls[i][0]);
			} else { /* fail */
				eprintf("failed to fork for %s:", calls[i][0]);
			}
		}

		return MNL_CB_OK;
	}

	return MNL_CB_OK;
}

int main(int argc, char* argv[])
{
	struct mnl_socket *nl;
	ssize_t len;
	char *buf;

	if ((struct mnl_socket*)-1 == (nl = mnl_socket_open(NETLINK_ROUTE)))
		die("could not open netlink socket:");

	if (-1 == mnl_socket_bind(nl, RTMGRP_IPV4_ROUTE, MNL_SOCKET_AUTOPID)) {
		eprintf("could not bind netlink socket:");
		goto exit;
	}

	if (!(buf = malloc(SOCK_BUF_SIZ))) {
		eprintf("could not malloc buffer:");
		goto fail;
	}

	for(;;) {
		if (-1 == (len = mnl_socket_recvfrom(nl, buf, SOCK_BUF_SIZ)))
			eprintf("failed to recv netlink msg:");

		mnl_cb_run(buf, len, 0, 0, cb, NULL);
	}

fail:
	free(buf);
exit:
	if (-1 == mnl_socket_close(nl))
		die("could not close netlink socket:");
	return 0;
}
