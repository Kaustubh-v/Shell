#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#define ANSI_GREEN   "\x1b[32m"
#define ANSI_RESET   "\x1b[0m"

char *PATH;


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
  //printf("buf = %s\n" , buf);
  while(itr != '\0' && itr != ' '){
    temp_path[temp_i] = itr;
    //printf("itr = %c\n" , temp_path[temp_i]);
    buf_i++;
    itr = buf[buf_i];
    temp_i++;
  }
  temp_path[temp_i] = '\0';
  //printf("tmp = %s\n" , temp_path);
}

int file_exists(char * temp_path , char * buf){
  //printf("in file_exists\n");
  concatenate_path(temp_path , buf);
  //printf("done concatenate_path with temp_path as = %s\n" , temp_path);
  if(access(temp_path , F_OK) == 0){
    //printf("the executable exists...");
      if(access(temp_path , X_OK) == 0){
      //printf("executable permission granted..");
      strcpy(buf , temp_path);
      return 1;
    }
    else {
      printf("There are no executable permissions to the user for the command : %s\n" , buf);
      return 0;
    }
  }
  else{
    //printf("file not found...");
    return 0;
  }
  return 0;
}


void find_path(char * buf){
  char iterator = PATH[0];
  char path_itr;
//printf("temp_path = %s\n" , temp_path);
  int temp_i = 0;
  int path_i = 0;

  while (iterator != '\0'){
    path_itr = iterator;
    char  temp_path[32];
    temp_i = 0;
    while (path_itr != ':' && path_itr != '\0') { 
      temp_path[temp_i] = path_itr;
      temp_i++;
      path_i++;
      path_itr = PATH[path_i];
    }
    temp_path[temp_i]= '\0';
    //printf("passing temp_path as : %s\n" , temp_path);
    if(file_exists(temp_path , buf)){
      break;
    }
    //printf("file did not exist\n");
    iterator = PATH[++path_i];
  }
  //printf("done running\n");   
}

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
  //printf("buf_i = %c\n" , itr);
  while(itr != '\0'){
    buf_change=1;
    //printf("inside this loop\n");
    char *arg;
    arg = (char*)malloc(sizeof(char)*32);
    arg_i =0;
    while(itr != ' ' && itr != '\0'){
      //printf("inside other loop\n");
      arg_flag = 1;
      arg[arg_i] = itr;
      arg_i++;
      buf_i++;
      itr = buf[buf_i];
    }
    //printf("arg here = %d\n" , arg_i);
    if(arg_flag == 1){
      //printf("arg = %s\n" , arg);
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
  //printf("command = %s and arg = %c\n" , command , args[1][8]);
  if(buf_change) strcpy(buf , command);
}


char* get_prompt(char * prompt){
  prompt = (char*)malloc(100*sizeof(char));
  getcwd(prompt, 100);
  //printf("prompt set to %s\n" , prompt);
  return prompt;
}


int run_options(char * buf){
  if(strncmp(buf, "PS1", 3) == 0){
    return 1;
  }
  else if(strcmp(buf ,"exit") == 0){
    return 2;
  }
  else if(strncmp(buf, "cd", 2) == 0){
    return 3;
  }
  else if(strncmp(buf, "PATH", 4) == 0){
    return 4;
  }
  else if(strncmp(buf , "echo $PATH", 9) == 0){
    return 6;
  }
  else return 5;
}


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
  chdir(new_dir);
  
}

void get_path(){
  PATH = getenv("PATH");
}

void set_path(char * buf){
  int buf_i = 5;
  char itr = buf[buf_i];
  int path_i = 0;
  while(itr != '\0'){
    PATH[path_i] = itr;
    buf_i++;
    path_i++;
    itr = buf[buf_i];
  }
  PATH[path_i] = '\0';
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
    //printf("here");
			//fgets(buf , 128 , stdin);
    successful_read = scanf("%[^\n]%*c" , buf);
    //printf("given input is : %d\n" , successful_read);
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
    else if(menu == 4){
      set_path(buf);
    }
    else if(menu == 6){
      printf("%s\n", PATH);
    }
    else if(menu == 5){
      char** args;
      args = (char**)malloc(10*sizeof(char*));
      format_input(buf , args);
      //printf("%s\n" , args[1]);
      find_path(buf);
      args[0] = buf;
    //printf("final buf = %s\n", buf);
		  pid = fork();
		  if(pid == 0) {
      //printf("inside child\n");
			  execv(buf,args);
        perror("Error");
        return 0;
		  } else {
			  wait(0);
        free(args);
		  }
	  }
    free(buf);
  }
	printf("Bye\n");
}

