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
struct packet{
  char ident;
  u_short checkSum;
  char data[1025];
};

u_short toCksum(char *data, int length);

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
                u_short ckSumSize = 2; // FIXMEv3**change!
                char ckSum1 = '0';
                char ckSum2 = '0';
                size_t read;

                // file exists
                //open file
                printf("File found.\n");
                file = fopen(fileName, "rb");

		//number of packets 
		fseek(file,0,SEEK_END);
		int fsize=ftell(file);
		rewind(file);
		int totalPacks = (fsize/packetSize);
		if((fsize%packetSize) > 0){
		  totalPacks++;
		}
		char packetNumMess[2];
		packetNumMess[0] = (totalPacks +48);
		packetNumMess[1] = 'p';
		sendto(sockfd, packetNumMess, 2, 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));
		
		
		struct packet packetLog[totalPacks];
		int packetNum;
                //read file data
		for(packetNum = 0; packetNum < 5; packetNum++){
		 read = fread(packet+1, 1, packetSize, file);
		  if(read > 0){
		    *packet = pacNum;
		    packetLog[packetNum].ident = pacNum;
		    memcpy (packetLog[packetNum].data, packet, strlen(packet) );
		    packetLog[packetNum].checkSum = toCksum(packet, strlen(packet));
		    int ssent = sendto(sockfd, packet, read+1+ckSumSize, 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));

		     printf("Sending, size is %d\n     Bytes read: %zd\n", ssent, read);
                     printf("packet num: %c\n", pacNum);
		     pacNum = pacNum+1;

		  }
		  else if(read == 0){
		    //if end of file 
		    break;
		  }
		  else{
		    printf("Error\n");
		  }		  
		}
		int exit = 1;
                while (exit) {

		  if ((min_c <= pacNum-1) && (pacNum-1<= max_c)) {
                        char ack[1];
			int a;
			
			if((a = recvfrom(sockfd, ack, ackSize, 0, (struct sockaddr*)&clientaddr, &len)) > 0){
			  printf("Ack size is %d\n", a);
			  printf("Ack: %c\n", ack[0]);
			  read = fread(packet+1, 1, packetSize, file);
			  if(ack[0] == (totalPacks+47)){
			    exit = 0;
			  }
 
			  if(totalPacks+47 != max_c){
			    MIN++;
			    MAX++;
			  
			    min_c = (MIN%10) + 48;
			    max_c = (MAX%10) + 48;
			  }
			  
			  if(totalPacks+48 != pacNum){
			     u_short ck =  toCksum(packet, strlen(packet));
			     u_short ck2 = toCksum(packet, strlen(packet));
			    *packet = pacNum;
			    printf("check sum: %hu\n", ck);
			    packet[1]= ck;
			    //printf("%c\n", packet[1]);
			    ck = ck>>8;
			    packet[2] = ck;
			    // printf("%c\n", packet[2]);
			     int ssent = sendto(sockfd, packet, read+1+ckSumSize, 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));
			     // printf("checking if check sum chars are added: %s", packet);

			    //fill the struct 
			    packetLog[packetNum].ident = pacNum;
			    memcpy (packetLog[packetNum].data, packet, strlen(packet) );
			    packetLog[packetNum].checkSum = ck2;

			    printf("%d\n",packetNum);
			    printf("Sending, size is %d\n     Bytes read: %zd\n", ssent, read);
			    printf("packet num: %c\n", pacNum); //testing **
                        
			    pacNum++;
			    packetNum++;
			    free(packet);
			    char* packet;
			    packet = (char*)malloc(packetSize*sizeof(char));
			  }
                        }
                    }
                }
		
		for(int i = 0; i < totalPacks; i++){
		  printf("packet: %c\n", packetLog[i].ident);
		  // printf("packet contents: %s\n", packetLog[i].data);
		  printf("check sum: %hu\n", packetLog[i].checkSum );
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

//talked to classmates about this method 
u_short toCksum(char *data, int length) {
    u_short checkSum= 0;
    unsigned int cl = length;
      while (cl != 0){
    checkSum -= *data++;
    cl--;
                }
      return checkSum;
}

//error checking occurs here --------------------------------------------

        //check cs bytes
        //case 4: if packet corrupt
        //----> discard packet

        //case 1: if duplicate, discard send ack

        //case 2: no packet made it... server solves see time out

        //case 3: packet out of order (or in order)
        //store in array 
        

        //File Writing
        //if min == array[min].ident (ie the struct ident, not recv ident) {
            //remember, only do this when the packet is in order (ie min == current packet)
            //min should reach all elements in a file
            //send ack
            //writeto file
            //remove/clr that element from array
            //move window
            //move total packet num (for getting out of loop)
	//}
