#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/**
 * @Author Megan Thomas & Cody Krueger 
 * @Date 24 SEP 2018
 * CIS 457 Data Comm
 * PRJ 1-A UDP Echo Client with Sliding Window
 * 
 **/

int main(int argc, char **argv){
    //Create Socket
	int sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd<0){
		printf("There was an ERROR(1) creating the socket\n"); //**does a n\ cause a problem here?
		return 1; 
	}

    //Port Num
	printf("Port Number: ");
	// char sPort[1000];
	// fgets(sPort, 1000, stdin);
	// int portNum = atoi (sPort);
    //for testing --------------- **
    int portNum = 9874;
    printf("9874 \n");
    if(portNum < 1023 || portNum > 49152){
		printf("Try again with valid port number\n");
		return 0; 
	}

    //IP Address
	printf("IP Address: ");
	//char addr[1000];
	// fgets(addr, 1000, stdin);
    //for testing --------------- **
    char addr[] = "127.0.0.1";
     printf("%s \n", addr);
    if(!(isValidIpAddress(addr))){
    	printf("Try again with a valid IP address\n");
    	return 0;
    }


    //Socket Struct 
	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(portNum);
	serveraddr.sin_addr.s_addr=inet_addr(addr);

    //File Request
    unsigned int len = sizeof(serveraddr);
	printf("File: ");
	char fName[1000];
	fgets(fName, 1000, stdin);
	char *pos;
	if ((pos=strchr(fName, '\n')) != NULL){
    	*pos = '\0';
    }
	sendto(sockfd,fName,strlen(fName)+1, 0,(struct sockaddr*)&serveraddr, len);
	
    //New File Creation
	printf("New file name: ");
	char newFName[1000];
	fgets(newFName, 1000, stdin);
	if ((pos=strchr(newFName, '\n')) != NULL){
    	*pos = '\0';
    }
	FILE* newFile;

    //Recieving Data
	int numBytes;
	char fContents[10];
	int count = 0;
	while (0 < (numBytes = recvfrom(sockfd, fContents, 10, 0,(struct sockaddr*)&serveraddr, &len))) {
		printf("%s\n", fContents );
		printf("%d\n", count );
		count++;
		newFile = fopen(newFName, "a");
		//fputs(fContents, newFile);
		fwrite(fContents, 1, numBytes+1, newFile);
		fclose(newFile);
	}

	close(sockfd);
}

//method to check if a string is an valid ip address 
//source: https://stackoverflow.com/questions/791982/determine-if-a-string-is-a-valid-ipv4-address-in-c
int isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}