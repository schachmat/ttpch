/* For license details see file LICENSE */

#include <errno.h>
#include <libmnl/libmnl.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
	struct rtmsg *rthdr;
	const struct nlattr *attr;
	const char *dst;
	const size_t clen = 67;
	char cmd[clen];

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

		dst = mnl_attr_get_payload(attr);

		/* unfortunately iptables provides no stable API and they recommend
		 * calling the command in their FAQ.
		 * TODO: Use fork, exec and wait instead to avoid the extra shell
		 * process */
		snprintf(cmd, clen, "iptables -w -%s filter_clearnet -d %hhu.%hhu.%hhu.%hhu/%hhu -j ACCEPT",
		         nlh->nlmsg_type == RTM_NEWROUTE ? "A" : "D",
		         dst[0], dst[1], dst[2], dst[3], rthdr->rtm_dst_len);
		if (0 == system(cmd))
			eprintf("succesfully completed: %s", cmd);
		snprintf(cmd, clen, "iptables -w -t nat -%s nat_clearnet -d %hhu.%hhu.%hhu.%hhu/%hhu -j ACCEPT",
		         nlh->nlmsg_type == RTM_NEWROUTE ? "A" : "D",
		         dst[0], dst[1], dst[2], dst[3], rthdr->rtm_dst_len);
		if (0 == system(cmd))
			eprintf("succesfully completed: %s", cmd);
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
