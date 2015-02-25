// For license details see file LICENSE

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
	struct rtmsg *hdr;

//	if (!nlh || nlh->nlmsg_pid || nlh->nlmsg_seq || nlh->nlmsg_flags)
//		return MNL_CB_OK;
	if (nlh->nlmsg_type != RTM_NEWROUTE && nlh->nlmsg_type != RTM_DELROUTE)
		return MNL_CB_OK;

	hdr = mnl_nlmsg_get_payload(nlh);
	if (hdr->rtm_dst_len == 0) /* We don't want to handle the default route */
		return MNL_CB_OK;
	if (hdr->rtm_family != AF_INET) /* Tor only works with IPv4 atm */
		return MNL_CB_OK;
	if (hdr->rtm_scope != RT_SCOPE_LINK)
		return MNL_CB_OK;
/*	if (hdr->rtm_table != RT_TABLE_MAIN)
		return MNL_CB_OK; */
	if (hdr->rtm_type != RTN_UNICAST)
		return MNL_CB_OK;

	eprintf("ipv4 route-%s: %u %u %u %u   %x",
			nlh->nlmsg_type == RTM_NEWROUTE ? "add" : "del",
			hdr->rtm_table, hdr->rtm_protocol, hdr->rtm_scope, hdr->rtm_type, hdr->rtm_flags);
	mnl_nlmsg_fprintf(stdout, nlh, nlh->nlmsg_len, sizeof(struct rtmsg));

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
