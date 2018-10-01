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
 * PRJ 1-A UDP Echo Server with Sliding Window
 * 
 **/

//used to hold packet information
struct packet{
  char ident;
  u_short checkSum;
  char data[1025];
};

//used to create check sum value 
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
      short ckFile = toCkSum(fileName, 1000);
      
      //if nothing is received
      if(n == -1){
     	   printf("Time out \n\n");
      }

      //if something is received see if it is a file 
      else{
      	FILE *file;
      	char * buffer = 0;
   	long length;

	// from https://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c-cross-platform
    	if (access( fileName, F_OK ) != -1){
             
	  //prep window
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

          //open file
          printf("File found.\n");
          file = fopen(fileName, "rb");

	  //sends number of packets as an ack for file name
	  fseek(file,0,SEEK_END);
	  int fsize=ftell(file);
	  rewind(file);
	  int totalPacks = (fsize/packetSize);
	  if((fsize%packetSize) > 0){
	    totalPacks++;
	  }
	  char packetNumMess[3];
	  packetNumMess[0] = (totalPacks +48);
	  packetNumMess[1]= ckFile & 0xFF;
	  packetNumMess[2] = ckFile>>8;

	  sendto(sockfd, packetNumMess, 2, 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));
		
	  //struct array of packets
	  struct packet packetLog[totalPacks];
	  int packetNum;

	  //read file data and sends the first packets 
	  for(packetNum = 0; packetNum < 5; packetNum++){
	    read = fread(packet+3, 1, packetSize, file);

	    //if there is data read
	    if(read > 0){
	      *packet = pacNum;
	      
	      //fill the struct
	      packetLog[packetNum].ident = pacNum;
	      memcpy (packetLog[packetNum].data, packet+3, read);
	      u_short ck =  toCksum(packetLog[packetNum].data, read);
	      packetLog[packetNum].checkSum = ck;

	      //add the check sum value to the packet 
	      packet[1]= ck & 0xFF;
	      packet[2] = ck>>8;

	       //send packet
	      int ssent = sendto(sockfd, packet, read+1+ckSumSize, 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));

	      //packet stats printed to consule
	      printf("Sending, size is %d\n     Bytes read: %zd\n", ssent, read);
	      printf("packet num: %c\n", pacNum);
	      pacNum = pacNum+1;

	    }

	    //if end of file we break
	    else if(read == 0){
	      break;
	    }

	    //anything else means there is a problem with the file 
	    else{
	      printf("Error\n");
	    }		  
	  }

	  //loop to send and receive the rest of the packets of the file 
	  int exit = 1;
	  while (exit) {

	    //if the packet is between the window 
	    if ((min_c <= pacNum-1) && (pacNum-1<= max_c)) {
	      char ack[1];
	      int a = recvfrom(sockfd, ack, ackSize, 0, (struct sockaddr*)&clientaddr, &len);
	      if(a == -1){
		u_short ck =  toCksum(packet, a);
		*packet = pacNum;
		
		printf("Time out, no ack\n");
 
		//**********************FIX ME*********************
		memcpy(packet,packetLog[packetNum].data, strlen(packetLog[packetNum].data));
		packet[0] = pacNum;
	        packet[1]= ck;
		ck = ck>>8;
		packet[2] = ck;
		int ssent = sendto(sockfd, packet, read+1+ckSumSize, 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));


	      //if we recieved an ack 
	      }else if(a > 0){
		printf("Ack size is %d\n", a);
		printf("Ack: %c\n", ack[0]);
		read = fread(packet+3, 1, packetSize, file);
		
		//when we recieved the last ack exit 
		if(ack[0] == (totalPacks+47)){
		  exit = 0;
		}
		
		//if the max for the window is equal to the total number of packets we stop incrementing 
		if(totalPacks+47 != max_c){
		  MIN++;
		  MAX++;
		  
		  min_c = (MIN%10) + 48;
		  max_c = (MAX%10) + 48;
		}

		//we keep sending packets as long as there are packets to send 
		if(totalPacks+48 != pacNum){
		  *packet = pacNum;
		  
		  //fill the struct 
		  packetLog[packetNum].ident = pacNum;
		  memcpy (packetLog[packetNum].data, packet+3, read);
		  u_short ck =  toCksum(packetLog[packetNum].data, read);
		  packetLog[packetNum].checkSum = ck;

		  //add the check sum to the array
		  printf("check sum: %hu\n", ck);
		  packet[1]= ck & 0xFF;
		  packet[2] = ck>>8;

		  u_short val = 0;
		  val = (packet[2] << 8) | packet[1];
		  printf("check sum sent: %hu\n\n", val);
		  int ssent = sendto(sockfd, packet, read+1+ckSumSize, 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));

		  //print data to console
		  printf("Sending, size is %d\n     Bytes read: %zd\n", ssent, read);
		  printf("packet num: %c\n", pacNum); 
                  
		  pacNum++;
		  packetNum++;
		  free(packet);
		  char* packet;
		  packet = (char*)malloc(packetSize*sizeof(char));
		}
	      }
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

