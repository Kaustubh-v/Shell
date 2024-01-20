#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>

// the following macros are used to give colour to the shell.
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_RESET   "\x1b[0m"

char *PATH;   // gloabal PATH variable


// concatenating the path with the command
void concatenate_path(char * temp_path ,const char * buf){
  char itr = temp_path[0];
  int temp_i = 0;
  while(itr != '\0'){
    temp_i++;
    itr = temp_path[temp_i];
  }
  temp_path[temp_i] = '/';
  temp_i++;
  itr = buf[0];
  int buf_i = 0;
  while(itr != '\0' && itr != ' '){
    temp_path[temp_i] = itr;
    buf_i++;
    itr = buf[buf_i];
    temp_i++;
  }
  temp_path[temp_i] = '\0';

}

// to check if the executable file of a command exists
int file_exists(char * temp_path , char * buf){
  concatenate_path(temp_path , buf);

  if(access(temp_path , F_OK) == 0){    // if file exitsts
      if(access(temp_path , X_OK) == 0){   // if the users has executable permissions on the file
      strcpy(buf , temp_path);
      return 1;
    }
    else {
      printf("There are no executable permissions to the user for the command : %s\n" , buf);
      return 0;
    }
  }
  else{
    return 0;
  }
  return 0;
}

// function to find the path of the executable file
void find_path(char * buf){
  char iterator = PATH[0];
  char path_itr;
  int temp_i = 0;
  int path_i = 0;

  while (iterator != '\0'){
    path_itr = iterator;
    char  temp_path[32];
    temp_i = 0;
    while (path_itr != ':' && path_itr != '\0') {     // taking each part of the PATH variable
      temp_path[temp_i] = path_itr;
      temp_i++;
      path_i++;
      path_itr = PATH[path_i];
    }
    temp_path[temp_i]= '\0';
    if(file_exists(temp_path , buf)){
      break;
    }
    iterator = PATH[++path_i];
  }
}


// string formatting to separate the command and arguments
void format_input(char * buf , char ** args){
  char command[32];
  int command_i =0;
  int arg_i = 0;
  int args_i = 1;
  int arg_flag = 0;
  int buf_change = 0;
  int buf_i = 0;
  char itr = buf[buf_i];

  while(itr != ' ' && itr != '\0'){   //separating the command
      command[command_i] = itr;
      buf_i++;
      command_i++;
      itr =buf[buf_i];
  }
  command[command_i] = '\0';
  itr = buf[++buf_i];
  while(itr != '\0'){     // separrating the commands
    buf_change=1;
    char *arg;
    arg = (char*)malloc(sizeof(char)*32);
    arg_i =0;
    while(itr != ' ' && itr != '\0'){
      arg_flag = 1;
      arg[arg_i] = itr;
      arg_i++;
      buf_i++;
      itr = buf[buf_i];
    }
    if(arg_flag == 1){
      arg[arg_i]= '\0';
      arg_flag = 0;
      args[args_i] = arg;
      args_i++;
    }

    if(itr == '\0'){
      break;
    }
    else{
      buf_i++;
      itr =buf[buf_i];
    }
  }
  args[args_i] = '\0';
  if(buf_change) strcpy(buf , command);
}

// method to get the default prompt 
char* get_prompt(char * prompt){
  prompt = (char*)malloc(100*sizeof(char));
  getcwd(prompt, 100);
  return prompt;
}


// checking whether the command is a input redirection command
int is_input_redirection(char * buf){
  char itr = buf[0];
  int buf_i = 0;
  while(itr != '\0'){
    if(itr == '<'){
      return 1;
    }
    itr = buf[++buf_i];
  }
  return 0;
}

// checking whether the command is a output redirection command
int is_output_redirection(char * buf){
  char itr = buf[0];
  int buf_i = 0;
  int count = 0;
  while(itr != '\0'){
    if(itr == '>'){
      count++;
    }
    itr = buf[++buf_i];
  }
  return count;
}

// different options to run the command, this functions returns a value relating to specific commands
int run_options(char * buf){
  if(strncmp(buf, "PS1", 3) == 0){    // changing the prompt
    return 1;
  }
  else if(strcmp(buf ,"exit") == 0){    // exit command
    return 2;
  }
  else if(strncmp(buf, "cd", 2) == 0){     // cd command
    return 3;
  }
  else if(strncmp(buf, "PATH", 4) == 0){     // chaning the PATH
    return 4;
  }
  else if(strncmp(buf , "echo $PATH", 9) == 0){    // for echo $PATH
    return 6;
  }
  else if(is_input_redirection(buf)){     // for input redirection
    return 7;
  }
  else if(is_output_redirection(buf) == 1){   // for output redirection , write mode
    return 8;
  }
  else if (is_output_redirection(buf) == 2) {   // for output redirection, append mode
    return 9;
  }
  else return 5;
}

// method to set the prompt
char * set_prompt(char * buf , char * prompt){
  int buf_i = 5;
  char* temp_prompt;
  temp_prompt = (char*)malloc(100*sizeof(char));
  int temp_prompt_i = 0;
  char itr = buf[buf_i];
  int quote_open = 0;
  while(itr != '"' && itr != '\0'){
      temp_prompt[temp_prompt_i] = itr;
      temp_prompt_i++;
      buf_i++;
      itr = buf[buf_i];
  }
  temp_prompt[temp_prompt_i] = '\0';
  if(strncmp(temp_prompt , "\\w$", 3) == 0){
    char * prompt;
    prompt = get_prompt(prompt);
    return prompt;
  }
  return temp_prompt;
}


