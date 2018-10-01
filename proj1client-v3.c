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
 * PRJ 1-A UDP Echo Client with Sliding Window
 * 
 **/

struct packet{
  char ident;
  unsigned int numBytes; //added
  u_short checkSum;
  char data[1024];
};

int isValidIpAddress(char *ipAddress);

int main(int argc, char **argv){
    //Create Socket
	int sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd<0){
		printf("There was an ERROR(1) creating the socket\n"); //**does a n\ cause a problem here?
		return 1; 
	}

    //Port Num
	printf("Port Number: ");
	char sPort[100];
	// fgets(sPort, 100, stdin);
	// int portNum = atoi (sPort);

    // for testing --------------- **
    int portNum = 9874;
    printf("9874 \n");

    if(portNum < 1023 || portNum > 49152){
		printf("Try again with valid port number\n");
		return 0; 
	}

    //IP Address

	printf("IP Address: ");
	// char addr[100];
	// fgets(addr, 100, stdin);
	char *pos;
	// if ((pos=strchr(addr, '\n')) != NULL){
    // 	*pos = '\0';
 	// }

    // for testing --------------- **
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
	//char *pos;
	if ((pos=strchr(fName, '\n')) != NULL){
    	*pos = '\0';
    }
	sendto(sockfd, fName, strlen(fName)+1, 0,(struct sockaddr*)&serveraddr, len);

    //New File Creation
	printf("New file name: ");
	char newFName[1000];
	fgets(newFName, 1000, stdin);
	if ((pos=strchr(newFName, '\n')) != NULL){
    	*pos = '\0';
    }
	FILE* newFile;
    newFile = fopen(newFName, "w+");
    printf("\n");
    // fclose(newFile);

    //Recieving Data-----------------------------------------------

    //number of packets
    char packetNumMess[2];
    recvfrom(sockfd, packetNumMess, 2, 0,(struct sockaddr*)&serveraddr, &len);
    //printf("%c", packetNumMess[0]);
    //int totalPackets = (packetNumMess[0] + 48);
    
    //Window 
    char min_c = '0';
    char max_c = '4';
    int MIN = 0;
    int MAX = 4;

    int datOS = 1; //data offset for meta data added
    int packetSize = 1024+datOS;
    int numBytes;
	//char fContents[10];
    char* fContents;
    char ack[2] = "0a";
    fContents = (char*)malloc(packetSize*sizeof(char));
	int count = 0; //total packet count
	while (1)
	  {
	    numBytes = recvfrom(sockfd, fContents, packetSize+1, 0,(struct sockaddr*)&serveraddr, &len);
	  printf("packet recieved: %c\n", fContents[0] );
	  if ((min_c <= fContents[0]) && (fContents[0] <= max_c)) {

        //FIXME** will cause issues at 10 likely
        //FIXME** (min_c+0)%10
        //(min_c == fContents[0]) || (min_c+1 == fContents[0]) || (min_c+2 == fContents[0]) || (min_c+3 == fContents[0]) || (min_c+4 == fContents[0]))

        //error checking occurs here --------------------------------------------

        //check cs bytes
        //case 4: if packet corrupt
        //----> discard packet
        //memset (fContents,'X', 1); //sets ident in recieved data to X 

        /*********************************************
         * case 1 and 3 solution 
        //case 1: if duplicate, discard send ack
        //case 3: packet out of order (or in order)
        //store in array 
        **********************************************/
        int i = (fContents[0])-48;
        if (fContents[0] == packetLog[i].ident) {
            //packet is duplicate -> discard
            //ie dont copy it to array
        }
        else {
            //Packet is NOT a duplicate
            //save to array
            packetLog[i].ident = fContents[0];
            packetLog[i].size = numBytes;
            //TODO cksum conversion (or just check that in corruption module)
            //packetLog[i].checkSum = fContents[1]; TODO convert fContents[1]&fContents[2] into u_short
            memcpy(packetLog[i].data, fContents+3, sizeof(fContents)-3);
        }

        //send ack for case 0, 1, 3
        *ack = ((fContents[0]-48)%10)+48;
        printf("Ack sent: %c\n", ack[0]);
        //reply for packet recieved
        sendto(sockfd, ack, 1+1, 0,(struct sockaddr*)&serveraddr, len);

        //case 0: packet was new and not corrupt
        //case 2: no packet made it... server solves see time out

        //File Writing
        if (min_c == array[MIN].ident) { //(ie the struct ident, not recv ident)
            //remember, only do this when the packet is in order (ie min == an existing packet in array)
            //min should reach all elements in a file

            //writeto file
            //printf("packet contents: %s\n", fContents );
            newFile = fopen(newFName, "a");
            fwrite(array[MIN].data, 1, numBytes-datOS, newFile);
            fclose(newFile);

            //remove/clr that element from array
            array[MIN].ident = 'X';

            //adjust window bounds
	        if(packetNumMess[0] != max_c){
                MIN = (MIN+1)%10; //a
                MAX = (MAX+1)%10; //a

                min_c = MIN + 48; //a
                max_c = MAX + 48; //a
	        }
            //move total packet num (for getting out of loop)
            count++;
            int maxPack = packetNumMess[0]-48;
            if (max packets == count) {
               break;
            }
        }
	    
	  if((packetNumMess[0]-1) <= fContents[0] ){
	    break;
	  }

	  //reset buffer
            free(fContents);
            char* fContents;
            fContents = (char*)malloc(packetSize*sizeof(char));

	  }

	  
	} /*  */
    free(fContents);
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


