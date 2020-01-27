/***********************************************************************************
 *
 * Author: Billy Henry
 * Creation Date: November 4, 2019
 * Due Date: November 30, 2019
 * Course: CSC-328-010, Fall 2019
 * Filename: server.c
 * Compile: gcc server.c -o server NOTE: can also use makefile to compile
 * EXECUTE COMMAND: ./server <OPTIONAL PORT> > output.log
 * Purpose: Concurrent server program for Download Server Application
 * Language: C (GCC Version 4.4.7)
 *
 **********************************************************************************/ 
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>

#define SIZE sizeof(struct sockaddr_in)
#define PORT 34541
#define BACKLOG 5
#define BUFSIZE 128

// Initialize chat function
void serverFunc(int sockfd, char * ipaddr);

// Declare download function
int download(int sock, char * f);

// Declare time calculating function.
char * calcTime();

// Begin Main
int main(int argc, char* argv[])
{
  // Init listening socket, child sockets and portNum	
  int listenfd, connfd, portNum;
  
  // Client and server structs
  struct sockaddr_in cliaddr, servaddr;
  
  // Init length of client struct
  socklen_t cliAddrLen;
  
  // IP char array
  char ip[INET_ADDRSTRLEN];
  
  // Child PID
  pid_t pid;
  
  // User entered Port Number to listen on
  if (argc == 2)
    portNum = atoi(argv[1]);
  
  // User entered no port number, use default
  else
    portNum = PORT;
  
  // Fill server struct
  memset(&servaddr,0,sizeof(servaddr));
  servaddr.sin_family      = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port        = htons(portNum);

  
  // set up socket to listen for client connections
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("socket call failed");
    exit(-1);
  } 
  
  
  // bind and address to the end point
  if (bind(listenfd, (struct sockaddr *)&servaddr, SIZE) == -1)
  {
    perror("bind call failed");
    exit(-1);
  }
  
  
  // start listening for incoming connections
  if (listen(listenfd, BACKLOG) == -1)
  {
    perror("listen call failed");
    exit(-1);
  }
  
  // Server is ready
  printf("%s Server listening on port %d\n", calcTime(), portNum);
  
  // Listening for clients loop
  for(;;)
  {
	// Set length of client addresss struct  
    cliAddrLen = sizeof(cliaddr);
  
    // Accept incoming connection and error check
    if ((connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &cliAddrLen)) < 0) 
	{
	  // errno was set, listen again	
	  if (errno == EINTR)
	    continue;
	  else
	  {
	    perror("accept error");
		exit(-1);
	  } 
	}
	
    // Create child to handle client
	if ((pid = fork()) == 0) 
	{
	  // Close the listening socket.	
	  close(listenfd);
	  
      // Convert CLient IP address to a human-readable string.
      inet_ntop(AF_INET, &(cliaddr.sin_addr), ip, INET_ADDRSTRLEN);
	  
	  // Process the request
	  serverFunc(connfd, ip);
	  
	  // Close the client's socket
	  close(connfd);
	  exit(0);
	}
	
	// Parent process
	else if (pid > 0)
	{
      // Parent closes connected socket		
	  close(connfd);
	}
	
	// Error forking a child.
	else 
	{
	  close(listenfd);
	  close(connfd);
	  perror("Error creating process: ");
	}
  }
  
  return 0; 
}  // End Main

/**********************************************************************************
 * Function name: serverFunc
 * Description: Chat protocol between the client and server. 
 * Parameters: sockfd - int - the socket descriptor
 *             ipaddr - char * - array to hold the IP address of client            
 * Return Value: N/A
 *********************************************************************************/
