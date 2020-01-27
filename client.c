/*********************************************************************************************
* Author:           Jesse Stone
* Major:            Computer Science
* Creation Date:    November 1, 2019
* Due Date:         November 30, 2019
* Course:           CSC328 010
* Professor Name:   Dr. Frye
* Homework:         #6
* Filename:         client.c
* Language:         C, gcc version 6.4.0
* Compilation cmd:  make
* Execution cmd:    ./client < optional port number>
* Purpose:          Client that interfaces with a download server.
*********************************************************************************************/


#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
 
#define MAX 128 
#define PORT 34541
#define SA struct sockaddr 
#define cmdLength 10
#define cmdCount 9


int downloadFile(int, char*);

void clientCommands(int);


int main(int argc, char* argv[]) 
{
    int portNum;  

    // User entered Port Number to listen on
    if (argc == 2)
        portNum = atoi(argv[1]);

    // User entered no port number, use default
    else
        portNum = PORT;

    int sockfd, connfd; 
    struct sockaddr_in servaddr, cli; 

    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1)
    {
        printf("socket creation failed...\n"); 
        exit(0); 
    } 

    bzero(&servaddr, sizeof(servaddr)); 

    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    servaddr.sin_port = htons(portNum); 

    // connect the client socket to server socket 
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connection with the server failed...\n"); 
        exit(0); 
    } 

    // function for chat 
    clientCommands(sockfd); 

    // close the socket 
    close(sockfd); 
}


/*************************************************************************
* Function name:    downloadFile
* Description:      Downloads file from server.
* Parameters:       int sockfd: Socket file descriptor
*                   char* buff: A pointer to the char array that will hold the recieved data
* Return Value:     int - returns -1 if user decides to stop download, otherwise returns 1
*************************************************************************/
int downloadFile(int sockfd, char* buff)
{
	int fd, fd1;
	char inBuff[MAX];
	char newBuff[MAX];
	int bytes;
	
	// Get READY or FNF from Server
	bzero(newBuff, MAX);
	recv_msg(sockfd, newBuff);
	
    // If the server responded that it's ready
	if ((strncmp(newBuff, "READY", 5)) == 0)
	{
        // If the file already exists locally
		if (fd = open(buff, O_WRONLY | O_TRUNC) > 0)
		{
			bzero(inBuff, MAX);
			printf("File already exists, enter READY to overwrite, or STOP to cancel: ");
			fgets(inBuff, MAX, stdin);
			
			if ((strncmp(inBuff, "STOP", 4)) == 0)
			{
				send_msg(sockfd, inBuff);
				return -1;
			}
			else if ((strncmp(inBuff, "READY", 5)) == 0)
			{
			  // CLIENT READY
		      send_msg(sockfd, inBuff);
			  printf("%s\n", buff);
			  fd1 = open(buff, O_RDWR | O_CREAT | O_EXCL, 0644);
				while (1)
				{
					bzero(buff, MAX);	
					// Receive length from server
					bytes = recv_msg(sockfd, buff);
		
					if (strncmp(buff, "%%%", 3) == 0)
						break;
		
					// Write to file
					write(fd1, buff, MAX);
				}				  
			    close(fd1);
			}
			else
			{
				printf("Invalid command\n");
				return -1;
			}
			
			
		}
        // If the file does not exist locally
		else
		{
			bzero(inBuff, MAX);
			printf("File does not exist, enter READY to continue, or STOP to cancel: ");
			fgets(inBuff, MAX, stdin);
			
			if ((strncmp(inBuff, "STOP", 4)) == 0)
			{
				send_msg(sockfd, inBuff);
				return -1;
			}
			else if ((strncmp(inBuff, "READY", 5)) == 0)
			{
				// CLIENT READY
				send_msg(sockfd, inBuff);
				fd = open(buff, O_RDWR | O_CREAT | O_EXCL, 0644);
				while (1)
				{
					bzero(buff, MAX);	
					// Receive length from server
					bytes = recv_msg(sockfd, buff);
		
					if (strncmp(buff, "%%%", 3) == 0)
						break;
		
					// Write to file
					write(fd, buff, MAX);
				}
				close(fd);
			}
			else
			{
				printf("Invalid command\n");
				send_msg(sockfd, inBuff);
				return -1;
			}
			
		}
	}
    // If Server did not respond that it's ready
	else
	{
		printf("From Server: File Not Found\n");
		return -1;
	}
	  
    printf("Download complete\n");
    
    return 0;
}


