#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<netdb.h>
#include	<arpa/inet.h>

int	tcp_server(int prog, int rdwr);
int	tcp_done(int prog);
int	tcp_accept(int sock, int rdwr);
int	tcp_connect(char *host, int prog, int rdwr);
void	sock_optimize(int sock, int rdwr);
int	sockport(int s);
