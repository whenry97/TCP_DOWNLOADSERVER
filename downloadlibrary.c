/************************************************************/
/* Author: Michael Brown                                    */
/* Creation Date: Nov 15th, 2019                            */
/* Due Date: Nov 30th, 2019                                 */
/* Course: CSC328 010                                       */
/* Professor Name: Dr. Frye                                 */
/* Assignment: #7                                           */
/* Filename: library.c                                      */
/* Language: c                                              */
/* Compilation command: gcc -c library.c                    */
/*  ar rc dllibrary.a library.o                             */
/* Execution command: ./a.out                               */
/* Purpose: Library functions for download server project   */
/*                                                          */
/*                                                          */
/************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <dirent.h>
#include <stdint.h>

/*
Function Name: pwd
Description: Print the path of the current directory
Parameters: int - sock - socket file descriptor, int - BUFSIZE - Size of the buffer to put path name in, int - caller - Value to determine if client(0) or server(1) called the function
Return: 0 if success, -1 if failed.
 */
int pwd(int sock, int BUFSIZE, int caller){
	char pwd[BUFSIZE];
	int size;
	if (getcwd(pwd,BUFSIZE) == NULL){
		perror("Error getting directory name: ");
		return -1;
	}
	//PWD was called
	if(caller == 1){
        send_msg(sock, pwd);
		return 0;
	}
	//LPWD was called
	if(caller == 0){
		printf("Current working directory: %s\n", pwd);
		return 0;
	}
}


/*
Function Name: dir
Description: Print the content of the current directory
Parameters: int - sock - socket file descriptor, int - BUFSIZE - Size of the buffer to put path name in, int - caller - Value to determine if client(0) or server(1) called the function
Return: 0 if success, -1 if failed.
 */
int dir(int sock, int BUFSIZE, int caller){
  DIR *d;
  struct dirent *dir;
  char directory[BUFSIZE];
  int i = 0;
  int bytes;
  char * end = "%%%";
  int endLen = strlen(end);
  d = opendir(".");
  if (d)
  {
    while ((dir = readdir(d)) != NULL)
    {	
		if(caller == 0){
			printf("%s\n", dir->d_name);
		}
		if(caller == 1){
            send_msg(sock, dir->d_name);			
		}
	}
    closedir(d);
  }
  
  send_msg(sock, end);
  
  return(0); 
}


/*
Function Name: cd
Description: Change the current directory
Parameters: int - sock - socket file descriptor, int - BUFSIZE - Size of the buffer to put path name in, int - caller - Value to determine if client(0) or server(1) called the function
	char* - new - New directory path string
Return: 0 if success, -1 if failed.
 */
int cd(int sock, int BUFSIZE, int caller, char * new)
{  
  // Change the directory
  if(chdir(new) == -1)
  {
    char * err = "Error Changing Directories";
	
	// Server sends err to client
	if (caller == 1)
	  send_msg(sock, err);
    
	// Client prints err msg
	if (caller == 0)
	  printf("%s\n", err);
    
	return -1;
  }

  // CD worked, send confirmation to client
  if (caller == 1)
    send_msg(sock, "OK");    

  return 0;  
}

// From textbook "Beej's Guide to Network Programming"
// https://stackoverflow.com/questions/35833918/implementing-sendall-and-recvall-in-c-and-python
/*
Function Name: sendAll
Parameters: sockfd, socket descriptor. buf, buffer to send. numBytes, the size.
Return: The number of bytes sent.
 */
int sendAll(int sockfd, char * buf, int numBytes)
{
  int ret;           // Return value for 'send'
  int sentBytes = 0;     // Total number of bytes sent
  // Send the given number of bytes.
  while (sentBytes < numBytes) {
    ret = send(sockfd, buf + sentBytes, numBytes - sentBytes, 0);
    // Error encountered.
    if (ret == -1) {
      return -1;
    }
    // Data sent.
    else {
      sentBytes += ret;
    }
  }
  return sentBytes;
}  

/*
Function Name: send_msg
Description: Shortcut for sending length of data and actual data
Parameters: int - sock - socket file descriptor, char *buf - Pointer to a buffer
Return: size of data if success, -1 if failed.
 */
int send_msg(int sock,  char * buf)
{
  int len = strlen(buf);
  // Send 4-byte length
  if ((send(sock, &len, 4, 0)) == -1)
  {
    perror("Server: sending");
    return -1;    
  }  
  
  // Send actual data
  if ((sendAll(sock, buf, len)) == -1)
  {
    perror("Server: sending");
    return -1;	  
  }
  return len;
}	

/*
Function Name: recv_msg
Description: Shortcut for receiving length of data and actual data
Parameters: int - sock - socket file descriptor, char **buf - Pointer to a buffer
Return: size of data if success, -1 if failed.
 */
int recv_msg(int sock, char **buf)
{
  // Store length of message
  int len;
  
  // Receive the length as first 4 bytes
  if ((recv(sock, &len, 4, MSG_WAITALL) == -1))
  {
    perror("Server: receiving");
    return -1;	  
  }
  
  // Convert from Network Byte Order
  htonl(len);
  if ((recv(sock, buf, len, MSG_WAITALL)) == -1)
  {
	perror("Server: receiving");
    return -1;		
  }
  return len;
}