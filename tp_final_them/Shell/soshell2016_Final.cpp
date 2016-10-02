#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <cstdio>
#include <sys/wait.h>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <string>
using namespace std;

#define die(e) do { fprintf(stderr, "%s\n", e); exit(EXIT_FAILURE); } while (0);
#define TRUE 1

char** Construir_argv2(vector<string> argv){
	int cuenta;
	if(argv.size()<=1){
		die("Error argumento argv");
	}else{
		if(argv.size()==2 && argv[1] == "0")
			cuenta = 1;
		else 
			cuenta = argv.size();
	}
	//Construyo el argumento argv2, argumento necesario para execvp
	char **argv2;
	argv2 = new char * [cuenta + 1];//contiene los argumentos de la funcion a la que llamamos. El +1 es por el NULL
	argv2[cuenta] = NULL;			//execvp exige que argv2 tenga como terminador una referencia a NULL
	char arg_aux[50][256];			//siempre hay que crear algo en MEMORIA FISICA para poder referenciarla y que sea "const" para el compilador
	for (int i=0;i<cuenta;i++){
		strcpy(arg_aux[i],argv[i].c_str());		//strcpy agrega el \0 al final de la cadena. Suponemos que ningun argumento supera los 256 caracteres ni que habra mas de 50 comandos...
		argv2[i] = arg_aux[i];					//referenciamos punteros a char* que terminan en \0
	}
	return argv2;
}

int spawn_proc (int in, int out, char **argv2)
{
	pid_t pid;
	
	if ((pid = fork ()) == 0)
	{
		if (in != 0)		//Si no es el STDIN_FILE0, entonces duplicamos el descriptor de fichero
		{
			dup2 (in, 0);
			close (in);
		}
		
		if (out != 1)		//Si no es el STDOUT_FILE0, entonces duplicamos el descriptor de fichero
		{
			dup2 (out, 1);
			close (out);
		}
		
		return execvp (argv2[0],argv2);		//En el primer char* esta el comando a ejecutar y el resto son los argumentos (terminados en NULL)
	}
	
	return pid;
}

int fork_pipes (vector< vector<string> > commands)
{
	int n = commands.size();
	int i;
	pid_t pid;
	int in, fd [2];
	
	// El primer proceso obtiene su entrada desde el file descriptor original (0): STDIN_FILE0
	in = 0;
	
	//Se ejecuta hasta el ante ultimo comando
	for (i = 0; i < n - 1; ++i)
	{
		pipe (fd);
		
		char **argv2 = Construir_argv2(commands[i]);
		
		// f[1] es el lugar de escritura en el pipe, el in lo traemos desde la iteracion anterior (sea el STDIN_FILE0 o sea el pipe de lectura fd[0])
		spawn_proc (in, fd [1], argv2);
		
		// Cerramos la escritura ya que el proceso hijo en spawn_proc() escribira aqui
		close (fd [1]);
		
		//Nos guardamos el pipe de lectura, ya que el siguiente proceso hijo leera desde aqui
		in = fd [0];
	}
	
	//Se lee desde el ultimo pipe para la entrada del ultimo comando. No es necesario duplicar el out ya que sera la salida por consola del ultimo execvp
	if (in != 0)
		dup2 (in, 0);
	
	//Ejecuto el ultimo comando en un proceso hijo porque de lo contrario finaliza la ejecucion general del shell
	if(fork() == 0){
		char **argv2 = Construir_argv2(commands[i]);
		execvp (argv2[0],argv2);
	}
	else{
		wait(NULL);
	}
}

