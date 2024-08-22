
#include <stddef.h>
#include <stdio.h>  
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>       //umask
#include <signal.h>         //signal
#include <sys/wait.h>       //wait
#include <fcntl.h>          //O_RDONLY.....
#include <sys/times.h>      //times
#include <math.h>           //parse times
#include <pwd.h>            //getpwnam.metacharacters
#include <limits.h>         //limit
#include <sys/resource.h>   //limit

#define MAX_ARGS 128
#define MAXBUFF 2048

// needed to internal function time
static struct tms initTimeMinishell;
static clock_t initClockMinishell;

extern char **environ;

//Auxiliary methods
void setVarEnt(const char *name, const char *value);                        // Set enviroment variables
int internalCommand(char **argv, int argc);                                 // Internal commands (cd, umask, time, read, limit, set)
void metacharacters(char ** argv, int argc);                                // Metacharacters functions (~ $)
int parse_line_input(char ****argvv, char **filev, int *is_background);     // Parsing the input line 


int main(void){

    initClockMinishell = times(&initTimeMinishell); // It starts the clock of minishel


    char ***argvv = NULL; // Complete secuence(line)
    int argvc;            // Num args of whole secuence(line)

    char **argv = NULL;   // Secuence of a secuence argvv
    int argc;             // Num of secuence of a secuence argvv


    char *filev[3] = { NULL, NULL, NULL };
    int bg;
    char strAux[10];      // Varible for casting int -> string
    int status;           // aux var to save process status

    // Unbuffering
    setbuf(stdout, NULL);
    setbuf(stdin, NULL);

    // Ignoring signal INT and QUIT
    signal(SIGINT, SIG_IGN);
    signal (SIGQUIT, SIG_IGN);

    setVarEnt("prompt", "minishell> "); // Setting the minishell promt message as enviroment variable
    sprintf(strAux, "%d", getpid());    // cast int -> string, setenv(,strAux,)
    setVarEnt("mypid", strAux);

    while (1) {
        reboot:;                      // Go back tag in case of redirection errors 
        fprintf(stderr, "%s", getenv("prompt"));    // Minishel promt :)
        argvc = parse_line_input(&argvv, filev, &bg);
        if (argvc == -1) break;       // EOF
        if (argvc == 0) continue;     // Syntax error
        if (argvc == 0) continue;     // Empty line

        int numCommand = argvc;
        int fdPipe[2], pid, lastPid;
        int stdin = dup(0); 
        int stdout = dup(1);
        int stderr = dup(2);    

        //Handling redirections
        int fdHandler;
        if (filev[0]) {
            if ((fdHandler = open(filev[0], O_RDONLY)) < 0) {
                perror("open handlerIn");
                // Restoring descriptor in, out and err.
                close(stdin);
                close(stdout);
                close(stderr);                  
                goto reboot; // Go back to minishell prompt
            }
            if(dup2(fdHandler, 0)<0)
                perror("Error duplicating input file descriptor.");
            close(fdHandler);
            
        }

        if (filev[1]) {
            if ((fdHandler = creat(filev[1], 0666)) < 0) {
                perror("open handlerOut");
                // Restoring descriptor in, out and err.
                close(stdin);
                close(stdout);
                close(stderr);                          
                goto reboot;// Go back to minishell prompt
            }
            if(dup2(fdHandler, 1)<0)
                perror("Error duplicating out file descriptor.");
            close(fdHandler);    
        }

        if (filev[2]) {
            if ((fdHandler = creat(filev[2], 0666)) < 0) {
                perror("open handlerErr");
                // Restoring descriptor in, out and err.
                close(stdin);
                close(stdout);
                close(stderr);                          
                goto reboot;// Go back to minishell prompt
            }
            if (dup2(fdHandler, 2)<0)
                perror("Error duplicating error file descriptor.");
            close(fdHandler);    
        } 

        for (argvc = 0; (argv = argvv[argvc]); argvc++) { // Loop the command secuence
            for (argc = 0; argv[argc]; argc++){}          // Token count per command
                lastPid=-1;

                metacharacters(argv, argc);
                
                
                //printf("argc: %d\n", argc);
                
                if((strcmp(argv[0], "cd")    == 0 || strcmp(argv[0], "umask") == 0  || 
                    strcmp(argv[0], "time")  == 0 || strcmp(argv[0], "read")  == 0 || 
                    strcmp(argv[0], "limit") == 0 || strcmp(argv[0], "set")   == 0) 
                    && !bg && argvc == numCommand -1) {

                    status = internalCommand(argv, argc);
                    sprintf(strAux, "%d", status); //cast int -> string, setenv(,strAux,)
                    setVarEnt("status", strAux);

                }else{ // If not internal command
                    if(argvc != numCommand -1 ){ // If not last command
                        if (pipe(fdPipe) < 0) {
                            perror("Error with pipe\n");
                        }
                    }
                    switch(pid = fork()) {
                        case -1:
                            perror("fork");
                        case 0: // Child
                            if(!bg) {
                                // Reactivating ignored foreground signals
                                signal(SIGINT, SIG_DFL);    // Setting INT to default
                                signal (SIGQUIT, SIG_DFL);  // Setting QUIT to default
                            }

                            // It implements an iterative solution. Inserting a process "into" the standar input
                            // and the last one is handled different.
                            if(argvc == numCommand -1 ){} // Last command
                            else{
                                dup2(fdPipe[1],STDOUT_FILENO);
                                close(fdPipe[0]);
                                close(fdPipe[1]);
                            }

                            execvp(argv[0], &argv[0]);
                            perror(argv[0]);
                            exit(1);
                            // Finish child

                        default:    // parent

                            if(bg){ // Background process
                                sprintf(strAux, "%d", pid); // cast int -> string, setenv(,strAux,)
                                setVarEnt("bgpid", strAux); // Save its process ID in bgpid
                            }

                            if(argvc == numCommand -1 ){}   // Last command
                            else{
                                dup2(fdPipe[0],STDIN_FILENO);
                                close(fdPipe[0]);
                                close(fdPipe[1]);
                            }
                            lastPid=pid; // Recovering last PID to wait process
                    } // End switch
                } // End else(No internal command)
        }// End for

            
        if (!bg) { // If no backgroung process, waits...
            if (lastPid > 0) {
                waitpid(lastPid, &status, 0);
                if (WIFEXITED(status)) {
                    sprintf(strAux, "%d", status); // cast int -> string, setenv(,strAux,)
                    setVarEnt("status", strAux);   // Saving last process status.
                }
            }
        }
        // Restoring file descriptors
        dup2(stdin, 0); 
        dup2(stdout, 1);
        dup2(stderr, 2);
        // Closing file descriptors unused
        close(stdin);
        close(stdout);
        close(stderr);
    }
    exit(0);
    return 0;//Ending OK!!
} // End main



