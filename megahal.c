
/*===========================================================================*/

/*
 *  Copyright (C) 1998 Jason Hutchens
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the license or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the Gnu Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*===========================================================================*/

/*
 *		$Id: megahal.c,v 1.23 1998/05/19 03:02:02 hutch Exp hutch $
 *
 *		File:			megahal.c
 *
 *		Program:		MegaHAL v8r6
 *
 *		Purpose:		To simulate a natural language conversation with a psychotic
 *						computer.  This is achieved by learning from the user's
 *						input using a third-order Markov model on the word level.
 *						Words are considered to be sequences of characters separated
 *						by whitespace and punctuation.  Replies are generated
 *						randomly based on a keyword, and they are scored using
 *						measures of surprise.
 *
 *		Author:		Mr. Jason L. Hutchens
 *
 *		WWW:			http://ciips.ee.uwa.edu.au/~hutch/hal/
 *
 *		E-Mail:		hutch@ciips.ee.uwa.edu.au
 *
 *		Contact:		The Centre for Intelligent Information Processing Systems
 *						Department of Electrical and Electronic Engineering
 *						The University of Western Australia
 *						AUSTRALIA 6907
 *
 *		Phone:		+61-8-9380-3856
 *
 *		Facsimile:	+61-8-9380-1168
 *
 *		Notes:		This file is best viewed with tabstops set to three spaces.
 *
 *		Compilation Notes
 *		=================
 *
 *		When compiling, be sure to link with the maths library so that the
 *		log() function can be found.
 *
 *		On the Macintosh, add the library SpeechLib to your project.  It is
 *		very important that you set the attributes to Import Weak.  You can
 *		do this by selecting the lib and then use Project Inspector from the
 *		Window menu.
 *
 *		CREDITS
 *		=======
 *
 *		Amiga (AmigaOS)
 *		---------------
 *		Dag Agren (dagren@ra.abo.fi)
 *
 *		DEC (OSF)
 *		---------
 *		Jason Hutchens (hutch@ciips.ee.uwa.edu.au)
 *
 *		Macintosh
 *		---------
 *		Paul Baxter (pbaxter@assistivetech.com)
 *		Doug Turner (dturner@best.com)
 *
 *		PC (Linux)
 *		----------
 *		Jason Hutchens (hutch@ciips.ee.uwa.edu.au)
 *
 *		PC (OS/2)
 *		---------
 *		Bjorn Karlowsky (?)
 *
 *		PC (Windows 3.11)
 *		-----------------
 *		Jim Crawford (pfister_@hotmail.com)
 *
 *		PC (Windows '95)
 *		----------------
 *		Jason Hutchens (hutch@ciips.ee.uwa.edu.au)
 *
 *		PPC (Linux)
 *		-----------
 *		Lucas Vergnettes (Lucasv@sdf.lonestar.org)
 *
 *		SGI (Irix)
 *		----------
 *		Jason Hutchens (hutch@ciips.ee.uwa.edu.au)
 *
 *		Sun (SunOS)
 *		-----------
 *		Jason Hutchens (hutch@ciips.ee.uwa.edu.au)
 */

/*===========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#if !defined(AMIGA) && !defined(__mac_os)
#include <malloc.h>
#endif
#include <string.h>
#include <signal.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#if defined(__mac_os)
#include <types.h>
#include <Speech.h>
#else
#include <sys/types.h>
#endif
#include "megahal.h"
#if defined(DEBUG)
#include "debug.h"
#endif
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

/*===========================================================================*/

void add_aux(MODEL *, DICTIONARY *, STRING);
void add_key(MODEL *, DICTIONARY *, STRING);
void add_node(TREE *, TREE *, int);
void add_swap(SWAP *, char *, char *);
TREE *add_symbol(TREE *, BYTE2);
BYTE2 add_word(DICTIONARY *, STRING);
int babble(MODEL *, DICTIONARY *, DICTIONARY *);
bool boundary(char *, int);
void capitalize(char *);
void changevoice(DICTIONARY *, int);
void change_personality(DICTIONARY *, int, MODEL **);
int comeco(char *orig, char *dest,int num);
void delay(char *);
void die(int);
bool dissimilar(DICTIONARY *, DICTIONARY *);
void error(char *, char *, ...);
float evaluate_reply(MODEL *, DICTIONARY *, DICTIONARY *);
COMMAND_WORDS execute_command(DICTIONARY *, int *);
void exithal(void);
TREE *find_symbol(TREE *, int);
TREE *find_symbol_add(TREE *, int);
BYTE2 find_word(DICTIONARY *, STRING);
char *format_output(char *);
void free_dictionary(DICTIONARY *);
void free_model(MODEL *);
void free_tree(TREE *);
void free_word(STRING);
void free_words(DICTIONARY *);
char *generate_reply(MODEL *, DICTIONARY *);
void help(void);
void ignore(int);
void initialize_context(MODEL *);
void initialize_dictionary(DICTIONARY *);
bool initialize_error(char *);
DICTIONARY *initialize_list(char *);
#ifdef __mac_os
bool initialize_speech(void);
#endif
bool initialize_status(char *);
SWAP *initialize_swap(char *);
void learn(MODEL *, DICTIONARY *);
void listvoices(void);
void load_dictionary(FILE *, DICTIONARY *);
bool load_model(char *, MODEL *);
void load_personality(MODEL **);
void load_tree(FILE *, TREE *);
void load_word(FILE *, DICTIONARY *);
void lower(char *string);
void make_greeting(DICTIONARY *);
DICTIONARY *make_keywords(MODEL *, DICTIONARY *);
char *make_output(DICTIONARY *);
void make_words(char *, DICTIONARY *);
DICTIONARY *new_dictionary(void);
MODEL *new_model(int);
TREE *new_node(void);
SWAP *new_swap(void);
bool print_header(FILE *);
bool progress(char *, int, int);
char *read_input(char *);
DICTIONARY *reply(MODEL *, DICTIONARY *);
void save_dictionary(FILE *, DICTIONARY *);
void save_model(char *, MODEL *);
void save_tree(FILE *, TREE *);
void save_word(FILE *, STRING);
int search_dictionary(DICTIONARY *, STRING, bool *);
int search_node(TREE *, int, bool *);
int seed(MODEL *, DICTIONARY *);
void show_dictionary(DICTIONARY *);
void speak(char *);
bool status(char *, ...);
#ifdef __mac_os
char *strdup(const char *);
#endif
void train(MODEL *, char *);
void typein(char);
void update_context(MODEL *, int);
void update_model(MODEL *, int);
void upper(char *);
bool warn(char *, char *, ...);
int wordcmp(STRING, STRING);
bool word_exists(DICTIONARY *, STRING);
void write_input(char *);
void write_output(char *);
int rnd(int);
#if defined(DOS) || defined(__mac_os)
void usleep(int);
#endif
int fim(char *orig, char *dest, int num);
void usage(char *argv);

/*===========================================================================*/

int width=75;
int order=5;
int timeout=2;
int sd, port, quiet, debug;
bool typing_delay=FALSE;
bool speech=FALSE;
bool used_key;
bool connected;
DICTIONARY *ban=NULL;
DICTIONARY *aux=NULL;
DICTIONARY *fin=NULL;
DICTIONARY *grt=NULL;
SWAP *swp=NULL;
FILE *errorfp=stderr;
FILE *statusfp=stdout;
char *directory=NULL;
char *last=NULL;
char host[255],
  nick[32],
  pass[32],
  chan[32],
  sistema[32],
  address[32],
  ircname[32],
  tmp[3060],
  tmp2[3060],
  netbuf[256],
  input2[3060];

COMMAND command[] = {
	{ { 4, "QUIT" }, "quits the program and saves MegaHAL's brain", QUIT },
	{ { 4, "EXIT" }, "exits the program *without* saving MegaHAL's brain", EXIT },
	{ { 4, "SAVE" }, "saves the current MegaHAL brain", SAVE },
	{ { 6, "RELOAD" }, "reload the last MegaHAL brain saved (don't save it before reload)", RELOAD },
	{ { 5, "DELAY" }, "toggles MegaHAL's typing delay (off by default)", DELAY },
	{ { 6, "SPEECH" }, "toggles MegaHAL's speech (off by default)", SPEECH },
	{ { 6, "VOICES" }, "list available voices for speech", VOICELIST },
	{ { 5, "VOICE" }, "switches to voice specified", VOICE },
	{ { 5, "BRAIN" }, "change to another MegaHAL personality", BRAIN },
	{ { 4, "HELP" }, "displays this message", HELP }
};

COMMAND command_net[] = {
        { { 4, "QUIT" }, "quits the program and saves MegaHAL's brain", QUIT },
        { { 4, "EXIT" }, "exits the program *without* saving MegaHAL's brain", EXIT },
        { { 4, "SAVE" }, "saves the current MegaHAL brain", SAVE },
        { { 6, "RELOAD" }, "reload the last saved MegaHAL brain *without* save it", RELOAD },
        { { 5, "BRAIN" }, "change to another MegaHAL personality", BRAIN },
        { { 4, "HELP" }, "displays this message", HELP }
};

#ifdef AMIGA
struct Locale *_AmigaLocale;
#endif

#ifdef __mac_os
Boolean gSpeechExists = false;
SpeechChannel gSpeechChannel = nil;
#endif

/*===========================================================================*/

/*
 *		Function:	Main
 *
 *		Purpose:		Initialise everything, and then do an infinite loop.  In
 *						the loop, we read the user's input and reply to it, and
 *						do some housekeeping task such as responding to special
 *						commands.
 */
