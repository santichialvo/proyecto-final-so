#include "System.h"


System::System() {
	
}

constexpr unsigned int str2int(const char* str, int h = 0)
{
	return !str[h] ? 5381:(str2int(str, h+1)*33)^str[h];
}

void System::splitBySpaces(string &command, list<string> &commands)
{
	int pos=0,initialPos=0;
	
	while (command[pos]!='\0')
	{
		while (command[initialPos]==' ')
		{
			initialPos++;
			pos++;
		}
		while (command[pos]!=' ' && command[pos]!='\0')
			pos++;
		if (command[pos]=='\0' && command[initialPos]=='\0') break;
		string s(command,initialPos,pos-initialPos);
		commands.push_back(s);
		initialPos = pos;
	}
}

void System::showHelp() 
{
	rlutil::setColor(12);
	cout << "################################################################################" << endl;
	cout << "#                        Ayuda del sistema                                     #" << endl;
	cout << "#-cd                    : Mover al directorio raiz.                            #" << endl;
	cout << "#-cd ..                 : Subir un nivel en el arbol de directorios.           #" << endl;
	cout << "#-cd 'v'                : Mover al directorio 'v'.                             #" << endl;
	cout << "#-mkdir 'v'             : Crea el directorio 'v' en la posicion actual.        #" << endl;
	cout << "#-touch 'v1' 'v2'       : Crea el archivo 'v1' con el contenido 'v2'.          #" << endl;
	cout << "#-mv 'v1' 'v2'          : Mueve el archivo o directorio de 'v1' a 'v2'.        #" << endl;
	cout << "#-rm 'v'                : Elimina el archivo o directorio (recursivo).         #" << endl;
	cout << "#-ls                    : Lista los archivos y carpetas en el directorio acual.#" << endl;
	cout << "#-tree                  : Muestra el arbol del sistema completa.               #" << endl;
	cout << "#-exit                  : Termina el programa.                                 #" << endl;
	cout << "################################################################################" << endl;
	rlutil::setColor(15);
}

void System::runSystem()
{
	rlutil::setColor(15);  //color blanco brillante para consola
	FileSystem fs;
	fs.changeDir("root");
	char commandchar[500];
	list<string> commands;
	string firstcommand;
	while (true) 
	{
		fs.WritePrompt();
		cin.getline(commandchar,500);
		commands.clear();
		
		string command(commandchar);
		splitBySpaces(command,commands);
		list<string>::iterator itcommands = commands.begin();
//		list<string>::iterator it = commands.begin();
//		for (it=commands.begin();it!=commands.end();it++)
//			cout<<*it<<endl;
		firstcommand = itcommands!=commands.end()?*itcommands:"";
		
		switch (str2int(firstcommand.c_str()))
		{
		case str2int(""):
			{
				break;
			}
		case str2int("exit"):
			{
				exit(0);
				break;
			}
		case str2int("help"):
			{
				showHelp();
				break;
			}
		case str2int("mkdir"):
			{
				itcommands++;
				if (itcommands==commands.end()) {
					cout<<"mkdir: falta un operando"<<endl;
					break;
				}
				fs.makeDir(*itcommands);
				break;
			}
		case str2int("cd"):
			{
				itcommands++;
				if (itcommands==commands.end())
				{
					fs.changeDir("root");
					break;
				}
				else if(!(*itcommands).compare(".."))
				{
					fs.increaseLevel();
					break;
				}
				fs.changeDir(*itcommands);
				break;
			}
		case str2int("ls"):
			{
				itcommands++;
				if (itcommands==commands.end())
					fs.listChildren();
				else
					cout<<"ls: operacion invalida"<<endl;
				break;
			}
		case str2int("tree"):
			{
				itcommands++;
				if (itcommands==commands.end())
					fs.showTree();
				else
					cout<<"tree: operacion invalida"<<endl;
				break;
			}
		case str2int("mv"):
			{
				itcommands++;
				if (itcommands==commands.end())
				{
					cout<<"mv: operacion invalida"<<endl;
					break;
				}
				string from = *itcommands;
				itcommands++;
				if (itcommands==commands.end())
				{
					cout<<"mv: falta carpeta de destino"<<endl;
					break;
				}
				string to = *itcommands;
				fs.move(from,to);
				break;
			}
		case str2int("touch"):
			{
				itcommands++;
				if (itcommands==commands.end())
				{
					cout<<"touch: operacion invalida"<<endl;
					break;
				}
				string name = *itcommands;
				itcommands++;
				if (itcommands==commands.end())
				{
					cout<<"touch: falta el contenido del archivo"<<endl;
					break;
				}
				string content = *itcommands;
				fs.makeFile(name,(unsigned char*)(content.c_str()),(int)(content.length()));
				break;
			}
		case str2int("rm"):
			{
				itcommands++;
				if (itcommands==commands.end())
				{
					cout<<"rm: operacion invalida"<<endl;
					break;
				}
				fs.erase(*itcommands);
				break;
			}
		default:
			{
				cout<<"Comando invalido, para ayuda ingrese \"help\""<<endl;
				break;
			}
		}
	}
}

System::~System() {
	
}

