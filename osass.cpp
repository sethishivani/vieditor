#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

#include <iostream>
#include <sys/types.h>

using namespace std;

#define CTRL_KEY(k) ((k) & 0x1f)
/******************************************linked list********************************************/
struct ll{
	char num;
	struct ll* next;
	};
struct ll *head=NULL,*insmodehead=NULL;
struct editorConfig {
  int cx, cy;
  int screenrows;
  int screencols;
  struct termios orig_termios;
};
bool replaceflag=0,exitnormode=0,exitinsmode=0,headflag=0,insertwriteflag=0,insertdelflag=0,insertbkspflag=0,newfile=0;
void insertmode();
void commandmode(char []);
struct editorConfig E;
vector<struct ll*> col;
void insert(char data)
{
	struct ll* nn=(struct ll*)malloc(sizeof(struct ll));
	struct ll* ptr;
	nn->next=NULL;
	nn->num=data;
	if(head==NULL)
		head=nn;
	else
	{
		ptr=head;
		while(ptr->next!=NULL)
			ptr=ptr->next;
		ptr->next=nn;
	}
}
void insmodeinsert(char data)
{
	struct ll* nn=(struct ll*)malloc(sizeof(struct ll));
	//struct ll* ptr;
	nn->num=data;
	if(headflag==1)
	{
		nn->next=col[E.cy-1];
		col[E.cy-1]=nn;
		insmodehead=nn;
		//headflag=0;
	}
	else
	{
		nn->next=head->next;
		head->next=nn;
	}
	
}
void del(char data)
{
	struct ll *ptr,*pptr;
	ptr=head;
	while(ptr->num!=data)
	{	pptr=ptr;
		ptr=ptr->next;
	}	
	if(ptr==head)
	{
		col[E.cy-1]=head->next;
		free(ptr);
	}
	else
	{
		pptr->next=ptr->next;
		free(ptr);
	}
}
void dellinklist()
{
	struct ll* ptr=head;
	while(head!=NULL)
	{	
		ptr=head;		
		head=head->next;
		free(ptr);
	}	

}
void search(char data)
{
	struct ll *ptr;
	ptr=head;
	while(ptr!=NULL&&ptr->num!=data)
		ptr=ptr->next;
	if(ptr==NULL)
		printf("not found");
	else
		printf("found");
}
void display()
{
	struct ll *ptr;
	ptr=head;
	while(ptr!=NULL){
		printf("%d\t",ptr->num);
		ptr=ptr->next;
	}
	printf("\n");
}
/*****************************************************display the text in ds****************************************************/
void disp()
{
	write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);
	int lim=col.size();
	for(int i=0;i<lim;i++)
	{
		head=col[i];
		while(head!=NULL)
		{
			fprintf(stderr,"%c",head->num);
			head=head->next;
		}
		fprintf(stderr,"\n");
	}
	write(STDOUT_FILENO, "\x1b[H", 3);
}

/*****************************************************data strucure created***********************************************/
void ds(char ch)
{	if(newfile==1)
	fprintf(stderr,"%c",ch);
	if(ch=='\n')
	{
		//E.cy+=1;
		//E.cx=1;
		head=NULL;
		return;
	}
	if(head==NULL)
	{
		insert(ch);
		col.push_back(head);
	}
	else
	{
		insert(ch);
	}
	
}
/*****************************************************delete the linked lists*******************************************************/
void delstruct()
{
	for(int i=0;i<col.size();i++)
	{
		head=col[i];
		dellinklist();
	}
	col.clear();
	//printf("\ncleaned\n");
}

/******************************************************Reading from a file********************************************************/
void readfile(char argv[] ) {
	head=NULL;
  	FILE *fp = fopen(argv, "r");
  	if (!fp) 
	{
		printf("couldn't open file\n");
		exit(0);
	}
	char ch;
	ch = fgetc(fp);

    	while (ch != EOF){
		ds(ch);
	        ch = fgetc(fp);
    	}
	disp();
	fprintf(stderr, "\x1b[%d;%dH%s", E.screenrows, 1,"\x1b[2K");
	fprintf(stderr, "\x1b[%d;%dH%s", E.screenrows, 1,argv);
	fprintf(stderr, "\x1b[%d;%dH", E.cy,E.cx);
	//atexit(delstruct());
	fclose(fp);
	return;
}
/******************************************************window size****************************************************/
int getWindowSize(int *rows, int *cols) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    return -1;
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}
/*****************************************************replace function(after replace reprint)****************************************************/
void rep()
{
  //we have E.cx,E.cy
  char c;
  read(STDIN_FILENO, &c, 1);
  head=col[E.cy-1];
  //head=col[3];
  int x=1,temp=E.cx;
  while(x!=temp)
  {
	x++;
	head=head->next;
  }
  head->num=c;
  write(STDOUT_FILENO, "\x1b[H", 3);
  write(STDOUT_FILENO, "\x1b[2J", 4);
  disp();
}
/******************************************************Arrow keys mapping******************************************************/


