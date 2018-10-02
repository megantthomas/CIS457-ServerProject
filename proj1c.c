#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

/**
 * @Author Megan Thomas & Theodore Lang & Cody Krueger 
 * @Date 24 SEP 2018
 * CIS 457 Data Comm
 * PRJ 1-A UDP Echo Client with Sliding Window
 * 
 **/

//stuct used to hold packet info 
struct packet{
  char ident;
  u_short checkSum;
  char data[1024];
};

//create the check sum value
u_short toCksum(char *data, int length);

//check if an IP address is valid 
int isValidIpAddress(char *ipAddress);

int main(int argc, char **argv){
    //Create Socket
    int sockfd = socket(AF_INET,SOCK_DGRAM,0);
    if (sockfd<0){
      printf("There was an ERROR(1) creating the socket\n"); //**does a n\ cause a problem here?
      return 1; 
    }
    struct timeval timeout; 
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO, &timeout, sizeof(timeout));
    
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
    char *fName;
    fName =(char*)malloc(1000*sizeof(char));
    fgets(fName + 2, 1000, stdin);
    //char *pos;
    if ((pos=strchr(fName, '\n')) != NULL){
      *pos = '\0';
    }
    //number of packets
    char packetNumMess[3];

    int exist1 = -1;
    int tm;
    while(exist1 == -1){
    //time out needed to be added and check sum 

    u_short cksums = toCksum(fName + 2, (1000)-2);
    fName[0]= cksums & 0xFF;
	  fName[1] = cksums>>8;
    
    sendto(sockfd, fName, (1000)+1, 0,(struct sockaddr*)&serveraddr, len);
    tm = recvfrom(sockfd, packetNumMess, 3, 0,(struct sockaddr*)&serveraddr, &len);
    if(tm == -1){
      printf("Time out\n");
    
    } else{

    u_short pCK =(packetNumMess[1] << 8) | (0x00ff & packetNumMess[0]);
		packetNumMess[0]= '0';
	  packetNumMess[1] = '0';
    u_short fileck = toCksum(packetNumMess, 3);
      if(fileck == pCK){
      exist1 = 0;
      } 
    }
    }
 
  
    int totalPackets = (packetNumMess[2] - 48);
    printf("total: %d\n", totalPackets);

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

    //Recieving Data-----------------------------------------------
    
    //Window 
    char min_c = '0';
    char max_c = '4';
    int MIN = 0;
    int MAX = 4;
    struct packet packetLog[totalPackets];
    int datOS = 3; //data offset for meta data added
    int packetSize = 1024+datOS;
    int numBytes;
    char* fContents;
    char ident[2] = "0a";
    int next = 0; 
    fContents = (char*)malloc(packetSize*sizeof(char));
    int count = 0;
    
    //loop to receive packets 
    int exist11 = 1;
    while ( exist11 )
      {
        numBytes = recvfrom(sockfd, fContents, packetSize, 0,(struct sockaddr*)&serveraddr, &len);
	//if the packet we receive is within the window
	if ((min_c <= fContents[0]) && (fContents[0] <= max_c)) {

	  //convert packet aray value back to an u_short 
	  u_short val = 0;
	  val = (fContents[2] << 8) | (0x00ff & fContents[1]);
	  printf("check sum sent: %hu\n", val );
	  printf("check sum made: %hu\n", toCksum(fContents+3, numBytes-3));
	 
	  //if the packet is not corrupted
	  if ( val == toCksum(fContents+3, numBytes-3)){
	    int i = ((int)fContents[0])-48;
	    
	    //if the packet does not already exist in the struct (aka duplication)
	    if (fContents[0] != packetLog[i].ident) {
	      printf("not a dup\n");
	      //fill the struct
	      packetLog[i].checkSum = val;
        printf("ck\n");
        printf("%d\n%s\n%d\n", i, fContents+3, numBytes-3);

	      memcpy(packetLog[i].data, fContents+3, numBytes-3);
        printf("ck1\n");
	      packetLog[i].ident = fContents[0];
        printf("ck2\n"); 
	    }
	    
	     //if the packet was a duplicate send an ack saying we received it so the server does not send again but we do nothing with the data 
	    else{
	      *ident = ((fContents[0]-48)%10)+48;
	      printf("DUPLICATE Ack sent: %c\n", ident[0]);
	      //reply for packet recieved
	      sendto(sockfd, ident, 1+1, 0,(struct sockaddr*)&serveraddr, len);
	    }
	    
	    //File Writing
	    //writes the next packet in the struct
	    if (next == (packetLog[next].ident -48)) {
	      printf("correct order\n");
	      //write to file 
      
	      newFile = fopen(newFName, "a");
	      fwrite(packetLog[next].data, 1, numBytes-datOS, newFile);
	      fclose(newFile);
	      
	      //send an ack that we wrote and received the packet  
	      *ident = ((fContents[0]-48)%10)+48;

	      printf("Ack sent: %c\n", ident[0]);
	      sendto(sockfd, ident, 1+1, 0,(struct sockaddr*)&serveraddr, len);
	      
	      //adjust window bounds
	      if(totalPackets + 47 != max_c){
		MIN = (MIN+1)%10; 
		MAX = (MAX+1)%10; 
		
		min_c = MIN + 48; 
		max_c = MAX + 48;
	      }
	      
	      next++;
	    }
	    
	    //if we have received our last packet
      printf("PacketNum %c\nfContents %c\n", (totalPackets+47), fContents[0]);
	    if((totalPackets + 47) <= fContents[0] ){
        printf("exit");
	      exist11 = 0;
	      
	      //reset buffer
	      free(fContents);
	      char* fContents;
	      fContents = (char*)malloc(packetSize*sizeof(char));
	    }
	  }
	}
      }
    
    //prevent memory leaks
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
