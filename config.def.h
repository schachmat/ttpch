/* Every entry has to be of the form: (char *[]){STRINGLIST, (char*)NULL}
 * The first element of STRINGLIST is the program executed, the remaining ones
 * are the arguments passed to this program. The Argument following "-d" is
 * replaced with the route, that is added/removed in the form 192.168.42.128/25
 */

/* exec this commands when adding a route. */
char **const addcalls[] = {
	(char *[]){"iptables", "-w", "-A", "filter_clearnet", "-d", "PLACEHOLDER", "-j", "ACCEPT", (char*)NULL},
	(char *[]){"iptables", "-w", "-t", "nat", "-A", "nat_clearnet", "-d", "PLACEHOLDER", "-j", "ACCEPT", (char*)NULL}
};

/* exec this commands when removing a route */
char **const delcalls[] = {
	(char *[]){"iptables", "-w", "-D", "filter_clearnet", "-d", "PLACEHOLDER", "-j", "ACCEPT", (char*)NULL},
	(char *[]){"iptables", "-w", "-t", "nat", "-D", "nat_clearnet", "-d", "PLACEHOLDER", "-j", "ACCEPT", (char*)NULL}
};