void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  perror(s);
  exit(1);
}

char editorReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  return c;
}
void editorMoveCursor(char key) {
  switch (key) {
    case 'h':if(E.cx>1)
      		E.cx--;
      		break;
    case 'l':if(E.cx<E.screencols)
      		E.cx++;
      		break;
    case 'k':if(E.cy>1)
      		E.cy--;
      		break;
    case 'j':if(E.cy<E.screenrows)
      		E.cy++;
      		break;
  
  
  }
}

void editorProcessKeypress(char argv[]) {
  char c;
  while(1){
  c = editorReadKey();
  switch (c) {
    case CTRL_KEY('q'):
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exitnormode=1;
      //exit(0);
      break;
    case 'h':
    case 'j':
    case 'k':
    case 'l':
      editorMoveCursor(c);
      break;
    case 'r':replaceflag=1;
	rep();
      break;
    case 'G':E.cx=1;
	E.cy=col.size();
	break;
    case 'g':read(STDIN_FILENO, &c, 1);
	if(c=='g')
	{E.cx=1;
	E.cy=1;}
	break;
    case 'i'://go to insert mode
	fprintf(stderr, "\x1b[%d;%dH%s", E.screenrows, 1,"\x1b[2K");
	fprintf(stderr, "\x1b[%d;%dH%s", E.screenrows, 1,"--Insert--");
	fprintf(stderr, "\x1b[%d;%dH", E.cy, E.cx);
	insertmode();
    case ':'://go to command mode
	fprintf(stderr, "\x1b[%d;%dH%s", E.screenrows, 1,"\x1b[2K");
	fprintf(stderr, "\x1b[%d;%dH%c", E.screenrows, 1,':');
	fprintf(stderr, "\x1b[%d;%dH", E.cy, E.cx);
	commandmode(argv);
  }
  fprintf(stderr, "\x1b[%d;%dH", E.cy, E.cx);
  if(exitnormode==1)
	break;
}
}

