#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

/**
 * @Author Megan Thomas & Cody Krueger 
 * @Date 24 SEP 2018
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
	// int portNum; 
	// scanf("%d", &portNum);
    // for testing --------------- **
    int portNum = 9874;
    printf("9874 \n");

	if(portNum < 1023 || portNum > 49152){
		printf("Try again with valid port number\n");
		return 0; 
	}

    //Socket Struct 
    struct sockaddr_in serveraddr, clientaddr;
	serveraddr.sin_family = AF_INET; //serveraddr.sin_family+ AF_INET; **Was there a reason this was "+" instead of "="?
	serveraddr.sin_port = htons(portNum);
	serveraddr.sin_addr.s_addr = INADDR_ANY;
		
    //Binding
	int bErr = bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (bErr<0){
		printf("There was an ERROR(2) binding failed\n");
		return 2; 
	}

    /**
    //Sliding Window-------------------------------------------------

    //Window 
    char min_c = '0';
    char max_c = '4';

    int MIN = 0;
    int MAX = 4;

    char[10] ackarr;
    int i = 0;

    while  {min_c < packetNum < max_c)
        //send packets TODO
        //recieve acks TODO
        //concatinate into array?
        //progress through array looking at acks
        //do we need a loof around if to do that? or will constand send and recv take care of that?
        //or would they halt the progress of this?
        if (ackArr[i] == min_c) {
            //adjust window bounds
            MIN++;
            MAX++;
            
            min_c = (MIN%10) + 48;
            max_c = (MAX%10) + 48;

            i++;
        }
        //for v2
        //wait x seconds
        //if no ack, resend min 
    }
    //------------------------------------------------------------
**/

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
                //File Exists
                //----> send back # of packets +cs
                // file = fopen(fileName, "r");
    			// fseek (file, 0, SEEK_END);
				// length = ftell (file);
  				// fseek (file, 0, SEEK_SET);
  				// buffer = malloc (length*sizeof(char));
  				// fread (buffer, 1, length, file);
  				// sendto(sockfd, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));                
                // fclose (file);
                //Not Exists
                //-----> send file does not exist
                // ie send back 0 and cs for 0 (ie send 3 bytes: 000)


                //prep window
                //Window 
                char min_c = '0';
                char max_c = '4';

                int MIN = 0;
                int MAX = 4;

                //prep packet
                int packetSize = 1024;
                int ackSize = 1+1; //**fixme can add a 1 here also
                char* packet;
                packet = (char*)malloc(packetSize*sizeof(char));
                char pacNum = '0';
                u_short ckSumSize = 0; // FIXMEv3**change!
                char ckSum1 = '0';
                char ckSum2 = '0';
                ssize_t read;

                // file exists
                    //open file
                printf("File found.\n");
                file = fopen(fileName, "rb");

                    //read file data

                while ((read = fread(packet+1, 1, packetSize, file)) > 0) {
                    if (min_c < pacNum < max_c) {
                        //send packets TODO
                        //assemble packet
                        *packet = pacNum;
                        //packet[1] = ckSum1;
                        //packet[2] = ckSum2;

                        //read = fread(packet+1, 1, packetSize, file);
                        //memcpy(&packet[0], &pacNum, 4);
                        int ssent = sendto(sockfd, packet, read+1+ckSumSize, 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));
                        printf("Sending, size is %d\n     Bytes read: %zd\n", ssent, read);
                        printf("Contents: %s\n", packet); //testing **
                        
                        pacNum++;
                        free(packet);
                        char* packet;
                        packet = (char*)malloc(packetSize*sizeof(char));
                        //recieve acks TODO
                        char ack[1];
		                int a = recvfrom(sockfd, ack, ackSize, 0, (struct sockaddr*)&clientaddr, &len);
                        printf("Ack size is %d\n", a);
                        printf("Ack: %s\n", ack);
                        //concatinate into array?
                        //progress through array looking at acks
                        //do we need a loop around iff to do that? or will constand send and recv take care of that?
                        //or would they halt the progress of this?
                        //if (ackArr[i] == min_c) {
                            //adjust window bounds
                            pacNum = ((pacNum-48)%10)+48;

                            MIN++;
                            MAX++;
                            
                            min_c = (MIN%10) + 48;
                            max_c = (MAX%10) + 48;

                            //i++;
                        //}
                    }
                }
            free(packet);
			}
    		else{
                //File Does Not Exist
    			char err[] = "File does not exist \n";
    			printf("%s", err);
				sendto(sockfd, err, strlen(err)+1, 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));
			}
		}
	}
}