/*************************************************************************
* Function name:    clientCommands
* Description:      Enables user to enter commands
* Parameters:       int sockfd: Socket file descriptor
* Return Value:     void
*************************************************************************/
void clientCommands(int sockfd)
{ 
    char buff[MAX]; 
    int sendLen;
	int l = 1;
    int rc;
    
    // List of client commands
    const char ch_arr[cmdCount][cmdLength] =
    {
        "BYE",
        "LPWD",
        "PWD",
        "LDIR",
        "DIR",
        "LCD",
        "CD",
        "DOWNLOAD",
        "HELP"
    };
    
	// Receive HELLO from server
    recv_msg(sockfd, buff);
	
	printf("From Server: %s\n", buff);

    while(1)
    {
		// Reset Values
        bzero(buff, MAX);
        printf("\nEnter command: ");
        fgets(buff,MAX,stdin);

        // BYE command
        if(strncmp(buff, ch_arr, strlen(ch_arr))==0)
        {   //Use “BYE” to end communication with server
            
            // Send length and data
            send_msg(sockfd, buff);
            break;
        } 
        // LPWD command
        else if(strncmp(buff, ch_arr+1, strlen(ch_arr+1))==0)
        {  //Local PWD
            pwd(sockfd,MAX, 0);
        }
        // PWD command
        else if(strncmp(buff, ch_arr+2, strlen(ch_arr+2))==0)
        {
            // Send length and data
            send_msg(sockfd, buff);
            
			// Receive Reply
            recv_msg(sockfd, buff);
            
            printf("Server Directory: %s\n", buff);
        }
        // LDIR command
        else if(strncmp(buff, ch_arr+3, strlen(ch_arr+3))==0)
        {
            dir(sockfd,MAX,0);
        }
        // DIR command
        else if(strncmp(buff, ch_arr+4, strlen(ch_arr+4))==0)
        {
            send_msg(sockfd, buff);
            
			// Receive data from socket until signal is sent
            while(1)
            {
                bzero(buff, MAX);
                recv_msg(sockfd, buff);
                
				if (strncmp(buff, "%%%", 3) == 0)
					break;
				
                printf("%s\n", buff);
            }		
        }
        // LCD command
        else if(strncmp(buff, ch_arr+5, strlen(ch_arr+5))==0)
        {
            // Get length
            sendLen = strlen(buff);
            buff[sendLen-1] = '\0';
            cd(sockfd, MAX, 0, (char *)&buff[4]);
        }
        // CD command
        else if(strncmp(buff, ch_arr+6, strlen(ch_arr+6))==0)
        {
            // Get length
            sendLen = strlen(buff);
            buff[sendLen-1] = '\0';
            // Send length and data
            send_msg(sockfd, buff);
			
			bzero(buff, MAX);
			
			recv_msg(sockfd, buff);
			
			if (strncmp(buff, "Error", 5) == 0)
			{
              printf("Error Changing Directories\n");
              continue;			  
			}				
        }
        // DOWNLOAD command
        else if (strncmp(buff, ch_arr+7, strlen(ch_arr+7))==0)
        {
			
			if ((buff[strlen(ch_arr+7)]) == '\n' || (buff[strlen(ch_arr+7)+1]) == '\n')
			{
				printf("usage: DOWNLOAD <filename>\n", buff);
				continue;
			}
			
            // Get length
            sendLen = strlen(buff);
            buff[sendLen-1] = '\0';
			
			send_msg(sockfd, buff);
			
			downloadFile(sockfd, (char*)&buff[9]);
			continue;
        }
        // HELP command
        else if (strncmp(buff, ch_arr+8, strlen(ch_arr+8))==0)
        {
			int i = 0;
            
            printf("Commands:\n");
            
            for(i = 0; i < cmdCount; i++)
            {
                printf("%s\n", ch_arr + i);
            }
        }
        else
        {
            printf("Command not recognized: %s\n", buff);
        }
    }
}
