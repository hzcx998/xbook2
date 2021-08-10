#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<netdb.h>
#include	<arpa/inet.h>

int	udp_server(u_long prog, int rdwr);
void	udp_done(int prog);
int	udp_connect(char *host, u_long prog, int rdwr);
void	sock_optimize(int sock, int rdwr);
int	sockport(int);