int main(int argc, char *argv[])
{
	struct sockaddr_in sa;
	struct hostent *he;

	bool enabled[3];

	char *input=NULL;
	char *output=NULL;
	DICTIONARY *words=NULL;
	DICTIONARY *greets=NULL;
	MODEL *model=NULL;
	int position=0;
	int opt, kind;

	/*
	 *		Do some initialisation 
	 */
	bzero(&host ,sizeof(host));
	bzero(&nick ,sizeof(nick));
	bzero(&address ,sizeof(address));
	bzero(&sistema ,sizeof(sistema));
	bzero(&ircname ,sizeof(ircname));
	bzero(&pass ,sizeof(pass));
	bzero(&chan ,sizeof(chan));
	kind = -1;
	port = 0;
	quiet = 0;
	debug = 0;
	enabled[0] = FALSE;
	enabled[1] = FALSE;
	enabled[2] = FALSE;

	while ((opt = getopt(argc, argv, "h:p:a:i:d:c:n:w:s:qu")) != -1)
	switch (opt) {
		case 'h':                                         // server  //
			sprintf(host, "%s", optarg);
			break;
		case 'p':                                         //  port   //
			port = atoi(optarg);
			break;
		case 'a':                                         // address //
			sprintf(address, "%s", optarg);
			break;
		case 'i':                                         // ircname //
			sprintf(ircname, "%s", optarg);
			break;
		case 'd':                                         // passwd  //
			sprintf(pass, "%s", optarg);
			break;
		case 'c':                                         // chan    //
			sprintf(chan, "#%s", optarg);
			break;
		case 'n':                                         // nick    //
			sprintf(nick, "%s", optarg);
			break;
		case 'w':                                         //bot mode?//
			kind = atoi(optarg);
			break;
		case 's':                                         // system  //
			sprintf(sistema, "%s", optarg);
			break;
		case 'q':
			quiet = 1;
			break;
		case 'u':
			debug = 1;
			break;
		default:
			usage(argv[0]);
			exithal();
		}
	if ((kind != 0) && (kind != 1)) { usage(argv[0]); exithal(); }
	if ((debug == 1) && (quiet == 1)) { usage(argv[0]); exithal(); }
	
	initialize_error(".megahal/megahal.log");
	initialize_status(".megahal/megahal.txt");
	ignore(0);
#ifdef AMIGA
	_AmigaLocale=OpenLocale(NULL);
#endif
#ifdef __mac_os
	gSpeechExists = initialize_speech();
#endif

if (!quiet) fprintf(stdout,
"+------------------------------------------------------------------------+\n"
"|                                                                        |\n"
"|  #    #  ######   ####     ##    #    #    ##    #                     |\n"
"|  ##  ##  #       #    #   #  #   #    #   #  #   #               ###   |\n"
"|  # ## #  #####   #       #    #  ######  #    #  #              #   #  |\n"
"|  #    #  #       #  ###  ######  #    #  ######  #       #   #   ###   |\n"
"|  #    #  #       #    #  #    #  #    #  #    #  #        # #   #   #  |\n"
"|  #    #  ######   ####   #    #  #    #  #    #  ######    #     ###r6 |\n"
"|                                                                        |\n"
"|                    Copyright(C) 1998 Jason Hutchens                    |\n"
"+------------------------------------------------------------------------+\n"
);

	/*
	 *		Create a dictionary which will be used to hold the segmented
	 *		version of the user's input.
	 */
	words=new_dictionary();
	greets=new_dictionary();

	/*
	 *		Load the default MegaHAL personality.
	 */
	change_personality(NULL, 0, &model);
	make_greeting(greets);
	output=generate_reply(model, greets);

	if (kind == 1) {
	  if (!host[0]) { sprintf(host, "192.168.1.1"); }
	  if (!port) { port = 6667; }
	  if (!nick[0]) { sprintf(nick, "MegaHAL"); }
	  if (!sistema[0]) { sprintf(sistema, "LINUX"); }
	  if (!address[0]) { sprintf(address, "megahal"); }
	  if (!ircname[0]) { sprintf(ircname, "MegaHAL bot"); }
	  if (!chan[0]) { sprintf(chan, "#MegaHAL"); }

	  sd = socket (AF_INET, SOCK_STREAM, 0);

	  sa.sin_family = AF_INET;
	  sa.sin_port = htons(port);

	  if (debug) printf("\nResolving address... "); fflush(stdout);
	  he = gethostbyname (host);
	  if (!he) {
	    if ((sa.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE) {
	      fprintf(stderr, "Wrong ip address or unknown hostname\n"); exit(2);
	    }
	  }
	  else {
	    bcopy ( he->h_addr, (struct in_addr *) &sa.sin_addr, he->h_length);
	  }
	  if (debug) printf("resolved!\n");

	  if (debug) printf("Connecting to %s:%d...\n", host, port); fflush(stdout);
	  if (connect(sd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
	    fprintf (stderr, "Cannot connect to remote host: Connection refused\n"); exit(0);
	  }
	  else {
	    if (!quiet) printf("Connected to %s:%d\n", host, port);
	    fflush(stdout);
	    connected=TRUE;
	    }

	/* Now, send the requested informations to log into ircd
	USERID:SYSTEM:MegaHAL
	NICK NICKNAME
	PING :a_number --> we will receive
	PONG a_number  --> and we will reply
	USER ADDRESS +i MegaHAL IRCNAME
	*/

	  bzero(&input2, sizeof(input2));
	  if ((!quiet) && (!debug)) printf("Loggin... ");
	  if (debug) printf("\n");
	  fflush(stdout);
	  sprintf(input2, "USERID:%s:MegaHAL\n", sistema);
	  if (debug) printf("Sending USERID:%s:MegaHAL... ", sistema);
	  write(sd, input2, strlen(input2)); bzero(&input2, sizeof(input2));
	  if (debug) printf("sended.\n");
	  
	  sprintf(input2, "NICK %s\n", nick);
	  if (debug) printf("Sending NICK %s... ", nick); fflush(stdout);
	  write(sd, input2, strlen(input2)); bzero(&input2, sizeof(input2));
	  if (debug) printf("sended.\n");

	  sprintf(input2, "USER %s +i MegaHAL %s\n", address, ircname);
	  if (debug) printf("Sending USER %s +i MegaHAL %s... ", address, ircname); fflush(stdout);
	  write(sd, input2, strlen(input2)); bzero(&input2, sizeof(input2));
	  if (debug) printf("sended.\n");

	  if (debug) printf("Waiting for PING... "); fflush(stdout);
	  read(sd, netbuf, sizeof(netbuf));
	  if ((netbuf[0]=='P') && (netbuf[1]=='I') && (netbuf[2]=='N') && (netbuf[3]=='G') && (netbuf[5]==':'))
	    {
	    if (debug) printf("received.\n");
	    fim(netbuf, tmp, strlen(netbuf)-6);
	    sprintf(input2, "PONG %s\n", tmp);
	    if (debug) printf("Sending PONG... "); fflush(stdout);
	    write(sd, input2, strlen(input2)); bzero(&input2, sizeof(input2));
	    if (debug) printf("sended.\n");
	    enabled[2]=TRUE;
	    }
	  bzero (&netbuf, sizeof(netbuf));
	  if ((!debug) && (!quiet)) printf("logged!\n");
	  else printf("Logged.\n");
	  fflush(stdout);

	  while(read(sd, netbuf, sizeof(netbuf)))
	    {
	    fim(netbuf, tmp, strlen("Se voce nao troca-lo em 1 minuto, sera desconectado."));
	    if ((tmp[0]=='o') && (tmp[1]=='c') && (tmp[2]=='e') && (tmp[3]==' '))
	      {
	      bzero(&tmp, sizeof(tmp)); bzero (&netbuf, sizeof(netbuf));
	      sprintf(input2, "PRIVMSG NICKSERV :IDENTIFY %s\n", pass);
	      if (!quiet) printf("NickServ identify requested, sending the passwd... ");
	      write(sd, input2, strlen(input2)); bzero(&input2, sizeof(input2));
	      if (!quiet) printf("sended.\n");
	      }

	    else if ((netbuf[0]=='P') && (netbuf[1]=='I') && (netbuf[2]=='N') && (netbuf[3]=='G') && (netbuf[5]==':'))
	      {
	      bzero(&tmp, sizeof(tmp));
	      fim(netbuf, tmp, strlen(netbuf)-6);
	      sprintf(input2, "PONG %s\n", tmp);
	      if (debug) printf("Sending PONG... "); fflush(stdout);
	      write(sd, input2, strlen(input2));
	      bzero(&input2, sizeof(input2));
	      if (debug) printf("sended.\n");
	      bzero (&netbuf, sizeof(netbuf));
	      }

	    else if ((netbuf[0]==':') && (strstr(tmp, "MODE") == NULL) &&
	        (strstr(netbuf, "KICK") == NULL) && (strstr(netbuf, " :") != NULL))
		{
		bzero(&tmp, sizeof(tmp));
		fim(netbuf, tmp, strlen(strstr(netbuf, " :"))-2);
		bzero(&input, sizeof(input));
                if (strlen(tmp) > strlen(nick)+2)
		  {
                  sprintf(tmp2, "%s: ", nick);
                  sprintf(input2, " 372 %s :", nick);
		  if ((strstr(tmp, tmp2) != NULL) && (strstr(tmp, input2) == NULL))
		    {
		    input = strstr(tmp, tmp2);
		    fim(tmp, input, strlen(input)-strlen(nick)-2);
		    comeco(input, input, strlen(input)-1);
		    }
		  bzero(&tmp2, sizeof(tmp2));
		  bzero(&input2, sizeof(input2));
                  }
	        
	        if ((strcmp("End of /MOTD command.", netbuf)>0) && (!enabled[1]) && (!enabled[0]))
	          {
	          sprintf(tmp, "JOIN %s\n", chan);
	          if (!quiet) printf("Joining %s... ", chan);
	          write(sd, tmp, strlen(tmp));
	          enabled[1]=TRUE; fflush(stdout);
	          }
  		if ((strcmp("End of /NAMES list.", netbuf)>0) && (enabled[1]) && (!enabled[0]))
  		  {
  		  if (!quiet)
  		    {
  		    printf("joined.\n");
  		    printf("Starting the conversation with many people at the same time ;)\n");
  		    fflush(stdout);
  		    }
  		  enabled[0]=TRUE;
  		  bzero(&netbuf, sizeof(netbuf));
		  bzero(&tmp, sizeof(tmp));
		  sprintf(tmp, "PRIVMSG %s :%s\n", chan, output);
		  write(sd, tmp, strlen(tmp));
		  if (!quiet) printf("%s\n> ",output); fflush(stdout);
  		  }
  		  
	        else if ((enabled[0]) && (enabled[1]) && (enabled[2]) && (input))
 	          {
		  lower(input);
		  make_words(input,words);
		  switch(execute_command(words, &position)) {
			case EXIT:
			        sprintf(input, "PRIVMSG %s :Exiting now without save the brain...\nQUIT Quit requested.\n", chan);
			        write(sd, input, strlen(input));
		 	        close(sd);
			        exithal();
			case QUIT:
			        sprintf(tmp, "PRIVMSG %s :Exiting and saving the brain right now...\nQUIT Quit requested.\n", chan);
			        write(sd, input, strlen(input));
			        close(sd);
			        save_model(".megahal/megahal.brn", model);
			        exithal();
			case SAVE:
			        sprintf(input, "PRIVMSG %s :Saving the brain...\n", chan);
			        write(sd, input, strlen(input));
			        save_model(".megahal/megahal.brn", model);
			        continue;
			case RELOAD:
			        sprintf(input, "PRIVMSG %s :Reloading the brain without save...\n", chan);
			        write(sd, input, strlen(input));
				words=new_dictionary();
				greets=new_dictionary();
			        change_personality(NULL, 0, &model);
			        make_greeting(greets);
			        output=generate_reply(model, greets);
			        lower(output);
			        sprintf(input, "PRIVMSG %s :%s", chan, output);
			        write(sd, output, strlen(output));
				continue;
			case HELP:
				help();
				continue;
			case BRAIN:
				change_personality(words, position, &model);
				make_greeting(greets);
				output=generate_reply(model, greets);
				lower(output);
				bzero(&input2, sizeof(input2));
				sprintf(input2, "PRIVMSG %s : %s\n", chan, output);
				write(sd, input2, strlen(input2));
				if (!quiet) printf("%s\n> ",output); fflush(stdout);
				continue;
			default:
				break;	
		    }

		  make_words(input,words);
		  learn(model, words);
		  output=generate_reply(model, words);
		  lower(output);
		  bzero(&input2, sizeof(input2));
		  sprintf(input2, "PRIVMSG %s : %s\n", chan, output);
		  write(sd, input2, strlen(input2));
		  if (!quiet) printf("%s\n> ",output); fflush(stdout);
		  }
	        bzero(&output, sizeof(output));
	        bzero(&input, sizeof(input));
	        bzero(&tmp, sizeof(tmp));
	        }
	    bzero (&netbuf, sizeof(netbuf));
	    bzero(&tmp, sizeof(tmp));
	    }
	  close(sd);
	  }

	else if (kind == 0) {
	write_output(output);
	/*
	 *		Read input, formulate a reply and display it as output
	 */
	while(TRUE) {
		input=read_input("> ");
		write_input(input);
		make_words(input,words);

		/*
		 *		If the input was a command, then execute it
		 */
		switch(execute_command(words, &position)) {
			case EXIT:
				exithal();
			case QUIT:
				save_model(".megahal/megahal.brn", model);
				exithal();
			case SAVE:
				save_model(".megahal/megahal.brn", model);
				continue;
			case DELAY:
				typing_delay=!typing_delay;
				printf("MegaHAL typing is now %s.\n", typing_delay?"on":"off");
				continue;
			case SPEECH:
				speech=!speech;
				printf("MegaHAL speech is now %s.\n", speech?"on":"off");
				continue;
			case HELP:
				help();
				continue;
			case VOICELIST:
				listvoices();
				continue;
			case VOICE:
				changevoice(words, position);
				continue;
			case BRAIN:
				change_personality(words, position, &model);
				make_greeting(greets);
				output=generate_reply(model, greets);
				write_output(output);
				continue;
			default:
				break;	
		}

		upper(input);
		make_words(input,words);
		learn(model, words);
		output=generate_reply(model, words);
		write_output(output);
	}
	}

#ifdef AMIGA
	CloseLocale(_AmigaLocale);
#endif

	return(0);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	fim
 *
 *		Purpose:		return the n last bytes of a string and
 *						put it into dest string
 */
int fim(char *orig, char *dest,int num)
{
        int tam, i, j;
	
        tam = strlen (orig);
        for (i = tam - num, j = 0; i < tam, j < num; i++, j++)
                dest[j] = orig[i];
        dest[j] = '\0';
        return j;
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	comeco
 *
 *		Purpose:		return the n last bytes of a string and
 *						put it into dest string
 */
int comeco(char *orig, char *dest,int num)
{
        int tam, i;
	
        tam = strlen (orig);
        for (i=0;i<num;i++)
                dest[i] = orig[i];
        dest[i] = '\0';
        return i;
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	usage
 *
 *		Purpose:		Displays a help if main::argc != 1 and 
 *						!= 9
 */
void usage(char *argv)
{
printf("\033[0;0H\033[J"
"+------------------------------------------------------------------------+\n"
"|                                                                        |\n"
"|  #    #  ######   ####     ##    #    #    ##    #                     |\n"
"|  ##  ##  #       #    #   #  #   #    #   #  #   #               ###   |\n"
"|  # ## #  #####   #       #    #  ######  #    #  #              #   #  |\n"
"|  #    #  #       #  ###  ######  #    #  ######  #       #   #   ###   |\n"
"|  #    #  #       #    #  #    #  #    #  #    #  #        # #   #   #  |\n"
"|  #    #  ######   ####   #    #  #    #  #    #  ######    #     ###r6 |\n"
"|                                                                        |\n"
"|                    Copyright(C) 1998 Jason Hutchens                    |\n"
"+------------------------------------------------------------------------+\n"
);
printf("\nUsage:");
printf("\n  %s [params]     . Uh, the params are these:", argv);
printf("\n    -a <address>  that: <address>@127.0.0.1");
printf("\n    -c <chan>     channel to join");
printf("\n    -d <passwd>   the password of nickserv");
printf("\n    -h <server>   the irc server to connect");
printf("\n    -i <ircname>  your ircname");
printf("\n    -n <nick>     the irc nick to enter");
printf("\n    -p <port>     the irc port to connect");
printf("\n    -q            turn on quiet mode");
printf("\n    -s <system>   something that you want");
printf("\n    -u            turn on debug mode");
printf("\n    -w <number>   0 to normal mode, 1 to bot mode\n");
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Lower
 *
 *		Purpose:		Convert a string to its lowercase representation.
 */
void lower(char *string)
{
	register int i;

	for(i=0; i<(int)strlen(string); ++i) string[i]=(char)tolower((int)string[i]);
}
 


/*---------------------------------------------------------------------------*/

/*
 *		Function:	Execute_Command
 *
 *		Purpose:		Detect whether the user has typed a command, and
 *						execute the corresponding function.
 */
COMMAND_WORDS execute_command(DICTIONARY *words, int *position)
{
	register int i;
	register int j;

	/*
	 *		If there is only one word, then it can't be a command.
	 */
	*position=words->size+1;
	if(words->size<=1) return(UNKNOWN);

	/*
	 *		Search through the word array.  If a command prefix is found,
	 *		then try to match the following word with a command word.  If
	 *		a match is found, then return a command identifier.  If the
	 *		Following word is a number, then change the judge.  Otherwise,
	 *		continue the search.
	 */
	for(i=0; i<words->size-1; ++i)
		/*
		 *		The command prefix was found.
		 */
		if(words->entry[i].word[words->entry[i].length-1]=='#') {
			/*
			 *		Look for a command word.
			 */
			for(j=0; j<COMMAND_SIZE; ++j)
				if(wordcmp(command[j].word, words->entry[i+1])==0) {
					*position=i+1;
					return(command[j].command);
				}
		}

	return(UNKNOWN);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	ExitHAL
 *
 *		Purpose:		Terminate the program.
 */
void exithal(void)
{
#ifdef __mac_os
	/*
	 *		Must be called because it does use some system memory
	 */
	if (gSpeechChannel) {
		StopSpeech(gSpeechChannel);
		DisposeSpeechChannel(gSpeechChannel);
		gSpeechChannel = nil;
	}
#endif

	exit(0);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Read_Input
 *
 *		Purpose:		Read an input string from the user.
 */
char *read_input(char *prompt)
{
	static char *input=NULL;
	bool finish;
	int length;
	int c;

	/*
	 *		Perform some initializations.  The finish boolean variable is used
	 *		to detect a double line-feed, while length contains the number of
	 *		characters in the input string.
	 */
	finish=FALSE;
	length=0;
	if(input==NULL) {
		input=(char *)malloc(sizeof(char));
		if(input==NULL) {
			error("read_input", "Unable to allocate the input string");
			return(input);
		}
	}

	/* 
	 *		Display the prompt to the user.
	 */
	fprintf(stdout, prompt);
	fflush(stdout);

	/*
	 *		Loop forever, reading characters and putting them into the input
	 *		string.
	 */
	while(TRUE) {

		/*
		 *		Read a single character from stdin.
		 */
		c=getc(stdin);

		/*
		 *		If the character is a line-feed, then set the finish variable
		 *		to TRUE.  If it already is TRUE, then this is a double line-feed,
		 *		in which case we should exit.  After a line-feed, display the
		 *		prompt again, and set the character to the space character, as
		 *		we don't permit linefeeds to appear in the input.
		 */
		if((char)(c)=='\n') {
			if(finish==TRUE) break;
			fprintf(stdout, prompt);
			fflush(stdout);
			finish=TRUE;
			c=32;
		} else {
			finish=FALSE;
		}

		/*
		 *		Re-allocate the input string so that it can hold one more
		 *		character.
		 */
		++length;
		input=(char *)realloc((char *)input,sizeof(char)*(length+1));
		if(input==NULL) {
			error("read_input", "Unable to re-allocate the input string");
			return(NULL);
		}

		/*
		 *		Add the character just read to the input string.
		 */
		input[length-1]=(char)c;
		input[length]='\0';
	}

	while(isspace(input[length-1])) --length;
	input[length]='\0';

	/*
	 *		We have finished, so return the input string.
	 */
	return(input);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Initialize_Error
 *
 *		Purpose:		Close the current error file pointer, and open a new one.
 */
bool initialize_error(char *filename)
{
	if(errorfp!=stderr) fclose(errorfp);
	if(filename==NULL) return(TRUE);
	errorfp=fopen(filename, "a");
	if(errorfp==NULL) {
		errorfp=stderr;
		return(FALSE);
	}
	return(print_header(errorfp));
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Error
 *
 *		Purpose:		Print the specified message to the error file.
 */
void error(char *title, char *fmt, ...)
{
	va_list argp;

	fprintf(errorfp, "%s: ", title);
	va_start(argp, fmt);
	vfprintf(errorfp, fmt, argp);
	va_end(argp);
	fprintf(errorfp, ".\n");
	fflush(errorfp);

	fprintf(stderr, "MegaHAL died for some reason; check the error log.\n");

	exit(1);
}

/*---------------------------------------------------------------------------*/

bool warn(char *title, char *fmt, ...)
{
	va_list argp;

	fprintf(errorfp, "%s: ", title);
	va_start(argp, fmt);
	vfprintf(errorfp, fmt, argp);
	va_end(argp);
	fprintf(errorfp, ".\n");
	fflush(errorfp);

	fprintf(stderr, "MegaHAL emitted a warning; check the error log.\n");

	return(TRUE);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Initialize_Status
 *
 *		Purpose:		Close the current status file pointer, and open a new one.
 */
bool initialize_status(char *filename)
{
	if(statusfp!=stdout) fclose(statusfp);
	if(filename==NULL) return(FALSE);
	statusfp=fopen(filename, "a");
	if(statusfp==NULL) {
		statusfp=stdout;
		return(FALSE);
	}
	return(print_header(statusfp));
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Status
 *
 *		Purpose:		Print the specified message to the status file.
 */
bool status(char *fmt, ...)
{
	va_list argp;

	va_start(argp, fmt);
	vfprintf(statusfp, fmt, argp);
	va_end(argp);
	fflush(statusfp);

	return(TRUE);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Print_Header
 *
 *		Purpose:		Display a copyright message and timestamp.
 */
bool print_header(FILE *file)
{
	time_t clock;
	char timestamp[1024];
	struct tm *local;

	clock=time(NULL);
	local=localtime(&clock);
	strftime(timestamp, 1024, "Start at: [%Y/%m/%d %H:%M:%S]\n", local);

	fprintf(file, "MegaHALv8\n");
	fprintf(file, "Copyright (C) 1998 Jason Hutchens\n");
	fprintf(file, timestamp);
	fflush(file);

	return(TRUE);
}

/*---------------------------------------------------------------------------*/

/*
 *    Function:   Write_Output
 *
 *    Purpose:    Display the output string.
 */
void write_output(char *output)
{
   char *formatted;
   char *bit;
 
	capitalize(output);
	speak(output);

	width=75;
	formatted=format_output(output);
	delay(formatted);
	width=64;
	formatted=format_output(output);
 
	bit=strtok(formatted, "\n");
	if(bit==NULL) (void)status("MegaHAL: %s\n", formatted);
	while(bit!=NULL) {
		(void)status("MegaHAL: %s\n", bit);
		bit=strtok(NULL, "\n");
	}
}
 
/*---------------------------------------------------------------------------*/

/*
 *		Function:	Capitalize
 *
 *		Purpose:		Convert a string to look nice.
 */
void capitalize(char *string)
{
	register int i;
	bool start=TRUE;

	for(i=0; i<(int)strlen(string); ++i) {
		if(isalpha(string[i])) {
			if(start==TRUE) string[i]=(char)toupper((int)string[i]);
			else string[i]=(char)tolower((int)string[i]);
			start=FALSE;
		}
		if((i>2)&&(strchr("!.?", string[i-1])!=NULL)&&(isspace(string[i])))
			start=TRUE;
	}
}
 
/*---------------------------------------------------------------------------*/

/*
 *		Function:	Upper
 *
 *		Purpose:		Convert a string to its uppercase representation.
 */
void upper(char *string)
{
	register int i;

	for(i=0; i<(int)strlen(string); ++i) string[i]=(char)toupper((int)string[i]);
}
 
/*---------------------------------------------------------------------------*/

/*
 *    Function:   Write_Input
 *
 *    Purpose:    Log the user's input
 */
void write_input(char *input)
{
   char *formatted;
   char *bit;
 
	width=64;
   formatted=format_output(input);

   bit=strtok(formatted, "\n");
	if(bit==NULL) (void)status("User:    %s\n", formatted);
   while(bit!=NULL) {
      (void)status("User:    %s\n", bit);
      bit=strtok(NULL, "\n");
   }
}

/*---------------------------------------------------------------------------*/

/*
 *    Function:   Format_Output
 *
 *    Purpose:    Format a string to display nicely on a terminal of a given
 *                width.
 */
char *format_output(char *output)
{
   static char *formatted=NULL;
   register int i,j,c;
   int l;

   if(formatted==NULL) {
      formatted=(char *)malloc(sizeof(char));
      if(formatted==NULL) {
         error("format_output", "Unable to allocate formatted");
         return("ERROR");
      }
   }   

   formatted=(char *)realloc((char *)formatted, sizeof(char)*(strlen(output)+2));
   if(formatted==NULL) {
      error("format_output", "Unable to re-allocate formatted");
      return("ERROR");
   }

   l=0;
	j=0;
   for(i=0; i<(int)strlen(output); ++i) {
      if((l==0)&&(isspace(output[i]))) continue;
      formatted[j]=output[i];
      ++j;
      ++l;
      if(l>=width)
         for(c=j-1; c>0; --c)
            if(formatted[c]==' ') {
               formatted[c]='\n';
               l=j-c-1;
               break;
            }
   }
	if((j>0)&&(formatted[j-1]!='\n')) {
		formatted[j]='\n';
		++j;
	}
   formatted[j]='\0';

   return(formatted);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Add_Word
 *
 *		Purpose:		Add a word to a dictionary, and return the identifier
 *						assigned to the word.  If the word already exists in
 *						the dictionary, then return its current identifier
 *						without adding it again.
 */
BYTE2 add_word(DICTIONARY *dictionary, STRING word)
{
	register int i;
	int position;
	bool found;

	/* 
	 *		If the word's already in the dictionary, there is no need to add it
	 */
	position=search_dictionary(dictionary, word, &found);
	if(found==TRUE) goto succeed;

	/* 
	 *		Increase the number of words in the dictionary
	 */
	dictionary->size+=1;

	/*
	 *		Allocate one more entry for the word index
	 */
	if(dictionary->index==NULL) {
		dictionary->index=(BYTE2 *)malloc(sizeof(BYTE2)*
		(dictionary->size));
	} else {
		dictionary->index=(BYTE2 *)realloc((BYTE2 *)
		(dictionary->index),sizeof(BYTE2)*(dictionary->size));
	}
	if(dictionary->index==NULL) {
		error("add_word", "Unable to reallocate the index.");
		goto fail;
	}

	/*
	 *		Allocate one more entry for the word array
	 */
	if(dictionary->entry==NULL) {
		dictionary->entry=(STRING *)malloc(sizeof(STRING)*(dictionary->size));
	} else {
		dictionary->entry=(STRING *)realloc((STRING *)(dictionary->entry),
		sizeof(STRING)*(dictionary->size));
	}
	if(dictionary->entry==NULL) {
		error("add_word", "Unable to reallocate the dictionary to %d elements.", dictionary->size);
		goto fail;
	}

	/*
	 *		Copy the new word into the word array
	 */
	dictionary->entry[dictionary->size-1].length=word.length;
	dictionary->entry[dictionary->size-1].word=(char *)malloc(sizeof(char)*
	(word.length));
	if(dictionary->entry[dictionary->size-1].word==NULL) {
		error("add_word", "Unable to allocate the word.");
		goto fail;
	}
	for(i=0; i<word.length; ++i)
		dictionary->entry[dictionary->size-1].word[i]=word.word[i];

	/*
	 *		Shuffle the word index to keep it sorted alphabetically
	 */
	for(i=(dictionary->size-1); i>position; --i)
		dictionary->index[i]=dictionary->index[i-1];

	/*
	 *		Copy the new symbol identifier into the word index
	 */
	dictionary->index[position]=dictionary->size-1;

succeed:
	return(dictionary->index[position]);

fail:
	return(0);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Search_Dictionary
 *
 *		Purpose:		Search the dictionary for the specified word, returning its
 *						position in the index if found, or the position where it
 *						should be inserted otherwise.
 */
int search_dictionary(DICTIONARY *dictionary, STRING word, bool *find)
{
	int position;
	int min;
	int max;
	int middle;
	int compar;

	/*
	 *		If the dictionary is empty, then obviously the word won't be found
	 */
	if(dictionary->size==0) {
		position=0;
		goto notfound;
	}

	/*
	 *		Initialize the lower and upper bounds of the search
	 */
	min=0;
	max=dictionary->size-1;
	/*
	 *		Search repeatedly, halving the search space each time, until either
	 *		the entry is found, or the search space becomes empty
	 */
	while(TRUE) {
		/*
		 *		See whether the middle element of the search space is greater
		 *		than, equal to, or less than the element being searched for.
		 */
		middle=(min+max)/2;
		compar=wordcmp(word, dictionary->entry[dictionary->index[middle]]);
		/*
		 *		If it is equal then we have found the element.  Otherwise we
		 *		can halve the search space accordingly.
		 */
		if(compar==0) {
			position=middle;
			goto found;
		} else if(compar>0) {
			if(max==middle) {
				position=middle+1;
				goto notfound;
			}
			min=middle+1;
		} else {
			if(min==middle) {
				position=middle;
				goto notfound;
			}
			max=middle-1;
		}
	}

found:
	*find=TRUE;
	return(position);

notfound:
	*find=FALSE;
	return(position);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Find_Word
 *
 *		Purpose:		Return the symbol corresponding to the word specified.
 *						We assume that the word with index zero is equal to a
 *						NULL word, indicating an error condition.
 */
BYTE2 find_word(DICTIONARY *dictionary, STRING word)
{
	int position;
	bool found;

	position=search_dictionary(dictionary, word, &found);

	if(found==TRUE) return(dictionary->index[position]);
	else return(0);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Wordcmp
 *
 *		Purpose:		Compare two words, and return an integer indicating whether
 *						the first word is less than, equal to or greater than the
 *						second word.
 */
int wordcmp(STRING word1, STRING word2)
{
	register int i;
	int bound;

	bound=MIN(word1.length,word2.length);

	for(i=0; i<bound; ++i)
		if(toupper(word1.word[i])!=toupper(word2.word[i]))
			return((int)(toupper(word1.word[i])-toupper(word2.word[i])));

	if(word1.length<word2.length) return(-1);
	if(word1.length>word2.length) return(1);

	return(0);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Free_Dictionary
 *
 *		Purpose:		Release the memory consumed by the dictionary.
 */
void free_dictionary(DICTIONARY *dictionary)
{
	if(dictionary==NULL) return;
	if(dictionary->entry!=NULL) {
		free(dictionary->entry);
		dictionary->entry=NULL;
	}
	if(dictionary->index!=NULL) {
		free(dictionary->index);
		dictionary->index=NULL;
	}
	dictionary->size=0;
}

/*---------------------------------------------------------------------------*/

void free_model(MODEL *model)
{
	if(model==NULL) return;
	if(model->forward!=NULL) {
		free_tree(model->forward);
	}
	if(model->backward!=NULL) {
		free_tree(model->backward);
	}
	if(model->context!=NULL) {
		free(model->context);
	}
	if(model->dictionary!=NULL) {
		free_dictionary(model->dictionary);
		free(model->dictionary);
	}
	free(model);
}

/*---------------------------------------------------------------------------*/

void free_tree(TREE *tree)
{
	static int level=0;
	register int i;

	if(tree==NULL) return;

	if(tree->tree!=NULL) {
		if(level==0) progress("Freeing tree", 0, 1);
		for(i=0; i<tree->branch; ++i) {
			++level;
			free_tree(tree->tree[i]);
			--level;
			if(level==0) progress(NULL, i, tree->branch);
		}
		if(level==0) progress(NULL, 1, 1);
		free(tree->tree);
	}
	free(tree);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Initialize_Dictionary
 *
 *		Purpose:		Add dummy words to the dictionary.
 */
void initialize_dictionary(DICTIONARY *dictionary)
{
	STRING word={ 7, "<ERROR>" };
	STRING end={ 5, "<FIN>" };

	(void)add_word(dictionary, word);
	(void)add_word(dictionary, end);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	New_Dictionary
 *
 *		Purpose:		Allocate room for a new dictionary.
 */
DICTIONARY *new_dictionary(void)
{
	DICTIONARY *dictionary=NULL;

	dictionary=(DICTIONARY *)malloc(sizeof(DICTIONARY));
	if(dictionary==NULL) {
		error("new_dictionary", "Unable to allocate dictionary.");
		return(NULL);
	}

	dictionary->size=0;
	dictionary->index=NULL;
	dictionary->entry=NULL;

	return(dictionary);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Save_Dictionary
 *
 *		Purpose:		Save a dictionary to the specified file.
 */
void save_dictionary(FILE *file, DICTIONARY *dictionary)
{
	register int i;

	fwrite(&(dictionary->size), sizeof(BYTE4), 1, file);
	progress("Saving dictionary", 0, 1);
	for(i=0; i<dictionary->size; ++i) {
		save_word(file, dictionary->entry[i]);
		progress(NULL, i, dictionary->size);
	}
	progress(NULL, 1, 1);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Load_Dictionary
 *
 *		Purpose:		Load a dictionary from the specified file.
 */
void load_dictionary(FILE *file, DICTIONARY *dictionary)
{
	register int i;
	int size;

	fread(&size, sizeof(BYTE4), 1, file);
	progress("Loading dictionary", 0, 1);
	for(i=0; i<size; ++i) {
		load_word(file, dictionary);
		progress(NULL, i, size);
	}
	progress(NULL, 1, 1);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Save_Word
 *
 *		Purpose:		Save a dictionary word to a file.
 */
void save_word(FILE *file, STRING word)
{
	register int i;

	fwrite(&(word.length), sizeof(BYTE1), 1, file);
	for(i=0; i<word.length; ++i)
		fwrite(&(word.word[i]), sizeof(char), 1, file);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Load_Word
 *
 *		Purpose:		Load a dictionary word from a file.
 */
void load_word(FILE *file, DICTIONARY *dictionary)
{
	register int i;
	STRING word;

	fread(&(word.length), sizeof(BYTE1), 1, file);
	word.word=(char *)malloc(sizeof(char)*word.length);
	if(word.word==NULL) {
		error("load_word", "Unable to allocate word");
		return;
	}
	for(i=0; i<word.length; ++i)
		fread(&(word.word[i]), sizeof(char), 1, file);
	add_word(dictionary, word);
	free(word.word);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	New_Node
 *
 *		Purpose:		Allocate a new node for the n-gram tree, and initialise
 *						its contents to sensible values.
 */
TREE *new_node(void)
{
	TREE *node=NULL;

	/*
	 *		Allocate memory for the new node
	 */
	node=(TREE *)malloc(sizeof(TREE));
	if(node==NULL) {
		error("new_node", "Unable to allocate the node.");
		goto fail;
	}

	/*
	 *		Initialise the contents of the node
	 */
	node->symbol=0;
	node->usage=0;
	node->count=0;
	node->branch=0;
	node->tree=NULL;

	return(node);

fail:
	if(node!=NULL) free(node);
	return(NULL);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	New_Model
 *
 *		Purpose:		Create and initialise a new ngram model.
 */
MODEL *new_model(int order)
{
	MODEL *model=NULL;

	model=(MODEL *)malloc(sizeof(MODEL));
	if(model==NULL) {
		error("new_model", "Unable to allocate model.");
		goto fail;
	}

	model->order=order;
	model->forward=new_node();
	model->backward=new_node();
	model->context=(TREE **)malloc(sizeof(TREE *)*(order+2));
	if(model->context==NULL) {
		error("new_model", "Unable to allocate context array.");
		goto fail;
	}
	initialize_context(model);
	model->dictionary=new_dictionary();
	initialize_dictionary(model->dictionary);

	return(model);

fail:
	return(NULL);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Update_Model
 *
 *		Purpose:		Update the model with the specified symbol.
 */
void update_model(MODEL *model, int symbol)
{
	register int i;

	/*
	 *		Update all of the models in the current context with the specified
	 *		symbol.
	 */
	for(i=(model->order+1); i>0; --i)
		if(model->context[i-1]!=NULL)
			model->context[i]=add_symbol(model->context[i-1], (BYTE2)symbol);

	return;
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Update_Context
 *
 *		Purpose:		Update the context of the model without adding the symbol.
 */
void update_context(MODEL *model, int symbol)
{
	register int i;

	for(i=(model->order+1); i>0; --i)
		if(model->context[i-1]!=NULL)
			model->context[i]=find_symbol(model->context[i-1], symbol);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Add_Symbol
 *
 *		Purpose:		Update the statistics of the specified tree with the
 *						specified symbol, which may mean growing the tree if the
 *						symbol hasn't been seen in this context before.
 */
TREE *add_symbol(TREE *tree, BYTE2 symbol)
{
	TREE *node=NULL;

	/*
	 *		Search for the symbol in the subtree of the tree node.
	 */
	node=find_symbol_add(tree, symbol);

	/*
	 *		Increment the symbol counts
	 */
	if((node->count<65535)) {
		node->count+=1;
		tree->usage+=1;
	}

	return(node);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Find_Symbol
 *
 *		Purpose:		Return a pointer to the child node, if one exists, which
 *						contains the specified symbol.
 */
TREE *find_symbol(TREE *node, int symbol)
{
	register int i;
	TREE *found=NULL;
	bool found_symbol=FALSE;

	/* 
	 *		Perform a binary search for the symbol.
	 */
	i=search_node(node, symbol, &found_symbol);
	if(found_symbol==TRUE) found=node->tree[i];

	return(found);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Find_Symbol_Add
 *
 *		Purpose:		This function is conceptually similar to find_symbol,
 *						apart from the fact that if the symbol is not found,
 *						a new node is automatically allocated and added to the
 *						tree.
 */
TREE *find_symbol_add(TREE *node, int symbol)
{
	register int i;
	TREE *found=NULL;
	bool found_symbol=FALSE;

	/* 
	 *		Perform a binary search for the symbol.  If the symbol isn't found,
	 *		attach a new sub-node to the tree node so that it remains sorted.
	 */
	i=search_node(node, symbol, &found_symbol);
	if(found_symbol==TRUE) {
		found=node->tree[i];
	} else {
		found=new_node();
		found->symbol=symbol;
		add_node(node, found, i);
	}

	return(found);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Add_Node
 *
 *		Purpose:		Attach a new child node to the sub-tree of the tree
 *						specified.
 */
void add_node(TREE *tree, TREE *node, int position)
{
	register int i;

	/*
	 *		Allocate room for one more child node, which may mean allocating
	 *		the sub-tree from scratch.
	 */
	if(tree->tree==NULL) {
		tree->tree=(TREE **)malloc(sizeof(TREE *)*(tree->branch+1));
	} else {
		tree->tree=(TREE **)realloc((TREE **)(tree->tree),sizeof(TREE *)*
		(tree->branch+1));
	}
	if(tree->tree==NULL) {
		error("add_node", "Unable to reallocate subtree.");
		return;
	}

	/*
	 *		Shuffle the nodes down so that we can insert the new node at the
	 *		subtree index given by position.
	 */
	for(i=tree->branch; i>position; --i)
		tree->tree[i]=tree->tree[i-1];

	/*
	 *		Add the new node to the sub-tree.
	 */
	tree->tree[position]=node;
	tree->branch+=1;
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Search_Node
 *
 *		Purpose:		Perform a binary search for the specified symbol on the
 *						subtree of the given node.  Return the position of the
 *						child node in the subtree if the symbol was found, or the
 *						position where it should be inserted to keep the subtree
 *						sorted if it wasn't.
 */
int search_node(TREE *node, int symbol, bool *found_symbol)
{
	register int position;
	int min;
	int max;
	int middle;
	int compar;

	/*
	 *		Handle the special case where the subtree is empty.
	 */ 
	if(node->branch==0) {
		position=0;
		goto notfound;
	}

	/*
	 *		Perform a binary search on the subtree.
	 */
	min=0;
	max=node->branch-1;
	while(TRUE) {
		middle=(min+max)/2;
		compar=symbol-node->tree[middle]->symbol;
		if(compar==0) {
			position=middle;
			goto found;
		} else if(compar>0) {
			if(max==middle) {
				position=middle+1;
				goto notfound;
			}
			min=middle+1;
		} else {
			if(min==middle) {
				position=middle;
				goto notfound;
			}
			max=middle-1;
		}
	}

found:
	*found_symbol=TRUE;
	return(position);

notfound:
	*found_symbol=FALSE;
	return(position);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Initialize_Context
 *
 *		Purpose:		Set the context of the model to a default value.
 */
void initialize_context(MODEL *model)
{
	register int i;

	for(i=0; i<=model->order; ++i) model->context[i]=NULL;
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Learn
 *
 *		Purpose:		Learn from the user's input.
 */
void learn(MODEL *model, DICTIONARY *words)
{
	register int i;
	BYTE2 symbol;

	/*
	 *		We only learn from inputs which are long enough
	 */
	if(words->size<=(model->order)) return;

	/*
	 *		Train the model in the forwards direction.  Start by initializing
	 *		the context of the model.
	 */
	initialize_context(model);
	model->context[0]=model->forward;
	for(i=0; i<words->size; ++i) {
		/*
		 *		Add the symbol to the model's dictionary if necessary, and then
		 *		update the forward model accordingly.
		 */
		symbol=add_word(model->dictionary, words->entry[i]);
		update_model(model, symbol);
	}
	/*
	 *		Add the sentence-terminating symbol.
	 */
	update_model(model, 1);

	/*
	 *		Train the model in the backwards direction.  Start by initializing
	 *		the context of the model.
	 */
	initialize_context(model);
	model->context[0]=model->backward;
	for(i=words->size-1; i>=0; --i) {
		/*
		 *		Find the symbol in the model's dictionary, and then update
		 *		the backward model accordingly.
		 */
		symbol=find_word(model->dictionary, words->entry[i]);
		update_model(model, symbol);
	}
	/*
	 *		Add the sentence-terminating symbol.
	 */
	update_model(model, 1);

	return;
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Train
 *
 *		Purpose:	 	Infer a MegaHAL brain from the contents of a text file.
 */
void train(MODEL *model, char *filename)
{
	FILE *file;
	char buffer[1024];
	DICTIONARY *words=NULL;
	int length;

	if(filename==NULL) return;

	file=fopen(filename, "r");
	if(file==NULL) {
		printf("Unable to find the personality %s\n", filename);
		return;
	}

	fseek(file, 0, 2);
   length=ftell(file);
   rewind(file);

	words=new_dictionary();

	progress("Training from file", 0, 1);
	while(!feof(file)) {

		if(fgets(buffer, 1024, file)==NULL) break;
		if(buffer[0]=='#') continue;

		buffer[strlen(buffer)-1]='\0';

		upper(buffer);
		make_words(buffer, words);
		learn(model, words);

		progress(NULL, ftell(file), length);

	}
	progress(NULL, 1, 1);

	free_dictionary(words);
	fclose(file);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Show_Dictionary
 *
 *		Purpose:		Display the dictionary for training purposes.
 */
void show_dictionary(DICTIONARY *dictionary)
{
	register int i;
	register int j;
	FILE *file;

	file=fopen(".megahal/megahal.dic", "w");
	if(file==NULL) {
		warn("show_dictionary", "Unable to open file");
		return;
	}

	for(i=0; i<dictionary->size; ++i) {
		for(j=0; j<dictionary->entry[i].length; ++j)
			fprintf(file, "%c", dictionary->entry[i].word[j]);
		fprintf(file, "\n");
	}

	fclose(file);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Save_Model
 *
 *		Purpose:		Save the current state to a MegaHAL brain file.
 */
void save_model(char *modelname, MODEL *model)
{
	FILE *file;
	static char *filename=NULL;
	
	if(filename==NULL) filename=(char *)malloc(sizeof(char)*1);

	/*
	 *    Allocate memory for the filename
	 */
	filename=(char *)realloc(filename,
		sizeof(char)*(strlen(directory)+strlen(SEP)+12));
	if(filename==NULL) error("save_model","Unable to allocate filename");

	show_dictionary(model->dictionary);
	if(filename==NULL) return;

	sprintf(filename, "%s%s.megahal/megahal.brn", directory, SEP);
	file=fopen(filename, "wb");
	if(file==NULL) {
		warn("save_model", "Unable to open file `%s'", filename);
		return;
	}

	fwrite(COOKIE, sizeof(char), strlen(COOKIE), file);
	fwrite(&(model->order), sizeof(BYTE1), 1, file);
	save_tree(file, model->forward);
	save_tree(file, model->backward);
	save_dictionary(file, model->dictionary);

	fclose(file);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Save_Tree
 *
 *		Purpose:		Save a tree structure to the specified file.
 */
void save_tree(FILE *file, TREE *node)
{
	static int level=0;
	register int i;

	fwrite(&(node->symbol), sizeof(BYTE2), 1, file);
	fwrite(&(node->usage), sizeof(BYTE4), 1, file);
	fwrite(&(node->count), sizeof(BYTE2), 1, file);
	fwrite(&(node->branch), sizeof(BYTE2), 1, file);

	if(level==0) progress("Saving tree", 0, 1);
	for(i=0; i<node->branch; ++i) {
		++level;
		save_tree(file, node->tree[i]);
		--level;
		if(level==0) progress(NULL, i, node->branch);
	}
	if(level==0) progress(NULL, 1, 1);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Load_Tree
 *
 *		Purpose:		Load a tree structure from the specified file.
 */
void load_tree(FILE *file, TREE *node)
{
	static int level=0;
	register int i;

	fread(&(node->symbol), sizeof(BYTE2), 1, file);
	fread(&(node->usage), sizeof(BYTE4), 1, file);
	fread(&(node->count), sizeof(BYTE2), 1, file);
	fread(&(node->branch), sizeof(BYTE2), 1, file);

	if(node->branch==0) return;

	node->tree=(TREE **)malloc(sizeof(TREE *)*(node->branch));
	if(node->tree==NULL) {
		error("load_tree", "Unable to allocate subtree");
		return;
	}

	if(level==0) progress("Loading tree", 0, 1);
	for(i=0; i<node->branch; ++i) {
		node->tree[i]=new_node();
		++level;
		load_tree(file, node->tree[i]);
		--level;
		if(level==0) progress(NULL, i, node->branch);
	}
	if(level==0) progress(NULL, 1, 1);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Load_Model
 *
 *		Purpose:		Load a model into memory.
 */
bool load_model(char *filename, MODEL *model)
{
	FILE *file;
	char cookie[16];

	if(filename==NULL) return(FALSE);

	file=fopen(filename, "rb");
	if(file==NULL) {
		warn("load_model", "Unable to open file `%s'", filename);
		return(FALSE);
	}

	fread(cookie, sizeof(char), strlen(COOKIE), file);
	if(strncmp(cookie, COOKIE, strlen(COOKIE))!=0) {
		warn("load_model", "File `%s' is not a MegaHAL brain", filename);
		goto fail;
	}

	fread(&(model->order), sizeof(BYTE1), 1, file);
	load_tree(file, model->forward);
	load_tree(file, model->backward);
	load_dictionary(file, model->dictionary);

	return(TRUE);
fail:
	fclose(file);

	return(FALSE);
}

/*---------------------------------------------------------------------------*/

/*
 *    Function:   Make_Words
 *
 *    Purpose:    Break a string into an array of words.
 */
void make_words(char *input, DICTIONARY *words)
{
	int offset=0;

	/*
	 *		Clear the entries in the dictionary
	 */
	free_dictionary(words);

	/*
	 *		If the string is empty then do nothing, for it contains no words.
	 */
	if(strlen(input)==0) return;

	/*
	 *		Loop forever.
	 */
	while(1) {

		/*
		 *		If the current character is of the same type as the previous
		 *		character, then include it in the word.  Otherwise, terminate
		 *		the current word.
		 */
		if(boundary(input, offset)) {
			/*
			 *		Add the word to the dictionary
			 */
			if(words->entry==NULL)
				words->entry=(STRING *)malloc((words->size+1)*sizeof(STRING));
			else
				words->entry=(STRING *)realloc(words->entry, (words->size+1)*sizeof(STRING));

			if(words->entry==NULL) {
				error("make_words", "Unable to reallocate dictionary");
				return;
			}

			words->entry[words->size].length=offset;
			words->entry[words->size].word=input;
			words->size+=1;

			if(offset==(int)strlen(input)) break;
			input+=offset;
			offset=0;
		} else {
			++offset;
		}
	}

	/*
	 *		If the last word isn't punctuation, then replace it with a
	 *		full-stop character.
	 */
	if(isalnum(words->entry[words->size-1].word[0])) {
		if(words->entry==NULL)
			words->entry=(STRING *)malloc((words->size+1)*sizeof(STRING));
		else
			words->entry=(STRING *)realloc(words->entry, (words->size+1)*sizeof(STRING));
		if(words->entry==NULL) {
			error("make_words", "Unable to reallocate dictionary");
			return;
		}

		words->entry[words->size].length=1;
		words->entry[words->size].word=".";
		++words->size;
	}
	else if(strchr("!.?", words->entry[words->size-1].word[words->entry[words->size-1].length-1])==NULL) {
		words->entry[words->size-1].length=1;
		words->entry[words->size-1].word=".";
	}

   return;
}
 
/*---------------------------------------------------------------------------*/ 
/*
 *		Function:	Boundary
 *
 *		Purpose:		Return whether or not a word boundary exists in a string
 *						at the specified location.
 */
bool boundary(char *string, int position)
{
	if(position==0)
		return(FALSE);

	if(position==(int)strlen(string))
		return(TRUE);

	if(
		(string[position]=='\'')&&
		(isalpha(string[position-1])!=0)&&
		(isalpha(string[position+1])!=0)
	)
		return(FALSE);

	if(
		(position>1)&&
		(string[position-1]=='\'')&&
		(isalpha(string[position-2])!=0)&&
		(isalpha(string[position])!=0)
	)
		return(FALSE);

	if(
		(isalpha(string[position])!=0)&&
		(isalpha(string[position-1])==0)
	)
		return(TRUE);
	
	if(
		(isalpha(string[position])==0)&&
		(isalpha(string[position-1])!=0)
	)
		return(TRUE);
	
	if(isdigit(string[position])!=isdigit(string[position-1]))
		return(TRUE);

	return(FALSE);
}
 
/*---------------------------------------------------------------------------*/ 
/*
 *		Function:	Make_Greeting
 *
 *		Purpose:		Put some special words into the dictionary so that the
 *						program will respond as if to a new judge.
 */
void make_greeting(DICTIONARY *words)
{
	register int i;

	for(i=0; i<words->size; ++i) free(words->entry[i].word);
	free_dictionary(words);
	if(grt->size>0) (void)add_word(words, grt->entry[rnd(grt->size)]);
}
 
/*---------------------------------------------------------------------------*/ 
/*
 *    Function:   Generate_Reply
 *
 *    Purpose:    Take a string of user input and return a string of output
 *                which may vaguely be construed as containing a reply to
 *                whatever is in the input string.
 */
char *generate_reply(MODEL *model, DICTIONARY *words)
{
	static DICTIONARY *dummy=NULL;
	DICTIONARY *replywords;
	DICTIONARY *keywords;
	float surprise;
	float max_surprise;
	char *output;
	static char *output_none=NULL;
	int count;
	int basetime;

	/*
	 *		Create an array of keywords from the words in the user's input
	 */
	keywords=make_keywords(model, words);

	/*
	 *		Make sure some sort of reply exists
	 */
	if(output_none==NULL) {
		output_none=malloc(40);
		if(output_none!=NULL)
			strcpy(output_none, "I don't know enough to answer you yet!");
	}
	output=output_none;
	if(dummy==NULL) dummy=new_dictionary();
	replywords=reply(model, dummy);
	if(dissimilar(words, replywords)==TRUE) output=make_output(replywords);

	/*
	 *		Loop for the specified waiting period, generating and evaluating
	 *		replies
	 */
	max_surprise=(float)-1.0;
	count=0;
	basetime=time(NULL);
	progress("Generating reply", 0, 1);
	do {
		replywords=reply(model, keywords);
		surprise=evaluate_reply(model, keywords, replywords);
		++count;
		if((surprise>max_surprise)&&(dissimilar(words, replywords)==TRUE)) {
			max_surprise=surprise;
			output=make_output(replywords);
		}
		progress(NULL, (time(NULL)-basetime),timeout);
	} while((time(NULL)-basetime)<timeout);
	progress(NULL, 1, 1);

	/*
	 *		Return the best answer we generated
	 */
	return(output);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Dissimilar
 *
 *		Purpose:		Return TRUE or FALSE depending on whether the dictionaries
 *						are the same or not.
 */
bool dissimilar(DICTIONARY *words1, DICTIONARY *words2)
{
	register int i;

	if(words1->size!=words2->size) return(TRUE);
	for(i=0; i<words1->size; ++i)
		if(wordcmp(words1->entry[i], words2->entry[i])!=0) return(TRUE);
	return(FALSE);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Make_Keywords
 *
 *		Purpose:		Put all the interesting words from the user's input into
 *						a keywords dictionary, which will be used when generating
 *						a reply.
 */
DICTIONARY *make_keywords(MODEL *model, DICTIONARY *words)
{
	static DICTIONARY *keys=NULL;
	register int i;
	register int j;
	int c;

	if(keys==NULL) keys=new_dictionary();
	for(i=0; i<keys->size; ++i) free(keys->entry[i].word);
	free_dictionary(keys);

	for(i=0; i<words->size; ++i) {
		/*
		 *		Find the symbol ID of the word.  If it doesn't exist in
		 *		the model, or if it begins with a non-alphanumeric
		 *		character, or if it is in the exclusion array, then
		 *		skip over it.
		 */
		c=0;
		for(j=0; j<swp->size; ++j)
			if(wordcmp(swp->from[j], words->entry[i])==0) {
				add_key(model, keys, swp->to[j]);
				++c;
			}
		if(c==0) add_key(model, keys, words->entry[i]);
	}

	if(keys->size>0) for(i=0; i<words->size; ++i) {

		c=0;
		for(j=0; j<swp->size; ++j)
			if(wordcmp(swp->from[j], words->entry[i])==0) {
				add_aux(model, keys, swp->to[j]);
				++c;
			}
		if(c==0) add_aux(model, keys, words->entry[i]);
	}

	return(keys);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Add_Key
 *
 *		Purpose:		Add a word to the keyword dictionary.
 */
void add_key(MODEL *model, DICTIONARY *keys, STRING word)
{
	int symbol;

	symbol=find_word(model->dictionary, word);
	if(symbol==0) return;
	if(isalnum(word.word[0])==0) return;
	symbol=find_word(ban, word);
	if(symbol!=0) return;
	symbol=find_word(aux, word);
	if(symbol!=0) return;

	add_word(keys, word);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Add_Aux
 *
 *		Purpose:		Add an auxilliary keyword to the keyword dictionary.
 */
void add_aux(MODEL *model, DICTIONARY *keys, STRING word)
{
	int symbol;

	symbol=find_word(model->dictionary, word);
	if(symbol==0) return;
	if(isalnum(word.word[0])==0) return;
	symbol=find_word(aux, word);
	if(symbol==0) return;

	add_word(keys, word);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Reply
 *
 *		Purpose:		Generate a dictionary of reply words appropriate to the
 *						given dictionary of keywords.
 */
DICTIONARY *reply(MODEL *model, DICTIONARY *keys)
{
	static DICTIONARY *replies=NULL;
	register int i;
	int symbol;
	bool start=TRUE;

	if(replies==NULL) replies=new_dictionary();
	free_dictionary(replies);

	/*
	 *		Start off by making sure that the model's context is empty.
	 */
	initialize_context(model);
	model->context[0]=model->forward;
	used_key=FALSE;

	/*
	 *		Generate the reply in the forward direction.
	 */
	while(TRUE) {
		/*
		 *		Get a random symbol from the current context.
		 */
		if(start==TRUE) symbol=seed(model, keys);
		else symbol=babble(model, keys, replies);
		if((symbol==0)||(symbol==1)) break;
		start=FALSE;

		/*
		 *		Append the symbol to the reply dictionary.
		 */
		if(replies->entry==NULL)
			replies->entry=(STRING *)malloc((replies->size+1)*sizeof(STRING));
		else
			replies->entry=(STRING *)realloc(replies->entry, (replies->size+1)*sizeof(STRING));
		if(replies->entry==NULL) {
			error("reply", "Unable to reallocate dictionary");
			return(NULL);
		}

		replies->entry[replies->size].length=
			model->dictionary->entry[symbol].length;
		replies->entry[replies->size].word=
			model->dictionary->entry[symbol].word;
		replies->size+=1;

		/*
		 *		Extend the current context of the model with the current symbol.
		 */
		update_context(model, symbol);
	}

	/*
	 *		Start off by making sure that the model's context is empty.
	 */
	initialize_context(model);
	model->context[0]=model->backward;

	/*
	 *		Re-create the context of the model from the current reply
	 *		dictionary so that we can generate backwards to reach the
	 *		beginning of the string.
	 */
	if(replies->size>0) for(i=MIN(replies->size-1, model->order); i>=0; --i) {
		symbol=find_word(model->dictionary, replies->entry[i]);
		update_context(model, symbol);
	}

	/*
	 *		Generate the reply in the backward direction.
	 */
	while(TRUE) {
		/*
		 *		Get a random symbol from the current context.
		 */
		symbol=babble(model, keys, replies);
		if((symbol==0)||(symbol==1)) break;

		/*
		 *		Prepend the symbol to the reply dictionary.
		 */
		if(replies->entry==NULL)
			replies->entry=(STRING *)malloc((replies->size+1)*sizeof(STRING));
		else
			replies->entry=(STRING *)realloc(replies->entry, (replies->size+1)*sizeof(STRING));
		if(replies->entry==NULL) {
			error("reply", "Unable to reallocate dictionary");
			return(NULL);
		}

		/*
		 *		Shuffle everything up for the prepend.
		 */
		for(i=replies->size; i>0; --i) {
			replies->entry[i].length=replies->entry[i-1].length;
			replies->entry[i].word=replies->entry[i-1].word;
		}

		replies->entry[0].length=model->dictionary->entry[symbol].length;
		replies->entry[0].word=model->dictionary->entry[symbol].word;
		replies->size+=1;

		/*
		 *		Extend the current context of the model with the current symbol.
		 */
		update_context(model, symbol);
	}

	return(replies);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Evaluate_Reply
 *
 *		Purpose:		Measure the average surprise of keywords relative to the
 *						language model.
 */
float evaluate_reply(MODEL *model, DICTIONARY *keys, DICTIONARY *words)
{
	register int i;
	register int j;
	int symbol;
	float probability;
	int count;
	float entropy=(float)0.0;
	TREE *node;
	int num=0;

	if(words->size<=0) return((float)0.0);
	initialize_context(model);
	model->context[0]=model->forward;
	for(i=0; i<words->size; ++i) {
		symbol=find_word(model->dictionary, words->entry[i]);

		if(find_word(keys, words->entry[i])!=0) {
			probability=(float)0.0;
			count=0;
			++num;
			for(j=0; j<model->order; ++j) if(model->context[j]!=NULL) {
	
				node=find_symbol(model->context[j], symbol);
				probability+=(float)(node->count)/
					(float)(model->context[j]->usage);
				++count;
	
			}

			if(count>0.0) entropy-=(float)log(probability/(float)count);
		}

		update_context(model, symbol);
	}

	initialize_context(model);
	model->context[0]=model->backward;
	for(i=words->size-1; i>=0; --i) {
		symbol=find_word(model->dictionary, words->entry[i]);

		if(find_word(keys, words->entry[i])!=0) {
			probability=(float)0.0;
			count=0;
			++num;
			for(j=0; j<model->order; ++j) if(model->context[j]!=NULL) {
	
				node=find_symbol(model->context[j], symbol);
				probability+=(float)(node->count)/
					(float)(model->context[j]->usage);
				++count;
	
			}

			if(count>0.0) entropy-=(float)log(probability/(float)count);
		}

		update_context(model, symbol);
	}

	if(num>=8) entropy/=(float)sqrt(num-1);
	if(num>=16) entropy/=(float)num;

	return(entropy);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Make_Output
 *
 *		Purpose:		Generate a string from the dictionary of reply words.
 */
char *make_output(DICTIONARY *words)
{
	static char *output=NULL;
	register int i;
	register int j;
	int length;
	static char *output_none=NULL;
	
	if(output_none==NULL) output_none=malloc(40);

	if(output==NULL) {
		output=(char *)malloc(sizeof(char));
		if(output==NULL) {
			error("make_output", "Unable to allocate output");
			return(output_none);
		}
	}

	if(words->size==0) {
		if(output_none!=NULL)
			strcpy(output_none, "I am utterly speechless!");
		return(output_none);
	}

	length=1;
	for(i=0; i<words->size; ++i) length+=words->entry[i].length;

	output=(char *)realloc(output, sizeof(char)*length);
	if(output==NULL) {
		error("make_output", "Unable to reallocate output.");
		if(output_none!=NULL)
			strcpy(output_none, "I forgot what I was going to say!");
		return(output_none);
	}

	length=0;
	for(i=0; i<words->size; ++i)
		for(j=0; j<words->entry[i].length; ++j)
			output[length++]=words->entry[i].word[j];
			
	output[length]='\0';

	return(output);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Babble
 *
 *		Purpose:		Return a random symbol from the current context, or a
 *						zero symbol identifier if we've reached either the
 *						start or end of the sentence.  Select the symbol based
 *						on probabilities, favouring keywords.  In all cases,
 *						use the longest available context to choose the symbol.
 */
int babble(MODEL *model, DICTIONARY *keys, DICTIONARY *words)
{
	TREE *node;
	register int i;
	int count;
	int symbol;

	/*
	 *		Select the longest available context.
	 */
	for(i=0; i<=model->order; ++i)
		if(model->context[i]!=NULL)
			node=model->context[i];

	if(node->branch==0) return(0);

	/*
	 *		Choose a symbol at random from this context.
	 */
	i=rnd(node->branch);
	count=rnd(node->usage);
	while(count>=0) {
		/*
		 *		If the symbol occurs as a keyword, then use it.  Only use an
		 *		auxilliary keyword if a normal keyword has already been used.
		 */
		symbol=node->tree[i]->symbol;

		if(
			(find_word(keys, model->dictionary->entry[symbol])!=0)&&
			((used_key==TRUE)||
			(find_word(aux, model->dictionary->entry[symbol])==0))&&
			(word_exists(words, model->dictionary->entry[symbol])==FALSE)
		) {
			used_key=TRUE;
			break;
		}
		count-=node->tree[i]->count;
		i=(i>=(node->branch-1))?0:i+1;
	}

	return(symbol);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Word_Exists
 *
 *		Purpose:		A silly brute-force searcher for the reply string.
 */
bool word_exists(DICTIONARY *dictionary, STRING word)
{
	register int i;

	for(i=0; i<dictionary->size; ++i)
		if(wordcmp(dictionary->entry[i], word)==0)
			return(TRUE);
	return(FALSE);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Seed
 *
 *		Purpose:		Seed the reply by guaranteeing that it contains a
 *						keyword, if one exists.
 */
int seed(MODEL *model, DICTIONARY *keys)
{
	register int i;
	int symbol;
	int stop;

	/*
	 *		Fix, thanks to Mark Tarrabain
	 */
	if(model->context[0]->branch==0) symbol=0;
	else symbol=model->context[0]->tree[rnd(model->context[0]->branch)]->symbol;

	if(keys->size>0) {
		i=rnd(keys->size);
		stop=i;
		while(TRUE) {
			if(
				(find_word(model->dictionary, keys->entry[i])!=0)&&
				(find_word(aux, keys->entry[i])==0)
			) {
				symbol=find_word(model->dictionary, keys->entry[i]);
				return(symbol);
			}
			++i;
			if(i==keys->size) i=0;
			if(i==stop) return(symbol);
		}
	}

	return(symbol);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	New_Swap
 *
 *		Purpose:		Allocate a new swap structure.
 */
SWAP *new_swap(void)
{
	SWAP *list;

	list=(SWAP *)malloc(sizeof(SWAP));
	if(list==NULL) {
		error("new_swap", "Unable to allocate swap");
		return(NULL);
	}
	list->size=0;
	list->from=NULL;
	list->to=NULL;

	return(list);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Add_Swap
 *
 *		Purpose:		Add a new entry to the swap structure.
 */
void add_swap(SWAP *list, char *s, char *d)
{
	list->size+=1;

	if(list->from==NULL) {
		list->from=(STRING *)malloc(sizeof(STRING));
		if(list->from==NULL) {
			error("add_swap", "Unable to allocate list->from");
			return;
		}
	}

	if(list->to==NULL) {
		list->to=(STRING *)malloc(sizeof(STRING));
		if(list->to==NULL) {
			error("add_swap", "Unable to allocate list->to");
			return;
		}
	}

	list->from=(STRING *)realloc(list->from, sizeof(STRING)*(list->size));
	if(list->from==NULL) {
		error("add_swap", "Unable to reallocate from");
		return;
	}

	list->to=(STRING *)realloc(list->to, sizeof(STRING)*(list->size));
	if(list->to==NULL) {
		error("add_swap", "Unable to reallocate to");
		return;
	}

	list->from[list->size-1].length=strlen(s);
	list->from[list->size-1].word=strdup(s);
	list->to[list->size-1].length=strlen(d);
	list->to[list->size-1].word=strdup(d);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Initialize_Swap
 *
 *		Purpose:		Read a swap structure from a file.
 */
SWAP *initialize_swap(char *filename)
{
	SWAP *list;
	FILE *file=NULL;
	char buffer[1024];
	char *from;
	char *to;

	list=new_swap();

	if(filename==NULL) return(list);

	file=fopen(filename, "r");
	if(file==NULL) return(list);

	while(!feof(file)) {

		if(fgets(buffer, 1024, file)==NULL) break;
		if(buffer[0]=='#') continue;
		from=strtok(buffer, "\t ");
		to=strtok(NULL, "\t \n#");

		add_swap(list, from, to);
	}

	fclose(file);
	return(list);
}

/*---------------------------------------------------------------------------*/

void free_swap(SWAP *swap)
{
	register int i;

	if(swap==NULL) return;

	for(i=0; i<swap->size; ++i) {
		free_word(swap->from[i]);
		free_word(swap->to[i]);
	}
	free(swap->from);
	free(swap->to);
	free(swap);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Initialize_List
 *
 *		Purpose:		Read a dictionary from a file.
 */
DICTIONARY *initialize_list(char *filename)
{
	DICTIONARY *list;
	FILE *file=NULL;
	STRING word;
	char *string;
	char buffer[1024];

	list=new_dictionary();

	if(filename==NULL) return(list);

	file=fopen(filename, "r");
	if(file==NULL) return(list);

	while(!feof(file)) {

		if(fgets(buffer, 1024, file)==NULL) break;
		if(buffer[0]=='#') continue;
		string=strtok(buffer, "\t \n#");

		if((string!=NULL)&&(strlen(string)>0)) {
			word.length=strlen(string);
			word.word=strdup(buffer);
			add_word(list, word);
		}
	}

	fclose(file);
	return(list);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Delay
 *
 *		Purpose:		Display the string to stdout as if it was typed by a human.
 */
void delay(char *string)
{
	register int i;
	
	/*
	 *		Don't simulate typing if the feature is turned off
	 */
	if(typing_delay==FALSE)	{
		fprintf(stdout, string);
		return;
	}

	/*
	 *		Display the entire string, one character at a time
	 */
	for(i=0; i<(int)strlen(string)-1; ++i) typein(string[i]);
	usleep((D_THINK+rnd(V_THINK)-rnd(V_THINK))/2);
	typein(string[i]);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Typein
 *
 *		Purpose:		Display a character to stdout as if it was typed by a human.
 */
void typein(char c)
{
	/*
	 *		Standard keyboard delay
	 */
	usleep(D_KEY+rnd(V_KEY)-rnd(V_KEY));
	fprintf(stdout, "%c", c);
	fflush(stdout);
	
	/*
	 *		A random thinking delay
	 */
	if((!isalnum(c))&&((rnd(100))<P_THINK))
		usleep(D_THINK+rnd(V_THINK)-rnd(V_THINK));
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Ignore
 *
 *		Purpose:		Log the occurrence of a signal, but ignore it.
 */
void ignore(int sig)
{
	if(sig!=0) warn("ignore", "MegaHAL received signal %d", sig);

#if !defined(DOS)
	signal(SIGINT, ignore);
	signal(SIGILL, die);
	signal(SIGSEGV, die);
#endif
	signal(SIGFPE, die);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Die
 *
 *		Purpose:		Log the occurrence of a signal, and exit.
 */
void die(int sig)
{
	error("die", "MegaHAL received signal %d", sig);
	exithal();
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Rnd
 *
 *		Purpose:		Return a random integer between 0 and range-1.
 */
int rnd(int range)
{
	static bool flag=FALSE;

	if(flag==FALSE) {
#if defined(__mac_os) || defined(DOS)
		srand(time(NULL));
#else
		srand48(time(NULL));
#endif
	}
	flag=TRUE;
#if defined(__mac_os) || defined(DOS)
	return(rand()%range);
#else
	return(floor(drand48()*(double)(range)));
#endif
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Usleep
 *
 *		Purpose:		Simulate the Un*x function usleep.  Necessary because
 *						Microsoft provide no similar function.  Performed via
 *						a busy loop, which unnecessarily chews up the CPU.
 *						But Windows '95 isn't properly multitasking anyway, so
 *						no-one will notice.  Modified from a real Microsoft
 *						example, believe it or not!
 */
#if defined(DOS) || defined(__mac_os)
void usleep(int period)
{
	clock_t goal;

	goal=(clock_t)(period*CLOCKS_PER_SEC)/(clock_t)1000000+clock();
	while(goal>clock());
}
#endif

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Strdup
 *
 *		Purpose:		Provide the strdup() function for Macintosh.
 */
#ifdef __mac_os
char *strdup(const char *str)
{
	char *rval=(char *)malloc(strlen(str)+1);

	if(rval!=NULL) strcpy(rval, str);

	return(rval);
}
#endif

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Initialize_Speech
 *
 *		Purpose:		Initialize speech output.
 */
#ifdef __mac_os
bool initialize_speech(void)
{
	bool speechExists = false;
	long response;
	OSErr err;

	err = Gestalt(gestaltSpeechAttr, &response);
        
	if(!err) {
		if(response & (1L << gestaltSpeechMgrPresent)) {
			speechExists = true;
		}
	}
	return speechExists;
}
#endif

/*---------------------------------------------------------------------------*/

/*
 *		Function:	changevoice
 *
 *		Purpose:		change voice of speech output.
 */
void changevoice(DICTIONARY* words, int position)
{
#ifdef __mac_os
	register int i, index;
	STRING word={ 1, "#" };
	char buffer[80];
	VoiceSpec voiceSpec;
	VoiceDescription info;
	short count, voiceCount;
	unsigned char* temp;
	OSErr err;
	/*
	 *		If there is less than 4 words, no voice specified.
	 */
	if(words->size<=4) return;

	for(i=0; i<words->size-4; ++i)
		if(wordcmp(word, words->entry[i])==0) {

		err = CountVoices(&voiceCount);
		if (!err && voiceCount) {
			for (count = 1; count <= voiceCount; count++) {
				err = GetIndVoice(count, &voiceSpec);
				if (err) continue;
				err = GetVoiceDescription(&voiceSpec, &info, 
sizeof(VoiceDescription));
				if (err) continue;


				for (temp= info.name; *temp; temp++) {
					if (*temp == ' ')
						*temp = '_';
				}

				/*
				 *		skip command and get voice name
				 */
				index = i + 3;
				strcpy(buffer, words->entry[index].word);
				c2pstr(buffer);
				// compare ignoring case
				if (EqualString((StringPtr)buffer, info.name, false, false)) {
					if (gSpeechChannel) {
						StopSpeech(gSpeechChannel);
						DisposeSpeechChannel(gSpeechChannel);
						gSpeechChannel = nil;
					}
					err = NewSpeechChannel(&voiceSpec, &gSpeechChannel);
					if (!err) {
						p2cstr((StringPtr)buffer);
						printf("Now using %s voice\n", buffer);
						c2pstr(buffer);
						err = SpeakText(gSpeechChannel, &buffer[1], buffer[0]);
					}
				}
			}
		}
	}
#endif
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	listvoices
 *
 *		Purpose:		Display the names of voices for speech output.
 */
void listvoices(void)
{
#ifdef __mac_os
	VoiceSpec voiceSpec;
	VoiceDescription info;
	short count, voiceCount;
	unsigned char* temp;
	OSErr err;

	if(gSpeechExists) {
		err = CountVoices(&voiceCount);
		if (!err && voiceCount) {
			for (count = 1; count <= voiceCount; count++) {
				err = GetIndVoice(count, &voiceSpec);
				if (err) continue;

				err = GetVoiceDescription(&voiceSpec, &info, 
sizeof(VoiceDescription));
				if (err) continue;

				p2cstr(info.name);
				for (temp= info.name; *temp; temp++)
				if (*temp == ' ')
					*temp = '_';
				printf("%s\n",info.name);
			}
		}
	}
#endif
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Speak
 */
void speak(char *output)
{
	if(speech==FALSE) return;
#ifdef __mac_os
	if(gSpeechExists) {
		OSErr err;

		if (gSpeechChannel)
			err = SpeakText(gSpeechChannel, output, strlen(output));
		else {
			c2pstr(output);
			SpeakString((StringPtr)output);
			p2cstr((StringPtr)output);
		}
	}
#endif
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Progress
 *
 *		Purpose:		Display a progress indicator as a percentage.
 */
bool progress(char *message, int done, int total)
{
	static int last=0;
	static bool first=FALSE;
 
	/*
	 *    We have already hit 100%, and a newline has been printed, so nothing
	 *    needs to be done.
	 */
	if((done*100/total==100)&&(first==FALSE)) return(TRUE);

	/*
	 *    Nothing has changed since the last time this function was called,
	 *    so do nothing, unless it's the first time!
	 */
	if(done*100/total==last) {
		if((done==0)&&(first==FALSE)) {
			if (!quiet) fprintf(stderr, "%s: %3d%%", message, done*100/total);
			first=TRUE;
		}
		return(TRUE);
	}

	/*
	 *    Erase what we printed last time, and print the new percentage.
	 */
	last=done*100/total;

	if((done>0) && (!quiet)) fprintf(stderr, "%c%c%c%c", 8, 8, 8, 8);
	if (!quiet) fprintf(stderr, "%3d%%", done*100/total);

	/*
	 *    We have hit 100%, so reset static variables and print a newline.
	 */
	if(last==100) {
		first=FALSE;
		last=0;
		if (!quiet) fprintf(stderr, "\n");
	}

	return(TRUE);
}

/*---------------------------------------------------------------------------*/

void help(void)
{
	int j;

	if ((connected) && (sd)) {
	   for(j=0; j<COMMAND_SIZE2; ++j) {
        	   sprintf(tmp, "PRIVMSG %s :#%-7s: %s\n", chan, command_net[j].word.word, command_net[j].helpstring);
	           write(sd, tmp, strlen(tmp));
        	   }
	   }
	else {
	   for(j=0; j<COMMAND_SIZE; ++j) {
	           printf("#%-7s: %s\n", command[j].word.word, command[j].helpstring);
	           }
	   }
}

/*---------------------------------------------------------------------------*/

void load_personality(MODEL **model)
{
	FILE *file;
	static char *filename=NULL;

	if(filename==NULL) filename=(char *)malloc(sizeof(char)*1);

	/*
	 *		Allocate memory for the filename
	 */
	filename=(char *)realloc(filename,
		sizeof(char)*(strlen(directory)+strlen(SEP)+12));
	if(filename==NULL) error("load_personality","Unable to allocate filename");

	/*
	 *		Check to see if the brain exists
	 */
	if(strcmp(directory, DEFAULT)!=0) {
	sprintf(filename, "%s%s.megahal/megahal.brn", directory, SEP);
	file=fopen(filename, "r");
	if(file==NULL) {
		sprintf(filename, "%s%s.megahal/megahal.trn", directory, SEP);
		file=fopen(filename, "r");
		if(file==NULL) {
			fprintf(stdout, "Unable to change MegaHAL personality to \"%s\".\n"
				"Reverting to MegaHAL personality \"%s\".\n", directory, last);
			free(directory);
			directory=strdup(last);
			return;
		}
	}
	fclose(file);
	fprintf(stdout, "Changing to MegaHAL personality \"%s\".\n", directory);
	}

	/*
	 *		Free the current personality
	 */
	free_model(*model);
	free_words(ban);
	free_dictionary(ban);
	free_words(aux);
	free_dictionary(aux);
	free_words(grt);
	free_dictionary(grt);
	free_swap(swp);

	/*
	 *		Create a language model.
	 */
	*model=new_model(order);

	/*
	 *		Train the model on a text if one exists
	 */
	sprintf(filename, "%s%s.megahal/megahal.brn", directory, SEP);
	if(load_model(filename, *model)==FALSE) {
		sprintf(filename, "%s%s.megahal/megahal.trn", directory, SEP);
		train(*model, filename);
	}

	/*
	 *		Read a dictionary containing banned keywords, auxiliary keywords,
	 *		greeting keywords and swap keywords
	 */
	sprintf(filename, "%s%smegahal.ban", directory, SEP);
	ban=initialize_list(filename);
	sprintf(filename, "%s%smegahal.aux", directory, SEP);
	aux=initialize_list(filename);
	sprintf(filename, "%s%smegahal.grt", directory, SEP);
	grt=initialize_list(filename);
	sprintf(filename, "%s%smegahal.swp", directory, SEP);
	swp=initialize_swap(filename);
}

/*---------------------------------------------------------------------------*/

void change_personality(DICTIONARY *command, int position, MODEL **model)
{
	if(last!=NULL) { free(last); last=NULL; }
	if(directory!=NULL) last=strdup(directory);
	else directory=(char *)malloc(sizeof(char)*1);
	if(directory==NULL)
		error("change_personality", "Unable to allocate directory");
	if((command==NULL)||((position+2)>=command->size)) {
		directory=(char *)realloc(directory, sizeof(char)*(strlen(DEFAULT)+1));
		if(directory==NULL)
			error("change_personality", "Unable to allocate directory");
		strcpy(directory, DEFAULT);
		if(last==NULL) last=strdup(directory);
	} else {
		directory=(char *)realloc(directory,
			sizeof(char)*(command->entry[position+2].length+1));
		if(directory==NULL)
			error("change_personality", "Unable to allocate directory");
		strncpy(directory, command->entry[position+2].word,
			command->entry[position+2].length);
		directory[command->entry[position+2].length]='\0';
	}

	load_personality(model);
}

/*---------------------------------------------------------------------------*/

void free_words(DICTIONARY *words)
{
	register int i;

	if(words==NULL) return;

	if(words->entry!=NULL)
		for(i=0; i<words->size; ++i) free_word(words->entry[i]);
}

/*---------------------------------------------------------------------------*/

void free_word(STRING word)
{
	free(word.word);
}

/*===========================================================================*/

/*
 *		$Log: megahal.c,v $
 *		Revision 1.23  1998/05/19 03:02:02  hutch
 *		Removed a small malloc() bug, and added a progress display for
 *		generate_reply().
 *
 *		Revision 1.22  1998/04/24 03:47:03  hutch
 *		Quick bug fix to get sunos version to work.
 *
 *		Revision 1.21  1998/04/24 03:39:51  hutch
 *		Added the BRAIN command, to allow user to change MegaHAL personalities
 *		on the fly.
 *
 *		Revision 1.20  1998/04/22 07:12:37  hutch
 *		A few small changes to get the DOS version to compile.
 *
 *		Revision 1.19  1998/04/21 10:10:56  hutch
 *		Fixed a few little errors.
 *
 *		Revision 1.18  1998/04/06 08:02:01  hutch
 *		Added debugging stuff, courtesy of Paul Baxter.
 *
 *		Revision 1.17  1998/04/02 01:34:20  hutch
 *		Added the help function and fixed a few errors.
 *
 *		Revision 1.16  1998/04/01 05:42:57  hutch
 *		Incorporated Mac code, including speech synthesis, and attempted
 *		to tidy up the code for multi-platform support.
 *
 *		Revision 1.15  1998/03/27 03:43:15  hutch
 *		Added AMIGA specific changes, thanks to Dag Agren.
 *
 *		Revision 1.14  1998/02/20 06:40:13  hutch
 *		Tidied up transcript file format.
 *
 *		Revision 1.13  1998/02/20 06:26:19  hutch
 *		Fixed random number generator and Seed() function (thanks to Mark
 *		Tarrabain), removed redundant code left over from the Loebner entry,
 *		prettied things up a little and destroyed several causes of memory
 *		leakage (although probably not all).
 *
 *		Revision 1.12  1998/02/04 02:55:11  hutch
 *		Fixed up memory allocation error which caused SunOS versions to crash.
 *
 *		Revision 1.11  1998/01/22 03:16:30  hutch
 *		Fixed several memory leaks, and the frustrating bug in the
 *		Write_Input routine.
 *
 *		Revision 1.10  1998/01/19 06:44:36  hutch
 *		Fixed MegaHAL to compile under Linux with a small patch credited
 *		to Joey Hess (joey@kitenet.net).  MegaHAL may now be included as
 *		part of the Debian Linux distribution.
 *
 *		Revision 1.9  1998/01/19 06:37:32  hutch
 *		Fixed a minor bug with end-of-sentence punctuation.
 *
 *		Revision 1.8  1997/12/24 03:17:01  hutch
 *		More bug fixes, and hopefully the final contest version!
 *
 *		Revision 1.7  1997/12/22  13:18:09  hutch
 *		A few more bug fixes, and non-repeating implemented.
 *
 *		Revision 1.6  1997/12/22 04:27:04  hutch
 *		A few minor bug fixes.
 *
 *		Revision 1.5  1997/12/15 04:35:59  hutch
 *		Final Loebner version!
 *
 *		Revision 1.4  1997/12/11 05:45:29  hutch
 *		The almost finished version.
 *
 *		Revision 1.3  1997/12/10 09:08:09  hutch
 *		Now Loebner complient (tm).
 *
 *		Revision 1.2  1997/12/08 06:22:32  hutch
 *		Tidied up.
 *
 *		Revision 1.1  1997/12/05  07:11:44  hutch
 *		Initial revision (lots of files were merged into one, RCS re-started)
 *
 *		Revision 1.7  1997/12/04 07:07:13  hutch
 *		Added load and save functions, and tidied up some code.
 *
 *		Revision 1.6  1997/12/02 08:34:47  hutch
 *		Added the ban, aux and swp functions.
 *
 *		Revision 1.5  1997/12/02 06:03:04  hutch
 *		Updated to use a special terminating symbol, and to store only
 *		branches of maximum depth, as they are the only ones used in
 *		the reply.
 *
 *		Revision 1.4  1997/10/28 09:23:12  hutch
 *		MegaHAL is babbling nicely, but without keywords.
 *
 *		Revision 1.3  1997/10/15  09:04:03  hutch
 *		MegaHAL can parrot back whatever the user says.
 *
 *		Revision 1.2  1997/07/21 04:03:28  hutch
 *		Fully working.
 *
 *		Revision 1.1  1997/07/15 01:55:25  hutch
 *		Initial revision.
 */

/*===========================================================================*/

