/* exec this commands when adding a route */
char **const addcalls[] = {
	(char *[]){"iptables", "-w", "-A", "filter_clearnet", "-d", "PLACEHOLDER", "-j", "ACCEPT", (char*)NULL},
	(char *[]){"iptables", "-w", "-t", "nat", "-A", "nat_clearnet", "-d", "PLACEHOLDER", "-j", "ACCEPT", (char*)NULL}
};

/* exec this commands when removing a route */
char **const delcalls[] = {
	(char *[]){"iptables", "-w", "-D", "filter_clearnet", "-d", "PLACEHOLDER", "-j", "ACCEPT", (char*)NULL},
	(char *[]){"iptables", "-w", "-t", "nat", "-D", "nat_clearnet", "-d", "PLACEHOLDER", "-j", "ACCEPT", (char*)NULL}
};
