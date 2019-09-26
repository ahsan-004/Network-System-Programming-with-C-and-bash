/////////////////////////////////////////////////////////////////////////////
//Program: Hybrid Instant Messaging Client With BBS support                //
//Prepared By: Ahsan Ali                                                   //
//Reg#: SP-10-004                                                          //
//email: ahsan_ali_004@hotmail.com                                         //
/////////////////////////////////////////////////////////////////////////////
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<error.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#define MAX_BUFFER 1024
#define Listen_IP "127.0.0.1"
int PORT;
//check validity of input
int validity(char []);
//show help file
int help(void);
//establish connection to server
int establish_connection(struct sockaddr_in *,int);
//Send data to the server
int send_data(int,struct sockaddr_in *,char []);
//check valifilty of input arguments
int check_input(char []);
//Process the input
int process_input(int,char [], struct sockaddr_in *);
//Recieve data from server
int recieve_data(int);
//Create P2P connection
int create_session(char [],int,int);

int main ()
{
	struct sockaddr_in server,client,information;
	char input[MAX_BUFFER],*overhead,login_name[30],temp_buffer[MAX_BUFFER],dc[1024],out[20],*configuration;
	char current_room[20],udp_buffer[MAX_BUFFER];
	int ret_val,conn_socket,listen_socket,maxfd,LISTEN_PORT,dc_socket,maxfd2,b_port,udp_socket;
	int login_flag=0,room_flag=0,dc_flag=0,room_admin_flag=0,server_admin_flag=0,connected_flag=0;
	socklen_t length;
	fd_set rset,master;
	FILE *config;
	config=fopen("client.config","r+");
	if(config==NULL)
	{
		perror("\n Config File");
		exit(1);
	}
	fgets(input,1024,config);
	configuration=strtok(input,"=");
	PORT=atoi(strtok(NULL,"\n"));
	fgets(input,1024,config);
	configuration=strtok(input,"=");
	b_port=atoi(strtok(NULL,"\n"));
	fgets(input,1024,config);
	configuration=strtok(input,"=");
	LISTEN_PORT=atoi(strtok(NULL,"\n"));
	
	//Filling socket Adress Structure
	server.sin_family=AF_INET;
	server.sin_port=htons(PORT);
	server.sin_addr.s_addr=inet_addr(Listen_IP);
	length=sizeof(server);
	//getting listening socket
	information.sin_family=AF_INET;
	information.sin_port=htons(b_port);
	information.sin_addr.s_addr=inet_addr("127.0.0.1");
	udp_socket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	ret_val=bind(udp_socket,(struct sockaddr*)&information,sizeof(information));
	if(ret_val<0)
	{
		perror("\nUDP bind");
	}
	client.sin_family=AF_INET;
	client.sin_port=htons(LISTEN_PORT);
	client.sin_addr.s_addr=inet_addr("127.0.0.1");
	listen_socket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(listen_socket<0)
	{
		perror("\nSocket Listen");
	}
	ret_val=bind(listen_socket,(struct sockaddr*)&client,sizeof(client));
	if(ret_val<0)
	{
		perror("\nHIMC>>bind Listen");
		
	}
	listen(listen_socket,10);
	
	if(listen_socket<0)
	{
		fprintf(stderr,"\nHIMC>>Listen Socket Error");
		
	}
	
	//establish connection to the sever
	conn_socket=establish_connection((struct sockaddr_in*)&server,length);
 	if(conn_socket<0)
 	{
 		fprintf(stderr,"\nTry again Later");
 		exit(1);
 	}
	connected_flag=1;
	FD_ZERO(&rset);
	FD_SET(0,&rset);
	FD_SET(listen_socket,&rset);
	FD_SET(conn_socket,&rset);
	FD_SET(udp_socket,&rset);

	//Initial Messages on the screen
	fprintf(stderr,"\nMade By: Ahsan Ali");
	fprintf(stderr,"\n-------------------");
	fprintf(stderr,"\nReg# SP-10-004");
	fprintf(stderr,"\n---------------------");
	fprintf(stderr,"\n\nHIMC(Hybrid Instant Messaging Client)");
	fprintf(stderr,"\nType '/help' for Getting Started\n");
	
	if(listen_socket>conn_socket)
	{
		maxfd=listen_socket+1;
	}
	else
	{
		maxfd=conn_socket+1;
	}
	if(udp_socket>=maxfd)
	{
		maxfd=udp_socket+1;
	}
	ret_val=0;
	master=rset;
	strcpy(out,"HIMC>>");
	for(;;)
	{
		rset=master;
		fprintf(stderr,"\n%s",out);
		select(maxfd,&rset,NULL,NULL,NULL);
		if(FD_ISSET(0,&rset))                   //if Standard Input ready
		{
// 			fprintf(stderr,"login flag %d   server admin falg %d room_flag %d",login_flag,server_admin_flag,room_flag);
			fgets(input,1024,stdin);
			if(strlen(input)<2)
			{
				continue;
			}
			else
			if(input[0]!='/')
			{
				fprintf(stderr,"\nHIMC>>Invalid format for input");
				fprintf(stderr,"\nUse '/' as initial charachter");
				continue;
			}
			else
			if((strstr(input,"/login")!=NULL)&&login_flag==1)       //if user already login 
			{
				fprintf(stderr,"\nHIMC(Error)>>You are already Login first logout and then Login");
				continue;
			}
			else
			if((strstr(input,"/register")!=NULL)&&login_flag==1)     //if user already login 
			{
				fprintf(stderr,"\nHIMC(Error)>>You can not register yourself at this point");
				continue;
			}
			else
			if(strstr(input,"/clear")!=NULL)
			{
				system("clear all");
				continue;
			}
			else
			if((strstr(input,"/channel")!=NULL||strstr(input,"/quitchannel")!=NULL||strstr(input,"/dc")!=NULL||strstr(input,"/dcchat")!=NULL||strstr(input,"/kick")!=NULL||strstr(input,"/logout")!=NULL||strstr(input,"/radmin")!=NULL||strstr(input,"/aadmin")!=NULL||strstr(input,"/banip")!=NULL||strstr(input,"/msg ")!=NULL||strstr(input,"/list")!=NULL||strstr(input,"/bbs")!=NULL||strstr(input,"/rooms")!=NULL||strstr(input,"/banuser")!=NULL||strstr(input,"/msgall")!=NULL)&&login_flag==0)   //if usser not login 
			{
				if(connected_flag==0)
				{
					fprintf(stderr,"\nHIMc>>You are Not Connected To server");
				}
				else
					fprintf(stderr,"\nHIMC(Error)>>You are not login first login and then issue this command");
				continue;
			}
			else
			if(strstr(input,"/channel")!=NULL&&room_flag==1)
			{
				fprintf(stderr,"\nHIMC(Error)>>You are already in a room");
				continue;
			}
			else
			if(strstr(input,"/quitchannel")!=NULL&&room_flag==0)
			{
				fprintf(stderr,"\nHIMC[Error]>>You are not in any room");
				continue;
			}
			else
			if(strstr(input,"/dcchat")!=NULL&&dc_flag==0)
			{
				fprintf(stderr,"\nHIMC(Error)>>You Are not connected to any peer");
				continue;
			}
			else
			if(strstr(input,"/dc ")!=NULL&&dc_flag==1)
			{
				fprintf(stderr,"\nHIMC(Error)>>You are already connected to one peer");
				continue;		
			}
			else
			if(strstr(input,"/kick")!=NULL&&room_admin_flag==0)
			{
				fprintf(stderr,"\nHIMC(Errror)>>you are not owner of room");
				continue;
			}
			else
			if(strstr(input,"/qdirect")!=NULL&&dc_flag==1)
			{
				FD_ZERO(&rset);
				FD_SET(0,&rset);
				FD_SET(listen_socket,&rset);
				FD_SET(conn_socket,&rset);
				FD_SET(udp_socket,&rset);
				maxfd=maxfd2;
				close(dc_socket);
				dc_flag=0;
			}
			else
			if((strstr(input,"/radmin")!=NULL||strstr(input,"/aadmin")!=NULL||strstr(input,"/banip")!=NULL||strstr(input,"/bbs send all")!=NULL||strstr(input,"/banuser")!=NULL||strstr(input,"/reboot")!=NULL)&&server_admin_flag==0)
			{
				fprintf(stderr,"\nHIMC(Error)>>You are not admin of server");
				continue;
			}
			else
			if(strstr(input,"/msgall")!=NULL&&room_flag==0)
			{
				fprintf(stderr,"\nYou are not in any room");
				continue;
			}
			strcpy(temp_buffer,input);
			if(strstr(input,"/dcchat ")!=NULL&&dc_flag==1)
			{
				overhead=strtok(temp_buffer," ");
				overhead=strtok(NULL,"\0");
				sprintf(input,"\n%s Says:  %s",login_name,overhead);
				ret_val=send_data(dc_socket,NULL,input);
				if(ret_val<0)
				{
					fprintf(stderr,"\nHIMC(Error)>>Error sending Data");
				}
				continue;
			}
			else
			if(strstr(input,"/msgall")!=NULL&&room_flag==1)
			{
				sprintf(input,"%s %s %s",current_room,login_name,temp_buffer);
			}
			else
			if(strstr(input,"/msg")!=NULL)
			{
				sprintf(input,"%s %s",login_name,temp_buffer);
			}
			else
			if (strstr(input,"/channel ")!=NULL&&room_flag==0)
			{
				////fgets return /n at the end so to omit /n using snprintf
				snprintf(input,strlen(input),"%s",temp_buffer);  
				strcpy(temp_buffer,input);        //swapping
				sprintf(input,"%s %s",temp_buffer,login_name);
			}
			else
			if(strstr(input,"/kick")!=NULL)
			{
				sprintf(input,"%s %s %s",login_name,current_room,temp_buffer);
			}
			else
			if(strstr(input,"/quitchannel")!=NULL)
			{
				snprintf(temp_buffer,strlen(input),"%s",input);
				sprintf(input,"%s %s %s ",login_name,current_room,temp_buffer);
			}
			else
			if(strstr(input,"/logout")!=NULL)
			{
				snprintf(temp_buffer,strlen(input),"%s",input);
				if(room_flag==0)
				{	
					sprintf(input,"%s XXXXXX %s %s",login_name,temp_buffer,login_name);
				}
				else
				{
					sprintf(input,"%s %s %s %s",login_name,current_room,temp_buffer,login_name);
				}
			}
			else
			if(strstr(input,"/bbs")!=NULL)
			{
				sprintf(input,"%s %s",login_name,temp_buffer);
			}
			ret_val=process_input(conn_socket,input,(struct sockaddr_in*)&server);
		}
		else
		if(FD_ISSET(conn_socket,&rset))	
		{
			ret_val=recieve_data(conn_socket);
			if(ret_val<-13)
			{
				fprintf(stderr,"\nHIMC>>Error in Recieving Data");
			}
			else
			if(ret_val==0)
			{
				connected_flag=0;
				login_flag=0;
				room_flag=0;
				room_admin_flag=0;
				server_admin_flag=0;
				fprintf(stderr,"\nHIMC>>Connection Closed By Server");
				if(dc_flag==1)     //In case if user connected to any peer
				{
					FD_ZERO(&rset);
					FD_SET(dc_socket,&rset);
					FD_SET(0,&rset);
					maxfd=dc_socket+1;
					master=rset;
				}
				else        //exit client
				{
					exit(1);
				}
				
			}
			else
			if(ret_val==-2)        //Login Succesfull then send the listening IP and Port and UDP port
			{			
				fprintf(stderr,"\nHIMC[response]>>Succesfully Loged in");
				overhead=strtok(temp_buffer," ");
				overhead=strtok(NULL," ");
				strcpy(login_name,overhead);
				sprintf(dc,"$info$ %s %s %d\n",login_name,Listen_IP,LISTEN_PORT);
				ret_val=send_data(conn_socket,(struct sockaddr_in *)&server,dc);
				if(ret_val<0)
				{
					fprintf(stderr,"\nHIMC(Error)>>Sending Command");
				}
				sprintf(out,"%s>>",login_name);	
				sprintf(dc,"udp %s %s %d",login_name,Listen_IP,b_port);
				ret_val=send_data(conn_socket,(struct sockaddr_in *)&server,dc);
				if(ret_val<0)
				{
					fprintf(stderr,"\nHIMC(Error)>>Sending Command");
				}
				login_flag=1;
			}
			else
			if(ret_val==-3)        //if channel created succesfully 
			{
				
				overhead=strtok(input," ");
				overhead=strtok(NULL," ");
				strcpy(current_room,overhead);
				sprintf(out,"%s[%s]>>",login_name,current_room);
				room_flag=1;              //if room created succesfully
				room_admin_flag=1;        //if admin of room
			}
 			else
			if(ret_val==-4)        //if moved to already created channel
			{
				overhead=strtok(input," ");
				overhead=strtok(NULL," ");
				strcpy(current_room,overhead);
				sprintf(out,"%s[%s]>>",login_name,current_room);
				room_flag=1;
				room_admin_flag=0;
				room_flag=1;
			}
			else
			if(ret_val==-5)    //if quit from room
			{
				room_admin_flag=0;
				room_flag=0;
				sprintf(out,"%s>>",login_name);
			}
			else
			if(ret_val==-6)        //if admin of server
			{
				fprintf(stderr,"\nHIMC[Information]>>You Are Admin of Server");
				server_admin_flag=1;
			}
			else
			if(ret_val==-7)
			{
				room_admin_flag=0;
				room_flag=0;
				login_flag=0;
				server_admin_flag=0;
				strcpy(out,"HIMC>>");
			}
			else
 			if(ret_val>0)   //if session created with peer then remove listening port and add connected port to read set 
 			{
 				dc_socket=ret_val;
				FD_ZERO(&rset);
				FD_SET(0,&rset);
				FD_SET(conn_socket,&rset);
				FD_SET(dc_socket,&rset);
				FD_SET(udp_socket,&rset);
				if(maxfd<=dc_socket)
				{
					maxfd2=maxfd;
					maxfd=dc_socket+1;
				}
				master=rset;
				dc_flag=1;
			}		
 			
		}
		else
		if(FD_ISSET(listen_socket,&rset)) //is request arrives on listening socket
		{
 			dc_socket=create_session(NULL,0,listen_socket);	
			if(dc_socket<0)
 			{
 				fprintf(stderr,"\nHIMC>>Error in Creating Direct Chat");
 			}
			else
			if(dc_socket>0)
			{
				fprintf(stderr,"Session created");
				FD_ZERO(&rset);
				FD_SET(0,&rset);
				//FD_SET(listen_socket,&rset);
				FD_SET(conn_socket,&rset);
				FD_SET(dc_socket,&rset);
				FD_SET(udp_socket,&rset);
				if(maxfd<=dc_socket)
				{
					maxfd2=maxfd;
					maxfd=dc_socket+1;
				}
				master=rset;
				dc_flag=1;
			}
		}
		else
		if(dc_flag==1)       //if DC is enabled then also check connected socket in select 
		{
			if(FD_ISSET(dc_socket,&rset))
			{
				ret_val=recieve_data(dc_socket);
				if(ret_val==0)
				{
					FD_ZERO(&rset);
					FD_SET(0,&rset);
					FD_SET(listen_socket,&rset);
					FD_SET(conn_socket,&rset);
					dc_flag=0;
					maxfd=maxfd2;
					fprintf(stderr,"\nHIMC>>Connection Terminated from peer");
					master=rset;
				}
			}
		}
		else
		if(FD_ISSET(udp_socket,&rset))      //UDP socket to get messages
		{
			ret_val=recvfrom(udp_socket,udp_buffer,1024,0,NULL,NULL);
			if(ret_val<0)
			{
				fprintf(stderr,"\nEror in recieving data on Information Channel");
			}
			else
			if(strstr(udp_buffer,"You are Kicked")!=NULL)
			{
				fprintf(stderr,"\nHIMC[Information]>>You are kicked off from room");
				room_flag=0;
				sprintf(out,"%s>>",login_name);
			}
			else
			if(strstr(udp_buffer,"You are Admin")!=NULL)
			{
				fprintf(stderr,"\nHIMc[Information]>>You are Admin of Server");
				server_admin_flag=1;
			}
			else
			if(strstr(udp_buffer,"Your Admin Removed")!=NULL)
			{
				fprintf(stderr,"\nHIMC[Information]>>Your Admin Rights removed");
				server_admin_flag=0;
			}
			else
			if(strstr(udp_buffer,"Your IP banned")!=NULL)
			{
				if(login_flag==1)
				{
					sprintf(temp_buffer,"/logout");
					if(room_flag==0)
					{	
						sprintf(input,"%s XXXXXX %s %s",login_name,temp_buffer,login_name);
					}
					else
					{
						sprintf(input,"%s %s %s %s",login_name,current_room,temp_buffer,login_name);
					}
					ret_val=process_input(conn_socket,input,(struct sockaddr_in*)&server);
				}
				fprintf(stderr,"\nHIMC[Information]>>Your IP is Banned now onward");
				login_flag=0;
				
			}
			else
			if(strstr(udp_buffer,"Your Username Banned")!=NULL)
			{
				if(login_flag==1)
				{
					sprintf(temp_buffer,"/logout");
					if(room_flag==0)
					{	
						sprintf(input,"%s XXXXXX %s %s",login_name,temp_buffer,login_name);
					}
					else
					{
						sprintf(input,"%s %s %s %s",login_name,current_room,temp_buffer,login_name);
					}
					ret_val=process_input(conn_socket,input,(struct sockaddr_in*)&server);
				}
				fprintf(stderr,"\nHIMC[Information]>>Your Username is banned");
				login_flag=0;
			}
			else
			{
				udp_buffer[ret_val]='\0';
				fprintf(stderr,"\n%s",udp_buffer);
			}
		}					
	}
	return 1;
}
/////////////////////////////////////////////////////////////////////////////////
///// Recieve Data from server and return corresponding values                 //
/////////////////////////////////////////////////////////////////////////////////
int recieve_data(int conn_socket)
{
	int ret_val;
	char buffer[MAX_BUFFER+1];

	ret_val=recvfrom(conn_socket,buffer,MAX_BUFFER,0,NULL,NULL);
	buffer[ret_val]='\0';
	if(ret_val<0)
	{
		fprintf(stderr,"\nHIMC>>Error in Recieving");
		return -50;
	}
	else
	if(ret_val==0)
	{
		return 0;
	}
	buffer[ret_val]='\0';
	if(strstr(buffer,"dc_reply")!=NULL)
	{
		ret_val=create_session(buffer,1,0);   //direct chat request reply so creatng session
		if(ret_val<0)
		{
			fprintf(stderr,"\nSession not created");
		}
		else
			return ret_val;
	}
	//Gathering info from recieved message
	if(strstr(buffer,"Succesfully Login")!=NULL)       //if login correct 
	{
		return -2;
	}
	else
	if(strstr(buffer,"Channel Created")!=NULL)        //if channel creted succesfully 
	{
		fprintf(stderr,"\nHIMC[Information]>>You are Owner of this Room");
		return -3;
	}
	else
	if(strstr(buffer,"Channel Moved")!=NULL)        //if moved to already created channel
	{
		fprintf(stderr,"\nHIMC[Information]>>You are Moved to already created room");
		return -4;
	}
	else
	if(strstr(buffer,"Channel Quit Success")!=NULL)     //if channel quit success
	{
		return -5;
	}
	else
	if(strstr(buffer,"Admin user")!=NULL)        //if admin of server
	{
		return -6;
	}
	else
	if(strstr(buffer,"Already Login")!=NULL)
	{
		fprintf(stderr,"\nHIMC[response]>>username Already Loged in");
	}
	else
	if(strstr(buffer,"Username Banned")!=NULL)
	{
		fprintf(stderr,"\nHIMC[response]>>You are banned User");
	}
	else
	if(strstr(buffer,"User Not Found")!=NULL)
	{
		fprintf(stderr,"\nHIMC[response]>>Username or Password Incorrect");
	}
	else
	if(strstr(buffer,"registeration Success")!=NULL)
	{
		fprintf(stderr,"\nHIMC[response]>>Registration Success");
	}
	else
	if(strstr(buffer,"Logout Success")!=NULL)
	{
		fprintf(stderr,"\nHIMC[Response]>>You are Not Logged in");
		return -7;
	}
	else
	if(strstr(buffer,"Username Already Exist")!=NULL)
	{
		fprintf(stderr,"\nHIMC[response]>>Try another Username");
	}
	else
	{
		fprintf(stderr,"\n-------------------------------------------------");
		fprintf(stderr,"\n%s",buffer);
		fprintf(stderr,"\n-------------------------------------------------");
	}
	return -13;
}
//////////////////////////////////////////////////////////////////////////////////////
//// Create P2P session with peer                                                /////
//////////////////////////////////////////////////////////////////////////////////////
int create_session(char address[],int flag,int listen_socket)
{
 	char *overhead,*dc_ip,*dc_port;
 	int dc_socket,ret_val;
 	struct sockaddr_in dc_host;
	socklen_t len=sizeof(dc_host);
	if(flag==1)            //if user want to create session
	{
	 	overhead=strtok(address," ");
		overhead=strtok(NULL," ");
		dc_ip=strtok(NULL," ");
		dc_port=strtok(NULL," ");
		dc_socket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		if(dc_socket<0)
 		{
 			perror("\nDc Socket Creation");
  			return -1;
 		}
 		dc_host.sin_family=AF_INET;
 		dc_host.sin_port=htons(atoi(dc_port));
 		dc_host.sin_addr.s_addr=inet_addr(dc_ip);
 		ret_val=connect(dc_socket,(struct sockaddr*)&dc_host,sizeof(dc_host));
 		if(ret_val<0)
 		{
 			perror("dc connect:");
 			return -1;
 		}
 		return dc_socket;
	}	
	if(flag==0)            //if dc request arrives
	{
		dc_socket=accept(listen_socket,(struct sockaddr*)&dc_host,&len);
		if(dc_socket<0)
		{
			perror("\nDC accept:");
			return -1;
		}
		return dc_socket;
	}
	return -1;
}
////////////////////////////////////////////////////////////////////////////////////
///// Process input and if it is valid send it to the server else give some error //
////////////////////////////////////////////////////////////////////////////////////
int process_input(int conn_socket,char input[], struct sockaddr_in * server)
{	
	int ret_val;
	if((strcmp(input,"/help\n"))==0)
	{
		ret_val=help();
		return 1;
	}
	//regis	teration login and channel process
	if((strstr(input,"/register "))!=NULL||strstr(input,"/login ")!=NULL||strstr(input,"/channel ")!=NULL)
	{
		ret_val=check_input(input);
		if(ret_val!=2)
		{
			fprintf(stderr,"\nHIMC(Error)>>Invalid Arguments of input Type /help for proper format");
			return -1;
		}
		ret_val=send_data(conn_socket,(struct sockaddr_in *) server,input);
		if(ret_val<0)
		{
			fprintf(stderr,"\nHIMC(Error)>>Sending Command");
		}
		return 5;
	}
	
	else
	if(strstr(input,"/dc ")!=NULL||strstr(input,"/radmin ")!=NULL||strstr(input,"aadmin ")!=NULL||strstr(input,"/banip ")!=NULL||strstr(input,"/banuser ")!=NULL)
	{
		ret_val=check_input(input);
		if(ret_val!=1)
		{
			fprintf(stderr,"\nHIMC(Error)>>invalid arguments for dc");
			return -1;
		}
		ret_val=send_data(conn_socket,(struct sockaddr_in *) server,input);
		if(ret_val<0)
		{
			fprintf(stderr,"\nHIMC(Error)>>Sending Command");
			return -1;
		}
		
	}
	else
	if(strstr(input,"/list")!=NULL||strstr(input,"/rooms")!=NULL||strstr(input,"/reboot")!=NULL)
	{
		ret_val=check_input(input);
		if(ret_val>0)
		{
			fprintf(stderr,"\nHIMC(Error)>>Invalid arguments");
			return -1;
		}
		else 
		if(ret_val==0)
		{
			ret_val=send_data(conn_socket,(struct sockaddr_in*)server,input);
			if(ret_val<0)
			{
				fprintf(stderr,"\nHIMC(Error)>>Sending list");
				return -1;
			}
		}	
	}
	else
	if(strstr(input,"/kick ")||strstr(input,"/quitchannel ")!=NULL||strstr(input,"/logout ")!=NULL)
	{
		ret_val=check_input(input);
		if(ret_val!=3)
		{
			fprintf(stderr,"\nHIMC(Error)>>Invalid arguments");
			return -1;
		}
		ret_val=send_data(conn_socket,(struct sockaddr_in*)server,input);
		if(ret_val<0)
		{
			fprintf(stderr,"\nError Sending Input");
			return -1;
		}
		
	}
	
	else	
	if(strstr(input,"/msgall ")!=NULL)
	{
		ret_val=check_input(input);
		if(ret_val<3)
		{
			fprintf(stderr,"\nHIMC(Error)>>Invalid arguments for msg all");
			return -1;
		}
		ret_val=send_data(conn_socket,(struct sockaddr_in*)server,input);
		if(ret_val<0)
		{
			fprintf(stderr,"\nError sending command");
			return -1;
		}
	}
	else
	if(strstr(input,"/msg ")!=NULL)
	{
		ret_val=check_input(input);
		if(ret_val<3)
		{
			fprintf(stderr,"\nHIMC(Error)>>Invalid arguments");
			return -1;
		}
		ret_val=send_data(conn_socket,(struct sockaddr_in*)server,input);
		if(ret_val<0)
		{
			fprintf(stderr,"\nError sending command");
			return -1;
		}
	}
	else
	if(strstr(input,"/bbs send ")!=NULL)
	{
		ret_val=check_input(input);
		if(ret_val<4)
		{
			fprintf(stderr,"invlaid arguments");
			return -1;
		}
		ret_val=send_data(conn_socket,(struct sockaddr_in*)server,input);
		if(ret_val<0)
		{
			fprintf(stderr,"\nError sending admin commands");
			return -1;
		}
	}	
	else
	if(strstr(input,"/bbs read ")!=NULL)
	{
		ret_val=check_input(input);
		if(ret_val<3)
		{
			fprintf(stderr,"invlaid arguments");
			return -1;
		}
		ret_val=send_data(conn_socket,(struct sockaddr_in*)server,input);
		if(ret_val<0)
		{
			fprintf(stderr,"\nError sending admin commands");
			return -1;
		}
	}
	else
	if(strstr(input,"/bbs list")!=NULL)
	{
		ret_val=check_input(input);
		if(ret_val<2)
		{
			fprintf(stderr,"invlaid arguments");
			return -1;
		}
		ret_val=send_data(conn_socket,(struct sockaddr_in*)server,input);
		if(ret_val<0)
		{
			fprintf(stderr,"\nError sending admin commands");
			return -1;
		}
	}
	else
	if(strstr(input,"/bbs delete ")!=NULL)
	{
		ret_val=check_input(input);
		if(ret_val<3)
		{
			fprintf(stderr,"invlaid arguments");
			return -1;
		}
		ret_val=send_data(conn_socket,(struct sockaddr_in*)server,input);
		if(ret_val<0)
		{
			fprintf(stderr,"\nError sending admin commands");
			return -1;
		}
	}			
	else
	{
		fprintf(stderr,"\nHIMC(Error)>> Command Not Recognized");
		return -13;
	}
	return 1;
}
///////////////////////////////////////////////////////////////////////////////
//// Show help file to the user                                              //
///////////////////////////////////////////////////////////////////////////////
int help (void)
{
	FILE *help_file;
	char buffer [1024];
	help_file=fopen("help_file","r+");
	if(help_file==NULL)
	{
		perror("\nHIMC(Error)>>Error in Help File:");
		return -1;
	}
	while (fgets(buffer,1024,help_file)!=NULL)
	{
		fprintf(stderr,"%s",buffer);
	}
	return 1;
}
/////////////////////////////////////////////////////////////////////////////
//// Create session to the server                                          //
//// return Connected socket number                                        //
/////////////////////////////////////////////////////////////////////////////
int establish_connection(struct sockaddr_in * server,int length)
{
	int conn_socket;
	int ret_val;
	//Getting Socket
	conn_socket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(conn_socket<0)
	{
		perror("\nHIMC(Error)>>Socket Error");
		return -1;
	}
	ret_val=connect(conn_socket,(struct sockaddr*)server,length);
	if(ret_val<0)
	{
		perror("\nHIMC(Error)>>Connect:");
		return -1;
	}
	return conn_socket;
}
////////////////////////////////////////////////////////////////////////////////
// Send command to the server                                               ////
////////////////////////////////////////////////////////////////////////////////
int send_data(int conn_socket,struct sockaddr_in * server,char buffer[])
{
	int ret_val;
	socklen_t length;
	length=sizeof((struct sockaddr_in * )server);
	ret_val=send(conn_socket,buffer,MAX_BUFFER,0);
	//ret_val=sendto(conn_socket,buffer,strlen(buffer),0,(struct sockaddr*)server,length);
	if(ret_val<0)
	{
		fprintf(stderr,"\nError in sending");
		return -1;
	}
	return 1;
}
////////////////////////////////////////////////////////////////////////////////
//Check the Validilty of input arguments on the basis of sppaces              //
// return number of spaces in input                                           //
////////////////////////////////////////////////////////////////////////////////
int check_input(char input[])
{
	int i,counter=0;
	
	for(i=0;i<strlen(input);i++)
	{
		if(input[i]==' ')
		{
			counter++;
		}
	}
	return counter;
}
