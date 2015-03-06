/* exec this commands when adding a route */
char **const addcalls[] = {
	(char *[]){"echo", "iptables", "-w", "-A", "filter_clearnet", "-d", "123456789ABCDEFGHI", "-j", "ACCEPT", (char*)NULL},
	(char *[]){"echo", "iptables", "-w", "-t", "nat", "-A", "nat_clearnet", "-d", "123456789ABCDEFGHI", "-j", "ACCEPT", (char*)NULL}
};

/* exec this commands when removing a route */
char **const delcalls[] = {
	(char *[]){"echo", "iptables", "-w", "-D", "filter_clearnet", "-d", "123456789ABCDEFGHI", "-j", "ACCEPT", (char*)NULL},
	(char *[]){"echo", "iptables", "-w", "-t", "nat", "-D", "nat_clearnet", "-d", "123456789ABCDEFGHI", "-j", "ACCEPT", (char*)NULL}
};