// This function parse the line in commands including operators ( > < & and pipelines)
int parse_line_input(char ****argvv, char **filev, int *is_background) {
    char line[MAXBUFF];
    char *token;
    int argc = 0;
    int argv_idx = 0;
    int bg_flag = 0;
    char **argv = NULL;

    // Initializing outputs
    *argvv = NULL;
    filev[0] = NULL;
    filev[1] = NULL;
    filev[2] = NULL;
    *is_background = 0;

    // Reading command line
    if (fgets(line, MAXBUFF, stdin) == NULL) {
        return -1;  // End Of File (EOF)
    }

    // Removing line break
    line[strcspn(line, "\n")] = '\0';

    // Reserving memory for argvv
    *argvv = malloc(MAX_ARGS * sizeof(char **));
    if (*argvv == NULL) {
        perror("Error reserving memory for argvv.");
        return 0;
    }

    // Procesing command line
    token = strtok(line, " ");
    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            // Redirecting input
            token = strtok(NULL, " ");
            if (token != NULL) {
                filev[0] = strdup(token);
            } else {
                fprintf(stderr, "Error: Missing file of redirecting input.\n");
                return 0;
            }
        } else if (strcmp(token, ">") == 0) {
            // Redirecting output
            token = strtok(NULL, " ");
            if (token != NULL) {
                filev[1] = strdup(token);
            } else {
                fprintf(stderr, "Error: Missing file of redirecting output.\n");
                return 0;
            }
        } else if (strcmp(token, "2>") == 0) {
            // Redirecting error
            token = strtok(NULL, " ");
            if (token != NULL) {
                filev[2] = strdup(token);
            } else {
                fprintf(stderr, "Error: Missing file of redirecting error.\n");
                return 0;
            }
        } else if (strcmp(token, "&") == 0) {
            // Background process
            bg_flag = 1;
        } else if (strcmp(token, "|") == 0) {
            // Spliting commands
            (*argvv)[argv_idx++] = argv;
            argc = 0;
            argv = NULL;
        } else {
            if (argc == 0) {
                argv = malloc(MAX_ARGS * sizeof(char *));
                if (argv == NULL) {
                    perror("Error reserving memory for argv.");
                    return 0;
                }
            }
            argv[argc++] = strdup(token);
        }
        token = strtok(NULL, " ");
    }

    // Storing last command 
    if (argc > 0) {
        argv[argc] = NULL;
        (*argvv)[argv_idx++] = argv;
    }

    (*argvv)[argv_idx] = NULL;
    *is_background = bg_flag;

    return argv_idx;
}




