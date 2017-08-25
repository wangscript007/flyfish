#include <stdio.h>  
#include <sys/socket.h>  
#include <unistd.h>  
#include <sys/types.h>  
#include <netinet/in.h>  
#include <stdlib.h>  
#include <string.h>  
#include <errno.h> 
#include <time.h> 
#include <arpa/inet.h>  

#include <string>

struct header
{
	int length;

	int version;
	int command;
};

int encode_header(header *theader, u_char *writeptr, int *wlen)
{
	uint32_t version = 0;
	uint32_t length = 0;
	uint32_t command = 0;


	uint32_t len = sizeof(uint32_t);
	uint32_t offset = 0;

	length = htonl(theader->length);
	memcpy(writeptr + offset, &length, len);

	offset += len;
	version = htonl(theader->version);
	memcpy(writeptr + offset, &version, len);

	offset += len;
	command = htonl(theader->command);
	memcpy(writeptr + offset, &command, len);

	offset += len;
	*wlen = (int)offset;
	return 0;

}
#define SERVER_PORT 19388 
  
using namespace std;

int tcp_send(int fd, u_char *buf, int len);

void usage(char *name)  
{  
    printf("usage: %s IP\n", name);  
}  
int main(int argc, char **argv)  
{

    int client_fd;  
    struct sockaddr_in server_addr, client_addr;  
    socklen_t socklen = sizeof(server_addr);  
  
    if(argc < 2)  
    {  
        usage(argv[0]);  
        exit(1);  
    }  
    if((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)  
    {  
        printf("create socket error, exit!\n");  
        exit(1);  
    }  
    srand(time(NULL));  
    bzero(&client_addr, sizeof(client_addr));  
    client_addr.sin_family = AF_INET;  
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);  
  
    bzero(&server_addr, sizeof(server_addr));  
    server_addr.sin_family = AF_INET;  
    inet_aton(argv[1], &server_addr.sin_addr);  
    server_addr.sin_port = htons(SERVER_PORT);  
    printf ("client_fd=[%d]\n",client_fd); 
    if(connect(client_fd, (struct sockaddr*)&server_addr, socklen) < 0)  
    {  
        printf("can not connect to %s, exit!\n", argv[1]);  
        printf("%s\n", strerror(errno));  
        exit(1);  
    }
    else
    {
        printf ("connect sucessful\n");

		header tHeader;

		tHeader.command=1;
		tHeader.version=1;

		string bodystr="HELLO";

		int body_length = bodystr.length();

		tHeader.length = 12 + body_length;

		u_char writeptr[1000]={0};
		int  header_length = 0;
		encode_header(&tHeader, writeptr, &header_length);
		memcpy(writeptr + header_length, bodystr.c_str(), body_length);

		tcp_send(client_fd,writeptr,header_length+body_length);

		sleep(10);
		close(client_fd);
                
               
    }   

}


int tcp_send(int fd, u_char *buf, int len)
{
	int ret = 0;
	if (len <= 0)
	{
		return 0;
	}
		

	while (1)
	{

		int slen = send(fd, buf + ret, len - ret, 0);
		if (slen < 0)
		{
			if (errno == EINTR)
				continue;
			else if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				printf("tcp_send:EAGAIN, fd = %d len = %d\n", fd, len);
				break;
			}
			else
			{
				int err = errno;
				printf("send data error: ret = %d, err = %s, fd = %d\n",
					slen, strerror(err), fd);

				break;
			}
		}

		ret += slen;
		if (ret == len)
		{
			break;
		}

	}


     printf("send, fd = %d len = %d\n", fd, ret);


     return ret;
}