#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv){
	int WINDOW = 5; 
	int HEADER = 1; 
	int PACKET = 1024;
	int sockfd = socket(AF_INET,SOCK_DGRAM,0);
	struct timeval timeout; 
	struct sockaddr_in serveraddr, clientaddr;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	if (sockfd<0){
		printf("Problem creating socket");
		return 1; 
	}

	printf("Port number: ");
	int portNum; 
	scanf("%d", &portNum);

	serveraddr.sin_family+ AF_INET;
	serveraddr.sin_port = htons(portNum);
	serveraddr.sin_addr.s_addr = INADDR_ANY;
		
	setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO, &timeout, sizeof(timeout)); 

	int berr = bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (berr<0){
		printf("Problem with binding");
		return 2; 
	}

	while(1){
		int len = sizeof(clientaddr);
		char filename[5000];
		int n = recvfrom(sockfd,filename,5000,0,(struct sockaddr*)&clientaddr,&len);
		if(n == -1){
			printf("time out \n\n");
		}
		else{
			FILE *file;
			char * buffer = 0;
			long length;
    		if (file = fopen(filename, "r")){
    			fseek (file, 0, SEEK_END);
				length = ftell (file);
  				fseek (file, 0, SEEK_SET);
  				buffer = malloc (length);
  				fread (buffer, 1, length, file);
  				sendto(sockfd,buffer,strlen(buffer)+1, 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));
  				fclose (file);
			}
    		else{
    			char err[] = "File does not exist \n";
    			printf("%s", err);
				sendto(sockfd,err,strlen(err)+1, 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));
			}
		}
	}
}

