# ttpch

Tor transparent proxying clearnet helper is a tool, that monitors route changes
via netlink messages and appends/deletes rules to specific iptables chains
accordingly. This allows you to use Tors transparent Proxying on your laptop and
automatically adapt to your current internet setup.

## Dependencies

ttpch requires [libmnl](https://netfilter.org/projects/libmnl/) and the iptables
command.

## Usage

Run ttpch as a system level service. Logging information is sent to
stdout/stderr. Of course it needs a fitting iptables ruleset loaded to work
properly. If any errors occur, they are logged and ignored.

ttpch has fixed calls to iptables. If they do not fit your iptables setup, they
can be easily changed by editing the code. ttpch was created with the following
iptables ruleset in mind (There are two places where you have to change `111` to
the uid of the `tor` user on your system):

	*nat
	:OUTPUT ACCEPT
	# chain nat_clearnet accepts all packets that should not be transmitted over tor
	:nat_clearnet -
	
	# tor filtering (substitute the uid of your tor user)
	-A OUTPUT -o lo -j ACCEPT
	-A OUTPUT -d 127.0.0.0/8 -j ACCEPT
	-A OUTPUT -m owner --uid-owner 111 -j ACCEPT
	-A OUTPUT -p udp -m udp --dport 53 -j REDIRECT --to-ports 9053
	-A OUTPUT -j nat_clearnet
	-A OUTPUT -p tcp -m tcp --tcp-flags FIN,SYN,RST,ACK SYN -j REDIRECT --to-ports 9040
	COMMIT
	
	*filter
	:OUTPUT DROP
	# chain filter_clearnet accepts all packets that should not be transmitted over tor
	:filter_clearnet -
	
	-A OUTPUT -o lo -j ACCEPT
	-A OUTPUT -d 127.0.0.0/8 -j ACCEPT
	-A OUTPUT -m state --state RELATED,ESTABLISHED -j ACCEPT
	# see https://lists.torproject.org/pipermail/tor-talk/2014-March/032507.html
	-A OUTPUT -m state --state INVALID -j DROP
	# substitute the uid of your tor user
	-A OUTPUT -m owner --uid-owner 111 -j ACCEPT
	-A OUTPUT -j filter_clearnet
	COMMIT

You can use the following `torrc` snippet or adapt the ports above to your
setup:

	VirtualAddrNetworkIPv4 10.192.0.0/10
	AutomapHostsOnResolve 1
	TransPort 9040
	DNSPort 9053

## TODO

config.h for the iptables calls.
