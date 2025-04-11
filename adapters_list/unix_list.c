// List local networ addresses in Unix-based system
// This code is from Network programming in C - Lewis Van Winkle

#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>

int main(){

    // Declare struct which store the addresses
    struct ifaddrs *addresses;

    // A call to getifaddrs function allocates memory and fills in a linked list of addresses 
    if(getifaddrs(&addresses) == -1){
        printf("getifaddrs call failed \n");
        return -1;
    }

    struct ifaddrs *address = addresses;

    // For each address identify the family
    // AF_INET == IPv4
    // AF_INET6 == IPv6
    // Other types can be return from sa_family
    while(address){
        int family = address->ifa_addr->sa_family;
        if(family == AF_INET || family == AF_INET6){

            printf("%s\t", address->ifa_name);
            printf("%s\t", family == AF_INET ? "IPv4" : "IPv6");

            // Allocate buffer for save textual address and then print result
            char ap[100];

            const int family_size = family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);

            getnameinfo(address->ifa_addr, family_size, ap, sizeof(ap), 0, 0, NI_NUMERICHOST);
            printf("%s\n", ap);
        }
        address = address->ifa_next;
    }

    // Free allocated memeory before end program
    freeifaddrs(addresses);
    return 0;

}
