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

#define INF 30000



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
int initialize(char );
void initialize();
void createUpdateMessage();
void sendUpdateMessageToNeighbors();
void createReceiveStructures();
void DistanceVector();
void addNewNode();
void populateArray();
void initializeArray();
void updateRoutingTable();
void initNeighborsToCostMap();
void updateRoutingTableWithIncomingNeighborsDV();
void updateLocalStructures();
void updateLocalStructureAfterDV();

void tokenize(char *);
int docmd(char *);



/*
 * This is the structure which maintains the information about a node. 
 * Data read from the init file initially will be stored in this structure
 */

struct NODE{
    short int id;
    short int portNo;
    char ip[INET6_ADDRSTRLEN];
    short int cost;
    short int nextHopId;
};


/*
 *This is the structure which maintains the information about the servers and other fields extracting information from the 
 * update message 
 */
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

/*
 * These are the maps used to hold specific information required for the DV algorithm
 */
/*holds <id,count> used for making the cost to INF if there is no response for 3 time units*/
map <int,int> currentNeighbors;   
map <int,int>::iterator currentNeighborsItr;

/*Holds <id,cost> the cost is used for calculating the cost to neighbors*/
map <int,int> neighborsToCostMap;  
map <int,int>::iterator neighborsToCostMapItr;

/*This stores ID of the node which is disabled so this rejects the further DV from the specific node */
map <int,int> disableLinkMap;
map <int,int>::iterator disableLinkMapItr;


char hostIP[INET6_ADDRSTRLEN];
short int hostPort;
short int hostId;
char *tokens[MAXARG];
int tokenize_count;
char commandline_copy[256];
int interval = 0;
short int numberOfServers = 0;
short int numberOfNeighbors = 0;
bool serverFlag = false;
int serverListner = 0;
int count1= 0;
char updateMessage[MAXSIZE];
socklen_t addr_len;
char recv_data[MAXSIZE];
char receivedIP[INET6_ADDRSTRLEN];
short int numberofServersFollowing;
int noOfUpdatePackets = 0;
struct timeval tv; 
fd_set readfds;
int **routingTable;


/**
 * This is the function which performs the function of sending the update message only to the neighbors
 * This takes care of not sending the update message if the cost of the link is infinity
 */
void sendUpdateMessageToNeighbors()
{
    
    
    for(currentNeighborsItr = currentNeighbors.begin();currentNeighborsItr != currentNeighbors.end(); currentNeighborsItr++)
    {
        
        if(node[currentNeighborsItr->first].cost != INF)
        {
                struct sockaddr_in server_addr;
                struct hostent *host;
                host= (struct hostent *) gethostbyname((char *)node[currentNeighborsItr->first].ip);
                int sock;
        
                if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
                {
                        perror("socket");
                        exit(1);
                }
            
                server_addr.sin_family = AF_INET;
                server_addr.sin_port = htons(node[currentNeighborsItr->first].portNo);
                server_addr.sin_addr = *((struct in_addr *)host->h_addr);
                bzero(&(server_addr.sin_zero),8);
               // printf("strlen %d \n ",strlen(updateMessage));
                sendto(sock, updateMessage, 1024, 0,(struct sockaddr *)&server_addr, sizeof(struct sockaddr));
            
        }
    }
    
}
/*    
   number of servers(2) + Host server port(2) +Host serverIP(4)                                
                     (+)
   ( Server IP n(4) + Server port n(2) + 0x0(2) + Server ID n(2) + Cost n(2) ) * number of servers
  
  The above mentioned fields will be mem copied into a char[] and this is sent to the neighbors.
  The data is copied from the structure NODE. which will be updated by the Distance Vector Algorithm
*/
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
    for(int i = 1;i <= numberOfServers;i++)
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
    
}

/*
 * This is the function which will be called when the user wishes to update the link cost
 * This method takes care of checking the no of parameters , whether the user can update a link , 
 * whether a node is the neighbor of the current node. 
 * If the update is successful the link cost is updated and this information is sent to the neighbors in the next periodic update 
 * or when the user issues the command "Step"
 * 
 */
