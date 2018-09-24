#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv){
	int sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd<0){
		printf("Problem creating socket");
		return 1; 
	}

	printf("Port Number: ");
	char sPort[5000];
	fgets(sPort, 5000, stdin);
	int portNum = atoi (sPort);

	printf("IP Address: ");
	char addr[5000];
	fgets(addr, 5000, stdin);

	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(portNum);
	serveraddr.sin_addr.s_addr=inet_addr(addr);


	printf("File: ");
	char fName[5000];
	fgets(fName, 5000, stdin);
	char *pos;
	if ((pos=strchr(fName, '\n')) != NULL){
    	*pos = '\0';
    }
	sendto(sockfd,fName,strlen(fName) +1, 0,(struct sockaddr*)&serveraddr,sizeof(serveraddr));
	
	printf("New file name: ");
	char newFName[5000];
	fgets(newFName, 5000, stdin);
	if ((pos=strchr(newFName, '\n')) != NULL){
    	*pos = '\0';
    }
	FILE* newFile;
	newFile = fopen(newFName, "w+");

	int len = sizeof(serveraddr);
	char fContents[5000];
	int n = recvfrom(sockfd,fContents,5000,0,(struct sockaddr*)&serveraddr,&len);	
	printf("%s\n", fContents );
	fputs(fContents, newFile);

	close(sockfd);
}