int main(int argc, char *argv[]) {
	
	pid_t p0,_pid;		//p0 es el pid_t del proceso Padre General (el que invoca al shell)
	int paso=0;			//Esta variable sirve para obtener p0 y controlar las demas ejecuciones y copias del programa(forks) DURANTE el while
	
	while (TRUE) { // se repite en forma indefinida
		
		if(paso == 0 ){
			_pid = fork();
			if(_pid !=0)
				p0 = getppid();
			else
				return 0;
		}
		else{
			if(p0 != getppid()){
				return 0;	//Si el proceso actual no es hijo del Padre General entonces finalizo porque es una duplicacion anidada del fork() que no me interesa
			}
		}
		wait(NULL);			//Espero a que finalicen todos los hijos anidados del fork() para luego continuar con la ejecucion normal del shell
		
		char cwd[256];
		getcwd(cwd, sizeof(cwd));						//Obtener directorio actual
		char usr[256];
		getlogin_r(usr, sizeof(usr));					//Obtener usuario actual
		printf ("@%s:%s%s ", usr, cwd, " ~%");    		//Muestra el prompt
		string read_linea;
		getline(cin,read_linea);                        //Lee el comando con sus parametros
		//Detecto cuantos comandos encadenados hay (se ejecutan en serie)
		vector<int> pos_pipe;
		int pos_nuevo_pipe = read_linea.find_first_of('|',0);
		pos_pipe.push_back(0);
		int i = 1;
		while(pos_nuevo_pipe != -1){
			pos_pipe.push_back(pos_nuevo_pipe);
			pos_nuevo_pipe = read_linea.find_first_of('|',pos_nuevo_pipe+1);
			i++;
		}
		pos_pipe.push_back(-1);		//bandera final que indica el fin en el vector pos_pipe
		//si no hubo pipes (|), entonces pos_pipe contendra 2 elementos: 0 y -1
		int n = pos_pipe.size()-1;
		
		vector< vector<string> > commands;
		
		for (int j=0;j<n;j++){
			string read2;
			//Obtengo el comando+parametros(opcionales) de cada comando encerrado por el/los pipe/s (o no, si no hubiera pipes)
			if(j == 0){ 	//1? comando
				read2 = read_linea.substr(pos_pipe[j],pos_pipe[j+1]-pos_pipe[j]-1);
			}
			else{			//2?,3?,... comandos
				if(pos_pipe[j+1]==-1){		//ultimo comando
					read2 = read_linea.substr(pos_pipe[j]+2);//hasta el final
				}
				else{
					read2 = read_linea.substr(pos_pipe[j]+2,pos_pipe[j+1]-pos_pipe[j]-3); //los +2, -3 es porque asumo que cada comando junto con sus argumentos
					//estan separados por el pipe | y espacios entremedio, de esta manera: comando1 [argN] | comando2 [argN] | comando3 [argN]
				}
			}
			
			//Una vez obtenido el comando+parametros:
			vector<string> argv;
			
			//Obtengo el comando
			int pos = read2.find_first_of(' ',0);				//Asumo que los argumentos estan separados por 1 solo espacio
			argv.push_back(read2.substr(0,pos).c_str());		//Leo el comando
			
			//Obtengo los parametros
			int cuenta = 0;
			if (pos == -1){//parametros = 0
				argv.push_back("0");
			}
			else{//parametros != 0
				while(pos != -1){
					cuenta++;			//Siempre va a valer al menos 1
					pos = read2.find_first_of(' ',pos+1);
				}
				pos = read2.find_first_of(' ',0);//primer espacio
				int pos_nuevo = pos;
				int i = 0;
				while(i<cuenta){
					pos_nuevo = read2.find_first_of(' ',pos+1);
					if(pos_nuevo == -1)
						argv.push_back(read2.substr(pos+1));//hasta el final
					else
						argv.push_back(read2.substr(pos+1,pos_nuevo-pos-1));
					pos = pos_nuevo;
					i++;
				}
			}
			
			if(argv[0] == "chdir" || argv[0] == "cd"){
				//no existe un comando "change directory". En realidad es un cambio de entorno de ejecucion del shell
				if (!(argv[1] == "0")) chdir(argv[1].c_str());
				else chdir("/");
			}
			else if(argv[0] == "exit"){
				die("MiniShell finalizada");
			}
			//CAMINO------------------------------------------------------------------------------------------------------------------------------
			else if (argv[0] == "camino") { //Si el comando es "camino"
				if (argv[1] == "0") cout<<"Comando Erroneo"<<endl; //si no le paso argumento da error
				else if (argv.size() > 2) { //si tiene argv tiene más de 2 elementos, significa que el comando fue 'camino +/- directorio'
					
					if (argv[1] == "+") { //si el argumento es +, se agregara al path el directorio introducido
						bool ig = false; //comparador
						int p = 0;
						int c;
						string subdir;
						char* pPath = getenv ("PATH"); //pido la variable PATH
						string path_aux(pPath);
						string path_aux_2 = ":"+path_aux+":"; //esta variable se crea para simplificar el manejo de la cadena
						char curdir[256];
						strcpy(curdir,argv[2].c_str()); //obtengo el directorio introducido
						string curdir_aux(curdir);
						
						//Lo que se hace es asumir que todos los directorio de la cadena del PATH estan separados por ':'
						//Por lo cual vamos obteniendo subtrings que contienen a cada directorio, leido entre los 2 ':'
						
						c = path_aux_2.find_first_of(':',1); //busco el primer ':';
						if (c != -1) subdir = path_aux_2.substr(1,c-1); //si lo encuentra, obtengo el directorio
						
						ig = curdir_aux == subdir; //comparo el directorio introducido con el extraido del path
						
						while(!ig) { //mientras sean distintos seguire comparando con los demás directorios
							p = c+1; 
							c = path_aux_2.find_first_of(':',p); //busco el directorio siguiente
							if (c != -1) {
								subdir = path_aux_2.substr(p,c-p); //obtengo el directorio
								ig = curdir_aux == subdir; //comparo
								if (ig) break; //si elgun momento encuentra un directorio igual, rompe el ciclo.
							}
							else { //si entra aca quiere decir que recorrio todos los directorios y no encontro ninguno que sea igual
								//entonces lo agregamos al path
//								cout<<path_aux<<endl;
								path_aux= path_aux+":"+curdir_aux;
//								cout<<path_aux<<endl;
								setenv("PATH", path_aux.c_str(), true); //seteo la variable path  
								cout<<"El directorio fue agregado al PATH"<<endl;
								break; //corto el ciclo, si no lo agregaria indefinidamente.
							}
						}
						
						if (ig) cout<<"El directorio ya existía en PATH"<<endl;	//si encuentra dos directorios iguales no hace nada.		
					}
					
					if (argv[1] == "-") { //si el argumento es -, va a eliminar el directorio introducido del PATH
						
						bool ig = false;
						int p = 0;
						int c;
						string subdir;
						char* pPath = getenv ("PATH");
						string path_aux(pPath);
						string path_aux_2 = ":"+path_aux+":";
						char curdir[256];
						strcpy(curdir,argv[2].c_str());
						string curdir_aux(curdir);
						
						c = path_aux_2.find_first_of(':',1);
						if (c != -1) subdir = path_aux_2.substr(1,c-1);
						
						ig = curdir_aux == subdir;
						
						while(!ig){ //el codigo es similar pero la logica es inversa. 
							//ahora si el directorio es distinto a todos es porque no existía, entonces no hace nada.
							//si encuentra uno que sea igual, entonces lo elimina del path
							p = c+1;
							c = path_aux_2.find_first_of(':',p);
							if (c != -1) {
								subdir = path_aux_2.substr(p,c-p);
								ig = curdir_aux == subdir;
								if (ig) break;
							}
							else{
								cout<<"El directorio no existe en el PATH"<<endl;
								break;
							}
						}
						
						if (ig) {
//							cout<<path_aux<<endl;
							path_aux.erase(p, c-p+1);
//							cout<<path_aux<<endl;
							setenv("PATH", path_aux.c_str(), true);   
							cout<<"El directorio fue eliminado del PATH"<<endl;
						}
					}
				}
				
				//si entra aca significa que el comando ingresado fue 'camino +/-'
				//el código es el mismo que en los casos anteriores, solo que el directorio a agregar/eliminar ahora es el actual
				else {
					if (argv[1] == "+") {
						
						bool ig = false;
						int p3 = 0;
						int c3;
						string subdir;
						char* pPath = getenv ("PATH");
						string path_aux(pPath);
						string path_aux_2 = ":"+path_aux+":";
						char curdir[256];
						getcwd(curdir, sizeof(curdir));
						string curdir_aux(curdir);
						
						c3 = path_aux_2.find_first_of(':',1);
						
						if (c3 != -1) subdir = path_aux_2.substr(1,c3-1);
						
						ig = curdir_aux == subdir;
						
						while(!ig) {
							p3 = c3+1;
							c3 = path_aux_2.find_first_of(':',p3);
							if (c3 != -1) {
								subdir = path_aux_2.substr(p3,c3-p3);
								ig = curdir_aux == subdir;
								if (ig) break;
							}
							else {
//								cout<<path_aux<<endl;
								path_aux= path_aux+":"+curdir_aux;
//								cout<<path_aux<<endl;
								setenv("PATH", path_aux.c_str(), true);   
								cout<<"Directorio agregado al PATH"<<endl;
								break;
							}
						}
						
						if (ig) cout<<"El directorio ya existía en PATH"<<endl;
					}
					
					if (argv[1] == "-") {
						bool ig = false;
						int p4 = 0;
						int c4;
						string subdir;
						char* pPath = getenv ("PATH");
						string path_aux(pPath);
						string path_aux_2 = ":"+path_aux+":";
						char curdir[256];
						getcwd(curdir, sizeof(curdir));
						string curdir_aux(curdir);
						
						c4 = path_aux_2.find_first_of(':',1);
						if (c4 != -1) subdir = path_aux_2.substr(1,c4-1);
						
						ig = curdir_aux == subdir;
						
						while(!ig) {
							p4 = c4+1;
							c4 = path_aux_2.find_first_of(':',p4);
							if (c4 != -1) {
								subdir = path_aux_2.substr(p4,c4-p4);
								ig = curdir_aux == subdir;
								if (ig) break;
							}
							else{
								cout<<"El directorio no existe en el PATH"<<endl;
								break;
							}
						}
						
						if (ig) {
//							cout<<path_aux<<endl;
							path_aux.erase(p4-2, c4-p4+1);
//							cout<<path_aux<<endl;
							setenv("PATH", path_aux.c_str(), true);   
							cout<<"El directorio fue eliminado del PATH"<<endl;
						}
					}
				}
			}
			//CAMINO------------------------------------------------------------------------------------------------------
			
			//si no es ningun comando especial, entonces lo agrega al vector de comandos (por si hay comandos concatenados)
			else{
				commands.push_back(argv);
			}
		}
		
		int N = commands.size();
		
		if(N>1){				//Entra aca si hay uno o varios pipes
			if(fork() == 0){
				fork_pipes(commands);
			}
			else{
				wait(NULL);
			}
		}
		else if(N == 1){		//Entra aca si hay un solo comando (es decir, sin pipes)
			string command = commands[0][0];
			if(command == "chdir" || command == "cd"){		
				if (commands[0][1] != "0")
					chdir(commands[0][1].c_str());
				else chdir("/");
			}
			else{
				char **argv2 = Construir_argv2(commands[0]);
				if (fork() !=0) { 
					// Codigo del padre.
					int status;
					waitpid(-1, &status, 0); // espera a que el hijo termine
				} 
				else {
					// Codigo del hijo.
					execvp(argv2[0], argv2);
					break; //Corto el while(true) asi termina el hijo
				}
			}
		}
		paso++;
	}
	return 0;
}