void update()
{
    // check if the update command is correct
    // check if the update is for the server ie display a message if the command says update 2 3 7
    
    
    if(tokenize_count == 4 && serverFlag)
    {
        char tempId[1];
        bool isNeighbor = false;
        int n;
        
        n = sprintf (tempId,"%d",hostId);
        /*
         * This is to check if we are updating in the correct server
         */
        if(strncmp(tokens[1], tempId,1) == 0)
        {
            for(currentNeighborsItr = currentNeighbors.begin();currentNeighborsItr != currentNeighbors.end();currentNeighborsItr++)
            {
                if(currentNeighborsItr->first == atoi(tokens[2]))
                {
                    isNeighbor = true;
                    break;
                }
            }
            /*
             * This is to check if the node is the current neighbor
             */
            if(isNeighbor)
            {
            // update the local structure eg : - update 1 2 8 in server 1 
                if(strcmp(tokens[3],"inf") == 0 || strcmp(tokens[3],"INF") == 0)
                {
                        node[atoi(tokens[2])].cost = INF;
                        routingTable[atoi(tokens[1])][atoi(tokens[2])] = INF;
                        neighborsToCostMapItr = neighborsToCostMap.find(atoi(tokens[2]));
                        neighborsToCostMapItr->second = INF;
                        
                        
                }
                else
                {
                        node[atoi(tokens[2])].cost = atoi(tokens[3]);
                        routingTable[atoi(tokens[1])][atoi(tokens[2])] = atoi(tokens[3]);
                        
                        neighborsToCostMapItr = neighborsToCostMap.find(atoi(tokens[2]));
                        neighborsToCostMapItr->second = atoi(tokens[3]);
                }
           
                DistanceVector();
                cout << "40 <Update> SUCCESS";
		fflush(stdout);
                return;
            }
            else
            {
                cout << "40 <Update> <Not a neighbor so cannot update the cost>" << endl;
                fflush(stdout);
                return;
            }
            
            
        }
        else
        {
            cout << "40 <Update> <You are updating distance at the wrong server>" << endl;
		fflush(stdout);
                return;
        }
        
    }
    else
    {
        cout << " 40 <Update> <The update command is not correct or server command is not executed>" << endl;
        fflush(stdout);
	 return;
    }
    
}

/**
 * This the function which is called when the user wishes to send the update message to the neighbors.
 * 
 */
void step()
{
    if(serverFlag && (tokenize_count == 1))
    {
        createUpdateMessage();
        sendUpdateMessageToNeighbors();
        cout << "40 <Step> SUCCESS";
    }
    else
    {
        cout << "40 <Step> <The update command is not correct or server command is not executed\n>";
        fflush(stdout);
	return;
    }
}
/*
 * This is the function which is called when the user wishes to know the number of
 * packets the server received since the last invocation 
 */
void packets()
{
    if(serverFlag && (tokenize_count == 1))
    {
        cout << "40 <Packets> SUCCESS" << endl <<"The number of Update Packets received since the last invocation is " << noOfUpdatePackets << endl;
        noOfUpdatePackets = 0;
        return;
    }
    else
    {
        cout << "40 <Packets> <The update command is not correct or server command is not executed\n>";
        fflush(stdout);
	return;
        
    }
}

/**
 * This is the function which is called when the user issues the command to display the current
 * Nodes routing server.
 * The output will be show in the following order 
 * <destination-server-ID> <next-hop-server-ID> <cost-of-path>
 */