void serverFunc (int sockfd, char * ipaddr)
{
  // Buffer to hold socket stream msgs
  char buf[BUFSIZE];
  
  // Initialize return value for recv()
  int bytes = 0;
  
  // Values to store length of sent and received msgs.
  int sendLen, recvLen, rc;
  
  // Store HELLO in send buffer
  strcpy(buf, "Hello");
  
  // Send size and hello to socket.
  if((send_msg(sockfd, buf) == -1))
  {
    printf("%s Error Sending: ", calcTime());  
    exit -1;
  }

  // Loop to Receive CMDs from client.
  for (;;)
  {
    // Clear the buffer
    bzero (buf, BUFSIZE);
    rc = 0;
	
	// Receive Command from Client
	recv_msg(sockfd, buf);
	
	// If BYE received, disconnect and close socket.
	if (strncmp("BYE", buf, 3) == 0)
	{
	  printf("%s Client has exited...\n", calcTime());
	  break;		
	}

	// If PWD, call func to display working directory  
	else if (strncmp("PWD", buf, 3) == 0)
	{
	  printf("%s Client requested working directory...\n", calcTime()); 
	  pwd(sockfd,BUFSIZE,1);
	}

	// If DIR, display current directory contents
	else if (strncmp("DIR", buf, 3) == 0)
	{
	  printf("%s Client requested directory listing...\n", calcTime());
	  dir(sockfd,BUFSIZE,1);
	}

	// IF CD, change directory on the server 
	else if (strncmp("CD", buf, 2) == 0)
	{
	  printf("%s Client requested directory change...\n", calcTime());
	  rc = cd(sockfd, BUFSIZE, 1, (char *)&buf[3]);
	  // Error
	  if (rc == -1)
	  {
	    printf("%s Error in Directory Change...\n", calcTime());	   
	    continue;
	  }		
	}	  

	// If DOWNLOAD, RESPOND with READY and send the file   
	else if (strncmp("DOWNLOAD", buf, 8) == 0)
	{
      printf("%s Client requested file download...\n", calcTime());
	  rc = download(sockfd, (char *)&buf[9]);	
	  if (rc == -1)
	  {
	    printf("%s Error in Download...\n", calcTime());	   
	    continue;
	  }
	}		  

	// Invalid Command (Will never happen, client err checks)
	else
	{
	  continue;
	}
    
  }
} // End serverFunc

/**********************************************************************************
 * Function name: download
 * Description: Send client requested file over socket for download.
 * Parameters: soc - int - the socket descriptor
 *             f - char * - String containing requested filename           
 * Return Value: 0 success, -1 failure
 *********************************************************************************/
int download (int sock, char * f)
{
  char buffer[BUFSIZE];
  int bytes = 1;
  char * fileExists = "READY";
  char * fnf = "File Not Found";
  char * endFlag = "%%%";
  
  // Attempt to open file with read priveliges
  int fd = open(f, O_RDONLY);
  
  // Error opening file.
  if (fd == -1)
  {
	// Put error message in the buffer.
	strcpy(buffer,fnf);
	
	// Send to socket
	send_msg(sock, buffer); 
    return -1;	
  }
  
  // File exists
  else
  {
    // Put READY in buffer
	strcpy(buffer, fileExists);
	
	// Send READY
	send_msg(sock, buffer);
	
	// Zero buffer to prepare
    bzero(buffer, BUFSIZE);
	
	// Receive reply from client
	recv_msg(sock, buffer);
    
	// Client cancels DOWNLOAD.
    if ((strncmp(buffer, "STOP", 4) == 0))
	{	
      return -1;
	}

    // Client wants to proceed
    else if ((strncmp(buffer, "READY", 5) == 0))
	{
      while(bytes > 0)
      {
		// Zero the buffer  
		bzero(buffer, BUFSIZE);
        
		// Read from file into buffer
		bytes = read(fd, buffer, BUFSIZE);
		
		// Set terminating char
		buffer[bytes] = '\0';
		
		// Send buffer to socket
        send_msg(sock, buffer);		
      }
	  
	  // Send ending signal
      send_msg(sock, endFlag);
	}
	
	else
	{
	  send_msg(buffer, fnf);	
	  return -1;
	}

    // Confirm DOWNLOAD
    printf("%s Server sent %s to client\n", calcTime(), f);	
  }	

  close(fd);
  
  return 0;
} // End download

/**********************************************************************************
 * Function name: calcTime
 * Description: Display date and time on screen.
 * Parameters: N/A          
 * Return Value: ctime(&t) - A Char * to the date/time char array.
 *********************************************************************************/
char * calcTime()
{
  time_t t;
  time(&t);
  
  return ctime(&t);
}