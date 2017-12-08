//*********************************************************************
//// FILE:        assign9.cpp
//// AUTHOR:      Samuel Piecz
//// LOGON ID:    Z1732715
//// DUE DATE:    12/08/17
////
//// PURPOSE:     Make a forking TCP Server. 
////********************************************************************
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <algorithm>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <typeinfo>

using namespace std;

void processClientRequest( int connSock) {
	int received;
	char buffer[1024];
	
	// read a message from the client
	if ((received = read(connSock, buffer, sizeof(buffer))) <= 0) {
		perror("read");
		exit(EXIT_FAILURE);
	}
	
	cout << "Client sent " << received << " bytes: " << buffer << endl;
    
    ////////////////////////////////////////////////////////////////////
    
    // So we have a buffer, now we need to break it appart
    // The first section needs to be stored in a command variable
    // It can be either "GET" or "INFO"
    //
    // Next up, the string needs to be split by / signs.
    // If we have one / it needs to call "ls" from execvp or execlp whatever

    string command, pathname;

    // Split buffer into pch pointer to char
    char * pch;
    pch = strtok (buffer, " ");

    // Give us a counter variable
    int count = 0;

    // While we have variables in pch keep going
    while (pch != NULL)
    {
        // If it's the first command
        // fill the variable command
        // so we know what to run
        if (count == 0)
        {
            command = pch;
        }
        else if (count == 1)
        {
            pathname = pch;
        }

        // Reset pch
        pch = strtok (NULL, " ");
        // Count iteration
        count++;
    }

    // if command is get
    if (command == "GET")
    {
        // make variables for checking if file or dir
        struct stat s;
        
                    
        if (pathname.at(0) == '/' && pathname.size() > 1 )
        {
            pathname.erase(pathname.begin()+0);    
        }

        // if this is just a slash 
        if (pathname == "/")
        {
            int rs;
            // do execvp or execlp for ls
            rs = execl("/bin/ls", "/bin/ls", (char *)NULL); 
            cout << rs << endl;
        }
        else if ( stat(pathname.c_str(), &s) == 0 ) 
        {
           
            // if the command is a file
            if ( s.st_mode & S_IFDIR)
            {
                // it's a directory
                int ns;
                ns = execl("/bin/ls", "/bin/ls", pathname.c_str(), (char *)NULL);
                cout << ns << endl;
                cout << stat(pathname.c_str(),&s) << endl; 
            }    
            else if ( s.st_mode & S_IFREG )
            {

                if (pathname.substr(pathname.size()-2) == "..")
                {
                    cerr << "No .. pls." << endl;
                    exit(-1);
                }
                else
                {
                    // it's a file
                    int print_file;
                    print_file = execl("/bin/cat", "/bin/cat", pathname.c_str(), (char *)NULL);
                    cout << print_file << endl;
                }
            }
            else
            {
                // it is neither
                // This is where the error would go??? 
            }
        }
        else
        {
        }
    }
    // otherwise if it's info
    else if (command == "INFO")
    {
        int dateTime;
        dateTime = execl("/bin/date", "/bin/date", (char *)NULL);
        cout << dateTime << endl;
    }
    // else what are you doing
    else
    {
        // output an error
    }

    ///////////////////////////////////////////////////////////////////
		
	// write the message back to client 
	if (write(connSock, buffer, received) < 0)
    {
		perror("write");
		exit(EXIT_FAILURE);
	}

	close(connSock);
	exit(EXIT_SUCCESS);
}
        
int main(int argc, char *argv[]) {

	if (argc != 2) {
		cerr << "USAGE: TCPServerFork port\n";
		exit(EXIT_FAILURE);
	}
	
	// Create the TCP socket 
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}	
	// create address structures
	struct sockaddr_in server_address;  // structure for address of server
	struct sockaddr_in client_address;  // structure for address of client
	unsigned int addrlen = sizeof(client_address);	

	// Construct the server sockaddr_in structure 
	memset(&server_address, 0, sizeof(server_address));   /* Clear struct */
	server_address.sin_family = AF_INET;                  /* Internet/IP */
	server_address.sin_addr.s_addr = INADDR_ANY;          /* Any IP address */
	server_address.sin_port = htons(atoi(argv[1]));       /* server port */

	// Bind the socket
	if (bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}	
	
	// listen: make socket passive and set length of queue
	if (listen(sock, 64) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	
	cout << "TCPServer listening on port: " << argv[1] << endl;
	
	// Run until cancelled 
	while (true) {
		int connSock=accept(sock, (struct sockaddr *) &client_address, &addrlen);
		if (connSock < 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}
		// fork
		if (fork()) { 	    // parent process
			close(connSock);
		} else { 			// child process
			processClientRequest(connSock);
		}
	}	
	close(sock);
	return 0;
}