void display()
{
    if(tokenize_count == 1 && serverFlag)
    {
        cout << "<Host-server>" << "\t<Destination-server>" << "\t<next-hop-server>" << "\t\t<cost>" << endl;
        for(int i =1 ;i <= numberOfServers;i++)
        {
            if(node[i].cost == 0)
            {
                cout << "\t" << hostId << "\t\t\t" <<node[i].id << "\t\t\t" << node[i].id << "\t\t" << node[i].cost << endl;
            }
            else if(node[i].cost == INF)
            {
                cout << "\t" <<  hostId << "\t\t\t" << node[i].id << "\t\t\t" << " " << "\t\t" << "INF" << endl;
            }
            else
            {
                cout << "\t" <<  hostId << "\t\t\t" << node[i].id << "\t\t\t" << node[i].nextHopId << "\t\t" << node[i].cost << endl;
            }
        }
        cout << "40 <display> SUCCESS" << endl;
    }
    else
    {
        cout << "40 <display> <Command issued is not correct or the server is not started>" << endl;
    }
}


 /*
  * This is the function which is used to disable a neighbor.
  * On disable I add the Id into a disable map and I make sure that when I receive DV from this server I reject those
  * I remove the Id mentioned in the  currentNeighbors neighborsToCostMap maps and I set the cost to Inf in my local structures
  */   
 
    
void disable()
{
   
   
        map <int,int>::iterator tempCurrentNeighborsItr;
        if(tokenize_count == 2 && serverFlag)
        {
            /*
             * Getting the positions from the two maps
             */
                tempCurrentNeighborsItr = currentNeighbors.find(atoi(tokens[1]));
                neighborsToCostMapItr = neighborsToCostMap.find(atoi(tokens[1]));
                if(tempCurrentNeighborsItr == currentNeighbors.end() || neighborsToCostMapItr == neighborsToCostMap.end())
                {
                        cout << "40 <Disable><Disabled failed since the node you entered is not the current node's neighbor>";
                        fflush(stdout);
                        return;
                }
                else
                {
                        
                        
                        int tempID = tempCurrentNeighborsItr->first;
                        node[tempID].cost = INF;
                        currentNeighbors.erase(tempCurrentNeighborsItr);
                        neighborsToCostMap.erase(neighborsToCostMapItr);
                        routingTable[hostId][tempID] = INF;
                        DistanceVector();
                        disableLinkMap.insert(pair<int,int>(atoi(tokens[1]),1));
                        cout << "40 <Disable> SUCCESS" << endl;
                        fflush(stdout);
                        return;
                        
                }
        
        }
        else
        {
                cout << "40 <Disable> <Disable command not executed in a proper way>" << endl;
                fflush(stdout);
                return;
        }
        
    }

/**
 * This is used to crash the server . The server crashed will stop listening to 
 * the incoming DV and does not perform any operation
 */
void crash ()
{
   
    if(tokenize_count == 1 && serverFlag)
    {
        serverFlag = false;
        map<int,int>::iterator neighborMapItr;
        neighborMapItr = currentNeighbors.begin();
        currentNeighbors.erase(neighborMapItr,currentNeighbors.end());
        
        FD_CLR(serverListner,&readfds);
        serverListner = 0;
        cout << "40<Crash>SUCCESS" << endl;
    }
    else
    {
        cout << "40<Crash><Command not issued properly or the server is not started>" << endl;
        fflush(stdout);
        return;
    }
}


/**
 * This is the function which determines which function to call based on the 
 * input give by the user
 * @param cmdline
 * @return 
 * 
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
 *  This is the function which is called to initialize the local structures on server start up
 */
void initialize()
{
    for(int i = 1;i < 6;i++)
    {
        node[i].cost = INF;
        node[i].id = INF;
        strcpy(node[i].ip ," ");
        node[i].nextHopId = INF;
        node[i].portNo = INF;
    }
    
    
    
}

/*
 * This is used to create a 2 dimensional array 
 * This is used as the routing table. DV are stored in this array and updated on 
 * running the Bellman Ford algorithm
 */
int **populateArray(int nos)
{
    int i;
      int **Array2;
      nos++;
      /*  allocate array of pointers  */
      Array2 = (int **)malloc( nos*sizeof(int*));

      if(Array2==NULL) {
        printf("\nError allocating memory\n");
        exit(1);
      }
      /*  allocate each row  */
      for(i = 0; i < nos; i++) {
        Array2[i] = (int *)malloc( nos*sizeof(int));
      }
      if(Array2[i-1]==NULL) {
        printf("\nError allocating memory\n");
        exit(1);
      }


      return Array2;
}