//-------------INTERNAL COMMANDS-------------------
// Command cd
int CommandCd(char ** argv, int argc){

    // run: cd [Path]

    if (argc > 2) {
        fprintf(stderr,"Error with number of arguments.\n");
        return 1;
    }

    char currentDirectory[MAXBUFF];
    // Call cd without args:  cd \n
    if(argc == 1){
        chdir(getenv("HOME"));              // Change directory to $HOME
        getcwd(currentDirectory, MAXBUFF);  // Get current directory, alredy changed
        printf("%s\n", currentDirectory);   // Print with format

    // Call with args:   cd Path
    }else{
        if(chdir(argv[1]) == 0){;               // Change directory to given
            getcwd(currentDirectory, MAXBUFF);
            printf("%s\n", currentDirectory);   // Print with format
        // Handling some common errors
        }else if(errno == ENOENT){
            fprintf(stderr,"Directory does not exist.\n");
            return 11;
        }
        else if(errno == EACCES){
            fprintf(stderr,"Error with access permissions.\n");
            return 12;
        }
        else if(errno == ENOTDIR){
            fprintf(stderr,"Error input is not a directory.\n");
            return 13;
        }
    }
    return 0;// OK!!
}

// Command umask
int CommandUmask(char **argv, int argc) {

    // run: umask [Value] .Value->Octal number

    if (argc > 2) {
        fprintf(stderr,"Error with number of arguments.\n");
        return 1;
    }

    mode_t currentMask = umask(0);//backup current mask, in case of errors
    char *endptr;
    long value;

    // Call with args:   umask Value
    if(argv[1]){
        value = strtol(argv[1], &endptr, 8); //casting string to long int and octal base
        if(value == 0){
            umask(currentMask); // Restoring original mask
            fprintf(stderr,"Error with format mask value.\n");
            return 21;
        }else if(errno == ERANGE){
            umask(currentMask); // Restoring original mask
            fprintf(stderr,"Error mask value out of range.\n");
            return 22;
        }
        currentMask = value;

    // Call without args:   umask \n    
    }else{
        printf("%o\n", currentMask); /// Print with format
    }
    umask(currentMask); // Restore original mask in case of errors or change to the new one

    return 0; // OK!!
}

