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
 * PRJ 1-A UDP Echo Server with Sliding Window
 * 
 **/

int main(int argc, char **argv){
	int WINDOW = 5; 
	int HEADER = 1; 
	int PACKET = 1024;

    //Socket 
    int sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd<0){
		printf("There was an ERROR(1) creating the socket\n");
		return 1; 
	}
	
    //Time out set up
	struct timeval timeout; 
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
    setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO, &timeout, sizeof(timeout));

    //Get PortNum
	printf("Port number: ");
	int portNum; 
	scanf("%d", &portNum);
	if(portNum < 1023 || portNum > 49152){
		printf("Try again with valid port number\n");
		return 0; 
	}

    //Socket Struct 
    struct sockaddr_in serveraddr, clientaddr;
	serveraddr.sin_family = AF_INET; 
	serveraddr.sin_port = htons(portNum);
	serveraddr.sin_addr.s_addr = INADDR_ANY;
		
    //Binding
	int bErr = bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (bErr<0){
		printf("There was an ERROR(2) binding failed\n");
		return 2; 
	}

    //Data Transmission
	while(1){
		unsigned int len = sizeof(clientaddr);
		char fileName[1000];
		int n = recvfrom(sockfd, fileName, 1000, 0, (struct sockaddr*)&clientaddr, &len);
		if(n == -1){
			printf("Time out \n\n");
		}
		else{
			FILE *file;
			char * buffer = 0;
			long length;
            // from https://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c-cross-platform
    		if (access( fileName, F_OK ) != -1){
            	char* packet;
				packet = (char*)malloc(PACKET*sizeof(char));
				char pacNum = '0';
				ssize_t read;

				// file exists
				printf("File found.\n");
				file = fopen(fileName, "rb");

				//Calculate and send the number of packets
            	fseek(file, 0L, SEEK_END);
				int sz = ftell(file);
				fseek(file, 0L, SEEK_SET);
				sz = (sz/PACKET);
				char tots[10];
				snprintf(tots, sizeof(tots), "%d", sz);
				int pNum = sendto(sockfd, tots, strlen(tots)+1, 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));
				
				//Send Data
				int currentP = 0;
				int min = 0;
				int max = 4;
				int numAcks = 0;
				char ack[10];
				while ((read = fread(packet+1, 1, PACKET, file)) > 0) {
					//sends only packets within the window
					if(currentP>=min && currentP<=max){
						*packet = pacNum;
               			int ssent = sendto(sockfd, packet, read+1, 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));
               			printf("Sending packet %d\nSize: %d\nBytes read: %zd\n", currentP, ssent, read);
						//printf("Contents: %s\n", packet); //testing **
						printf("\n");
						
						//if we recieve an ack we move the window 
						//if no acks are recieved then the program will not send more than 5 packets
						//and will be stuck waiting for other packets for now. Will fix this in 
						//part two by resending lost packets 
						if(recvfrom(sockfd, ack, HEADER, 0, (struct sockaddr*)&clientaddr, &len)){
							//testing printf("ack num %d recieved\n", numAcks);
							numAcks++;
					 		if(!(max >= sz)){
					 			max++;
					 			min++;
					 		}	
						}

						//update current packet info
						pacNum+=1;
						free(packet);
						char* packet;
						packet = (char*)malloc(PACKET*sizeof(char));
						currentP++;	
					}				
				}
					free(packet);
			}
    		else{
                //File Does Not Exist
                //if server recieves unexpected input it automatically assumes its a new file and throws this error
    			char err[] = "File does not exist \n";
    			printf("%s", err);
				sendto(sockfd, err, strlen(err)+1, 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));
			}
		}
	}
}
