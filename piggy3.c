#ifndef unix
#define WIN32
#include <windows.h>
#include <winsock.h>
#else
#define closesocket close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#define PROTOPORT 36774 /* default protocol port number */
#define QLEN 6 /* size of request queue */

int Head (int rrport, char *rraddr);
int Tail (int llport);
int Middle (int llport, int rrport, char *rraddr);

/*Server/Client vars*/
int port; /* protocol port number */
int alen; /* length of address */
struct hostent *ptrh; /* pointer to a host table entry */
struct protoent *ptrp; /* pointer to a protocol table entry */
struct sockaddr_in sad; /* structure to hold an IP address */
struct sockaddr_in cad; /* structure to hold client's address */
static int sd, sd2, sd3; /* socket descriptor */
char *host; /* pointer to host name */
int n; /* number of characters read */
char buf[1000]; /* buffer for data from the server */

int main(int argc, char *argv[]) {

  int head, tail = 0;
  memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
  sad.sin_family = AF_INET; /* set family to Internet */
  sad.sin_addr.s_addr = INADDR_ANY; /* set the local IP address */
  if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
  fprintf(stderr, "cannot map \"tcp\" to protocol number");
  exit(EXIT_FAILURE);
  }

/*loop getting option arguments*/
  int c;
  //args
  int llport, rrport = 0;
  char *rraddr = NULL;
  char *display = NULL;
  char *output = NULL;


  while (1) {
    int option_index = 0;
    static struct option long_options[] = {
      {"noleft",  0, 0, 1},
      {"noright", 0, 0, 2},
      {"llport",  1, 0, 3},
      {"rraddr",  1, 0, 4},
      {"rrport",  1, 0, 5},
      {"outputl", 0, 0, 6},
      {"outputr", 0, 0, 7},
      {"output",  0, 0, 8},
      {"dsplr",   0, 0, 9},
      {"dsprl",   0, 0, 10},
      {"display", 0, 0, 11},
      {"dropr",   0, 0, 12},
      {"dropl",   0, 0, 13},
      {"right",   0, 0, 14},
      {"left",    0, 0, 15},
      {"loopr",   0, 0, 16},
      {"loopl",   0, 0, 17},
      {0,         0, 0, 0}
    };
    c = getopt_long_only(argc, argv, "", long_options, &option_index);
    if (c == -1) {
      break;
    }
    switch (c){

      case 1: //HEAD
        //read what is typed at the keyboard and write it out to right side.
	      head = 1;
        break;


      case 2: //TAIL
        //reads data from the left side connection and writes it out to screen.
	      tail = 1;
        break;


      case 3: //LLPORT
        //take the port address specified if provided and listen there.
        llport = atoi(optarg);
        break;


      case 4: //RRADDR
        //take the ipaddr specified and call gethostbyname(addr) to connect.
        rraddr = optarg;
        break;


      case 5: //RRPORT
        rrport = atoi(optarg);
        break;

      case 6: //OUTPUTL
        //SET THE OUTPUT DIRECTION TO LEFT
        output = "left";
        break;

      case 7: //OUTPUTR
      //SET THE OUTPUT DIRECTION TO RIGHT
        output = "right";
        break;

      case 8: //OUTPUT
      //SHOW WHAT DIRECTION THE OUTPUT IS SET TO
        printf("%s\n", output);
        break;

      case 9: //DSPLR
      //DISPLAY THE LEFT TO RIGHT DATA STREAM ON SCREEN
        display = "lr";
        break;

      case 10: //DSPRL
      //DISPLAY THE RIGHT TO LEFT DATA STREAM ON SCREEN
        display = "rl";
        break;

      case 11: //DISPLAY
      //SHOW THE DIRECTION THE DISPLAY IS SET TO
        printf("%s\n", display);
          break;

      case 12: //DROPR
      //DROP THE RIGHT SIDE CONNECTION
        break;

      case 13: //DROPL
      //DROP THE LEFT SIDE CONNECTION
        break;

      case 14: //RIGHT
      //SHOW INFORMATION ABOUT THE RIGHT SIDE CONNECTION
        break;

      case 15: //LEFT
      //SHOW INFORMATION ABOUT THE LEFT SIDE CONNECTION
        break;

      case 16: //LOOPR
      //TAKE DATA WRITTEN OUT TO RIGHT SIDE AND INJECT IT TO DATA FLOWING TO LEFT SIDE
        break;

      case 17: //LOOPL
      //TAKE DATA WRITTEN OUT TO LEFT SIDE AND INJECT IT TO DATA FLOWING TO RIGHT SIDE
        break;

      case ':':
        //missing option argument
        break;
      case '?':
        //invalid argument
        break;

      default:
        printf("?? getopt returned character code 0%o ??\n", c);
        break;
    }
  }


  if (optind < argc) {
    printf("non-option ARGV-elements: ");
    while (optind < argc){
      printf("%s ", argv[optind++]);
    }
    printf("\n");
  }
  if (tail == 1){
    Tail(llport);
  }else if (head == 1){
    Head(rrport, rraddr);
  }else{
    Middle(llport, rrport, rraddr);
  }
}



