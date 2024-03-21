/*	
     Author:  Adam Valiant
     Filename:  nautilus.c
     Description:  a basic command-line interface shell for linux implemented in C
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <errno.h>




void  main(void)
{
	// definitions and declarations
	char * input;
	char * shell_prompt = "nautilus>";
	char * token;
	char * safety;
  	char *args[41];		/* command line arguments w/ command at arg[0] */
	char *args2[41];
	char *args3[41];
	pid_t pid;
	int bg_pids[50];
	int status;
	int i;
	int j;
	int k;
	int l;  //zombie index
	int m;
	int amp_position;
	int bg_flag = 0;
	int redirectionflag =  0;
	static int id_index;
	FILE *fp;
	 
	 

	// readline autocomplete feature
	rl_bind_key('\t', rl_complete);
	

	
	
	id_index = 0;
	// infinite loop for prompt display
 	while(1) {

		// look for zombies and reap them
		if(id_index!=0){
			for(l=0; l<id_index; l++)
			{
				if(waitpid(bg_pids[l],&status,WNOHANG) == bg_pids[l])
					printf("\njob %d exited with status 0\n\n", bg_pids[l]);
			}

		}
		
		

 
        // display prompt and read input (note: input must be freed after use since readline uses malloc)
        input = readline(shell_prompt);
        
        if (!input || input == NULL || strcmp(input, "")==0)
            continue;
 
        // add input to readline history.
        add_history(input);	

		// parse out arguments from input and place into args array
		char whitechars[] = " \n\t\r";
		token = strtok_r(input, whitechars, &safety);
		args[0] = token;
		i=1;
		while (token != NULL) {
			token = strtok_r(NULL, whitechars, &safety);
			args[i] = token;
			i++;
		}
		i=1;  //reset index
		
		// if the exit command is given, proceed to exit
		if(strcmp(args[0], "exit")==0)
			exit(0);
		else if(strcmp(args[0], "jobs")==0)
		{
		
			printf("\nLIVE PROCESSES:  \n\n");
			if(id_index!=0){
				for(l=0; l<id_index; l++)
				{
					if(kill(bg_pids[l], 0)==0)
						printf("%d\n", bg_pids[l]);
				}
				printf("\n");
			}
			continue;
			
		}
		else if(strcmp(args[0], "cd")==0)
		{
			chdir(args[1]);
			continue;
		}

		else
		{
			// check for & or >
			j=0;
			k=0;
			while (args[j] != NULL)
			{
				// look for ampersand, save index, and set flag
				if(strcmp(args[j], "&")==0){
					amp_position=j;				// save position of & so that it can be excluded later
					bg_flag = 1;		// background process execution flag
				}
				else if( (strcmp(args[j], ">")==0) )
				//these will be implemented later
				//if( (strcmp(args[0], ">")==0)||(strcmp(args[0], "<")==0)||(strcmp(args[0], ">>")==0)||(strcmp(args[0], "<<")==0))
				{
				redirectionflag = 1;
				k = j+1; // position of file argument
				m = j-1; // position of command and argument whose output is being redirected
				}
				
				
				j++;
			}
			j=0;
			
		}	
			
			
			
		
		 // fork from parent
		 pid = fork();
		 
		if (pid < 0) {
			printf("FORK FAILURE!\n");
			exit(1);
		}
		else if (pid == 0)   // if child is forked, execute
		{
			//child process

			
			
			
			if(bg_flag == 1){ //child background execution
			
				// extract necessary arguments and exclude &
				j=0;
				while (j<amp_position)
				{	
					args2[j] = args[j];			
					j++;
				}
				args2[amp_position] = NULL;
				j=0;
				k=0;


				fclose(stdin); // close child's stdin
				fopen("/dev/null", "r"); // open a new stdin that is always empty

				execvp(args2[0], args2);
				// if this line is reached, execvp has failed
				printf("\nEXECVP ERROR! (bg execution failure)\n\n");
				printf("errno = %d.\n", errno);
				exit(1);
				
			}
			else if(redirectionflag == 1){	//child redirection execution
								
				
				// I/O redirection procedures
				/* File descriptors:
					0 =  stdin
					1 = stdout
				*/
				//open file
				fp = fopen(args[k], "w+");
				//duplicate file descriptors
				dup2( fileno(fp), 1);
				
				
				// extract necessary input
				j=0;
				while (j<=m )
				{	
					args3[j] = args[j];			
					j++;
				}
				args3[m+1] = NULL;
				j=0;
				k=0;
				

				//close((fileno(fp)));
				execvp(args3[0], args3);
				// if this line is reached, execvp has failed
				printf("\nEXECVP ERROR!  (primary execution failure)\n\n");
				printf("errno = %d.\n", errno);
				exit(1);
				
			}
			
			else{ //child foreground execution

				execvp(args[0], args);
				// if this line is reached, execvp has failed
				printf("\nEXECVP ERROR!  (primary execution failure)\n\n");
				printf("errno = %d.\n", errno);
				exit(1);
			}
		}
		else if (pid >0){	// if parent ....
				//parent process
	
	
			// if child executed as a foreground process suspend execution of 
			// the current process (parent) until child has exited...
			//  else if the child was executed as a background process, print appropriate message
			//  and add the child's pid to the list of background processes

			if((bg_flag == 0))		//foreground execution
				pid = wait(&status);
			else if (bg_flag==1){	//background execution
				printf("started job %d\n\n", (int)pid);
				bg_pids[id_index] = pid;
				id_index++;
				bg_flag = 0;	//reset flag
			}
			
			if(redirectionflag==1)	
				redirectionflag =  0; //reset flag

				
				
				


		}
	
	
		// we don't need the input anymore so we free it since
		// readline used malloc
		free(input);

	} //infinite while loop for prompt


	return;
}