#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

/**
 * @Author Megan Thomas & Cody Krueger 
 * @Date 25 SEP 2018
 * CIS 457 Data Comm
 * PRJ 1-A UDP Echo Client with Sliding Window
 * 
 **/

//Function Declarations
int isValidIpAddress(char *ipAddress);

int main(int argc, char **argv){
    //Create Socket
	int sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd<0){
		printf("There was an ERROR(1) creating the socket");
		return 1; 
	}

    //Port Num
	char sPort[100];
	printf("Port Number: ");
	fgets(sPort, 100, stdin);
	int portNum = atoi (sPort);
    if(portNum < 1023 || portNum > 49152){
		printf("Try again with valid port number\n");
		return 0; 
	}

    //IP Address
    char addr[100];
	printf("IP Address: ");
	fgets(addr, 100, stdin);
    char *pos;
	if ((pos=strchr(addr, '\n')) != NULL){
    	*pos = '\0';
 	}
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
	char fName[100];
	fgets(fName, 100, stdin);
	if ((pos=strchr(fName, '\n')) != NULL){
    	*pos = '\0';
    }
	sendto(sockfd,fName,strlen(fName)+1, 0,(struct sockaddr*)&serveraddr, len);
	
    //New File Creation
	printf("New file name: ");
	char newFName[100];
	fgets(newFName, 100, stdin);
	if ((pos=strchr(newFName, '\n')) != NULL){
    	*pos = '\0';
    }
	FILE* newFile;
	
	//Number of packets
	char* total;
    recvfrom(sockfd, total, 1, 0,(struct sockaddr*)&serveraddr, &len);
    int totalPackets = atoi(total);


    //Recieving Data
	int numBytes;
    char* fContents;
    fContents = (char*)malloc(1025*sizeof(char));
	int count = 0;
	while (1) {
		numBytes = recvfrom(sockfd, fContents, 1024+1, 0,(struct sockaddr*)&serveraddr, &len);
		char packetNum = fContents[0];
		int pNum = packetNum - 48;
		//printf("%s\n", fContents);
		if(totalPackets+1 == pNum+1){
			close(sockfd);
			return 1;
		}

		printf("Packet number %c of %d recieved\n", packetNum, totalPackets);
		("%s\n", fContents);
		if(packetNum == '0'){
			newFile = fopen(newFName, "w+");
			fwrite(fContents+1, 1, numBytes-1, newFile);
			fclose(newFile);
			sendto(sockfd, fContents, 1, 0, (struct sockaddr*)&serveraddr, len);
		}
		else{
			newFile = fopen(newFName, "a");
			fwrite(fContents+1, 1, numBytes-1, newFile);
			fclose(newFile);
			sendto(sockfd, fContents, 1, 0, (struct sockaddr*)&serveraddr, len);
		}
	}
}

//method to check if a string is an valid ip address 
//source: https://stackoverflow.com/questions/791982/determine-if-a-string-is-a-valid-ipv4-address-in-c
int isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}

