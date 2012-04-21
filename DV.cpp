/**
 * Abhishek Srinath
 * 17th April 9:00pm
 */

#include <iostream>
#include <string.h>
#include <map>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>      



using namespace  std;

#define MAXARG 7

#define MAXCMD 7

#define MAXSIZE 1024



void update(); 
void step();
void packets();
void display();
void disable();
void crash();
void server();
void (*f)();

void start_udp_server();
void start_shell();
void check_on_stdin();
void check_on_udp_main_server();
void find_ip_of_current_Server();
void initialize(char );
void initialize();
void createUpdateMessage();
void sendUpdateMessageToNeighbors();
void createReceiveStructures();
void DistanceVector();

void tokenize(char *);
int docmd(char *);





struct NODE{
    short int id;
    short int portNo;
    char ip[INET6_ADDRSTRLEN];
    short int cost;
    short int nextHopId;
};

struct RECEIVESTRUCTURE{
    short int id;
    short int portNo;
    char ip[INET6_ADDRSTRLEN];
    short int cost;
    short int nextHopId;
    short int dummy;
};


/**
 * Structure to hold the commands
 */
struct pcmds
{
  char cmd[30];
  void (*handler) ();
  int id;
};


struct pcmds cmds[] = { 
    {"server", server, 1},
    {"update", update, 2},
    {"step", step, 3},
    {"packets", packets, 4},
    {"display", display, 5},
    {"disable", disable, 6},
    {"crash", crash, 7}
  };



struct NODE node[6];
struct RECEIVESTRUCTURE receivingStructure[6];
vector<int> currentNeighbors;
vector<int>::iterator currentNeighbors_itr;
char hostIP[INET6_ADDRSTRLEN];
short int hostPort;
short int hostId;
char *tokens[MAXARG];
int tokenize_count;
char commandline_copy[256];
int interval;
short int numberOfServers = 0;
short int numberOfNeighbors = 0;
bool serverFlag = false;
int serverListner;
int count1= 0;
char updateMessage[MAXSIZE];
socklen_t addr_len;
char recv_data[MAXSIZE];
char receivedIP[INET6_ADDRSTRLEN];
short int numberofServersFollowing;


void sendUpdateMessageToNeighbors()
{
    
    
    for(currentNeighbors_itr = currentNeighbors.begin();currentNeighbors_itr != currentNeighbors.end(); currentNeighbors_itr++)
    {
        
        if(node[*currentNeighbors_itr].cost != 32767)
        {
                struct sockaddr_in server_addr;
                struct hostent *host;
                host= (struct hostent *) gethostbyname((char *)node[*currentNeighbors_itr].ip);
                int sock;
        
                if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
                {
                        perror("socket");
                        exit(1);
                }
            
                server_addr.sin_family = AF_INET;
                server_addr.sin_port = htons(node[*currentNeighbors_itr].portNo);
                server_addr.sin_addr = *((struct in_addr *)host->h_addr);
                bzero(&(server_addr.sin_zero),8);
               // printf("strlen %d \n ",strlen(updateMessage));
                sendto(sock, updateMessage, 1024, 0,(struct sockaddr *)&server_addr, sizeof(struct sockaddr));
            
        }
    }
    
}
//    number of servers(2) + Host server port(2) +Host serverIP(4)
//                                +
//   ( Server IP n(4) + Server port n(2) + 0x0(2) + Server ID n(2) + Cost n(2) ) * number of servers
void createUpdateMessage()
{
    
    short int zero = 0;
    unsigned int tempSourceIP = inet_addr(hostIP);
    int pos=0;
    memset(updateMessage,'\0',MAXSIZE);
    
    memcpy(updateMessage+pos,&numberOfServers,sizeof(short int));  //0
   // cout << "Copying no of servers-" <<  numberOfServers;
    pos+=sizeof(short int);
   // cout << pos << endl;
    memcpy(updateMessage+pos,&hostPort,sizeof(short int));   //2
   // cout << "Copying host port" << hostPort;
    pos+=sizeof(short int);
    //cout << pos << endl;
    memcpy(updateMessage+pos,&tempSourceIP,sizeof(unsigned int)); //4
    pos+=sizeof(unsigned int);
    //cout << tempSourceIP << endl;
    for(int i = 1;i < numberOfServers;i++)
    {
        unsigned int tempSourceIP1 = inet_addr(node[i].ip);
        memcpy(updateMessage+pos,&tempSourceIP1,sizeof(unsigned int)); //8
        pos+=sizeof(unsigned int);
        memcpy(updateMessage+pos,&node[i].portNo,sizeof(short int)); //12
        pos+=sizeof(short int);
        memcpy(updateMessage+pos,&zero,sizeof(short int));  //14
        pos+=sizeof(short int);
        memcpy(updateMessage+pos,&node[i].id,sizeof(short int));  //16
        pos+=sizeof(short int);
        memcpy(updateMessage+pos,&node[i].cost,sizeof(short int)); //18
        pos+=sizeof(short int);
    }
    
   // printf("message -->%d",strlen(updateMessage));//-" << updateMessage << endl ;

}

