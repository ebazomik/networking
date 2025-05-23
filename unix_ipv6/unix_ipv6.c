// Like unix_ipv4 but in ipv6
// This version also have double-stack socket

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

int main() {
  // ***** CONFIGURATION *****
  printf("Local address configuration...\n");
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));

  hints.ai_family = AF_INET6;       // ipv6 address (AF_INET for ipv4)
  hints.ai_socktype = SOCK_STREAM;  // TCP
  hints.ai_flags = AI_PASSIVE;      // accept connection from every network interface

  struct addrinfo *bind_address;
  getaddrinfo(0, "8080", &hints, &bind_address);

  // ***** CREATION *****
  printf("Creating socket...\n");
  SOCKET socket_listen;
  socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype,
                         bind_address->ai_protocol);

  if (!ISVALIDSOCKET(socket_listen)) {
    fprintf(stderr, "socket() failed (%d) \n", GETSOCKETERRNO());
    return 1;
  }

  // ***** DOUBLE-STACK STEP *****

  // For use double-stack (both ipv4 and ipv6 socket), set IPV6_ONLY to false with value 0;
  // ipv4 address -> ::ffff:127.0.0.1 | ipv6 address -> ::1
  int v6only = 0;
  if(setsockopt(socket_listen, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only))){
      fprintf(stderr, "setsockopt() failed (%d) \n", GETSOCKETERRNO());
      return 1;
  }


  // ***** BINDINGS *****
  printf("Binding socket to local address... \n");
  if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
    fprintf(stderr, "bind() failed (%d) \n", GETSOCKETERRNO());
    return 1;
  }
  freeaddrinfo(bind_address);

  // ***** LISTEN *****
  printf("Listening... \n");
  if (listen(socket_listen, 10) < 0) {
    fprintf(stderr, "listen() failed (%d) \n", GETSOCKETERRNO());
    return 1;
  }

  // ***** ACCEPTING *****
  printf("Waiting for connection... \n");
  struct sockaddr_storage client_address;
  socklen_t client_len = sizeof(client_address);
  SOCKET socket_client =
      accept(socket_listen, (struct sockaddr *)&client_address, &client_len);
  if (!ISVALIDSOCKET(socket_client)) {
    fprintf(stderr, "accept() falied. (%d) \n", GETSOCKETERRNO());
    return 1;
  }

  // ***** CONNECTION COMPLETED *****
  printf("Client is connected... \n");
  char address_buffer[100];
  getnameinfo((struct sockaddr *)&client_address, client_len, address_buffer,
              sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
  printf("%s \n", address_buffer);

  // ***** RECIVED CLIENT *****
  printf("REading request... \n");
  char request[1024];
  int bytes_recived = recv(socket_client, request, 1024, 0);
  printf("Recived %d bytes \n", bytes_recived);

  // ***** SENDING RESPONSE *****
  printf("Sending response... \n");
  const char *response = "HTTP/1.1 200 OK\r\n"
                         "Connection: close\r\n"
                         "Content-Type: text/plain\r\n\r\n"
                         "Local time is: ";
  int bytes_sent = send(socket_client, response, strlen(response), 0);
  printf("Sent %d of %d bytes. \n", bytes_sent, (int)strlen(response));

  time_t timer;
  time(&timer);
  char *time_msg = ctime(&timer);
  bytes_sent = send(socket_client, time_msg, strlen(time_msg), 0);
  printf("Sent %d of %d bytes. \n", bytes_sent, (int)strlen(response));

  // ***** CLOSING CONNECTION *****
  printf("Closing connection...\n");
  CLOSESOCKET(socket_client);

  // ***** CLOSING LISTENING SOCKET *****
  printf("Closing listening socket...\n");
  CLOSESOCKET(socket_listen);

  printf("Finished \n");
  return 0;
}