// Command time
int CommandTime(char ** argv, int argc){

    // run: time [command]
    
    // Needed parameters
    struct tms finalTime; // To save the final timing when needed
    clock_t endClock;

    double intUser, decUser, intSys, decSys, intReal, decReal; // Needed to parse the timing in format
    
    // Call with no arg:    time \n
    if (argc == 1) {
        endClock = times(&finalTime);       // Stop clock for Minishell
        // Parsing with lib "mod" to print with format
        decUser = modf(finalTime.tms_utime - initTimeMinishell.tms_utime, &intUser);
        decSys = modf(finalTime.tms_stime - initTimeMinishell.tms_stime,&intSys);
        decReal = modf(endClock - initClockMinishell, &intReal);
        // Minishell timing
        printf("%d.%03du %d.%03ds %d.%03dr\n", (int)intUser, (int)(decUser *1000), (int)intSys, (int)(decSys *1000), (int)intReal, (int)(decReal *1000));

        // Parsing with lib "mod" to print with format
        decUser = modf(finalTime.tms_cutime - initTimeMinishell.tms_cutime, &intUser);
        decSys = modf(finalTime.tms_cstime - initTimeMinishell.tms_cstime,&intSys);
        // Child timing
        printf("%d.%03du %d.%03ds %d.%03dr\n", (int)intUser, (int)(decUser *1000), (int)intSys, (int)(decSys *1000), (int)intReal, (int)(decReal *1000));

    // Call with arg:    time command
    }else{
        // Params needed to cath the command time
        struct tms initTime;         // To save the initial time
        clock_t initClock;
        char ** argvTime = &argv[1]; // Save command time
        char strAux[10];             // Variable auxiliar para cast de int -> string
        int pidTime, status;

        // Call with arg:    time InternalCommand
        if(strcmp(argv[0], "cd")    == 0 || strcmp(argv[0], "umask") == 0 || 
           strcmp(argv[0], "time")  == 0 || strcmp(argv[0], "read")  == 0 || 
           strcmp(argv[0], "limit") == 0 || strcmp(argv[0], "set")   == 0)  {
            initClock = times(&initTime);   // Start clock for this command
            status = internalCommand(argvTime, argc - 1);
            endClock = times(&finalTime);   // Stop clock for this command
            sprintf(strAux, "%d", status);  // cast int -> string, setenv(,strAux,)
            setVarEnt("status", strAux);

            // Parsing with lib "mod" to print with format
            decUser = modf(finalTime.tms_utime - initTime.tms_utime, &intUser);
            decSys = modf(finalTime.tms_stime - initTime.tms_stime,&intSys);
            decReal = modf(endClock - initClock, &intReal);
            // Internal command timing
            printf("%d.%03du %d.%03ds %d.%03dr\n", (int)intUser, (int)(decUser *1000), (int)intSys, (int)(decSys *1000), (int)intReal, (int)(decReal *1000));

        // Call with arg:    time no InternalCommand
        }else{
            initClock = times(&initTime); // Start clock for this command

            switch(pidTime = fork()) {
                case -1:
                    perror("fork");
                case 0:     // Child
                    execvp(argv[1], &argv[1]);
                    perror(argv[1]);
                    exit(1);            
                default:    // Parent
                    waitpid(pidTime, &status, 0);
                    endClock = times(&finalTime);  // Stop clock for this command
                    sprintf(strAux, "%d", status); //cast int -> string, setenv(,strAux,)
                    setVarEnt("status", strAux);
            }

            // Parsing with lib "mod" to print with format
            decUser = modf(finalTime.tms_utime - initTime.tms_utime, &intUser);
            decSys = modf(finalTime.tms_stime - initTime.tms_stime,&intSys);
            decReal = modf(endClock - initClock, &intReal);
            // No internal command timing
            printf("%d.%03du %d.%03ds %d.%03dr\n", (int)intUser, (int)(decUser *1000), (int)intSys, (int)(decSys *1000), (int)intReal, (int)(decReal *1000));       
        }
    }
    return 0; // OK!!
}