/**
 * 
 * 
 * 
 * Have to check whether there exist a link btw the current server and the specified server
 * 
 * 
 * 
 * 
 */
void update()
{
    // check if the update command is correct
    // check if the update is for the server ie display a message if the command says update 2 3 7
    
    
    if(tokenize_count == 4 && serverFlag)
    {
        char tempId[1];
        char noOfFields[2];
        char hostPort[2];
        
        int n;
        n = sprintf (tempId,"%d",hostId);
        if(strncmp(tokens[1], tempId,1) == 0)
        {
            // update the local structure eg : - update 1 2 8 in server 1 
            if(strcmp(tokens[3],"inf") == 0)
            {
                node[atoi(tokens[2])].cost = 32767;
            }
            else
            {
                node[atoi(tokens[2])].cost = atoi(tokens[3]);
            }
           // create a string according to the msg format
          //  createUpdateMessage();
           // send the string to the neighbors if the link is not equal to infinity
           // sendUpdateMessageToNeighbors();
           
            cout << "Update success";
		fflush(stdout);
            
        }
        else
        {
            cout << "You are updating distance at the wrong server";
		fflush(stdout);
        }
        
    }
    else
    {
        cout << "The update command is not correct or server command is not executed\n";
        fflush(stdout);
	 return;
    }
    
}
void step()
{
    
}
void packets()
{
    
}
void display()
{
    
}
void disable()
{
    
}
void crash ()
{
    
}


/**
 * This is the function which determines which function to call based on the 
 * input give by the user
 * @param cmdline
 * @return 
 */
int docmd(char *cmdline)
{
//printf("docmd\n");

  int i = 0;
  if (*cmdline == '\0')
  {
    write(1,"Bulls>",6);
    return 0;
  }
  tokenize(cmdline); 
  for (i = 0; i < MAXCMD; i++)
    {
      if (!strcmp (cmds[i].cmd, tokens[0]))
	{
	  f = cmds[i].handler;
	  f();
	  write(1,"Bulls>",6);
	  return 0;
	}
    }
  printf ("invalid command\n");
  fflush(stdout);
  write(1,"Bulls>",6);
  return 1;
}


/**
 * This is the function which takes the input command and it tokenizes it based on spaces
 * @param cmdline
 */
void tokenize(char *cmdline)
{
    char *result;
    char delims[]=" ";
    int j=0;
    tokenize_count=0;
    result = strtok( cmdline, delims );
			tokens[j]=result;
			j++;
		
			while(result != NULL)
			{
				result=strtok(NULL,delims);
				tokens[j]=result;
				j++;
                                tokenize_count++;
			}
                        
                      
}


/**
 * 
 */
void initialize()
{
    for(int i = 1;i < 6;i++)
    {
        node[i].cost = 32767;
        node[i].id = 32767;
        strcpy(node[i].ip ," ");
        node[i].nextHopId = 32767;
        node[i].portNo = 32767;
    }
    
}

/**
 * 
 * @param initFile
 * @return 
 */