// method to change the direction
void chage_dir(char * buf){
  int buf_i = 3;
  char itr = buf[buf_i];
  char new_dir[128];
  int new_dir_i = 0;
  while(itr != '\0'){
    new_dir[new_dir_i] = itr;
    new_dir_i++;
    buf_i++;
    itr = buf[buf_i];
  }
  new_dir[new_dir_i] = '\0';
  int ret = chdir(new_dir);
  if(ret == -1){
    perror("Error");
  }
  
}

// method to get the default path
void get_path(){
  PATH = getenv("PATH");
}

// method to set the path
char* set_path(char * buf){
  int buf_i = 5;
  char itr = buf[buf_i];
  int path_i = 0;
  while(itr != '\0'){
    PATH[path_i] = itr;
    buf_i++;
    path_i++;
    itr = buf[buf_i];
  }
  PATH[path_i++] = '\0';
  PATH[path_i++] = '\0';

  return PATH;
}


// separating the command , used in input/output redirection
int separate_input(char * buf, char * new_stream){
  if(buf[0] == '<' || buf[0] == '>'){
    return -1;
  }
  char itr = buf[0];
  int buf_i = 0;
  char temp_buffer[128];
  int temp_i = 0;
  while(itr != ' ' && itr != '<' && itr != '>'){
    temp_buffer[temp_i] = itr;
    temp_i++;
    itr= buf[++buf_i];
  }
  temp_buffer[temp_i] = '\0';
  printf("temp_buffer : %s\n", temp_buffer);
  if(itr != ' '){
    return -2;
  }
  else{
    while(itr != '<' && itr != '>'){
      itr = buf[++buf_i];
    }
    while(itr == '<' || itr == '>'){
      itr = buf[++buf_i];
    }

    while(itr == ' '){
      itr = buf[++buf_i];
    }
  }
 
  temp_i= 0;
  while(itr != '\0'){

    new_stream[temp_i] = itr;
    temp_i++;
    itr = buf[++buf_i];
  }
  new_stream[temp_i] = '\0';
  printf("new_stream = %s\n", new_stream);

  strcpy(buf, temp_buffer);
  return 0;
}



int main() {
  get_path();
	int pid;
  char * prompt;
  prompt = get_prompt(prompt);
  int menu;
  int successful_read;
	while(1) {
		printf(ANSI_GREEN "%s>" ANSI_RESET, prompt);
    char * buf;
    buf = (char*)malloc(128*sizeof(char));

    successful_read = scanf("%[^\n]%*c" , buf);

    if(successful_read == 0){
      getchar();
      free(buf);
      continue;
    }
    if(successful_read == EOF ){
      break;
    }
    menu = run_options(buf);
    if(menu == 1){ // PS1
      prompt = set_prompt(buf, prompt); 
    }
    else if(menu == 2){ // exit
      break;
    } 
    else if(menu == 3){ // cd
      chage_dir(buf);
      prompt = get_prompt(prompt);
    }
    else if(menu == 4){  // setting the PATH 
      PATH = set_path(buf);
    }
    else if(menu == 6){  // echo $PATH
      printf("%s\n", PATH);
    }
    else if(menu == 5){    // runnig all basic commands
      char** args;
      args = (char**)malloc(10*sizeof(char*));
      format_input(buf , args);
      find_path(buf);
      args[0] = buf;
		  pid = fork();
		  if(pid == 0) {
			  execv(buf,args);
        perror("Error");
        return 0;
		  } else {
			  wait(0);
        free(args);
		  }
	  }
    else if (menu == 7) {  // input redirection

      
      char * new_ip;
      new_ip = (char*)malloc(128*sizeof(char));
      int check = separate_input(buf , new_ip);
      if(check == -1){
        printf("no command given...\n");
        free(buf);
        free(new_ip);
        continue;
      }
      else if(check == -2){
        printf("wrong command\n");
        free(buf);
        free(new_ip);
        continue;
      }
      find_path(buf);
      printf("final buf = %s , new_ip = %s\n", buf , new_ip);
      pid = fork();
      if(pid == 0){
        close(0);
        int fd = open(new_ip, O_RDONLY);
        execl(buf,buf, NULL);
        perror("Error");
      }
      else wait(0);
      
    }
    else if (menu == 8 || menu == 9) {  // output redirection

      
      char * new_op;
      new_op = (char*)malloc(128*sizeof(char));
      int check = separate_input(buf , new_op);
      if(check == -1){
        printf("no command given...\n");
        free(buf);
        free(new_op);
        continue;
      }
      else if(check == -2){
        printf("wrong command\n");
        free(buf);
        free(new_op);
        continue;
      }
      find_path(buf);
      printf("final buf = %s , new_op = %s\n", buf , new_op);
      pid = fork();
      if(pid == 0){
        if(menu == 8){   // write mode
          printf("inside 8\n");
          close(1);
          int fd = open(new_op, O_WRONLY | O_CREAT, S_IRWXU);
          execl(buf,buf, NULL);
          perror("Error");
        }
        else{   // append mode
          printf("inside 9\n");
          close(1);
          int fd = open(new_op, O_WRONLY | O_APPEND);
          execl(buf,buf, NULL);
          perror("Error");
        }
      }
      else wait(0);
      
    }    
    free(buf);
  }
	printf("Bye\n");
}