void initEditor() {
  E.cx = 1;
  E.cy = 1;
  if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
}
/*****************************************************enable and disable raw mode***************************************************/
void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
    die("tcsetattr");
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
  atexit(disableRawMode);
  struct termios raw = E.orig_termios;
  raw.c_lflag &= ~(ICANON|ECHO);
  raw.c_iflag &= ~(IXON);
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}
/*********************************************************Write into file from backend structure*****************************************************/
void backendwrite(char argv[])
{
   FILE *fp = fopen(argv, "w");
   int x=0;
 	 while (x<col.size()) 
 	 {
 	  	head=col[x];
		x++;
  		while (head!=NULL) 
		{
			fprintf(fp,"%c",head->num);
			head=head->next;
		}
		fprintf(fp,"\n");
  	}
 	 fclose(fp);
}
//////////////////////////////////////////////////////*INSERT MODE SECTION*////////////////////////////////////////////////////////
////////////////////////////////////////////////////////*insert mode delete*//////////////////////////////////////////////////
void insmodedel()
{
	struct ll *pptr,*ptr=col[E.cy-1];
	int x=1;
	if(E.cx!=1)
	{pptr=ptr;
	while(x!=E.cx)
	{
		x++;
		pptr=ptr;
		ptr=ptr->next;
			
	}
	pptr->next=ptr->next;}
	else{
		col[E.cy-1]=col[E.cy-1]->next;
	}
	free(ptr);
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);
	disp();
	fprintf(stderr, "\x1b[%d;%dH", E.cy, E.cx);
}
/////////////////////////////////////////////////////////*insert mode backspace*//////////////////////////////////////////////
void insmodebcksp()
{
	struct ll *pptr,*ptr=col[E.cy-1];
	if(E.cx==1)
		return;
	if(E.cx==2)
	{
		col[E.cy-1]=col[E.cy-1]->next;
	}
	else if(E.cx>2){	
	int x=1;
	pptr=ptr;
	while(x!=(E.cx-1))
	{
		x++;
		pptr=ptr;
		ptr=ptr->next;
			
	}
	pptr->next=ptr->next;}
	free(ptr);
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);
	disp();
	E.cx-=1;
	fprintf(stderr, "\x1b[%d;%dH", E.cy, E.cx);
}
/////////////////////////////////////////////////////////*main insert mode*///////////////////////////////////////////////////
void insertmode()
{
	//disableRawMode();
	insertwriteflag=1;
	struct ll *ptr,*insmodehead;
	char c;
	if(newfile==0)
	{
		ptr=col[E.cy-1];
		insmodehead=ptr;
		int x=1;
		while(x!=E.cx)
		{
		x++;
		insmodehead=ptr;
		ptr=ptr->next;
			
		}
		if(x==1)
		headflag=1;
	}
	while(1){
	read(STDIN_FILENO, &c, 1);
	switch(c)
	{
		case('\x7f'):fprintf(stderr, "\x1b[%d;%dH%s", E.screenrows, 1,"\x1b[2K");
			fprintf(stderr, "\x1b[%d;%dH", E.cy, E.cx);
			insertbkspflag=1;
			insmodebcksp();
			break;
		case ('\x1b'):read(STDIN_FILENO, &c, 1);
			if(c=='[')
			{read(STDIN_FILENO, &c, 1);
			 if(c=='3')
				{read(STDIN_FILENO, &c, 1);
				 if(c=='~'){
					insertdelflag=1;
					insmodedel();}	
				}
			}
				
			else exitinsmode=1;
			break;
		default:if(newfile)
			{
				ds(c);
			}
			else{
			head=insmodehead;
			insmodeinsert(c);
			insmodehead=insmodehead->next;
			if(headflag==1){
			insmodehead=col[E.cy-1];}
			headflag=0;
			write(STDOUT_FILENO, "\x1b[2J", 4);
			write(STDOUT_FILENO, "\x1b[H", 3);
			disp();
			E.cx+=1;
			fprintf(stderr, "\x1b[%d;%dH", E.cy, E.cx);}
			break;
	}
	if(exitinsmode)
	{
		exitinsmode=0;
		newfile=0;
		break;
	}
   }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~COMMAND MODE SECTION~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

void commandmode(char argv[])
{
	char c,ch[10];
	read(STDIN_FILENO, &c, 1);
	int x;
	fprintf(stderr, "\x1b[%d;%dH%c", E.screenrows, 2,c);
	switch(c)
	{
	case 'w':backendwrite(argv);
		fprintf(stderr, "\x1b[%d;%dH%s", E.screenrows, 1,"\x1b[2K");
		fprintf(stderr, "\x1b[%d;%dH%s", E.screenrows, 1,"File Written");
		replaceflag=0;
		insertwriteflag=0;
		insertdelflag=0;
		insertbkspflag=0;
		break;
	case 'q':read(STDIN_FILENO, &c, 1);
		if(c=='\n')
		{
			if(replaceflag==1||insertwriteflag==1||insertdelflag==1||insertbkspflag)
			{
				//show warning
				fprintf(stderr, "\x1b[%d;%dH%s", E.screenrows, 1,"\x1b[2K");
				fprintf(stderr, "\x1b[%d;%dH%s", E.screenrows, 1,"E37: No write since last change (add ! to override)");
			}
			else 
			{
				//quit
				exitnormode=1;
				write(STDOUT_FILENO, "\x1b[2J", 4);
  				write(STDOUT_FILENO, "\x1b[H", 3);
			}
		}
		else if(c=='!'){exitnormode=1;
			replaceflag=0;
			insertwriteflag=0;
			insertdelflag=0;
			insertbkspflag=0;
			write(STDOUT_FILENO, "\x1b[2J", 4);
  			write(STDOUT_FILENO, "\x1b[H", 3);
			}
		break;
	case '!'://execute the system command
		int i=0;
		int stemp=3;
		while(1)
		{
			read(STDIN_FILENO, &c, 1);
			if(c!='\n')
			{fprintf(stderr, "\x1b[%d;%dH%c", E.screenrows, stemp,c);
			stemp++;
			ch[i]=c;
			i++;}
			else
			break;
		}
		ch[i]='\0';
		char* args[3];
		args[0]=ch;	
		args[1]=NULL;
		int pid=fork();
		if(pid==0)
		{
			write(STDOUT_FILENO, "\x1b[2J", 4);
  	        	write(STDOUT_FILENO, "\x1b[H", 3);
			if(execvp(args[0],args)==-1)
			{ perror("exec");
			}
			
		}
		if(pid>0)
		{	
			wait(0);
			write(STDOUT_FILENO, "Press any key to continue", 25);
			read(STDIN_FILENO, &c, 1);
			write(STDOUT_FILENO, "\x1b[2J", 4);
  	        	write(STDOUT_FILENO, "\x1b[H", 3);
			disp();
		}
		 }
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/******************************************************Read Character from screen***************************************************/

int main(int argc,char* argv[]) {
  enableRawMode();
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  initEditor();
  FILE *fp = fopen(argv[1], "r");
  if(fp!=NULL)
  	readfile(argv[1]);
  else
	{//writefile(argv[1]);
	 fprintf(stderr, "\x1b[%d;%dH%s", E.screenrows, 1,"\x1b[2K");
	 fprintf(stderr, "\x1b[%d;%dH%s", E.screenrows, 1,argv[1]);
	 fprintf(stderr, "\x1b[%d;%dH", E.cy,E.cx);
	 newfile=1;
	}
  initEditor();
  editorProcessKeypress(argv[1]);
  exitnormode=0;
  delstruct();
  return 0;
}	
