# python-ip-stack

An user-level TCP/IP stack built with Python. 
This is an experimental and educational project, with 2 main objectives:

* Brush up knowledge on different protocols used by the TCP/IP stack (e.g., ARP, ICMP, UDP, TCP)
* Learn more about the structure of the TCP/IP stack in Linux
* Learn more about the Python language, which seems to be quite versatile and 
easy to apply to different problems. The main focus is learning Python's 
tools for networking (e.g. [Scapy](http://www.secdev.org/projects/scapy/), 
[pytun](https://pypi.python.org/pypi/python-pytun/2.2.1), etc.).

This work is based on the work of saminiir@gmail.com, who coded 
a minimal TCP/IP stack from scratch in C (check [saminirr's Github](https://github.com/saminiir/level-ip)).

## Minimal example

In the nodes running the custom Python TCP/IP stack:

```
#: cd <python-ip-stack-dir>/src
#: sudo python stack.py --tap-addr='10.0.0.1/24' --node-ip-addr='10.0.0.<x>'
```

* The option `tap-addr` sets the subnet address for the TAP device automatically created by the custom stack, and which acts as the interface over which traffic moves in/out of the custom stack
* The option `node-ip-addr` is the IP address to be adopted by the specific node. This IP address should be in the same subnet as that specified with `tap-addr`. Replace `<x>` with a number between 2 and 254.

As you'll be testing your code over an existing network infrastructure, you may need to add static routes to have traffic routed to the custom Python TCP/IP stack nodes.

E.g., say you're running your Python TCP/IP stack on a node with 'native' IP address 192.168.1.129, and you want to test it against standard network applications (e.g., `ping`) in your MacBook Pro.

Then, you should add the following static route:

```
#: sudo route -n add 10.0.0.1/24 192.168.1.129
```
