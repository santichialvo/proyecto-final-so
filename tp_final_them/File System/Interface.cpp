#include "Interface.h"

void EscribirAyuda();

Interface::Interface()
{
	
}

void Interface::Run()
{
	FSystem fs;
	
	bool end = false;
	char comandof[500];
	list<string> comandos;
	string cm;
	
	do{
		fs.EscribirPrompt();
		
		cin.getline(comandof, 500);
		comandos.clear();
		
		string comando(comandof);
		comando.append(1,' ');	
		int pos = 0;
		
		int count = 0;
		do
		{
			if(comando[count] == ' ')
			{
				string s(comando, pos, count - pos);
				comandos.push_back(s);
				
				while(comando[count] == ' ')
					count++;
				
				pos = count;
				count--;
			}
			
			count++;
		}while(comando[count] != '\0');
		
		cm = *(comandos.begin());
		comandos.pop_front();
		
		if(cm.compare("exit") == 0)
		{
			end = true;
		}
		else if(cm.compare("help") == 0)
		{
			EscribirAyuda();
		}
		else if(cm.compare("cd") == 0)
		{
			if(comandos.empty())
				continue;
			else if(comandos.begin()->compare("..") == 0)
			{
				fs.SubirNivel();
				continue;
			}
			
			fs.CambiarDirectorioActual(*(comandos.begin()));
		}
		else if(cm.compare("cd..") == 0)
		{
			fs.SubirNivel();
		}
		else if(cm.compare("mkdir") == 0)
		{
			fs.CrearDirectorio(*(comandos.begin()));
		}
		else if(cm.compare("mv") == 0)
		{
			string par1 = *(comandos.begin());
			comandos.pop_front();
			string par2 = *(comandos.begin());
			
			fs.Mover(par1, par2);
		}
		else if(cm.compare("rm") == 0)
		{
			fs.Eliminar(*(comandos.begin()));
		}
		else if(cm.compare("ls") == 0)
		{
			fs.ListarHijos();
		}
		else if(cm.compare("st") == 0)
		{
			fs.MostrarArbol();
		}
		else if(cm.compare("mkfil") == 0)
		{
			string par1 = *(comandos.begin());
			comandos.pop_front();
			string par2 = *(comandos.begin());
			
			fs.CrearArchivo(par1, (unsigned char*)(par2.c_str()), (int)(par2.length()));
		}
		else
		{
			cout << "Comando invalido, ingrese 'help'." << endl;
		}
		
		
	} while(!end);
	
}

void EscribirAyuda()
{
	cout << "################################################################################" << endl;
	cout << "#                                     Help                                     #" << endl;
	cout << "#-cd                    : Cambiar directorio.                                  #" << endl;
	cout << "#-cd..                  : Subir un nivel en el arbol de directorios.           #" << endl;
	cout << "#-cd 'value'            : Mover al directorio 'value'.                         #" << endl;
	cout << "#-mkdir 'value'         : Crea el directorio 'value' en la posicion actual.    #" << endl;
	cout << "#-mkfil 'value1' 'value2: Crea el archivo 'value1' con el contenido 'value2'.  #" << endl;
	cout << "#-mv 'value1' 'value2'  : Mueve el archivo o directorio de 'value1' a 'value2'.#" << endl;
	cout << "#-rm 'value'            : Elimina el archivo o directorio(recursivo).          #" << endl;
	cout << "#-ls                    : Lista los archivos y carpetas en el directorio acual.#" << endl;
	cout << "#-st                    : Muestra el arbol del sistema completa.               #" << endl;
	cout << "#-exit                  : Termina el programa.                                 #" << endl;
	cout << "################################################################################" << endl;
}