/**
 * Intializing the routing table
 */
void initializeArray()
{
    for(int i = 0;i <= numberOfServers;i++)
    {
        
        for(int j = 0 ; j <= numberOfServers;j++)
        {
            if(i == j)
            {
                routingTable[i][i] = 0;
            }
            else
            {
                 routingTable[i][j] = INF;
            }
        }
    }
        
}
/**
 * This is the function which is used to read the init file on issuing the server command
 * This method creates the local structures which holds the information like the <id><cost><ip><portNo> etc
 * @param initFile
 * @return 
 */
int initialize(char initFile[256])
{
    ifstream initializationFile;
    
    string STRING;
    
    initializationFile.open(initFile, ifstream::in);
    
    if(initializationFile.fail())
    {
        cout << "File not found" << endl;
        fflush(stdout);
        return 1;
    }
    else
    {
        
	getline(initializationFile,STRING); // Saves the line in STRING.
        numberOfServers = atoi(STRING.c_str());
	//cout << "Number of servers"<< numberOfServers << endl; // Prints our STRING.
        
        getline(initializationFile,STRING); // Saves the line in STRING.
        numberOfNeighbors = atoi(STRING.c_str());
	//cout << "Number of neighbors"<< numberOfNeighbors << endl;
        
        /*
         *  Here I am creating a two dimensional array for maintaining the DV of itself and other neighbors
         *  Then init it to INF
         */
         
       routingTable = populateArray(numberOfServers);
       initializeArray();
        /**
         *  Populate structures
         *  Extract the neighbors (2 field) and store in an array
         * 
         */
        char delims[] = " ";
        for(int i = 0;i < numberOfServers;i++)     
        {
            /**
             * I will get a line from the file .Parse it based on a space and populate the data structures
             * 
             */
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
		
              // cout << "\nnode[id].ip-->" << node[id].ip << "\t" << strlen(node[id].ip);
		//cout << "\nHostIP--->" << hostIP ;//<< "\t" << strlen(hostIP);	
	
               /**
                * This is to make sure that the cost to itself is made zero
                *  
                */
		if(strncmp(node[id].ip,hostIP,16)==0)
              {
		  //cout << "Inside the if condition\n"; 
                  node[id].cost = 0;
                  hostId = id;
                  hostPort = atoi(initLine[2].c_str()); 
                //  cout << "cost-"<< node[id].cost << endl;
              }
              node[id].portNo = atoi(initLine[2].c_str());
      
        }
        
        /*
         * Getting the neighbors information
         */
        for(int i = 0;i < numberOfNeighbors;i++)
        {
            /**
             * I will get a line from the file .Parse it based on a space and populate the data structures
             * 
             */
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
              /*
               * Inserting into the current neighbors map with count = 3 . This is for handling timeout
               */
              currentNeighbors.insert(pair<int,int>(id,3));
              node[id].cost = atoi(initLine[2].c_str());
              /*
               * Initially the next hop for a neighbor is the current node. It may change after applying DV
               */
              node[id].nextHopId = atoi(initLine[1].c_str());
        }    
        
        /*
         * Here I populate the neighbors to cost map
         */
                initNeighborsToCostMap();
        
        /*
         * Initializing the timer for the value entered by the user
         */
              tv.tv_sec = interval;
              tv.tv_usec = 0;
              return 0;
}

}

/**
 *    UPDATING THE ROUTING TABLE WHEN WE READ DATA FROM THE FILE
 * 
 */
void updateRoutingTable()
{
   
    for(int i = 1;i <= numberOfServers;i++)
    {
        routingTable[hostId][i] = node[i].cost;
        
    }
    
}