// Command read
int CommandRead(char ** argv, int argc){

    // run: read variable [Variable...]

    // Needed parameters
    char *variable, *value;
    char varFinal[MAXBUFF]; // Final variable value
    char buff[MAXBUFF];     // Save read line
    int ivar=1;             // Variable count


    fgets (buff, sizeof(buff), stdin);
    buff[strcspn(buff, "\n")] = 0;

    // Handling errors
    if (argc < 2) { 
        fprintf(stderr,"Error with number of arguments.\n");
        return 1;
    }

    value = strtok(buff, " \t");

    while(value !=NULL){
        variable = argv[ivar];
        strcpy(varFinal, value);
        value = strtok(NULL, " \t");

        if(ivar == argc -1){                     // Last or only variable
            while(value !=NULL){                 // while still letters
                if(value != NULL){
                    strcat(varFinal, " ");       // adding space like stdin
                    strcat(varFinal, value);     // adding space like stdin
                    value = strtok(NULL, " \t"); //next...
                }
            }               
        }else{
            ivar++;         
        }
        varFinal[strlen(varFinal)] = '\0';
        setVarEnt(variable, varFinal);
        memset(varFinal,0,strlen(varFinal)); // Clean memory
    }// End while
    return 0; // OK!!
}



// Command set
int commandSet(char **argv, int argc) {

    // run: set [variable [Value...]]

    char *variable = argv[1];
    int i;
    char value[MAXBUFF] = "";
    
    // Call with arg variable: set Variable...
    if (variable) {
        // Call with arg variable and value: set Variable Value
        if (argc > 2) {
            for (i = 2; argv[i]; i++) {
                strcat(value, argv[i]);
                strcat(value, " ");
            }
            value[strlen(value) - 1] = '\0';
            setVarEnt(variable, value);

        // Call with arg variable and no value: set Variable
        } else {
            printf("%s=%s\n", variable, getenv(variable));
        }

    // Call with no arg: set \n
    } else {
        for (i = 0; environ[i]; i++) {
            printf("%s\n", environ[i]);
        }
    }
    return 0; // OK!!
}


struct rs_types {
    const char * name;
    int number;
    long ceiling;
};

// Resources limits (user for internal command limit)
struct rs_types resArguments[] = {
        {"cpu",    RLIMIT_CPU, -1},
        {"fsize",  RLIMIT_FSIZE, -1},
        {"data",   RLIMIT_DATA, -1},
        {"stack",  RLIMIT_STACK, -1},
        {"core",   RLIMIT_CORE, -1},
        {"nofile", RLIMIT_NOFILE, 2048},    //max fd in linux 2048
        {NULL, -1}
};


// Command limit
int commandLimit(char **argv, int argc) {

    // run: limit [Resource[Maximum]]

    if (argc > 3) {
        return 1;
    }

    const char *resName = argv[1];  // Resource's name given
    char *maxVal = argv[2];         // Max value given
    struct rlimit limit;
    int resource;
    long value;
    int curVal;
    char *aux;
    int i;

    // Call with arg Resource:  limit Resource...
    if (resName) {
        resource = -1;
        // Searching resource
        for (i = 0; resArguments[i].name != NULL; i++) {
            if (!strcmp(resArguments[i].name, resName)) {
                resource = resArguments[i].number;          // Save descriptor
                limit.rlim_max = resArguments[i].ceiling;   // and limit celing
                break;
            }
        }
        if (resource < 0) {
            //fprintf(stderr, "Error with resource's name.\n");
            return 4;
        }
        // Call with arg Resource and Value:  limit Resource Value
        if (maxVal) {
            value = strtol(maxVal, &aux, 10);
            //Handling errors
            errno = 0;
            if ((errno == ERANGE && (value == LONG_MAX || value == LONG_MIN)) || (errno != 0 && value == 0)) {
                return 2;
            }
            if (maxVal == aux || *aux != 0) {
                return 5;
            }
            limit.rlim_cur = value;

            if (setrlimit(resource, &limit)) {
                return 6;
            }

        // Call with arg Resource and no Value:  limit Resource
        } else {
            if (getrlimit(resource, &limit)) {
                return 3;
            }
            curVal = limit.rlim_cur;
            printf("%s\t%d\n", resName, curVal); // Print resource with limit
        }

    // Call with no arg
    } else {
        for (i = 0; resArguments[i].name != NULL; i++) {
            resource = resArguments[i].number;
            if (getrlimit(resource, &limit)) {
                return 2;
            }
            curVal = limit.rlim_cur;
            printf("%s\t%d\n", resArguments[i].name, curVal); // Print minishell limits
        }
    }
    return 0; // OK!!
}


