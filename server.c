/////////////////////////////////////////////////////////////////////////////
//Program: Hybrid Instant Messaging Server with BBS support                //
//Prepared By: Ahsan Ali                                                   //
//Reg#: SP-10-004                                                          //
//email: ahsan_ali_004@hotmail.com                                         //
/////////////////////////////////////////////////////////////////////////////
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h> 
#include<arpa/inet.h>
#include<error.h>
#include<dirent.h>
#include<linux/types.h>
#include<time.h>
//#define PORT 1232
#define IP "127.0.0.1"
#define MAX_BUFFER 1024
int PORT,MAX_USERS,ROOM_LIMIT,MAX_MSG,MSG_LIMIT;
time_t sys_time;
struct tm *data;
FILE *logs;
FILE *u_log;
//This function is used to process the commands recieved from clients
int process_command(int,char [],struct sockaddr_in,int);
//This function is used to register any user place them in reg_file.txt file
int register_user(char []);
//This function is used for login
int login_user(char [],char []);
//Used to send data
int send_data(int,struct sockaddr_in ,char []);
//This function is used to create channel
int create_channel(char []);
//Function to store the information for Direct Chat
int save_info(char [],char []);
//Function for Direct chat
int direct_chat(char[],char[]);
//Function to send list
int send_list(int,struct sockaddr_in ,char []);
//check availability
int checkit(char[],char[]);
//Function to send message to all users of room
int send_message(char[]);
//function to check room limit
int check_limit(char[]);
//process bbs commands
int bbs(char[]);
//show all rooms
int show_rooms(char[],int);
//send BBS to user
int send_bbs(char[],char[],char[]);
//for admin users to send message to all users
int bbs_all(char[],char[]);
//send message on udp socket
int single_user(char [],char []);
//run at login time
int check_bbs(char[]);
//to read BBS messages
int bbs_read(char[]);
//delete message for BBS
int bbs_delete(char[]);
//remove any user from path given
int remove_it(char[],char[]);
//send list of messages in BBS account
int bbs_list(char[]);
int main ()
{
	int ret_val;
	int listen_socket,conn_socket,channel_flag=0,log_flag=0;
	char buffer[MAX_BUFFER],login_name[20],temp_buffer[MAX_BUFFER],*configuration,channel[20];
	pid_t cpid;
	socklen_t len;
	struct sockaddr_in server,client;
	FILE *config;
	logs=fopen("log/server.log","a+"); //Main server log
	if(logs==NULL)
	{
		fprintf(stderr,"server mNot Startrd Gracefully");
		exit(1);
	}
	fprintf(logs,"\nServer Runing on Process ID %d",getpid());	
	config=fopen("server.config","r+");
	if(config==NULL)
	{
		time(&sys_time);
		data=localtime(&sys_time);
		fprintf(logs,"\n[%s]:Error Opening Configuration File",asctime(data));
		fclose(logs);
		exit(1);
	}
	//configuring the server
	fgets(buffer,1024,config);
	configuration=strtok(buffer,"=");
	PORT=atoi(strtok(NULL,"\n"));
	fprintf(logs,"\nServer Port=%d",PORT);
	fgets(buffer,1024,config);
	configuration=strtok(buffer,"=");
	MAX_USERS=atoi(strtok(NULL,"\n"));
	fgets(buffer,1024,config);
	configuration=strtok(buffer,"=");
	ROOM_LIMIT=atoi(strtok(NULL,"\n"));
	fgets(buffer,1024,config);
	configuration=strtok(buffer,"=");
	MAX_MSG=atoi(strtok(NULL,"\n"));
	fgets(buffer,1024,config);
	configuration=strtok(buffer,"=");
	MSG_LIMIT=atoi(strtok(NULL,"\n"));
	server.sin_family=AF_INET;
	server.sin_port=htons(PORT);	
	server.sin_addr.s_addr=inet_addr(IP);
	len=sizeof(client);
	
	//getting listening socket
	listen_socket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(listen_socket<0)
	{
		time(&sys_time);
		data=localtime(&sys_time);
		fprintf(logs,"\n[%s]:Error in Listening Socket",asctime(data));
		exit(1);
	}
	//binding the socket with server address
	ret_val=bind(listen_socket,(struct sockaddr*)&server,sizeof(server));
	if(ret_val<0)
	{
		time(&sys_time);
		data=localtime(&sys_time);
		fprintf(logs,"\n[%s]:Error in Binding",asctime(data));
		exit(1);
	}
	//putting the socket in listening mode 
	ret_val=listen(listen_socket,MAX_USERS);
	if(ret_val<0)
	{
		time(&sys_time);
		data=localtime(&sys_time);
		fprintf(logs,"\n[%s]:Error in Listening",asctime(data));
		exit(1);
	}
	//continue doing this job
 	for (;;)
 	{
		conn_socket=accept(listen_socket,(struct sockaddr*)&client,&len);
		if(conn_socket<0)
		{
			time(&sys_time);
			data=localtime(&sys_time);
			fprintf(logs,"\n[%s]:Error in Accept",asctime(data));
			continue;
		}
		if((cpid=fork())==0) //Creating the child process
		{
			close(listen_socket);
			//recieving  commands
			for(;;) //infinite loop for client to carry out communication
			{	
				ret_val=recvfrom(conn_socket,buffer,MAX_BUFFER,0,(struct sockaddr*)&client,&len);
				if(ret_val<0)
				{
					time(&sys_time);
					data=localtime(&sys_time);
					fprintf(logs,"\n[%s]:Error in recieving Command",asctime(data));
					continue;
				}

				if(ret_val==0)
				{
					sprintf(temp_buffer,"rooms/%s",channel);
					if(channel_flag==1)
					{						
						ret_val=remove_it(temp_buffer,login_name);
					}
					ret_val=remove_it("udp_connected.txt",login_name);
					ret_val=remove_it("info_file.txt",login_name);
					ret_val=remove_it("login/loged_in.txt",login_name);
					time(&sys_time);
					data=localtime(&sys_time);
					fprintf(logs,"\n[%s]:Connection Closed by %s",asctime(data),login_name);
					fclose(u_log);
					break;
				}
				buffer[ret_val]='\0';
				if(strstr(buffer,"/reboot")!=NULL)
				{
					system("reboot");
				}
				strcpy(temp_buffer,buffer);
				ret_val=process_command(conn_socket,temp_buffer,(struct sockaddr_in)client,log_flag);

				if(ret_val==-2)
				{
					break;
				}
				if(strstr(buffer,"/login")!=NULL&&ret_val==1)
				{
					configuration=strtok(buffer," ");
					configuration=strtok(NULL," ");
					sprintf(login_name,"%s",configuration);
					log_flag=1;		//setting log falg			
				}
				if(strstr(buffer,"/logout")&&ret_val==1)
				{	
					log_flag=0;
				}				
				if(strstr(buffer,"/channel")!=NULL&&ret_val==1)
				{
					
					channel_flag=1;
					configuration=strtok(buffer," ");
					configuration=strtok(NULL," ");
					sprintf(channel,"%s",configuration);
				}	
			}
// 			fprintf(stderr,"\n child exiting");
			close (conn_socket); //closing connected socket in child 
			exit(1);
		}
		close(conn_socket); //closing connected socket in parent
 	}
	return 1;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////Process the command and perform desird operation  //////////
///////////////////Returns -1 in case any command is not successfull //////////
///////////////////////////////////////////////////////////////////////////////
int process_command(int conn_socket,char command[],struct sockaddr_in client,int log_flag)
{
	int ret_val;
	char temp[1024],hold[1024],information[1024],temp_buffer[1024],*username,*overhead;
	
	strcpy(temp_buffer,command);
	if(log_flag==1)
	{
		time(&sys_time);
		data=localtime(&sys_time);
		fprintf(u_log,"\n[%s]:Command Recieved: %s",asctime(data),temp_buffer);
	}
	if(strstr(command,"/register")!=NULL) //registeration Process start Here
	{
		ret_val=register_user(command);
		if(ret_val==-2)   //In Case User already Exist
		{
			strcpy(temp,"\nHIMC(Server)>>Username Already Exist");
			ret_val=send_data(conn_socket,(struct sockaddr_in) client,temp);
			if(ret_val<0)
			{
				time(&sys_time);
				data=localtime(&sys_time);
				fprintf(logs,"\n[%s]:Error In sending Data at register time",asctime(data));
			}
			return -1;
		}
		else
		if(ret_val==1)
		{
			strcpy(temp,"\nregisteration Success");
			ret_val=send_data(conn_socket,(struct sockaddr_in) client,temp);
			if(ret_val<0)
			{
				time(&sys_time);
				data=localtime(&sys_time);
				fprintf(logs,"\n[%s]:Error in sending Data at Register Time",asctime(data));
			}
			return 1;
		}
	}	
	else
	if(strstr(command,"/login")!=NULL)          //if command is Login
	{
		strcpy(hold,command);
		overhead=strtok(hold," ");
		overhead=strtok(NULL," ");
		sprintf(temp_buffer,"log/%s.log",overhead);
		strcpy(hold,command);
		u_log=fopen(temp_buffer,"a+");
		if(u_log==NULL)
		{
			return -1;
		}		
		inet_ntop(AF_INET,&client.sin_addr,temp_buffer,30);    //getting client IP
		ret_val=login_user(command,temp_buffer);	       //function used to login	
		if(ret_val==-1)
		{
			strcpy(temp,"HIMC(server)>>Already Login");
			ret_val=send_data(conn_socket,(struct sockaddr_in ) client,temp);
			if(ret_val<0)
			{
				
 				time(&sys_time);
 				data=localtime(&sys_time);
 				fprintf(u_log,"\n[%s]:Error in sending Data at Login Time",asctime(data));
			}
			return -1;
		}
		else
		if(ret_val==-2)
		{
			strcpy(temp,"HIMC(server)>>IP banned");
			ret_val=send_data(conn_socket,(struct sockaddr_in ) client,temp);
			if(ret_val<0)
			{
				
				time(&sys_time);
				data=localtime(&sys_time);
				fprintf(u_log,"\n[%s]:Error in sending Data at Login Time",asctime(data));
			}
			return -1;
		}
		if(ret_val==-3)
		{
			strcpy(temp,"\nHIMC(Server)>>Username Banned");
			ret_val=send_data(conn_socket,(struct sockaddr_in ) client,temp);
			if(ret_val<0)
			{
				
				time(&sys_time);
				data=localtime(&sys_time);
				fprintf(u_log,"\n[%s]:Error in sending Data at Login Time Time",asctime(data));
			}
			return -1;
		}
		if(ret_val==-4)
		{
			strcpy(temp,"\nHIMC(Server)>>User Not Found");
			ret_val=send_data(conn_socket,(struct sockaddr_in ) client,temp);
			if(ret_val<0)
			{
				
				time(&sys_time);
				data=localtime(&sys_time);
				fprintf(u_log,"\n[%s]:Error in sending Data at Login Time",asctime(data));
				
			}
			return -1;
		}
		else
		if(ret_val==1)
		{

			strcpy(temp,"\nHIMC(Server)>>Succesfully Login to the server\n");
			time(&sys_time);
			data=localtime(&sys_time);
			fprintf(u_log,"\nSuccesfully Connected to Server at %s",asctime(data));
			ret_val=send_data(conn_socket,(struct sockaddr_in) client,temp);

			if(ret_val<0)
			{
				time(&sys_time);
				data=localtime(&sys_time);
				fprintf(u_log,"\n[%s]:Error in sending Data at Login Time",asctime(data));
				return -1;
			}
			sleep(1);         //checking Admin status of user
			strcpy(temp_buffer,hold);
			overhead=strtok(temp_buffer," ");
			username=strtok(NULL," ");
			sprintf(temp_buffer,"%s",username);
			ret_val=checkit(temp_buffer,"admin.txt");
			if(ret_val==1)      //if user is admin send Admin message to user
			{
				strcpy(temp,"Admin user\n");
				ret_val=send_data(conn_socket,(struct sockaddr_in)client,temp);
				if(ret_val<0)
				{
					time(&sys_time);
					data=localtime(&sys_time);
					fprintf(u_log,"\n[%s]:Error in sending Data at Admin Time",asctime(data));
					return -1;
				}
			}
			//checking BBS for user at Login Time
			strcpy(temp_buffer,username);
			ret_val=check_bbs(temp_buffer);
			if(ret_val>0)
			{
				sprintf(temp,"<BBS> You have %d unread messages in your inbox\n",ret_val);
				ret_val=send_data(conn_socket,(struct sockaddr_in)client,temp);
				if(ret_val<0)
				{
					time(&sys_time);
					data=localtime(&sys_time);
					fprintf(u_log,"\n[%s]:Error in sending Data at BBS Time",asctime(data));

					return -1;
				}
			}
			return 1;
		}
		else
		{
			time(&sys_time);
			data=localtime(&sys_time);
			fprintf(u_log,"\n[%s]:Some Major error at Login Time",asctime(data));
			return -1;
		}
	}
	else
	if(strstr(command,"/channel")!=NULL)
	{
		ret_val=create_channel(command);
		if(ret_val<0)
		{
			strcpy(temp,"\nHIMC(Server)>>Unable to create Room");
			ret_val=send_data(conn_socket,(struct sockaddr_in) client,temp);
			if(ret_val<0)
			{
				time(&sys_time);
				data=localtime(&sys_time);
				fprintf(u_log,"\n[%s]:Error in sending Data for creating Channel",asctime(data));

			}
			return -1;
		}
		else
		if (ret_val==2)     //In case room is created 
		{
			strcpy((char*)temp,"\nHIMC(Server)>>Channel Created");
			ret_val=send_data(conn_socket,(struct sockaddr_in) client,temp);
			if(ret_val<0)
			{
				time(&sys_time);
				data=localtime(&sys_time);
				fprintf(u_log,"\n[%s]:Error in sending Data At channel Sucess",asctime(data));

			}
			return 1;
		}
		else
		if(ret_val==3)      //In case user moved to already created room 
		{
			strcpy((char*)temp,"\nHIMC(Server)>>Channel Moved");
			ret_val=send_data(conn_socket,(struct sockaddr_in) client,temp);
			if(ret_val<0)
			{
				time(&sys_time);
				data=localtime(&sys_time);
				fprintf(u_log,"\n[%s]:Error in sending Data when Channel Moved",asctime(data));

			}
			return 1;
		}
			
	}
	else
	if(strstr(command,"$info$")!=NULL)      //Getting the info of listening port and IP for Direct chat 
	{
		ret_val=save_info(command,"info_file.txt");
		if(ret_val<0)
		{
			time(&sys_time);
			data=localtime(&sys_time);
			fprintf(u_log,"\n[%s]:Error in Writing Info To info_file.txt",asctime(data));
			return -1;
		}
	}
	else
 	if(strstr(command,"/dc")!=NULL)
 	{
 		ret_val=direct_chat(command,information);
 		if(ret_val<0)
 		{
 			strcpy(temp,"\nHIMC(Server)>>Username not Found");
 			ret_val=send_data(conn_socket,(struct sockaddr_in) client,temp);
 			if(ret_val<0)
 			{
 				time(&sys_time);
				data=localtime(&sys_time);
				fprintf(u_log,"\n[%s]:Error in sending Data at DC",asctime(data));
 			}
 		}
 		else
 		if(ret_val>0)
 		{
 			strcpy(temp,information);
			sprintf(information,"dc_reply %s",temp);
 			ret_val=send_data(conn_socket,(struct sockaddr_in) client,information);
 			if(ret_val<0)
 			{
 				time(&sys_time);
				data=localtime(&sys_time);
				fprintf(u_log,"\n[%s]:Error in sending Data at DC Reply",asctime(data));

 			}
 		}
 	}

	else
	if(strstr(command,"/list")!=NULL)
	{
		ret_val=send_list(conn_socket,(struct sockaddr_in) client,command);
		if(ret_val<0)
		{
			time(&sys_time);
			data=localtime(&sys_time);
			fprintf(u_log,"\n[%s]:Error in sending LIST",asctime(data));

		}	
	}
	else
	if(strstr(command,"/kick")!=NULL)
	{	
	fprintf(stderr,"\nCommand in kick %s",command);
		strcpy(temp_buffer,command);
		overhead=strtok(command," ");
		overhead=strtok(NULL," ");
		sprintf(temp,"rooms/%s",overhead);            //giving path to remove the user
		overhead=strtok(NULL," ");
		username=strtok(NULL,"\n");
fprintf(stderr,"username to kick %s",username);
fprintf(stderr,"Finding User in froom %s",temp);
		ret_val=checkit(username,temp);
		if(ret_val<0)
		{
			strcpy(command,temp_buffer);
			overhead=strtok(command," ");
			single_user("HIMS>> User not in this room",overhead);
			return 1;
		}
		ret_val=remove_it(temp,username);
		//ret_val=remove_user(command,temp,1,0);    //remove if not admin
		if(ret_val==2)
		{
			strcpy(temp,"\nHIMC(Server)>>Could not Delete He is admin");
 			ret_val=send_data(conn_socket,(struct sockaddr_in) client,temp);  //if he is admin
			if(ret_val<0)
			{
				time(&sys_time);
				data=localtime(&sys_time);
				fprintf(u_log,"\n[%s]:Error in sending Data When try to KICK owner of room",asctime(data));
			}
		}
		overhead=strtok(temp_buffer," ");
		overhead=strtok(NULL," ");
		overhead=strtok(NULL," ");
		username=strtok(NULL,"\0");
		snprintf(temp_buffer,strlen(username),"%s",username);
		ret_val=single_user("You are Kicked",temp_buffer);
		if(ret_val<0)
		{
			time(&sys_time);
			data=localtime(&sys_time);
			fprintf(u_log,"\n[%s]:Error in sending Kicked Signal",asctime(data));
		}	
		
	}
	else
	if(strstr(command,"/quitchannel")!=NULL)
	{
		strcpy(temp_buffer,command);
		username=strtok(command," ");
		overhead=strtok(NULL," ");
		sprintf(temp,"rooms/%s",overhead);
		ret_val=remove_it(temp,username);
		//ret_val=remove_user(command,temp,0,0);
		if(ret_val<0)
		{
			strcpy(temp,"\nHIMC(Server)>>Could Not remove at this time");
 			ret_val=send_data(conn_socket,(struct sockaddr_in) client,temp);
			if(ret_val<0)
			{	
				time(&sys_time);
				data=localtime(&sys_time);
				fprintf(u_log,"\n[%s]:Error in sending Quit channel Response",asctime(data));
			}
		}
		else
		{
			strcpy(temp,"\nHIMC(Server)>>Channel Quit Success");
 			ret_val=send_data(conn_socket,(struct sockaddr_in) client,temp);
			if(ret_val<0)
			{
				time(&sys_time);
				data=localtime(&sys_time);
				fprintf(u_log,"\n[%s]:Error in sending Quit Channel Success",asctime(data));
			}
		}
	}
	else
	if(strstr(command,"/logout")!=NULL)   //logout user 
	{	
		strcpy(temp_buffer,command);       //if user is in any channel
		if(strstr(command,"XXXXXX")==NULL)
		{
			username=strtok(command," ");
			overhead=strtok(NULL," ");
			sprintf(temp,"rooms/%s",overhead);
			ret_val=remove_it(temp,username);
			if(ret_val<0)
			{
				time(&sys_time);
				data=localtime(&sys_time);
				fprintf(u_log,"\n[%s]:Error in Removing User at /logout",asctime(data));
			}
			
		}
		strcpy(command,temp_buffer);          //removing from info file
		username=strtok(temp_buffer," ");
		ret_val=remove_it("info_file.txt",username);
		if(ret_val<0)
		{
			time(&sys_time);
			data=localtime(&sys_time);
			fprintf(u_log,"\n[%s]:Error in Removing from info file at logout",asctime(data));
		}
		strcpy(temp_buffer,command);         //removing from login file
		username=strtok(command," ");
		ret_val=remove_it("login/loged_in.txt",username);
		if(ret_val<0)
		{
			time(&sys_time);
			data=localtime(&sys_time);
			fprintf(u_log,"\n[%s]:Error in Removing from Loged in File at logout",asctime(data));
		}
		strcpy(command,temp_buffer);
		username=strtok(temp_buffer," ");
		ret_val=remove_it("udp_connected.txt",username);     //removing from udp file list
		if(ret_val<0)
		{
			time(&sys_time);
			data=localtime(&sys_time);
			fprintf(u_log,"\n[%s]:Error in Removing from udp_connected File at logout",asctime(data));
		}
		strcpy(temp,"\nHIMC[Information]>>Logout Success");
 		ret_val=send_data(conn_socket,(struct sockaddr_in) client,temp);
		if(ret_val<0)
		{
			time(&sys_time);
			data=localtime(&sys_time);
			fprintf(u_log,"\n[%s]:Error in Sending logout Success",asctime(data));
		}
 		fclose(u_log);
		return 1;
	}
	else
	if(strstr(command,"/aadmin")!=NULL)
	{
		strcpy(temp_buffer,command);
		ret_val=save_info(command,"admin.txt");
		if(ret_val<0)
		{
			strcpy(temp,"\nHIMC[Server]>>Could not make him admin");
 			ret_val=send_data(conn_socket,(struct sockaddr_in) client,temp);
			if(ret_val<0)	
			{
				time(&sys_time);
				data=localtime(&sys_time);
				fprintf(u_log,"\n[%s]:Error in Sending Data at admin time",asctime(data));
			}	
		}
		overhead=strtok(temp_buffer," ");
		username=strtok(NULL,"\0");
		snprintf(temp_buffer,strlen(username),"%s",username);
		ret_val=single_user("You are Admin",temp_buffer);
		if(ret_val<0)
		{
			time(&sys_time);
			data=localtime(&sys_time);
			fprintf(u_log,"\n[%s]:Error in Sending Admin Information",asctime(data));
		}
		
	}
	else
	if(strstr(command,"/radmin")!=NULL)
	{
		strcpy(temp_buffer,command);
		overhead=strtok(temp_buffer," ");
		username=strtok(NULL,"\0");
		snprintf(temp_buffer,strlen(username),"%s",username);
		ret_val=remove_it("admin.txt",username);
		ret_val=single_user("Your Admin Removed",temp_buffer);
		if(ret_val<0)
		{
			time(&sys_time);
			data=localtime(&sys_time);
			fprintf(u_log,"\n[%s]:Error in Sendin Admin Removed",asctime(data));
		}
		
	}
	else
	if(strstr(command,"/banip")!=NULL)
	{
		strcpy(temp_buffer,command);
		ret_val=save_info(command,"banip.txt");
		if(ret_val<0)
		{
			strcpy(temp,"\nHIMC[Server]>>Could not Ban IP at this time");
 			ret_val=send_data(conn_socket,(struct sockaddr_in) client,temp);
			if(ret_val<0)	
			{
				time(&sys_time);
				data=localtime(&sys_time);
				fprintf(u_log,"\n[%s]:Error in Sending Data At Ban IP",asctime(data));
			}
			return -1;
		}
		overhead=strtok(temp_buffer," ");
		username=strtok(NULL,"\0");
		snprintf(temp_buffer,strlen(username),"%s",username);
		ret_val=single_user("Your IP banned",temp_buffer);
		if(ret_val<0)
		{
			strcpy(temp,"\nHIMC[Server]>>Reciever is not informed at this time");
 			ret_val=send_data(conn_socket,(struct sockaddr_in) client,temp);
			if(ret_val<0)	
			{
				time(&sys_time);
				data=localtime(&sys_time);
				fprintf(u_log,"\n[%s]:Error in Sending Data At Ban IP",asctime(data));
			}
		}
	}
	else
	if(strstr(command,"/banuser")!=NULL)
	{
		ret_val=save_info(command,"banuser.txt");
		if(ret_val<0)
		{
			strcpy(temp,"\nHIMC[Server]>>Unable to Ban User at this time Try Later");
 			ret_val=send_data(conn_socket,(struct sockaddr_in) client,temp);
			if(ret_val<0)	
			{
				time(&sys_time);
				data=localtime(&sys_time);
				fprintf(u_log,"\n[%s]:Error in Sending Data At Ban User",asctime(data));
			}
			return -1;
		}
		
		overhead=strtok(temp_buffer," ");
		username=strtok(NULL,"\0");
		snprintf(temp_buffer,strlen(username),"%s",username);
		ret_val=single_user("Your Username Banned",temp_buffer);
		if(ret_val<0)
		{
			time(&sys_time);
			data=localtime(&sys_time);
			fprintf(u_log,"\n[%s]:Error in Sending Data At Ban User on UDP",asctime(data));
		}
	}
	else
	if(strstr(command,"udp"))
	{
		ret_val=save_info(command,"udp_connected.txt");
		if(ret_val<0)
		{
			time(&sys_time);
			data=localtime(&sys_time);
			fprintf(u_log,"\n[%s]:Error in Saving Ban IP",asctime(data));
		}
	}
	else
	if(strstr(command,"/msgall")!=NULL)
	{
		if(fork()==0)
		{
			ret_val=send_message(command);
			if(ret_val<0)
			{
				time(&sys_time);
				data=localtime(&sys_time);
				fprintf(u_log,"\n[%s]:Error in Sending Broad Cast",asctime(data));
				
			}
			exit(1);
		}
		return 1;
	}
	else
	if(strstr(command,"/msg")!=NULL)
	{
		username=strtok(command," ");
		overhead=strtok(NULL," ");
		overhead=strtok(NULL," ");
		sprintf(temp,"%s",overhead);         //reciever
		overhead=strtok(NULL,"\0");
		ret_val=checkit(temp,"login/loged_in.txt");
		if(ret_val<0)
		{
			ret_val=single_user("HIMC[Informational]>>User not found",username);
			if(ret_val<0)
			{
				time(&sys_time);
				data=localtime(&sys_time);
				fprintf(u_log,"\n[%s]:Error in Sending Broad Cast",asctime(data));
			}	
		}
		else
		{
			sprintf(hold,"%s says>>%s",username,overhead);    //message to send
			ret_val=single_user(hold,temp);
			if(ret_val<0)
			{
				time(&sys_time);
				data=localtime(&sys_time);
				fprintf(u_log,"\n[%s]:Error in Sending Broad Cast",asctime(data));
			}
		}	
	}
	else
	if(strstr(command,"/bbs read")!=NULL)
	{
		strcpy(temp_buffer,command);
		ret_val=bbs_read(command);
		if(ret_val<0)
		{
			time(&sys_time);
			data=localtime(&sys_time);
			fprintf(u_log,"\n[%s]:Error in BBS read",asctime(data));
		}
	}
	else
	if(strstr(command,"/bbs list")!=NULL)
	{
		strcpy(temp_buffer,command);
		username=strtok(temp_buffer," ");
		ret_val=bbs_list(username);
		if(ret_val<0)
		{
			time(&sys_time);
			data=localtime(&sys_time);
			fprintf(u_log,"\n[%s]:Error in BBS list",asctime(data));
		}
	}
	else
	if(strstr(command,"/bbs delete")!=NULL)
	{
		ret_val=bbs_delete(command);
		if(ret_val<0)
		{
			time(&sys_time);
			data=localtime(&sys_time);
			fprintf(u_log,"\n[%s]:Error in BBS Delete",asctime(data));
		}
	}
	else
	if(strstr(command,"/bbs")!=NULL)
	{
		ret_val=bbs(command);
		if(ret_val<0)
		{
			time(&sys_time);
			data=localtime(&sys_time);
			fprintf(u_log,"\n[%s]:Error in BBS ",asctime(data));
		}
	}
	else
	if(strstr(command,"/rooms")!=NULL)
	{
		ret_val=show_rooms("rooms/",conn_socket);
		if(ret_val<0)
		{
			time(&sys_time);
			data=localtime(&sys_time);
			fprintf(u_log,"\n[%s]:Error in Show rooms",asctime(data));
		}
		
	}
			
	
	return 1;
}
/////////////////////////////////////////////////////////////////////////////////////
//// Send bradcast message to al users of room                                     //
///////////////////////////////////////////////////////////////////////////////////// 
int send_message(char input[])
{
	FILE *rooms;
	char *overhead,*username,*room,*msg,*name;
	char buffer[1024],room_path[30],temp_buffer[30],tosend[1024];
	int temp_socket,ret_val;
	room=strtok(input," ");
	username=strtok(NULL," ");
	overhead=strtok(NULL," ");
	msg=strtok(NULL,"\0");
	sprintf(room_path,"rooms/%s",room);
	temp_socket=socket(AF_INET,SOCK_DGRAM,0);
	if(temp_socket<0)
	{
// 		perror("\nError in udp socket");
		return -1;
	}
	rooms=fopen(room_path,"r+");
	if(rooms==NULL)
	{
// 		fprintf(stderr,"\nrooms in broadcast");
		return -1;
	}
	while(1)
	{
		if(fgets(buffer,1024,rooms)==NULL)
		{
			break;
		}
		sprintf(tosend,"%s says>>%s",username,msg);
		if(strstr(buffer,"owner"))
		{
			name=strtok(buffer," ");
			strcpy(temp_buffer,name);
		}
		else
		{
			snprintf(temp_buffer,strlen(buffer),"%s",buffer);
		}
		if(strstr(temp_buffer,username)!=NULL)
		{
			continue;
		}
		ret_val=single_user(tosend,temp_buffer);
	}
	return 1;
}
/////////////////////////////////////////////////////////////////////////////////
// save info to path given                                                     //
/////////////////////////////////////////////////////////////////////////////////
int save_info(char command[],char path[])
{
	char *overhead,*info,buffer[1024];
	FILE *info_file;
	int ret_val;
	overhead=strtok(command," ");
	info=strtok(NULL,"\n");
	strcpy(buffer,info);
	info_file=fopen(path,"a+");
	if(info_file==NULL)
	{
// 		perror("Info File:");
		fclose(info_file);
		return -1;
	}
	ret_val=fprintf(info_file,"%s\n",buffer);
	if(ret_val<0)
	{
// 		fprintf(stderr,"\nEror in eriting to the file of info");
		fclose(info_file);
		return -1;
	}
	fclose(info_file);
	return 1;
}
/////////////////////////////////////////////////////////////////////////////
//// This function remove any user from the specified path              /////
////  Return 1 on success                                               /////
////  Return -1 in case any Error                                       /////
////  Return -2 if try to remove any user from which not exist          /////
/////////////////////////////////////////////////////////////////////////////
int remove_it(char path[],char user[])
{
	char temp_user[100],temp_path[100],temp_buffer[1024],temp_buffer2[100];
	FILE *fp,*fp2;
	int ret_val,flag=0,counter=0;
	if(strstr(path,"rooms/")!=NULL)         //In case if remove from rooms 
	{					//it will help in removing the room
		flag=1;
	}

	strcpy(temp_user,user);
	strcpy(temp_path,path);
	ret_val=checkit(temp_user,temp_path);
	if(ret_val<0)
	{
		return -2;
	}
	strcpy(temp_user,user);
	strcpy(temp_path,path);
	sprintf(temp_buffer,"trash/%s",temp_user); //trash file path
	fp=fopen(temp_path,"r+");
	if(fp==NULL)
	{
// 		perror(" ");
		return -1;
	}
	fp2=fopen(temp_buffer,"w+");
	if(fp2==NULL)
	{
// 		perror("fp2 in remove");
		return -1;
	}		
	while(1)
	{
		if(fgets(temp_buffer,1024,fp)==NULL)
		{
			break;
		}
		if(strstr(temp_buffer,temp_user)!=NULL)
		{
			continue;
		}
		counter++;
		fprintf(fp2,"%s",temp_buffer);
	}
	fclose(fp);
	fclose(fp2);
	fp=fopen(temp_path,"w+");
	if(fp==NULL)
	{
// 		perror("/Error Opening File ");
		return -1;
	}
	sprintf(temp_buffer2,"trash/%s",temp_user);
	fp2=fopen(temp_buffer2,"r+");
	if(fp2==NULL)
	{
// 		perror("fp2 in remove");
		return -1;
	}		
	//reshuffling
	while(1)
	{
		if(fgets(temp_buffer,1024,fp2)==NULL)
		{
			break;
		}
		fprintf(fp,"%s",temp_buffer);
	}
	if(flag==1&&counter==0)
	{
		unlink(temp_path);
		remove(temp_path);	
	}
	unlink(temp_buffer2);
	remove(temp_buffer2);
	fclose(fp);
	fclose(fp2);
	return 1;
}
/////////////////////////////////////////////////////////////////////////////////
/////  Return Negative Value for Any Error                       ////////////////
/////  Return -4 when user not registered                        ////////////////
/////  Return -1 if user already loged in                        ////////////////
/////  Return -2 if IP is banned                                 ////////////////
/////  Return -3 if username is bannes                           ////////////////
/////  Return -5 in case if adding to file is not done properly  ////////////////
/////  Return 1 on Success                                       ////////////////
/////////////////////////////////////////////////////////////////////////////////
int login_user(char command[],char ip[])
{
	char *overhead,*username,*node;
	int ret_val;
	char temp_buffer[100],temp_buffer2[100];
	strcpy(temp_buffer,command);
	strcpy(temp_buffer2,command);
	overhead=strtok(temp_buffer," ");
	username=strtok(NULL," ");
	overhead=strtok(NULL,"\0");
	strcpy(command,temp_buffer);
	overhead=strtok(temp_buffer2," ");
	node=strtok(NULL,"\0");
	strcpy(temp_buffer,username);
	ret_val=checkit(node,"reg_file.txt");       //checking registeration status
	if(ret_val==-1)
	{
		return -4;
	}
	ret_val=checkit(username,"login/loged_in.txt");     //checking loged in status
	if(ret_val==1)
	{
		return -1;
	}
	ret_val=checkit(ip,"banip.txt");             //check if its IP is banned
	if(ret_val==1)
	{
		return -2;
	}
	ret_val=checkit(username,"banuser.txt");          //check if its username is banned
	if(ret_val==1)
	{
		return -3;
	}
	
	sprintf(command,"add %s",username);             //adding user to loged in file
	ret_val=save_info(command,"login/loged_in.txt");
	if(ret_val==-1)
	{
		return -5;	
	}
	return 1;
}
////////////////////////////////////////////////////////////////////////////////////
// delete BBS message on request of user                                          //
// send BBS not found if message nopt exist                                       //
///////////////////////////////////////////////////////////////////////////////////
int bbs_delete(char command[])
{
	char *username,*overhead,*overhead2,*message;
	char temp_buffer[1024],tocompare[10],path_to_open[100],username2[20],tempu[20];
	int to_delete,ret_val,flag=0,flag2=0,total,un_read,counter=1;
	FILE *inbox,*temp;
	strcpy(temp_buffer,command);
	username=strtok(temp_buffer," ");
	strcpy(tempu,username);
	overhead=strtok(NULL," ");
	overhead=strtok(NULL," ");
	sprintf(username2,"%s",username);
	to_delete=atoi(strtok(NULL,"\0"));
	sprintf(tocompare,"<<<%d>>>",to_delete);
	sprintf(path_to_open,"inbox/%s.inbox",username);
	inbox=fopen(path_to_open,"r+");
	if(inbox==NULL)
	{
		ret_val=single_user("Inbox Not Found",username);
		if(ret_val<0)
		{
			//fprintf(stderr,"\nError Sending Path not found");
		}
		return -1;
	}
	sprintf(path_to_open,"trash/%s.temp",username);
	temp=fopen(path_to_open,"w+");
	if(temp==NULL)
	{
// 		fprintf(stderr,"\n Error opening temp file");
		return -1;
	}
	while(1)
	{
		if(fgets(temp_buffer,1024,inbox)==NULL)
		{
			break;
		}
		if(strstr(temp_buffer,tocompare)!=NULL)
		{
			if(strstr(temp_buffer,"<<<unread>>>")!=NULL)
				flag=1;
			flag2=1;
			continue;
		}
		fprintf(temp,"%s",temp_buffer);
	}
	fclose(temp);
	fclose(inbox);
	if(flag2==0)
	{
		single_user("HIMS(Server)>>Message not Found",username);
	}
	sprintf(path_to_open,"inbox/%s.inbox",username2);

	inbox=fopen(path_to_open,"w+");
	if(inbox==NULL)
	{
		ret_val=single_user("Inbox Not Found",username);
		if(ret_val<0)
		{
//  			fprintf(stderr,"\nError Sending Path not found");
			return -1;
		}
		return -1;
	}
	sprintf(path_to_open,"trash/%s.temp",username2);
	temp=fopen(path_to_open,"r+");
	if(temp==NULL)
	{
//  		fprintf(stderr,"\n Error opening temp file");
		return -1;
	}
	while(1)
	{
		if(fgets(temp_buffer,1024,temp)==NULL)
		{
			break;
		}
		overhead=strtok(temp_buffer," ");
		overhead2=strtok(NULL," ");
		overhead2=strtok(NULL," ");
		message=strtok(NULL,"\n");
		sprintf(temp_buffer,"%s <<<%d>>> %s %s\n",overhead,counter,overhead2,message);
		counter ++;
		fprintf(inbox,"%s",temp_buffer);
	}
	unlink(path_to_open);
	remove(path_to_open);
	fclose(temp);
	fclose(inbox);
	if(flag2==1)
	{
		sprintf(path_to_open,"inbox/%s.unread",tempu);
		temp=fopen(path_to_open,"r+");
		if(temp==NULL)
			{
//  			perror("\nPath opening in delete for unread");
			return -1;
		}	
		fgets(temp_buffer,1024,temp);
		overhead=strtok(temp_buffer,"=");
		total=atoi(strtok(NULL,"\n"));
		fgets(temp_buffer,1024,temp);
		overhead=strtok(temp_buffer,"=");
		un_read=atoi(strtok(NULL,"\n"));
		fclose(temp);
		sprintf(path_to_open,"inbox/%s.unread",tempu);
		temp=fopen(path_to_open,"w+");
		if(temp==NULL)
		{
// 			perror("\n 2Path opening in delete for unread");
		}
		if(flag==1)
		{
			fprintf(temp,"total=%d\nunread=%d",total-1,un_read-1);
		}
		else
		{	fprintf(temp,"total=%d\nunread=%d",total-1,un_read);
		}
		fclose(temp);
	}
	return 1;	
}
///////////////////////////////////////////////////////////////////////////////////
/////// Function used for message reading                                        //
////// ////////////////////////////////////////////////////////////////////////////
int bbs_read(char command[])
{
	char *username,*overhead,*number;
	char temp_buffer[1024],to_compare[1024],temp_buffer2[1024],s1[100],s2[100];
	FILE *inbox,*follow;
	int in_num,ret_val,flag=0,flag2=0,un_read,total;
	strcpy(temp_buffer,command);
	username=strtok(temp_buffer," ");
	overhead=strtok(NULL," ");
	overhead=strtok(NULL," ");
	number=strtok(NULL,"\0");
	in_num=atoi(number);
	sprintf(to_compare,"<<<%d>>>",in_num);      //message number to find
	sprintf(temp_buffer2,"inbox/%s.inbox",username);
	strcpy(s1,temp_buffer2);
	inbox=fopen(temp_buffer2,"r+");
	if(inbox==NULL)
	{
		single_user("No BBS account activated Yet",username);
// 		perror("\n Error in BBS open");
		return 1;
	}
	sprintf(temp_buffer,"%s",username);
	strcpy(s2,temp_buffer);
	follow=fopen(temp_buffer,"w+");
	while(1)
	{
		
		if(fgets(temp_buffer2,1024,inbox)==NULL)
		{
			flag=1;
			break;
		}
		if(strstr(temp_buffer2,to_compare)!=NULL)
		{
			
// 			strcpy(temp_buffer,temp_buffer2);
			ret_val=single_user(temp_buffer2,username);
			if(ret_val<0)
			{
// 				fprintf(stderr,"\n unable to send data");
			}
			if(strstr(temp_buffer2,"<<<unread>>>")!=NULL)
			{
				overhead=strtok(temp_buffer2," ");
				number=strtok(NULL,"\n");
				sprintf(temp_buffer,"<<<read>>> %s  \n",number);
				fprintf(follow,"%s",temp_buffer);
				flag2=1;
				continue;
			}
			break;
		}
		fprintf(follow,"%s",temp_buffer2);
	}
	
	if(flag2==1)
	{
		fclose(follow);
		fclose(inbox);
		sprintf(temp_buffer,"%s",username);

		sprintf(temp_buffer2,"inbox/%s.inbox",temp_buffer);
		inbox=fopen(s1,"w+");
		if(inbox==NULL)
		{
			single_user("BBS Service>>Some Error on Server Side",username);
// 			perror("\n Error in BBS open");
			return 1;
		}
		sprintf(temp_buffer,"%s",username);
		follow=fopen(s2,"r+");
		while(1)
		{
			if(fgets(temp_buffer2,1024,follow)==NULL)
			{
				break;
			}
			fprintf(inbox,"%s",temp_buffer2);
		}
		unlink(s2);
		remove(s2);
		fclose(follow);
		sprintf(temp_buffer,"inbox/%s.unread",s2);
		follow=fopen(temp_buffer,"r+");
		fgets(temp_buffer,1024,follow);
		overhead=strtok(temp_buffer,"=");
		total=atoi(strtok(NULL,"\n"));
		fgets(temp_buffer,1024,follow);
		overhead=strtok(temp_buffer,"=");
		un_read=atoi(strtok(NULL,"\n"));
		fclose(follow);
		sprintf(temp_buffer,"inbox/%s.unread",s2);
		follow=fopen(temp_buffer,"w+");
		fprintf(follow,"total=%d\nunread=%d",total,un_read-1);
	}
	else
	{
		unlink(s2);
		remove(s2);
	}
	fclose(follow);
	fclose(inbox);
	
	if(flag==1)
	{
		single_user("BBS Service>>Mesage not found",username);
	}
	return 1;
}
///////////////////////////////////////////////////////////////////////////
//// Semnd list of messages to user                                      //
///////////////////////////////////////////////////////////////////////////
int bbs_list(char reciever[])
{
	FILE *inbox;
	char temp_buffer[100],list[50];
	int ret_val,flag=0;
	sprintf(temp_buffer,"inbox/%s.inbox",reciever);
	inbox=fopen(temp_buffer,"r+");
	if(inbox==NULL)
	{
		return -1;
	}
	while(1)
	{
		if(fgets(temp_buffer,1024,inbox)==NULL)
		{
// 			fprintf(stderr,"\nbreaking loop");
			break;
		}
		flag=flag+1;
		snprintf(list,40,"%s",temp_buffer);
		list[40]='\0';
		ret_val=single_user(list,reciever);
		if(ret_val<0)
		{
// 			fprintf(stderr,"\n Eror in sending BBS list");
		}
	}
	sprintf(temp_buffer,"HIMS>>You have %d Messages in your account",flag);
	ret_val=single_user(temp_buffer,reciever);
	return 1;
}
////////////////////////////////////////////////////////////////////////////////////
//// Function to show list of rooms                                               //
////////////////////////////////////////////////////////////////////////////////////	
int show_rooms(char path[],int conn_socket)
{
	struct S_DIR *directory;       //structure to hold the adress of opened directory 
	struct dirent *dir;	       //structure to read the contents of directory 	
	char buffer[MAX_BUFFER+1];
	int ret_val,i=1;
	directory=(struct S_DIR *)opendir(path);     //opening the directory 
	if(directory==NULL)
	{
// 		perror("Directory:");
		return -1;
	}
	//reading contents of directory and transfering them to client
	while(1)
	{ 
		
		dir=readdir((void *)directory);
		if(dir==NULL)
		{
			
			ret_val=send(conn_socket,"end of rooms",MAX_BUFFER,0);
			if(ret_val<0)
			{
				return -1;
// 				fprintf(stderr,"\nError in sending end of directory");
			}
			break;	
		}
		//do not send trash files and backup files
		if((strcmp(dir->d_name,".")==0)||(strcmp(dir->d_name,"..")==0)||(strstr(dir->d_name,"~")!=NULL))
			continue;
		sprintf(buffer,"HIMS>>Room # %d=>%s",i,dir->d_name);
		ret_val=send(conn_socket,buffer,MAX_BUFFER,0);
		if(ret_val<0)
		{
// 			perror("Error in Sending");
			continue;
		}
		i++;
	}

	return 1;
}
//////////////////////////////////////////////////////////////////////
//// check BBS messge at login time                                 //
//// return unread messages                                         //
//////////////////////////////////////////////////////////////////////
int check_bbs(char reciever[])
{
	FILE *unread;
	char temp_buffer[1024],*overhead;
	int in_unread;
	sprintf(temp_buffer,"inbox/%s.unread",reciever);
	unread=fopen(temp_buffer,"r");
	if(unread==NULL)
	{
		return -1;
	}
	fgets(temp_buffer,1024,unread);
	fgets(temp_buffer,1024,unread);
	overhead=strtok(temp_buffer,"=");
	in_unread=atoi(strtok(NULL,"\n"));
	return in_unread;
}
//////////////////////////////////////////////////////////////////////////
//// Sending BBS to all users                                           //
//////////////////////////////////////////////////////////////////////////
int bbs_all(char sender[],char message[])
{
	FILE *reg_file;
	char temp_buffer[1024],*reciever;
	int ret_val;
	reg_file=fopen("reg_file.txt","r+");
	if(reg_file==NULL)
	{
// 		perror("\nbreg_file");
		return -1;
	}
	while(1)
	{
		if(fgets(temp_buffer,1024,reg_file)==NULL)
			break;
		reciever=strtok(temp_buffer," ");
		strcpy(temp_buffer,reciever);
		ret_val=send_bbs(sender,temp_buffer,message);
		if(ret_val<0)
		{
// 			fprintf(stderr,"\nError in sending to %s",reciever);
		}
	}
	return 1;
}
////////////////////////////////////////////////////////////////////////////////////
////Send BBS to the user                                                          //
///////////////////////////////////////////////////////////////////////////////////
int send_bbs(char sender[],char reciever[],char message[])
{
	char *msg_total,*msg_unread,*overhead;
	char temp_buffer[1024],reciever_inbox[100],reciever_unread[100],towrite[1024];
	int ret_val,flag=0,in_total,in_unread;
	FILE *inbox,*unread;
	strcpy(temp_buffer,reciever);
	ret_val=checkit(temp_buffer,"login/loged_in.txt");   //check if user is loged in or not 
	if(ret_val==1)                   //if login send message to him/her
	{
		sprintf(temp_buffer,"BBS message from %s >>> %s",sender,message);
		ret_val=single_user(temp_buffer,reciever);
		flag=1;                    //setting login flag
	}
	//Saving message to the user
	sprintf(reciever_inbox,"inbox/%s.inbox",reciever);   
	sprintf(reciever_unread,"inbox/%s.unread",reciever);
	inbox=fopen(reciever_inbox,"a+");
	if(inbox==NULL)
	{
// 		fprintf(stderr,"\n Error Opening Inbox");
		return -1;
	}
	unread=fopen(reciever_unread,"r+");       //opening unread and total file
	if(unread==NULL)      //in case new user than create files/inbox for him/her
	{
		//fclose(unread);
		unread=fopen(reciever_unread,"w+");				//In case
		fprintf(unread,"total=0\n");					// first
		fprintf(unread,"unread=0\n");					//message
// 		fprintf(stderr,"\nError opening Unread Message File");		//to 
		fclose(unread);							//user
		unread=fopen(reciever_unread,"r+");
 	}
	fgets(temp_buffer,1024,unread);         //getting info from unread file
	overhead=strtok(temp_buffer,"=");
	msg_total=strtok(NULL,"\n");
	in_total=atoi(msg_total);
	fgets(temp_buffer,1024,unread);
	overhead=strtok(temp_buffer,"=");
	msg_unread=strtok(NULL,"\n");
	in_unread=atoi(msg_unread);
	fclose(unread);
	if(in_total>MAX_MSG)    //check if number of messages exceed MAX_MSG than dont save it
	{
		single_user("BBS Service >>Message Could Not be delivered to User",sender);
		return 1;
	}
	if(flag==1)       //if message delivered to user
	{
		sprintf(towrite,"<<<read>>> <<<%d>>> <<<%s>>> %s",in_total+1,sender,message);
	}
	else      //in case message not delivered to user
	{
		sprintf(towrite,"<<<unread>>> <<<%d>>> <<<%s>>> %s",in_total+1,sender,message);
	}
	fprintf(inbox,"%s",towrite); // Saving message to the file
	fclose(inbox);
	unread=fopen(reciever_unread,"w");  //opening unread file for further ammendments
	if(unread==NULL)
	{
		//perror("temp:");
	}
	sprintf(towrite,"total=%d\n",in_total+1);
	fprintf(unread,"%s",towrite);      //writing total message to file
	if(flag==1)      //if messgae delivered to the user then dont increment unread loop
	{
			sprintf(towrite,"unread=%d",in_unread);
			fprintf(unread,"%s\n",towrite);
	}
	else
	{
		sprintf(towrite,"unread=%d\n",in_unread+1);
		fprintf(unread,"%s\n",towrite);
	}
	fclose(unread);
	return 1;
}
////////////////////////////////////////////////////////////////////////////////
// Process the BBS Command                                                    //
// Also check the message limit of 1000 charachters                           //
// Inform the sendder if length exceed                                        //
///////////////////////////////////////////////////////////////////////////////
int bbs(char command[])
{
	char *sender,*overhead,*todo,*reciever,*message;
	char temp_buffer[1024];
	int ret_val;
	
	strcpy(temp_buffer,command);
	sender=strtok(temp_buffer," ");
	overhead=strtok(NULL," ");
	todo=strtok(NULL," ");
	reciever=strtok(NULL," ");
	message=strtok(NULL,"\0");
	if(strlen(message)>=MSG_LIMIT)      //Checking the message length
	{
		single_user("HIMS[Error]>>Message Length should not excees 1000",sender);
		return 1;
	}
 	if(strstr(reciever,"all"))      //in case message is destined for all users 
 	{
 		ret_val=bbs_all(sender,message);
 		if(ret_val<0)
 		{
//  			fprintf(stderr,"\nEror Sending to all user");
 			return -1;
 		}
 		return 1;
 	}
 	else
	{
		ret_val=checkit(reciever,"reg_file.txt");
		if(ret_val<0)
		{
			return -2;
		}
		ret_val=send_bbs(sender,reciever,message);       // In case message is for single user
		if(ret_val<0) 
		{
			return -3;
		}
	}
	return 1;
}
///////////////////////////////////////////////////////////////////////////////////
// Send Message to the user on UDP socket                                        //
// Return 1 on success                                                           //
// return -1 on anay error                                                       //
///////////////////////////////////////////////////////////////////////////////////
int single_user(char message[],char reciever[])
{
	FILE *udp_file;
	int ret_val,temp_socket;
	struct sockaddr_in peer;
	char *p_ip,*p_port,*p_name;
	char buffer[1024];
	udp_file=fopen("udp_connected.txt","r+");
	temp_socket=socket(AF_INET,SOCK_DGRAM,0);
	if(temp_socket<0)
	{
// 		perror("\nError in udp socket");
		return -1;
	}
	if(udp_file==NULL)
	{
// 		fprintf(stderr,"\n Error Opening UDP FIle");
		return -1;
	}
	while(1)
	{
		if(fgets(buffer,1024,udp_file)==NULL)
		{
			return 0;
		}
		if(strstr(buffer,reciever)!=NULL)
		{
			p_name=strtok(buffer," ");
			p_ip=strtok(NULL," ");
			p_port=strtok(NULL,"\0");
			peer.sin_family=AF_INET;
			peer.sin_port=htons(atoi(p_port));
			peer.sin_addr.s_addr=inet_addr(p_ip);
			ret_val=sendto(temp_socket,message,1024,0,(struct sockaddr*)&peer,sizeof(peer));
			if(ret_val<0)
			{
// 				fprintf(stderr,"\n Sending on Udp socket");
				return -1;
			}
		}	
	}
	return 1;
}
	
////////////////////////////////////////////////////////////////////////////////////
// Check the availablitiy of any user in any file                                 //
// Return 1 if user found                                                         //
// return -1 on any error or user not found                                       //
////////////////////////////////////////////////////////////////////////////////////
int checkit(char username[],char path[])
{
	FILE *fp;
	char buffer[40];
	fp=fopen(path,"r+");
	if(fp==NULL)
	{
// 		perror("\nCheckit: file ");
		return -1;
	}
	while(1)
	{
		if(fgets(buffer,1024,fp)==NULL)
		{
			break;			
		}
		if(strstr(buffer,username)!=NULL)
		{
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return -1;
}
////////////////////////////////////////////////////////////////////////////////
//// Function used to send the list of all loged in users                     //
//// return -1 on error                                                       //
////////////////////////////////////////////////////////////////////////////////		
		
int send_list(int conn_socket,struct sockaddr_in  client,char command[])
{
	FILE *login;
	char buffer[MAX_BUFFER];
	int ret_val;
	login=fopen("login/loged_in.txt","r+");
	if(login==NULL)
	{
// 		perror("\nList Show List");
		return -1;
	}
	while(fgets(buffer,1024,login)!=NULL)
	{		
		ret_val=send_data(conn_socket,(struct sockaddr_in) client,buffer);
		if(ret_val<0)
		{
// 			fprintf(stderr,"\nError in Sending names");
			return -1;
		}
	}
	return 1;
}
//////////////////////////////////////////////////////////////////////////////////////
// Open Info File and return the Listening port and IP of user in information Filed //
//   Return -1 on any error                                                         //
//////////////////////////////////////////////////////////////////////////////////////		
int direct_chat(char command[],char information[])
{
	char *overhead,*name,temp[30],*read_name,buffer[50];
	FILE *info_file;
	overhead=strtok(command," ");
	name=strtok(NULL,"\0");
	info_file=fopen("info_file.txt","r+");
	if(info_file==NULL)
	{
// 		perror("\nDirect Chat File:");
		return -1;
	}
	while(1)
	{
		if((fgets(buffer,100,info_file))==NULL)
		{
			return -1;
		}
		strcpy(temp,buffer);
		read_name=strtok(temp," ");
		if(strstr(name,read_name)!=NULL)
 		{
			strcpy(information,buffer);
			return 1;
		}
	}
}
				

//////////////////////////////////////////////////////////////////////////////
// This function is used to create channel take simple command              //
// Return Value: 2 on newly created                                         // 
//               3 on already created room                                  //
//               -1 on any Error                                            //
//////////////////////////////////////////////////////////////////////////////		
int create_channel(char command[])
{
	FILE *room_p;
	char *room,*overhead,*username;
	struct S_DIR *directory;       //structure to hold the adress of opened directory 
	struct dirent *dir;	       //structure to read the contents of directory 	
	int ret_val;
	char buffer[MAX_BUFFER+1],current_directory[20];
	directory=(struct S_DIR *)opendir("rooms/");     //opening the directory 
	ret_val=check_limit("rooms/");
	if(ret_val==0)
	{
		return -2;
	}
	if(directory==NULL)
	{
// 		perror("Directory:");
		return -1;
	}
	overhead=strtok(command," ");
	room=strtok(NULL," ");
	username=strtok(NULL,"\0");
	while(1)          //check the status of room and perform mentioned operation 
	{ 
		
		dir=readdir((void*)directory);
		if(dir==NULL)               //if room not exist
		{
			sprintf(current_directory,"rooms/%s",room);
			//opening room
			room_p=fopen(current_directory,"w+");
			if(room_p==NULL)
			{
// 				perror("\nChannel file");
				return -1;
			}
			sprintf(buffer,"%s owner\n",username);     //preparing buffer to write on to the file 
			fprintf(room_p,"%s",buffer);
			fclose(room_p);            //writing username to the file
			return 2;
		}
		else
		{
			sprintf(current_directory,"rooms/%s",room);
			sprintf(buffer,"%s",dir->d_name);
			if(strcmp(buffer,room)==0)           //if room exist
			{
				//opening room
				room_p=fopen(current_directory,"a+");
				if(room_p==NULL)
				{
// 					perror("\nunable to open already created room");
					return -1;
				}
				fprintf(room_p,"%s\n",username);     //writing username to the file 
				fclose(room_p);
				return 3;
			}
		}
	}
	return 1;
}
////////////////////////////////////////////////////////////////////////////////////
//This Function is used to check the Number of rooms                          //////
// Return 0 if Limit Exeed than Given                                         //////
// Return 1 if Limit Doesnot Exceed                                           //////
////////////////////////////////////////////////////////////////////////////////////
int check_limit(char path[])
{
	struct S_DIR *directory;       //structure to hold the adress of opened directory 
	struct dirent *dir;
	int counter=0;
	directory=(struct S_DIR *)opendir(path);
	if(directory==NULL)
	{
//		perror("Directory:");
		return 0;
	}
	while(1)          //check the status of room and perform mentioned operation 
	{ 
		
		dir=readdir((void*)directory);
		if(dir==NULL)               //if room not exist
		{
			break;
		}
		counter++;
	}
	if(dir==NULL&&counter>=ROOM_LIMIT)
	{
		return 0;
	}
	return 1;
}
// /////////////////////////////////////////////////////////////////////////
// Register the user							  //
// Return -2 if user already exist					  //
//        -1 in case of any error					  //
//         1 on success 						  //
////////////////////////////////////////////////////////////////////////////
int register_user(char command[])
{
	int ret_val;
	char *name,*password,*overhead;
	char temp_buffer[1024];
	strcpy(temp_buffer,command);
	FILE *fp;
	overhead=strtok(temp_buffer," ");
	name=strtok(NULL," ");
	password=strtok(NULL,"\0");
	ret_val=checkit(name,"reg_file.txt");
	if(ret_val==1) //in case user already register
	{
		return -2;
	}
	fp=fopen("reg_file.txt","a+");
	if(fp==NULL)
	{
//		perror("Reg File:");
		return -1;
	}
	fprintf(fp,"%s %s",name,password);
	fclose(fp);
	return 1;
}
///////////////////////////////////////////////////////////////////////
// Simply Send Data to the Client                                    //
//  return -1 on error and 1 on success                              //
///////////////////////////////////////////////////////////////////////
int send_data(int conn_socket,struct sockaddr_in  client,char buffer[])
{
	int ret_val;
	socklen_t length;
	length=sizeof(client);

	ret_val=sendto(conn_socket,buffer,strlen(buffer),0,(struct sockaddr*)&client,length);
	if(ret_val<0)
	{
//		fprintf(stderr,"\nError in sending");
		return -1;
	}
	return 1;
}