void initialize(char initFile[256])
{
    ifstream initializationFile;
    
    string STRING;
    
    initializationFile.open(initFile, ifstream::in);
        
	getline(initializationFile,STRING); // Saves the line in STRING.
        numberOfServers = atoi(STRING.c_str());
	cout << "Number of servers"<< numberOfServers << endl; // Prints our STRING.
        
        getline(initializationFile,STRING); // Saves the line in STRING.
        numberOfNeighbors = atoi(STRING.c_str());
	cout << "Number of neighbors"<< numberOfNeighbors << endl;
        
        
        
        /**
         *  Populate structures
         *  Extract the neighbors (2 field) and store in an array
         * 
         */
        char delims[] = " ";
        for(int i = 0;i < numberOfServers;i++)
        {
            getline(initializationFile,STRING);
            
            
             char *result = NULL;
             vector<string> initLine;
             vector<string>::iterator initLine_itr;
             
             int id;
             char *str1=(char *)STRING.c_str();
              result = strtok(str1, delims);

              while( result != NULL ) 
              {
                  initLine.push_back(result);
                   result = strtok( NULL, delims );
                         
              }
              
              initLine_itr = initLine.begin();
              
              id=atoi((*initLine_itr).c_str());
              
              node[id].id = id;
              strcpy(node[id].ip,initLine[1].c_str());
		
               cout << "\nnode[id].ip-->" << node[id].ip << "\t" << strlen(node[id].ip);
		cout << "\nHostIP--->" << hostIP << "\t" << strlen(hostIP);	
	
		if(strncmp(node[id].ip,hostIP,16)==0)
              {
		  cout << "Inside the if condition\n"; 
                  node[id].cost = 0;
                  hostId = id;
                  hostPort = atoi(initLine[2].c_str()); 
                  cout << "cost-"<< node[id].cost << endl;
              }
              node[id].portNo = atoi(initLine[2].c_str());
      
        }
        
        for(int i=0;i<numberOfNeighbors;i++)
        {
            getline(initializationFile,STRING);
            vector<string> initLine;
            char *result = NULL;  
            vector<string>::iterator initLine_itr;
            int id;
             
             //int id;
             char *str1=(char *)STRING.c_str();
             result = strtok(str1, delims);

              while( result != NULL ) 
              {
                  initLine.push_back(result);
                  result = strtok( NULL, delims );             
              }
              
              initLine_itr = initLine.begin();
              initLine_itr++;
              id=atoi(initLine[1].c_str());
              currentNeighbors.push_back(id);
              node[id].cost = atoi(initLine[2].c_str());
        }
        
//        for(int i=1;i<6;i++)
//        {
//            cout << node[i].id << "\t" << node[i].ip << "\t" << node[i].portNo << "\t"<< node[i].cost << endl;
//        }
//	initializationFile.close();
//        cout << "\nhostIP" << hostIP;
//        cout << "\nhostPort" << hostPort;
//        cout << "\nCurrent neighbors" << endl;
//	cout << "\nHostID-" << hostId << endl;
//        for(currentNeighbors_itr = currentNeighbors.begin();currentNeighbors_itr != currentNeighbors.end();currentNeighbors_itr++)
//        {
//            cout << *currentNeighbors_itr << "\t" ;
//            fflush(stdin);
//        }
//    
}


/**
 * This is the function which is called from the main function. This starts 
 * the tcp server and it loops continuously over th stdin , tcp-main-server, 
 * connection fd's
 */
void start_shell()
{
	
	write(1,"Bulls>",6);
	for(;;)
	{
		check_on_stdin();
	}
}


