/**
 * Abhishek Srinath
 * 17th April 9:00pm
 */

#include <iostream>
#include <string>
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



using namespace  std;

#define MAXARG 7

#define MAXCMD 7



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

void tokenize(char *);
int docmd(char *);





struct NODE{
    int id;
    int portNo;
    char ip[INET6_ADDRSTRLEN];
    int cost;
    int nextHopId;
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
vector<int> currentNeighbors;
vector<int>::iterator currentNeighbors_itr;
char hostIP[INET6_ADDRSTRLEN];
short int hostPort;
short int hostId;
char *tokens[MAXARG];
int tokenize_count;
char commandline_copy[256];
int interval;
int numberOfServers = 0;
int numberOfNeighbors = 0;
bool serverFlag = false;
int serverListner;
int count1= 0;



void update()
{
    // check if the update command is correct
    // check if the update is for the server ie display a message if the command says update 2 3 7
    // create a string according to the msg format
    // send the string to the neighbors
    // update the local structure
    // 
    if(tokenize_count == 3)
    {
        char tempId[10];
        char noOfFields[2];
        char hostPort[2];
        
        int n;
        n = sprintf (tempId,"%d",hostId);
        if(strcmp(tokens[1], tempId) == 0)
        {
            cout << "Update success";
            
        }
        else
        {
            cout << "You are updating distance at the wrong server";
        }
        
    }
    else
    {
        cout << "The update command is not correct\n";
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
                        /*for(i=0;i<3;i++)
                        {
                            printf("%s\n",tokens[i]);
                        }*/
                        
                      
}


/**
 * 
 */
void initialize()
{
    for(int i = 1;i < 6;i++)
    {
        node[i].cost = 999999;
        node[i].id = 999999;
        strcpy(node[i].ip ," ");
        node[i].nextHopId = 999999;
        node[i].portNo = 999999;
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
         * Extract the neighbors (2 field) and store in an array
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
              
//              for(;initLine_itr != initLine.end();initLine_itr++)
//              {
//                  cout << *initLine_itr << endl;
//              }
//              cout << endl;
              
              initLine_itr = initLine.begin();
              
              id=atoi((*initLine_itr).c_str());
              
              node[id].id = id;
              strcpy(node[id].ip,initLine[1].c_str());
		
               cout << "\nnode[id].ip-->" << node[id].ip << "\t" << strlen(node[id].ip);
		cout << "\nHostIP--->" << hostIP << "\t" << strlen(hostIP);	
	//	cout << ""
		if(strncmp(node[id].ip,hostIP,16)==0)
              {
		cout << "Inside the if condition"; 
                  node[id].cost = 0;
                  hostId = id;
                  hostPort = atoi(initLine[2].c_str());
                  
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
        
        for(int i=1;i<6;i++)
        {
            cout << node[i].id << "\t" << node[i].ip << "\t" << node[i].portNo << "\t"<< node[i].cost << endl;
        }
	initializationFile.close();
        cout << "\nhostIP" << hostIP;
        cout << "\nhostPort" << hostPort;
        cout << "\nCurrent neighbors" << endl;
        for(currentNeighbors_itr = currentNeighbors.begin();currentNeighbors_itr != currentNeighbors.end();currentNeighbors_itr++)
        {
            cout << *currentNeighbors_itr << "\t" ;
            fflush(stdin);
        }
    
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
		//check_on_udp_main_server();
		
		
		
	}
}


void start_udp_server()
{
   int sock;
       // int addr_len;
        
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

        //addr_len = sizeof(struct sockaddr);
//		
	cout << "\nUDPServer Waiting for client on port" << hostPort;
        fflush(stdout);
        
        serverListner = sock;

}

//int count= 0;

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
              cout << "Receiving some information on the server";
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
    
    char option; 
    char *topologyFileName = NULL;
    
    char fileName[512];
   
    
//    while ((option = getopt(argc, argv, "t::i::")) != -1)
//    {
//        switch(option)
//        {
//            case 'i' : routingUpdateInterval = optarg;
//            break;
//            case 't' : topologyFileName = optarg;
//            break;
//        }
//    }
    
    
   // strcpy(fileName,topologyFileName);
    
    initialize();
   // strcpy(hostIP,"128.205.36.8");
    start_shell ();
    
   // find_ip_of_current_Server();
    

    
   // initialize(fileName);
    
	
	
        
        return 0;
}