void initNeighborsToCostMap()
{
    
    for(int i = 1;i <= numberOfServers;i++)
    {
        if(hostId == i)
        {
            continue;
        }
        else
        {
                neighborsToCostMap.insert(pair<int,int>(node[i].id,node[i].cost));
        }
    }

}

/*
 * This is the function which is called from the main function. This starts 
 * the UDP server and it loops continuously over th stdin , UDP-main-server, 
 * 
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

	//cout << "\nUDPServer Waiting for client on port" << hostPort;
        fflush(stdout);
        
        serverListner = sock;

}

/*
 * This function adds new node into the currentNeighbors neighborsToCostMap
 */
void addNewNode(int id)
{
    currentNeighbors.insert(pair<int,int>(id,3));
    int i;
    for( i = 1;i <= numberOfServers;i++)
    {
        if(receivingStructure[i].id == hostId)
        {
            break;
        }
    }
    neighborsToCostMap.insert(pair<int,int>(id,receivingStructure[i].cost));
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
    int tempId = 0;
     
    memcpy(&numberofServersFollowing,recv_data+posToRead,sizeof(short int) );
    posToRead+=sizeof(short int);
    memcpy(&receivedFromServerPortNo,recv_data+posToRead,sizeof(short int) );
    posToRead+=sizeof(short int);
   cout << "Received data from " << receivedFromServerPortNo << endl;
    memcpy(&(temp_in.s_addr),recv_data+posToRead,sizeof(unsigned int));
    posToRead+=sizeof(unsigned int);  
    tempIP = inet_ntoa(temp_in);
    strcpy(receivedIP,tempIP);        
    
    /*
     This is to update the count field in the map to 3 . Index of the map has to be searched from the structure with the incoming IP. 
     */
    
    for(int i = 1 ;i <= numberOfServers;i++)
    {
        if(strcmp(node[i].ip,receivedIP) == 0)
        {
            tempId = node[i].id;
            break;
        }
    }
    
    
    map <int,int>::iterator tempCurrentNeighborsItr;
    
    tempCurrentNeighborsItr = currentNeighbors.find(tempId);
    disableLinkMapItr = disableLinkMap.find(tempId);
    /*
     * If the tempCurrentNeighborItr returns a value other than map::end()
     * Then set its count field to 3
     * else
     * this is a new node and it has to be handled
     */
    if(disableLinkMapItr == disableLinkMap.end())
    {
        if(tempCurrentNeighborsItr != currentNeighbors.end())
        {
                tempCurrentNeighborsItr->second = 3;
        }
        else
        {
                addNewNode(tempId);
        }
    }
    
    
    
    
    for(int i = 1;i <= numberofServersFollowing;i++)
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

}

          
/*
 * Dx(Y) = min {(c(x,y) + Dy(Y)),c(x,z)+Dz(Y)}
 * This is the method which executes the Bellman Ford algorithm  mention above and it updates the structures accordingly
 */
void DistanceVector()
{
    
    for(int i = 1;i <= numberOfServers;i++)
    {
        short int min = INF;
       
        map<int,int>::iterator it;
        
        if(i == hostId)
        {
             continue;
        }
        for(it = neighborsToCostMap.begin();it != neighborsToCostMap.end();it++)
        {
            
            if(min > (it->second + routingTable[it->first][i]))
            {
                min = (it->second + routingTable[it->first][i]);
                node[i].nextHopId = it->first;
                //sendFlag = true;
            }
        }
         
    
        routingTable[hostId][i] = min;
        node[i].cost = min;
    }  

}

/*
 * This function updates the routing table when a DV is received at a node
 * Based on the id of the node the Routing Table gets updated
 */
void updateRoutingTableWithIncomingNeighborsDV()
{
    int tempid;
   
    for(int i = 1;i <= numberOfServers; i++)
    {
      
        if(strcmp(node[i].ip,receivedIP) == 0)
        {
            tempid = node[i].id;
            break;
        }
        
        
    }
   
    for(int j = 1;j <= numberOfServers;j++)
    {
        routingTable[tempid][j] = receivingStructure[j].cost;
    }
    
    DistanceVector();
    
}

