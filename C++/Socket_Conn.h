#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string>
#include <vector>

using std::string;
using std::vector;
#ifndef SOCKET_CONN_H
#define SOCKET_CONN_H
#define MAXLINE 1024
class Socket_Conn
{
public:
	Socket_Conn(int port, bool isServer);//blocked, will not return till the client connects, the client will try to connect till the server is built.
	//don't forget to set the connection as non-blocked after the connection is built.
	void sendInfo(string info);//send info out, always contain a '\n' at the end!!
	vector<string> receiveInfo();//try to receive, non-blocked, if no info is inside, simply return an empty string.
	void shutDown();
	~Socket_Conn();
private:
	int sockfd;
	int connfd;
	bool isServer;
	string last_time_left;
	void error(char *msg)
	{
		perror(msg);
		exit(1);
	}
};

Socket_Conn::Socket_Conn(int portno, bool s)
{
	isServer = s;
	last_time_left = "";
	if(isServer)
	{
		//for the server, bind a socket to the port and listen, 
		//build a new socket when a client is connected.
		int newsockfd, clilen;
		struct sockaddr_in serv_addr, cli_addr;
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0) 
			error("ERROR opening socket");
		bzero((char *) &serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(portno);
		int flag = 1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) 
			error("ERROR setting sockets.");
		if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
			error("ERROR on binding");
		listen(sockfd,5);
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *)&clilen);
		if (newsockfd < 0) 
			error("ERROR on accept");
		connfd = newsockfd;
	}
	else
	{

		struct sockaddr_in serv_addr;
		struct hostent *server;

		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0) 
		    error("ERROR opening socket");
		server = gethostbyname("localhost");
		if (server == NULL) {
		    fprintf(stderr,"ERROR, no such host\n");
		    exit(0);
		}
		bzero((char *) &serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		bcopy((char *)server->h_addr, 
		     (char *)&serv_addr.sin_addr.s_addr,
		     server->h_length);
		serv_addr.sin_port = htons(portno);
		if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
		    error("ERROR connecting");
		//if the object is a client, connfd and sockfd are the same.
		connfd = sockfd;
	}
}

void Socket_Conn::sendInfo(string info)
{
	//use connfd to send
	char sendline[MAXLINE];
	strcpy(sendline, info.c_str());
	if(send(connfd, sendline, strlen(sendline), MSG_DONTWAIT)<0)
	{
		error("ERROR sending messages.");
	}
}

vector<string> Socket_Conn::receiveInfo()
{
	char buffer[MAXLINE];
	vector<string> result;
	int n = recv(connfd, buffer, MAXLINE, MSG_DONTWAIT);
	if(n<0)
	{
		return result;
	}
	string buff_str(buffer, n);
	while(n==MAXLINE)
	{
		n = recv(connfd, buffer, MAXLINE, MSG_DONTWAIT);
		string temp(buffer,n);
		buff_str += temp;
	}

	int index = buff_str.find("\n");
	while(index!=-1)
	{
		string temp = buff_str.substr(0, index);
		temp = last_time_left+temp;
		last_time_left = "";
		result.push_back(temp);
		buff_str = buff_str.substr(index+1, string::npos);
		index = buff_str.find("\n");
	}
	if(buff_str.size()!=0)
		last_time_left = buff_str;
	return result;

}
void Socket_Conn::shutDown()
{
	if(isServer)
		close(connfd);
	close(sockfd);
}
Socket_Conn::~Socket_Conn()
{
	if(isServer)
		close(connfd);
	close(sockfd);
}

#endif