void start_udp_server()
{
        int sock;
        
        struct sockaddr_in server_addr ;


        if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
        {
            perror("Socket");
            exit(1);
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(hostPort);
        server_addr.sin_addr.s_addr = INADDR_ANY;
        bzero(&(server_addr.sin_zero),8);


        if (bind(sock,(struct sockaddr *)&server_addr,
            sizeof(struct sockaddr)) == -1)
        {
            perror("Bind");
            exit(1);
        }
        
        addr_len = sizeof(struct sockaddr);

	cout << "\nUDPServer Waiting for client on port" << hostPort;
        fflush(stdout);
        
        serverListner = sock;

}


//    number of servers(2) + Host server port(2) +Host serverIP(4)
//                                +
//   ( Server IP n(4) + Server port n(2) + 0x0(2) + Server ID n(2) + Cost n(2) ) * number of servers
void createReceiveStructures()
{
    
    short int receivedFromServerPortNo;
    int posToRead = 0;
    
    struct in_addr temp_in;
    char *tempIP;
     
    memcpy(&numberofServersFollowing,recv_data+posToRead,sizeof(short int) );
    posToRead+=sizeof(short int);
    memcpy(&receivedFromServerPortNo,recv_data+posToRead,sizeof(short int) );
    posToRead+=sizeof(short int);
    memcpy(&(temp_in.s_addr),recv_data+posToRead,sizeof(unsigned int));
    posToRead+=sizeof(unsigned int);  
    tempIP = inet_ntoa(temp_in);
    strcpy(receivedIP,tempIP);        
    
    for(int i = 1;i < numberofServersFollowing;i++)
    {
        char *tempIP1;
        struct in_addr temp_in1;
        
        memcpy(&(temp_in1.s_addr),recv_data+posToRead,sizeof(unsigned int));
        tempIP1 = inet_ntoa(temp_in1);
        strcpy(receivingStructure[i].ip,tempIP1);      
        posToRead+=sizeof(unsigned int);
        
        memcpy(&receivingStructure[i].portNo,recv_data+posToRead,sizeof(short int));
        posToRead+=sizeof(short int);
        
        memcpy(&receivingStructure[i].dummy,recv_data+posToRead,sizeof(short int));
        posToRead+=sizeof(short int);
        
        memcpy(&receivingStructure[i].id,recv_data+posToRead,sizeof(short int));
        posToRead+=sizeof(short int);
        
        memcpy(&receivingStructure[i].cost,recv_data+posToRead,sizeof(short int));
        posToRead+=sizeof(short int);
    }
    cout << "numberofServers" << numberofServersFollowing << endl;
    cout << "receivedServerPortNo" << receivedFromServerPortNo << endl;
    cout << "Received Server Ip" << receivedIP << endl;        
    
    for(int i = 1;i < numberofServersFollowing;i++)
    {
        cout << receivingStructure[i].ip << endl;
        cout << receivingStructure[i].portNo << endl;
        cout << receivingStructure[i].dummy << endl;
        cout << receivingStructure[i].id << endl;
        cout << receivingStructure[i].cost << endl;
        cout << "-----------------------------------------------------" << endl;
    }
    //exit(0);
}

             


               /* get the server ID from the incoming server IP
               * get cost = node[id].cost
               * 
               * for(i=1 to number of servers)
               * {
               *        if(cost + receivedStructure[i].cost) < node[i].cost
               *        update the node[i].cost
               *        set boolean sendflag to true
               * 
               * }
               * 
               * if(sendFlag)
               * {
               *        Send it to neighbors
               * }
               *
               */
void DistanceVector()
{
    short int id;
    short int cost;
    bool sendFlag = false;
    
    for(int i = 1;i < numberOfServers;i++)
    {
        if(strcmp(receivedIP,node[i].ip)==0)
        {
           id = node[i].id; 
           cout << "Assigned the id to the id variable" << endl;
        }
    }
    
    cost = node[id].cost;
    
    for(int i = 1;i < numberofServersFollowing;i++ )
    {
        if((cost + receivingStructure[i].cost) < node[i].cost)
        {
            node[i].cost = (cost + receivingStructure[i].cost);
            sendFlag = true;
        }
    }
    
    if(sendFlag)
    {
        createUpdateMessage();
        sendUpdateMessageToNeighbors();
    }
    
    
    
}
void check_on_stdin()
{

	fd_set readfds;
	struct timeval tv; 
	char cmdline[256];
        int n =0;
        if(count1 == 0)
        {
                FD_SET(0,&readfds);
        }
        else
        {
                FD_SET(serverListner,&readfds);
                FD_SET(0,&readfds);
        }       
        n = serverListner + 1;
	tv.tv_sec=0;
	tv.tv_usec=0;

        if((select(n,&readfds,NULL, NULL, &tv)) == -1)
	    {
	      perror ("select");
	      exit (1);
	    }
	  if (FD_ISSET(0, &readfds))
	    {
	   //   check=1;
	      fgets(cmdline, 256, stdin);
	      if (cmdline[strlen (cmdline) - 1] != '\0')
	      	 cmdline[strlen (cmdline) - 1] = '\0';
	        strcpy(commandline_copy,cmdline);
                count1++;
		docmd(cmdline);
	    }
          else if(FD_ISSET(serverListner,&readfds))
          {

             // cout << "Receiving some information on the server";
              int bytes_read =0;
              
              struct sockaddr_in client_addr;
              bytes_read = recvfrom(serverListner,recv_data,1024,0,(struct sockaddr *)&client_addr, &addr_len);
              recv_data[bytes_read] = '\0';
              
              createReceiveStructures();
          
              DistanceVector();
              
              
          }
	
}

char *routingUpdateInterval = NULL;
    
void server()
{
    serverFlag = true;
    if(tokenize_count == 5)
    {
        find_ip_of_current_Server();
    
        initialize(tokens[2]);
    
        routingUpdateInterval = tokens[4];
    
        interval = atoi(routingUpdateInterval);
        
        start_udp_server();
    }
    else
    {
        cout << "Invalid number of arguments\n";
        //fflush(stdout);
        return;
    }
    
}
/**
 * //This is the logic used to find the host IP. This is using system command
 */
void find_ip_of_current_Server()
{
    
        char cmd[60];
        int fd;
        char *host;
        host=getenv("HOST");
        char hostIP1[INET6_ADDRSTRLEN];
	system("rm temphost temphost1");
	system("touch temphost temphost1");
	sprintf(cmd,"host %s>temphost",host);
	system(cmd);
	sprintf(cmd,"cut -d \" \" -f4 temphost>temphost1");
	system(cmd);
        fd=open("temphost1",O_RDWR,0);
	read(fd,hostIP1,INET_ADDRSTRLEN);
	strncpy(hostIP,hostIP1,strlen(hostIP1)-1);
  	close(fd);
}



int main(int argc, char** argv)
{
    initialize();
    start_shell ();
    return 0;
}
