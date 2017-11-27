#define MAXINODE 50
#define READ 1
#define WRITE 2
#define MAXFILESIZE 1024
#define START 0
#define REGULAR 1
#define CURRENT 1
#define END 2


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>

typedef struct superblock
{
	int totalinode;
	int freeinode;
}SUPERBLOCK,*PSUPERBLOCK;


typedef struct inode 	
{
	char filename[50];
	int inodenumber;
	int filesize;
	int fileactualsize;
	int filetype;
	char * buffer;
	int linkcount;
	int referancecount;
	int permissions;
	struct inode * next;
}INODE,*PINODE;


typedef struct filetable
{
	int readoffset;
	int writeoffset;
	int count;
	int mode;
	PINODE * ptrinode;
}FILETABLE,*PFILETABLE;


typedef struct ufdt
{
	PFILETABLE ptrfiletable;
}UFDT;


UFDT UFDTarr[50];
SUPERBLOCK SUPERBLOCKobj;
PINODE head=NULL;

void man(char * name)
{	
	if(name == NULL)
	{
		return ;
	}
	else if(strcmp(name,"name")==0)
	{
		printf("Description: Used to create a new regular file \n"); 
		printf("Usage : create file_name permissions \n");	
	}
	else if(strcmp(name,"read")==0)
	{
		printf("Description: Used to read data from regular file \n"); 
		printf("Usage : read file_name numeber_of_bytes_to_read\n");
	}
	else if(strcmp(name,"write")==0)
	{
		printf("Description: Used to write data in to  a new regular file \n"); 
		printf("Usage : write file_name numeber_of_bytes_to_write\n");
	}
	else if(strcmp(name,"ls")==0)
	{
		printf("Description: Used to list all information of the file\n ");
		printf("Usage : ls\n");
	}
	else if(strcmp(name,"stat")==0)
	{
		printf("Description : Used to display information of file \n ");
		printf("Usage : stat file_name\n");
	}
	else if(strcmp(name,"fstat")==0)
	{
		printf("Description : Used to display information of file \n ");
		printf("Usage : stat file_descriptor\n");
	}
	else if(strcmp(name,"truncate")==0)
	{
		printf("Description : Used to remove data from  file \n ");
		printf("Usage : truncate file_name\n");
	}
	else if(strcmp(name,"open")==0)
	{
		printf("Description : Used to open existing file \n ");
		printf("Usage : open file_name mode");
	}
	else if(strcmp(name,"close")==0)
	{
		printf("Description : Used to close all existing file \n ");
		printf("Usage : closeall\n");
	}
	else if(strcmp(name,"lseek")==0)
	{
		printf("Description : Used to chaneg the offset of the file \n ");
		printf("Usage : lseek file_name change_in_offset start_point");
	}
	else if(strcmp(name,"rm")==0)
	{
		printf("Description : Used to delete the file \n ");
		printf("Usage : rm file_name");
	}
	else 
	{
		printf("No manual entry available");
	}



}

void displayhelp()
{
	printf("ls : To list out all files\n");
	printf("clear : To clear console\n");
	printf("open : To open the file\n");
	printf("close : To close the file\n");
	printf("closeall : To close all opened files\n");
	printf("read : To read the contents from files\n");
	printf("write : To write the contents into files\n");
	printf("exit : To terminate filesystem\n");
	printf("stat : To display information of the file using name\n");
	printf("fstat : To display information of file using file_descriptor\n");
	printf("truncate : To remove all data from file\n");
	printf("rm : To delete the file\n");
}
	

void initialize_super_block()
{
	int i=0;
	while(i<50)
	{	
		UFDTarr[i].ptrfiletable=NULL;
		i++;
	}
	SUPERBLOCKobj.totalinode=MAXINODE;
	SUPERBLOCKobj.freeinode=MAXINODE;
}


void create_dilb()
{
	int i=1;
	PINODE newnode=NULL;
	PINODE temp=head;

	while(i <= MAXINODE)
	{	
		newnode=(PINODE)malloc(sizeof(INODE));
		newnode->inodenumber=i;
		newnode->filesize=0;
		newnode->filetype=0;
		newnode->buffer=NULL;	
		newnode->linkcount=0;
		newnode->referancecount=0;
		newnode->next=NULL;
		if(temp==NULL)
		{
			head=newnode;
			temp=head;
		}
		else
		{
			temp->next=newnode;
			temp=temp->next;
		}
		i++;
	}

}

int createfile(char * name, int permission)
{
	int i=0;
	PINODE temp=head;
	if(name ==NULL || permission ==0 || permission >3)
	{
		return -1;
	}
	if(SUPERBLOCKobj.freeinode==0)
	{
		return -2;
	}
	if(getinode(name)!=NULL)
	{
		return -3;
	}

	(SUPERBLOCKobj.freeinode)--;
	
	while(temp!=NULL)
	{
		if(temp->filetype==0)
		{
			break;
		}
		temp = temp->next;
	}
	while(i<50)
	{
		if(UFDTarr[i].ptrfiletable==NULL)
		{
			break;
		}
		i++;
	}

	UFDTarr[i].ptrfiletable=(PFILETABLE)malloc(sizeof(FILETABLE));
	if(UFDTarr[i].ptrfiletable=NULL)
	{
		return -4;
	}

	UFDTarr[i].ptrfiletable->count =1;
	UFDTarr[i].ptrfiletable->mode =permission;
	UFDTarr[i].ptrfiletable->readoffset =0;
	UFDTarr[i].ptrfiletable->writeoffset =0;

	UFDTarr[i].ptrfiletable->ptrinode =temp;
	
	strcpy(UFDTarr[i].ptrfiletable->ptrinode->filename,name);
	UFDTarr[i].ptrfiletable->ptrinode->inodenumber
	UFDTarr[i].ptrfiletable->ptrinode->referancecount =1;
	UFDTarr[i].ptrfiletable->ptrinode->linkcount =1;
	UFDTarr[i].ptrfiletable->ptrinode->filetype=REGULAR;
	UFDTarr[i].ptrfiletable->ptrinode->filesize =MAXFILESIZE;
	UFDTarr[i].ptrfiletable->ptrinode->fileactualsize =0;
	UFDTarr[i].ptrfiletable->ptrinode->permission =permission;
	UFDTarr[i].ptrfiletable->ptrinode->buffer =(char*)malloc(MAXFILESIZE);
	
	memset(	UFDTarr[i].ptrfiletable->buffer,0,1024);	

	return i;
}

int main()
{
	char * ptr=NULL;
	int ret=0,fd=0,count=0;
	char command[4][80],str[80],arr[1024];
	initialize_super_block();
	create_dilb();

	while(1)
	{
		strcpy(str,"");
		printf("\nVirtual File System : \n");
		fgets(str,80,stdin);

		count=sscanf(str,"%s%s%s%s",command[0],command[1],command[2],command[3]);
		if(count==1)
		{
			
		}
		else if(count==2)
		{
			
		}
		else if(count==3)
		{
			if(strcmp(command[0],"create")==0)
			{
				ret=createfile(command[1],atoi(command[2]));
			}
			else if(ret>=0)
			{
				printf("File is successfully created with the file descriptor : %d\n",ret);
			}
			else if(ret==-1)
			{
				printf("Error : Incorrect Parameter\n");
			}
			else if(ret==-2)
			{
				printf("Error : There is no Inodes\n");
			}
			else if(ret==-3)
			{
				printf("Error : File Already Exists\n");
			}
			else if(ret==-4)
			{
				printf("Error : Memory allocation failure\n");
			}
			else 
			{
				printf("Unknown error\n");
			}
		}
	}
	
return 0;
}