//End----------INTERNAL COMMANDS-------------------

/*---------------AUXILIARY COMMANDS---------------*/

// Control enviroment variables
void setVarEnt(const char *name, const char *value) {
    if (setenv(name, value, 1) < 0) {
        perror("Error, setting enviroment variable.");
    }
}



// Control internal commands
int internalCommand(char **argv, int argc) {
    if        (strcmp("cd", argv[0]) == 0) {
        return CommandCd(argv, argc);
    } else if (strcmp("umask", argv[0]) == 0) {
        return CommandUmask(argv, argc);
    } else if (strcmp("time", argv[0]) == 0) {
        return CommandTime(argv, argc);
    } else if (strcmp("read", argv[0]) == 0) {
        return CommandRead(argv, argc);
    } else if (strcmp("limit", argv[0]) == 0) {
        return commandLimit(argv, argc);     }
     else if (strcmp("set", argv[0]) == 0) {
        return commandSet(argv, argc);
    }
    return -1;
}

void metacharacters(char ** argv, int argc){

    //Cases:
        //Case 1: ~[User] 
        //Case 2: $Variable

    char *charAddr; // To save metachar position
    int i;

    for (i = 0; i < argc; i++) {
        //// For metachar ~ (tilde)
        charAddr= strchr(argv[i], '~'); // Search for ~

        if(charAddr!=NULL){
            char *user, *dir;
            struct passwd *dirPwd;
            sscanf(charAddr, "~%m[_a-zA-Z0-9]", &user);

            // Call with arg:   ~User
            if(user!=NULL){//Si hay parametro Usuario, llamada ~Usuario, //imprime $HOME de Usuario
                if((dirPwd = getpwnam(user))==NULL){
                    fprintf(stderr,"Error, The input user does not exist.\n");
                    break;
                }else
                    dir = dirPwd->pw_dir;

            // Call with no arg:   ~ \n
            }else{
                dir = getenv("HOME");
            }

            argv[i] = realloc(argv[i],strlen(argv[i]) - (user==NULL ? 0 : strlen(user)) + strlen(dir)); // Reserving memory: Original param - swaped(old) + swaped(new)
            charAddr= strchr(argv[i], '~'); // Locating char ~
            strcpy(charAddr, dir);
        } // End if ~
         
        // For metachar $ (dollar)
        charAddr= strchr(argv[i], '$'); // Search for ~
        if(charAddr!=NULL){
            char *variable, *value,*charAddrRemai;
            char copyCommand[strlen(argv[i])+1];
            sscanf(charAddr, "$%m[_a-zA-Z0-9]", &variable); // Verifying format

            // Handling errors if there is arg Variable or does not exist
            if(variable==NULL || (value = getenv(variable))==NULL){
                fprintf(stderr,"Error, enviroment variable.\n");
                break;
            }
            strcpy(copyCommand,argv[i]);

            argv[i] = realloc(argv[i],strlen(argv[i]) - strlen(variable) + strlen(value)); // Reserving memory: Original param - swaped(old) + swaped(new)
            charAddr = strchr(argv[i], '$'); // Locating char $
            charAddrRemai = strchr(copyCommand, '$');           


            strcpy(charAddr, value); // Copy new value
            strcpy(charAddr + strlen(value), charAddrRemai+ strlen(variable)+1); //Copy the remainder


        }// End if $
    }// End for
}



/*End---------------AUXILIARY COMMANDS---------------*/


/*
ERRORS TABLE  *There just a few commons programed errors
        cd
    1   Error with number of arguments
    11  Directory does not exist
    12  Error with access permissions
    13  Error input is not a directory.
    
        umask
    1   Error with number of arguments.
    21  Error with format mask value
    22  Error mask value out of range.

        time

    
        read
    1 Error en el numero de argumentos

        metacharacters
    5

*/
