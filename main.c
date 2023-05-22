#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>
#include "parse.h"
#include <fcntl.h>
#include<signal.h>


int total_backJobs = 0; //To keep track of total number of background jobs

int local_pids[10]; //Stores Local PIDs

struct node{
    int val; //PID Value
    char com[100]; //Name of Command
    struct node * next;
    int pd;
};

struct node *head = NULL; //Linked list for background processes


//Reference: Linked List Code is similar to lab 6

void push(struct node **headaddr, int val, char comm[100]){

    int c = 0;
    int pid;
    while (c < 10){
        if (local_pids[c] == 0){
            local_pids[c] = 1;
            pid = c + 1;
            c = 10;
        }
        c++;
    }

    if (headaddr == NULL){
        fprintf(stderr, "NULL ptr passed\n");
        exit(1);
    }

    struct node * n = malloc(sizeof(struct node));

    if (n == NULL){
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    n->val = val;
    strcpy(n->com, comm);
    n->next = NULL;
    n->pd = pid;

    if (*headaddr == NULL){ //If Linked List is empty
        *headaddr = n; //The head will point at the new node
    }
    else{
        struct node *tmp = *headaddr; 
        while (tmp->next != NULL){ 
            tmp = tmp->next;
        }
        tmp->next = n; //Adds to the end of the list
    }

}

void swap(struct node *a, struct node *b) 
{ 
    int temp_pid = a->pd; 
    int temp_val = a->val;
    
    char temp_comm[100];
    strcpy(temp_comm, a->com);

    a->pd = b->pd; 
    strcpy(a->com, b->com);
    a->val = b->val; 

    b->pd = temp_pid;
    b->val = temp_val; 
    strcpy(b->com,temp_comm);
}

//Reference: https://www.geeksforgeeks.org/c-program-bubble-sort-linked-list/
void bubbleSort(struct node *start) 
{ 
    int swapped, i; 
    struct node *current_ptr; 
    struct node *next_ptr = NULL; 
  
    /* Checking for empty list */
    if (start == NULL) 
        return; 
  
    do
    { 
        swapped = 0; 
        current_ptr = start; 
  
        while (current_ptr->next != next_ptr) 
        { 
            //Checking the local pids of each node 
            if (current_ptr->pd > current_ptr->next->pd) 
            { 
                //If Current pid is greater than the next pid then swap
                swap(current_ptr, current_ptr->next); 
                swapped = 1; 
            } 
            current_ptr = current_ptr->next; 
        } 
        next_ptr = current_ptr; 
    } 
    while (swapped); 
} 
  

void print(struct node * head){ //To print the Linked List

    bubbleSort(head);

    if (head == NULL){
        fprintf(stdout,"There are no processes running\n");
    }
    else{
        while (head != NULL){
            fprintf(stdout, "[%d] %s\n", head->pd, head->com);
            head = head->next;
        }
        //fprintf(stdout, "\n");
    }
}

int exist(struct node * head, int find){ //Checks if node is in Linked List
    if (head == NULL){
        return 0;
        //fprintf(stdout,"empty queue\n");
    }
    else{
        while (head != NULL){
            if (head->val == find){
                return 1;
            }
            head = head->next;
        }
        return 0;
    
    }
    return 0;
}

int local_exist(struct node * head, int find){ //Checks if node is in Linked List

    if (head == NULL){
        return -1;
        //fprintf(stdout,"empty queue\n");
    }
    else{
        while (head != NULL){
            if (head->pd == find){
                return head->val;
            }
            head = head->next;
        }
        return -1;
    
    }
    return -1;
}

int delete(struct node ** headaddr, int find){ //To delete a specific node from the Linked List
    
    if (headaddr == NULL){
        fprintf(stderr, "NULL ptr passed\n");
        return -1;
    }

    if (*headaddr == NULL){
        return -1;
    }

    struct node *a = *headaddr;

    if (a->next == NULL){
        if (a->val == find){
            local_pids[a->pd - 1] = 0;
            free(a);
            *headaddr = NULL;
            return 1;
        }
    }
    else{
        struct node *current = *headaddr;

        if (current->val == find){
            *headaddr = current->next;
            local_pids[current->pd - 1] = 0;
            free(current);
            return 1;
        }
        struct node *previous;
        //struct node *tmp3;

        while (current->val != find && current->next != NULL){
            previous = current;
            current = current->next;

        }

        if (current->val == find){
            previous->next = current->next;
            local_pids[current->pd - 1] = 0;
            free(current);
            return 1;
        }

        return 0;

    }

    return 0;
}

void print_pid(int sig){ //Signal Handler

    int status;
    int id = waitpid(0, &status, WNOHANG);

    int boo;
    boo = exist(head, id);

    if (boo == 1){ //If PID belongs to that of a background process

        int err; 
        err = delete(&head, id); 

        if (err == 0){
            fprintf(stderr, "Delete Failed\n");
        }

        total_backJobs--; 
        
    }
}

int main(){

    printf("Welcome to my Shell\n");

    char cwd[1024];
    parseInfo *Command; 
    int status;

    signal(SIGCHLD, print_pid); //Signals everytime a child process is completed

    char *list_of_commands[10];

    int count = 0; //Keeps count of every command

    //Initialising local_pid array to 0
    for (int i = 0; i < 10; i++){ 
        local_pids[i] = 0;
    }

    while(1){
        int flag = 0; //To check if exec has worked or not 

        //Print Directory
        getcwd(cwd, sizeof(cwd)); //Gets Current Directory

        char t[] = " ";
        strcat(cwd, t);

        //Takes Input
        char *input;
        input = readline(cwd); 

        //Implementation for !CMD
        if (strlen(input) > 1){

            if(input[0] == '!'){

                char *token;
                const char s[2] = "!";
                //Tokenize the string

                token = strtok(input, s);
                //Convert to integer
                int val = atoi(token);

                if (val > count){
                    printf("Command does not exist.\n");
                    continue;
                }
                else if(val < count - 10){
                    printf("Command no longer exists.\n");
                    continue;
                }
                else{

                    if (count <= 10){
                        //printf("%d", val - 1);
                        //printf("Command %s", list_of_commands[val - 1]);
                        strcpy(input, list_of_commands[val - 1]);
                    }
                    else{
                        int difference = count - 9;
                        //printf("%d", val - difference);
                        //printf("Command %s", list_of_commands[val - difference]);
                        strcpy(input, list_of_commands[val - difference]);
                        //input = list_of_commands[val - difference];
                    }
                    
                }
                
            }
        }
        else if (strlen(input) != 0){
            printf("Error: No Command number provided.\n");
        }
        
        //Command is parsed 
        if (strlen(input) != 0){
            Command = parse(input); 
        }
        else{
            continue;
        }
       
        
        if (Command->boolBackground == 1){ 
            //If there are more than 10 Background processes running, user will not be able to run more
            if(total_backJobs >= 10){
                fprintf(stderr, "Maximum background processes running.\n");
                continue; //Exits this iteration
            }
        }
     
        
        struct commandType *first;  
        first = &(Command->CommArray[0]); //The first command given by user - To check for builtin commands
        
        //Implementation for help
        if (strcmp(input, "help") == 0){

            printf("1. jobs - provides a list of all background processes and their local pIDs.\n");
            printf("2. cd PATHNAME - sets the PATHNAME as working directory.\n");
            printf("3. history - prints a list of previously executed commands. Assume 10 as the maximum history size.\n");
            printf("4. kill PID - terminates the background process identified locally with PID in the jobs list.\n");
            printf("5. !CMD - runs the command numbered CMD in the command history.\n");
            printf("6. exit - terminates the shell only if there are no background jobs.\n");
            printf("7. help - prints the list of builtin commands along with their description.\n");

        }

        //Implementation for cd PATHNAME
        else if(strcmp(first->command, "cd") == 0){
            
            if (first->VarList[1] == NULL){
                printf("Error: No Pathname Given.\n");
                //exit(1);
            }
            else{
                int c = chdir(first->VarList[1]);
                if(c != 0){
                    printf("Error. Pathname does not exist.\n");
                    
                }
            }
        }

        //Implementation of kill PID
        else if(strcmp(first->command, "kill") == 0){

            if (first->VarNum > 1){
                char *token;
                const char s[2] = " ";
                //Tokenize the string
                token = strtok(input, s);
                token = strtok(NULL, s);
                //Convert to integer
                int val = atoi(token);

                int global_pid = local_exist(head, val);

                if (global_pid == -1){
                    printf("Command does not exist.\n");
                }
                else{
                    //printf("%d", global_pid);
                    int err = kill(global_pid, SIGKILL);

                    if (err == 0){
                        printf("Kill Successful.\n");
                        continue;
                    }
                    else{
                        printf("Kill Unsuccessful.\n");
                    }
                }
            }
            else{
                printf("Error: Please provide a PID.\n");
            }
        }

        //Implementation of jobs
        else if(strcmp(input, "jobs") == 0){
            print(head);
        }

        //Implementation for exit
        else if(strcmp(input, "exit") == 0){

            if (total_backJobs == 0){ 
                //If there are no background processes running, then program will exit
                return 0;
            }
            else{
                printf("Background processes still running. Try Later.\n");
            }
        }

        //Implementation for history 
        else if(strcmp(input, "history") == 0){

    
            if (count != 0){

                if (count < 10){
                    for (int i = 0; i < count; i++){
                        printf("[%d] %s\n" , i + 1, list_of_commands[i]);
                    }
                }
                else{
                    for (int i = 0; i < 10; i++){
                        printf("[%d] %s\n" , count - (9 - i), list_of_commands[i]);
                    }
                }

            }
            else{
                printf("You have not run any commands.\n");
            }
        }

        else{
            
            if (Command->pipeNum == 0){

                int status;
                int pid = fork();
                int flag = 0;

                if (pid < 0){ 
                    fprintf(stderr, "fork failed.\n");
                    exit(1);
                }
                else if (pid == 0){

                    if (Command->boolInfile){ 

                        if (access(Command->inFile, F_OK) == 0) { //Checks if file exists

                            if ( access(Command->inFile, R_OK) == 0) { //Checks if file can be read
                            
                                int in_file = open(Command->inFile, O_RDONLY); //Open the file for reading
                                int booldup_IN = dup2(in_file, STDIN_FILENO); //Replaces the std input with new file descriptor

                                if(booldup_IN < 0){
                                    printf("unable to duplicate in input.\n");
                                }
                               

                                close(in_file);
                            }
                            else{
                                printf("You do not have permission to read %s\n", Command->inFile);
                                exit(1);
                            }
                        }
                        else{
                            printf("File %s does not Exist.\n", Command->inFile);
                            exit(1);
                        }
                    } 
                    
                    if (Command->boolOutfile){

                        int out_file = open(Command->outFile, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR); 
                        //Opens the file for writing or creates one if it does not exist already
                        int booldup = dup2(out_file, STDOUT_FILENO);
                        //Replaces the std input with new file descriptor

                        if(booldup < 0){
                            printf("unable to duplicate in output\n");
                            exit(1);
                        }

                    }

                    struct commandType *temp; 
                    temp = &(Command->CommArray[0]);

                    int exec_no = execvp(strdup(temp->command), temp->VarList); //Executes the first command 

                    if (exec_no < 0){
                        printf("Ambiquous Command. Please try again.\n");
                        flag = 1; 
                        exit(1);
                    };

                }
                else{

                    if (flag == 0){

                        if (Command->boolBackground == 0){
                            waitpid(pid, &status, 0);  
                        }
                        else{
                            
                            push(&head, pid, input); //If it is a background process then push to the Linked
                            //List containing all background processes.
                            total_backJobs++;
                            
                            
                        }
                    }
                    else{
                        exit(1);
                    }
                    
                    
                }
                
            }
            else if (Command->pipeNum > 0){

                int status;
                int i = 0;
                pid_t pid;


                int pd[2*Command->pipeNum]; 

                for(i = 0; i < Command->pipeNum; i++){ //Creates all the pipes needed
                    if(pipe(pd + i*2) < 0){
                        fprintf(stderr, "Pipe failed\n");
                        exit(1);

                    }
                }

                int j = 0;

                int num = 0;
                struct commandType *comm;
                comm = &(Command->CommArray[num]);

                while(comm->command != NULL){
                    pid = fork();

                    if (pid == 0){

                        // I/O Redirection

                        if (Command->boolInfile){ 

                            if (access(Command->inFile, F_OK) == 0) { //Check if File Exists

                                if ( access(Command->inFile, R_OK) == 0) { //Check if File can be Read
                                
                                    int in_file = open(Command->inFile, O_RDONLY);
                                    int booldup_IN = dup2(in_file, STDIN_FILENO);

                                    if(booldup_IN < 0){
                                        printf("unable to duplicate in input");
                                    }
                                    printf("input happending\n");

                                    close(in_file);
                                }
                                else{
                                    printf("You do not have permission to read %s\n", Command->inFile);
                                }
                            }
                            else{
                                printf("File %s does not Exist\n", Command->inFile);
                                exit(1);
                            }
                    } 
                    
                        if (Command->boolOutfile){

                            int out_file = open(Command->outFile, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);  
                            int booldup = dup2(out_file, STDOUT_FILENO);

                            if(booldup < 0){
                                printf("unable to duplicate in output\n");
                            }

                        }

                        //If not last command

                        //Child will output to next command since it is
                        //not the last command
                        struct commandType *temp;
                        temp = &(Command->CommArray[num + 1]);
                        if (temp != NULL){
                            if (dup2(pd[j+1], 1) < 0){ 
                                fprintf(stderr, "Dup2 Error\n");
                                exit(1);
                
                            }
                        }

                        //if not first command && j != 2* Num of Pipes

                        //Child will get input from previous command 
                        //since it is not the first Command
                        if(j != 0){
                            if (dup2(pd[j-2], 0) < 0){
                                fprintf(stderr, "Dup2 Error\n");
                                exit(1);
                            }
                        }

                        for(i = 0; i < 2*Command->pipeNum; i++){
                            close(pd[i]);
                        }

                        if (execvp(strdup(comm->command), comm->VarList) < 0){
                            fprintf(stderr, "Ambiquous Command. Please try again\n");
                            exit(1);
                        }

                    } else if (pid < 0){
                        fprintf(stderr, "Fork Failed\n");
                        exit(1);
                    }
                    num++;
                    comm = &(Command->CommArray[num]); 
                    j += 2;

                }

                
                //Parent will close all its copies at the end 
                for(i = 0; i < 2 * Command->pipeNum; i++){
                    close(pd[i]);
                }
                
                if (Command->boolBackground == 0){
                    //Waits for all the child processes to finish
                    for(i = 0; i < Command->pipeNum + 1; i++){
                        wait(&status);
                    }
                }
                else{
                    push(&head, pid, input); //If it is a background process then push to the Linked
                    //List containing all background processes.
                    total_backJobs++;
                }
     
            }
            
        }

        //Creating List of Commands
        if (count <= 9){ 
            list_of_commands[count] = strdup(input);
        }
        else{
            char *temp[10];

            for(int i = 0; i < 9; i++){
                temp[i] = strdup(list_of_commands[i+1]);
            }
            temp[9] = strdup(input);

            for(int i = 0; i < 10; i++){
                list_of_commands[i] = strdup(temp[i]);
            }

        }

        count++;
        

    }

}


/*Reference: https://stackoverflow.com/questions/8389033/implementation-of-multiple-pipes-in-c*/