int Head (int rrport, char *rraddr){
  /*Check to make sure rrport is legit*/
  if (rrport == 0) {
    rrport = PROTOPORT;
  }
  sad.sin_port = htons((u_short)rrport);

  /*Gets host addr*/
  ptrh = gethostbyname(rraddr);
  if ( ((char *)ptrh) == NULL ) {
    fprintf(stderr,"invalid host: %s\n", rraddr);
    exit(EXIT_FAILURE);
  }
  /*Puts host addr and length into addr struct*/
  memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

  /*Create a socket*/
  sd  = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
  if(sd < 0){
    fprintf(stderr, "socket creation failed\n");
  }

  /* Connect the socket to the specified LISTENING server. */
  if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
    fprintf(stderr,"connect failed\n");
    exit(EXIT_FAILURE);
  }
  while (1) {
    send(sd,buf,strlen(buf),0);
  }

  return 1;

}


int Tail (int llport) {
  /*Set llport legit value*/
  if (llport == 0) {
    llport = PROTOPORT;
  }
  sad.sin_port = htons((u_short)llport);

  /*Create a socket*/
  sd  = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
  if(sd < 0){
    fprintf(stderr, "socket creation failed\n");
  }

  /*Bind to local addr*/
  if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
    fprintf(stderr,"bind failed\n");
    exit(EXIT_FAILURE);
  }

  if (listen(sd, QLEN) < 0) {
  fprintf(stderr,"listen failed\n");
  exit(EXIT_FAILURE);
  }

  while (1) {
    alen = sizeof(cad);
    if ( (sd2=accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
      fprintf(stderr, "accept failed\n");
      exit(EXIT_FAILURE);
    }
    /* Repeatedly read data from socket and write to user's screen. */
    n = recv(sd, buf, sizeof(buf), 0);
    while (n > 0) {
    write(1,buf,n);
    n = recv(sd, buf, sizeof(buf), 0);
    }
  }
  return 1;

}


int Middle (int llport, int rrport, char *rraddr){
  printf("Middle\n");

  /*Set llport legit value*/
  if (llport == 0) {
    llport = PROTOPORT;
  }
  sad.sin_port = htons((u_short)llport);


  /*Create a socket*/
  sd3  = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
  if(sd3 < 0){
    fprintf(stderr, "socket creation failed\n");
  }

  /*Bind to local addr*/
  if (bind(sd3, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
    fprintf(stderr,"bind failed\n");
    exit(EXIT_FAILURE);
  }

  if (listen(sd3, QLEN) < 0) {
  fprintf(stderr,"listen failed\n");
  exit(EXIT_FAILURE);
  }

  while (1) {
    alen = sizeof(cad);
    if ( (sd2=accept(sd3, (struct sockaddr *)&cad, &alen)) < 0) {
      fprintf(stderr, "accept failed\n");
      exit(EXIT_FAILURE);
    }
    /* Repeatedly read data from socket and write to user's screen. */
    n = recv(sd3, buf, sizeof(buf), 0);
    while (n > 0) {
    write(1,buf,n);
    n = recv(sd3, buf, sizeof(buf), 0);
    }
  }


  /*END OF LISTENING LEFT SIDE*/
  /*START OF CONNECTING RIGHT SIDE*/


  /*Check to make sure rrport is legit*/
  if (rrport == 0) {
    rrport = PROTOPORT;
  }
  sad.sin_port = htons((u_short)rrport);

  /*Gets host addr*/
  ptrh = gethostbyname(rraddr);
  if ( ((char *)ptrh) == NULL ) {
    fprintf(stderr,"invalid host: %s\n", rraddr);
    exit(EXIT_FAILURE);
  }
  /*Puts host addr and length into addr struct*/
  memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

  /* Connect the socket to the specified LISTENING server. */
  if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
    fprintf(stderr,"connect failed\n");
    exit(EXIT_FAILURE);
  }
  return 1;

}