/*
 * 
 */
void check_on_stdin()
{
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
	

        if((select(n,&readfds,NULL, NULL, &tv)) == -1)
	    {
	      perror ("select");
	      exit (1);
	    }
	  if (FD_ISSET(0, &readfds))  // This handles the case when there is input from the command line
	    {
	   //   check=1;
	      fgets(cmdline, 256, stdin);
	      if (cmdline[strlen (cmdline) - 1] != '\0')
	      	 cmdline[strlen (cmdline) - 1] = '\0';
	        strcpy(commandline_copy,cmdline);
                count1++;
		docmd(cmdline);
	    }
          else if(FD_ISSET(serverListner,&readfds))  // This handles the case when there is input on the server
          {

             // cout << "Receiving some information on the server";
              int bytes_read =0;
              
              struct sockaddr_in client_addr;
              bytes_read = recvfrom(serverListner,recv_data,1024,0,(struct sockaddr *)&client_addr, &addr_len);
              recv_data[bytes_read] = '\0';
              noOfUpdatePackets++;
              createReceiveStructures();
              updateRoutingTableWithIncomingNeighborsDV();
              //DistanceVector();  
             // updateLocalStructures();
          }
          else if(serverFlag)  // This is on timeout
          {
              /*Decrement the value in a map if value is equal to zero then set the cost to inf*/
               
              
              
              for(currentNeighborsItr = currentNeighbors.begin();currentNeighborsItr != currentNeighbors.end(); currentNeighborsItr++)
                     {
                                if(currentNeighborsItr->second == 0)
                                {
                                    continue;
                                }
                                else
                                {
                                    currentNeighborsItr->second -= 1;
                                    if(currentNeighborsItr->second == 0)
                                    {
                                        node[currentNeighborsItr->first].cost = INF;
                                        
                                        neighborsToCostMapItr = neighborsToCostMap.find(currentNeighborsItr->first);
                                        neighborsToCostMapItr->second = INF;
                                        routingTable[hostId][currentNeighborsItr->first] = INF;
                                        DistanceVector();   
                                    }
                                }
                     }
              createUpdateMessage();
              sendUpdateMessageToNeighbors();
              
              tv.tv_sec = interval;
              tv.tv_usec = 0;
        
          }
	
}

char *routingUpdateInterval = NULL;

/*
 * Executed on issuing the server command.
 * This inturn calls other methods which does initialization , creating structures, sets the routing update interval
 */
void server()
{
    serverFlag = true;
    if(tokenize_count == 5 && (strcmp(tokens[1],"-t") == 0) && (strcmp(tokens[3],"-i") == 0))
    {
        int flag = 1;
        find_ip_of_current_Server();
        
        routingUpdateInterval = tokens[4];
        interval = atoi(routingUpdateInterval);
    
        /*
         * Reading from a file then populate the local structures like routing table,local structures,
         * neighbors to cost map,neighbors list
         */
        flag = initialize(tokens[2]);
        updateRoutingTable();
        
        if(!flag)
        {
        
                start_udp_server();
        }
        else
        {
            cout << "40 <Server> <File not found>" << endl ;
            fflush(stdout);
            return;
        }
    
    }
    else
    {
        cout << "40 <Server> <Invalid number of arguments or invalid options given\n>";
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
        ifstream readFile;
        string STRING;
        
        host=getenv("HOST");
        char hostIP1[INET6_ADDRSTRLEN];
	system("rm temphost temphost1");
	system("touch temphost temphost1");
	sprintf(cmd,"host %s>temphost",host);
	system(cmd);
	sprintf(cmd,"cut -d \" \" -f4 temphost>temphost1");
	system(cmd);
       
        readFile.open("temphost1", ifstream::in);
    
    
    
        
	getline(readFile,STRING); // Saves the line in STRING.
        strcpy(hostIP,STRING.c_str());
       
}


int main(int argc, char** argv)
{
    initialize();
    start_shell ();
    return 0;
}
