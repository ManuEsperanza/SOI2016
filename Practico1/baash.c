#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFSIZE 256

void parser(char *entrada,char *argu[]);
void comandos(char *argumentos[]);
void agregaBarra (char *s1);
char* concatenar(const char *s1, const char *s2);
void path(char *argumentos[]);
int background(char *argumentos[]);
void handler(int signum);


int main (int argc, char **argv){
	char entrada[100];	
	int i=0;
	int fin=0;
	char host[BUFFSIZE+1], *usuario, *argumentos[10], *exit, dirActual[100];

	int n;	
	FILE *fd;
	char buffer[BUFFSIZE+1];
	fd = fopen("/etc/hostname","r");
	fgets(buffer, BUFFSIZE+1, fd);
	sscanf(buffer, "%s",host);	
	fclose(fd);			//almaceno en la variable host, el nombre del host que ejecuto este prompt
	usuario = getlogin();		//con la funcion getlogin() obtengo el nombre del usuario actual

	signal(SIGCHLD, handler); // señal para las tareas hijas en background que terminan

	do {
		getcwd(dirActual,100);			//obtengo el directorio en donde estoy en dicho momento
		memset(entrada,0,100);			//vacio el vector donde se almacena la entrada por teclado
		memset(argumentos,0,100);		//vacio el vector donde se almacena el comando y sus argumentos
		printf("%s@%s : %s ",usuario,host,dirActual);		
		exit = fgets (entrada, 100, stdin); 		//entrada por teclado
		
		parser(entrada,argumentos);	
		comandos(argumentos);
	}
	while (exit != NULL);		
	
	return 0;
}

void parser(char *entrada,char *argu[]){  //funcion donde a la entrada por teclado la parseo en un array de argumentos
	
	char *ptr;
	char delimitador[4] = " \n\t";
	ptr = malloc(sizeof (char) * BUFFSIZE);
	ptr = strtok(entrada, delimitador);    // Primera llamada => Primer token
   	argu[0] = ptr;						//la posicion 0 apunta al primer argumento pasado
   	int i = 1;
   	while( (ptr = strtok( NULL, delimitador )) != NULL ){  // Posteriores llamadas
    	argu[i] = ptr;
    	i++;
    }
}

void comandos(char *argumentos[]){ //funcion que interpreta los comandos ingresados 
	int flag_bckgrnd = 0;
	int tamanio=0;
	while (argumentos[tamanio]!='\0'){
	tamanio++;
	}
	if (tamanio != 0){ 
		if (strcmp(argumentos[0],"exit") == 0){	//cierra el prompt al ingresar el comando exit
				exit(0);
		}			
		if (strcmp(argumentos[0],"cd") == 0){	// comando cd 
			if (strcmp(argumentos[1],"..") == 0){
			chdir(".."); 		//cambio de directorio ".."
			}
			else {
					chdir(argumentos[1]);		
			}	
		}
		else {
		
			path(argumentos);	//esta funcion busca el comando en la variable PATH		
		}
	}
	
}

void path(char *argumentos[]){		//
	
	char *directorioPtr,*pathPtr,*argumento,*com_argu,*path;
  	char directorio[] = "";
  	int pid, status;

  	pathPtr = getenv("PATH");
  	path = strdup(pathPtr);		//copio el pathPtr a path porque la func strtok rompe el path
  	directorioPtr = strtok(path, ":\n" );
	strcpy(directorio,directorioPtr);
	agregaBarra(directorio);
	com_argu = concatenar(directorio,argumentos[0]);
    while( (directorioPtr = strtok(NULL, ":\n")) != NULL ){
		strcpy(directorio,directorioPtr);
		agregaBarra(directorio);
		com_argu = concatenar(directorio,argumentos[0]);
		pid = fork();	  
      	if ( pid < 0 ){   
      		exit(1);
      	}
      	if(pid == 0)   { // Ejecucion del Hijo
      		execv(com_argu, argumentos); 
       		exit(1);
      	} 
      	else { // Ejecucion del Padre
      		if (background(argumentos) == 1){ //modo background|
      			//no hacer nada, la señal sigchld se encarga de avisar al proc padre que el hijo terminó
      		}
      		else{
      			wait(&status);
      		}
      	  
  		}
   	}
}

void agregaBarra (char *s1){ //agrega una barra "/" al final del directorio PATH
	
	size_t longitud = strlen(s1);
	s1[longitud]='/';
	s1[longitud+1] = '\0';
}

char* concatenar(const char *s1, const char *s2){ // une el directorio del PATH con el comando solicitado para pasarselo a la funcion exec()

	char *resultado = malloc(strlen(s1)+strlen(s2)+1);//+1 para el '\0'
   	strcpy(resultado, s1);
    strcat(resultado, s2);
    return resultado;
}

int background(char *argumentos[]){
	
	int tam=0;
	while (argumentos[tam]!='\0'){		//busco el ultimo argumento pasado
	tam++;
	}
	
	if (strcmp(argumentos[tam-1],"&") == 0){ //si el ultimo argumento es un &. return 1 sino 0
		argumentos[tam-1] = NULL;			//borro el &
		return 1;
	}
	else {
		return 0;
	}
}

// Función handler que maneja la señal que envía el proceso hijo que está ejecutándose en background
void handler(int signum) {
    int status;
    pid_t pid;

    pid = waitpid(-1, &status, WNOHANG);
}


