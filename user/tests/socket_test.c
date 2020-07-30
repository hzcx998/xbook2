#include "test.h"

#include <arpa/inet.h>

#define MAXLINE 4096
int socket_test(int argc, char *argv[])
{
    int sockfd, n; 
    char recvline[4096], sendline[4096]; 
    struct sockaddr_in servaddr; 
    
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    { 
        printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
        exit(0); 
    }

    printf("socket fd: %d\n", sockfd); 

    memset(&servaddr, 0, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port =htons(8080); 
    servaddr.sin_addr.s_addr = inet_addr("192.168.0.104");

    if( connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    { 
        printf("connect error: %s(errno: %d)\n",strerror(errno),errno); 
        exit(0); 
    } 
    printf("send msg to server: \n"); 
    strcpy(sendline, "helllo!\n");
    if( send(sockfd, sendline, strlen(sendline), 0) < 0) 
    { 
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
     exit(0); 
    } 
    memset(sendline, 0, 4096);
    if (recv(sockfd, sendline, 4096, 0) < 0) {
        printf("recv msg error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0); 
    }
    printf("recv: %s\n", sendline);
    close(sockfd); 
    exit(0);
}

int socket_test2(int argc, char *argv[])
{
    printf("socket2 test start!\n");
        
    int err;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        printf("create socket failed!\n");
        return -1;
    }
    printf("create socket %d\n", fd);
    struct sockaddr_in myaddr;
    memset(&myaddr, 0, sizeof(struct sockaddr_in));
    myaddr.sin_addr.s_addr = htonl(0);
    myaddr.sin_port = htons(8080);
    myaddr.sin_family = AF_INET;
    
    err = bind(fd, (struct sockaddr *) &myaddr, sizeof(struct sockaddr));
    if (err < 0) {
        printf("socket bind failed!\n");
        return -1;
    }

    err = listen(fd, 5);
    if (err < 0) {
        printf("socket listen failed!\n");
        return -1;
    }

    struct sockaddr client_addr;
    socklen_t client_len;
    int client_fd;
    while (1) {
        client_fd = accept(fd, NULL, NULL);
        printf("accept %d done!\n", client_fd);
        if (client_fd >= 0) {
            char buf[512];
            memset(buf, 0, 512);
            read(client_fd, buf, 512);
            printf("recv done %s!\n", buf);
            
            write(client_fd, buf, strlen(buf));
            printf("send done!\n");

            close(client_fd);
        }
    }
    return 0;
}

#define SERVER_IP   "192.168.0.104"
#define BUF_LEN 512
int socket_test3(int argc, char *argv[])
{
    printf("socket3 test start!\n");
        
    int err;
    /* udp连接 */
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        printf("create socket failed!\n");
        return -1;
    }
    printf("create socket %d\n", fd);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_addr.s_addr = htonl(IPADDR_ANY);
    serv_addr.sin_port = htons(8080);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_len = sizeof(struct sockaddr_in);
    bind(fd, &serv_addr, sizeof(struct sockaddr_in));
    
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serv_addr.sin_port = htons(8080);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_len = sizeof(struct sockaddr_in);

    struct sockaddr src;
    socklen_t len;
    int client_fd;
    while (1) {
        
        char buf[BUF_LEN] = "hello! Test!\n";
        len = sizeof(struct sockaddr_in);
        sendto(fd, buf, BUF_LEN, 0, (struct sockaddr *)&serv_addr, len);
        printf("client: %s\n", buf);
        memset(buf, 0, BUF_LEN);
        recvfrom(fd, buf, BUF_LEN, 0, &src, &len);
        printf("server: %s\n", buf);
        break;
    }
    close(fd);
    return 0;
}


#define MAX_SERV                 5         /* Maximum number of chargen services. Don't need too many */
#define CHARGEN_THREAD_NAME      "chargen"
#define CHARGEN_PRIORITY         254       /* Really low priority */
#define CHARGEN_THREAD_STACKSIZE 0
#define SEND_SIZE 512             /* If we only send this much, then when select
                                              says we can send, we know we won't block */
struct charcb 
{
    struct charcb *next;
    int socket;
    struct sockaddr_in cliaddr;
    socklen_t clilen;
    char nextchar;
};

static struct charcb *charcb_list = 0;
static int do_read(struct charcb *p_charcb);
static void close_chargen(struct charcb *p_charcb);

/**************************************************************
 * void chargen_thread(void *arg)
 *
 * chargen task. This server will wait for connections on well
 * known TCP port number: 19. For every connection, the server will
 * write as much data as possible to the tcp port.
 **************************************************************/
int socket_test4(int argc, char *argv[])
{
    int listenfd;
    struct sockaddr_in chargen_saddr;
    fd_set readset;
    fd_set writeset;
    int i, maxfdp1;
    struct charcb *p_charcb;

    /* First acquire our socket for listening for connections */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
        printf("chargen_thread(): Socket create failed.\n");

    memset(&chargen_saddr, 0, sizeof(chargen_saddr));
    chargen_saddr.sin_family = AF_INET;
    chargen_saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    chargen_saddr.sin_port = htons(19);     /* Chargen server port */

    if (bind(listenfd, (struct sockaddr *) &chargen_saddr, sizeof(chargen_saddr)) == -1) {
        printf("chargen_thread(): Socket bind failed.\n");
        return -1;
    }
        
    /* Put socket into listening mode */
    if (listen(listenfd, MAX_SERV) == -1) {
        printf("chargen_thread(): Listen failed.\n");
        return -1;
    }

    /* Wait forever for network input: This could be connections or data */
    for (;;)
    {
        maxfdp1 = listenfd+1;

        /* Determine what sockets need to be in readset */
        FD_ZERO(&readset);
        FD_ZERO(&writeset);
        FD_SET(listenfd, &readset);
        for (p_charcb = charcb_list; p_charcb; p_charcb = p_charcb->next)
        {
                if (maxfdp1 < p_charcb->socket + 1)
                    maxfdp1 = p_charcb->socket + 1;
                FD_SET(p_charcb->socket, &readset);
                FD_SET(p_charcb->socket, &writeset);
        }
        /* Wait for data or a new connection */
        i = select(maxfdp1, &readset, &writeset, 0, 0);
        
        if (i == 0)
            continue;
        /* At least one descriptor is ready */
        if (FD_ISSET(listenfd, &readset))
        {
            /* We have a new connection request!!! */
            /* Lets create a new control block */
            p_charcb = (struct charcb *)malloc(sizeof(struct charcb));
            if (p_charcb)
            {
                p_charcb->socket = accept(listenfd,
                                        (struct sockaddr *) &p_charcb->cliaddr,
                                        &p_charcb->clilen);
                if (p_charcb->socket < 0)
                    free(p_charcb);
                else
                {
                    /* Keep this tecb in our list */
                    p_charcb->next = charcb_list;
                    charcb_list = p_charcb;
                    p_charcb->nextchar = 0x21;
                }
            } else {
                /* No memory to accept connection. Just accept and then close */
                int sock;
                struct sockaddr cliaddr;
                socklen_t clilen;

                sock = accept(listenfd, &cliaddr, &clilen);
                if (sock >= 0)
                    close(sock);
            }
        }
        /* Go through list of connected clients and process data */
        for (p_charcb = charcb_list; p_charcb; p_charcb = p_charcb->next)
        {
            if (FD_ISSET(p_charcb->socket, &readset))
            {
                /* This socket is ready for reading. This could be because someone typed
                 * some characters or it could be because the socket is now closed. Try reading
                 * some data to see. */
                if (do_read(p_charcb) < 0)
                    break;
            }
            if (FD_ISSET(p_charcb->socket, &writeset))
            {
                char line[80];
                char setchar = p_charcb->nextchar;

                for( i = 0; i < 59; i++)
                {
                    line[i] = setchar;
                    if (++setchar == 0x7f)
                        setchar = 0x21;
                }
                line[i] = 0;
                strcat(line, "\n\r");
                if (write(p_charcb->socket, line, strlen(line)) < 0)
                {
                    close_chargen(p_charcb);
                    break;
                }
                if (++p_charcb->nextchar == 0x7f)
                    p_charcb->nextchar = 0x21;
            }
            
        }
    }
    
    return 0;
}

/**************************************************************
 * void close_chargen(struct charcb *p_charcb)
 *
 * Close the socket and remove this charcb from the list.
 **************************************************************/
static void close_chargen(struct charcb *p_charcb)
{
    struct charcb *p_search_charcb;

        /* Either an error or tcp connection closed on other
         * end. Close here */
        close(p_charcb->socket);
        /* Free charcb */
        if (charcb_list == p_charcb)
            charcb_list = p_charcb->next;
        else
            for (p_search_charcb = charcb_list; p_search_charcb; p_search_charcb = p_search_charcb->next)
            {
                if (p_search_charcb->next == p_charcb)
                {
                    p_search_charcb->next = p_charcb->next;
                    break;
                }
            }
        free(p_charcb);
}


/**************************************************************
 * void do_read(struct charcb *p_charcb)
 *
 * Socket definitely is ready for reading. Read a buffer from the socket and
 * discard the data.  If no data is read, then the socket is closed and the
 * charcb is removed from the list and freed.
 **************************************************************/
static int do_read(struct charcb *p_charcb)
{
    char buffer[80];
    int readcount;

    /* Read some data */
    readcount = read(p_charcb->socket, &buffer, 80);
    if (readcount <= 0)
    {
        close_chargen(p_charcb);
        return -1;
    }
	printf("recv data len = %d\n", readcount);
    return 0;
}
