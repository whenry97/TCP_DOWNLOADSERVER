/************************************************************/
/* Author: Michael Brown, William Henry, Jesse Stone        */
/* Creation Date: Nov 15th, 2019                            */
/* Due Date: Nov 26th, 2019                                 */
/* Course: CSC328 010                                       */
/* Professor Name: Dr. Frye                                 */
/* Assignment: #7  Download Client+Sever+Library            */
/* Filename: readme.txt                                     */
/* Language:                                                */
/* Compilation command: N/A                                 */
/* Execution command: N/A                                   */
/* Purpose: Readme file for the download                    */
/*           server,client,library project                  */
/************************************************************/

The project is a concurrent download server, a client that can communicate with the
server and a library for the shared functionality. The server is a concurrent server
so multiple clients may connect at one time. The Client can use protocols to traverse
the server as well as local files. The Client can download a file from the server.

To compile: There is a make file included in the project to make compilation easy.
Just type make and all of the compilation commands will be run. If there is no make file:
1. gcc -c downloadlibrary.c //Compiles the library into object code
2. ar rc libraryname.a downloadlibrary1_3.o //Create library with object file
3. gcc server.c libraryname.a -o server //Compile server with library, output file renamed server
4. gcc client.c libraryname.a -o client //Compile client with library, output file renamed client

To run: The client and server must both be running for the system to work properly.
1. In a putty shell, ./server *PORTNUMBER* > output.log (We have been using 35444)
	This will start the server and put it in listening mode. There is no output to the screen
	at this point.
2. In another putty shell, ./client
		This will start the client.
		
Design Overview:
-	How will the Server indicate the end of the directory listing?
	The server could read in the name of all files in the order they are stored (most likely alphabetical).
	A count could be kept of all the files in that specific directory, and after the count is reached and all files are printed, it can stop printing and move to a new line waiting for the next command.
	This implementation can look similar to the UNIX command ‘ls’.
	
-	How will the client know it has received the entire directory name?
	We could implement a sentinel character to be placed at the end of the path of the current directory.
	Once this sentinel value is reached, the program will stop reading the directory name and send it to the client.
	While we did use the sentinel value for some of the protocols we mostly switched over to using the MSG_WAITALL flag
	included in the send() and receive() functions. This made sure that all data would be sent and received.
	
-	What will server response be for successful and unsuccessful cd commands?
	On a successful cd command, the server can just respond with a success message.
	On an unsuccessful cd command, an error message can be sent.
	It can either tell the user to enter an existing directory name, or an error message if they do not have permissions to access the directory they entered.
	
-	How does the client know when the server has completed sending the file?
	The size of the file that is being downloaded can be sent by the server to the client before the download starts.
	Once the size of the file has been reached on the client’s side, it will stop receiving data.
	Each chunk of data can be sent to the socket with a specified size before it.
	The receiver can grab all that data using the size, and stop calling recv() once a sentinel is received from the socket.
	
-	How does the server know when the client has completed sending the file?
	The size of the requested file will be sent to the server at the start of the upload.
	This size will be kept track of as the server reads in the data, and when the size of the data file has been reached, it will stop execution and send a response back to the server that the file has been successfully uploaded. 

Type of library: A static library was used because of simplicity as well as speed.

Protocol Developed:
	The calls to our library functions are as follows:
		pwd(int sock, int BUFSIZE, int caller)
		dir(int sock, int BUFSIZE, int caller)
		cd(int sock, int BUFSIZE, int caller, char * new)
 
	In all cases we send the socket, the size of the buffer and the caller to the function.
	This allows the library function to know which process (client or server) called the 
	function and which directory path should be printed, which directory contents should be
	printed, and which process should change directory.
	The cd also requires a 4th argument, new, which is the new path to change to.
	The change directory accepts full and partial directory names, and will display an error on failure.

KNOWN ISSUES:
-	When compiling there are warnings, none of these prevent the compile from happening. This happens becuase of the interaction between strncmp() function in the client and the character
	array made to store all of the commands.
-   Could not find a graceful way to close the server. Once you are done with the server, you can just use Ctrl+C to kill the program.

How we worked as a team:
	We met multiple times to complete the project and worked in tandem to complete the code. Each person was in charge of
	one aspect of the project but we all worked together to complete the task. 
	
USER GUIDE:
The list of commands are:
	BYE, stops the client
	LPWD, Prints the clients current directory path
	PWD, Prints the servers current directory path
	LDIR, Prints the clients current directory contents
	DIR, Prints the servers current directory contents
	LCD, Changes the clients directory
	CD, Changes the servers directory
	DOWNLOAD *file*, Downloads a file from the server. Requires the file name as the second commandline argument
	HELP, Displays the commands available
	
	*ALL COMMANDS MUST BE CAPITAL*

