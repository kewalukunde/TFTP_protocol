// C program to display hostname
// and IP address
// Returns hostname for the local computer
#include <tftp.h>

// Driver code
char *get_ip_address(void)
{
    char hostbuffer[256];
    static char *IPbuffer;
    struct hostent *host_entry;
    int hostname;
 
    // To retrieve hostname
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
	if (hostname == -1)
		error(1, errno, "get host name failed");
 
    // To retrieve host information
    host_entry = gethostbyname(hostbuffer);
    if (host_entry == NULL)
		error(1, errno, "get host by name failed");
 
    // To convert an Internet network
    // address into ASCII string
    IPbuffer = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
 
    //printf("Hostname: %s\n", hostbuffer);
    printf("Host IP: %s\n", IPbuffer);
 
    return IPbuffer;
}
