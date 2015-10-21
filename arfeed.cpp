#include <iostream>
#include <string.h>
#include <cstring>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define HTTP 1
#define HTTPS 2
#define DEBUG

using namespace std;

using Param = struct parametry {
	string adresaParamStr;
	string fParamStr;
	string cParamStr;
	string CParamStr;
	int adresaParam = 0;
	int fParam = 0;
	int cParam = 0;
	int CParam = 0;
	int lParam = 0;
	int TParam = 0;
	int aParam = 0;
	int uParam = 0;
};

void printHelp();
int parseParameters(int argc, char* argv[], Param* parametr);

int main(int argc, char* argv[])
{
	Param* parametr = new Param;
	if (parseParameters(argc, argv, parametr) == -1)
	{
		cerr << "Spatne zadane parametry" << endl;
		delete parametr;
		return -1;
	}
	
	/*
	SSL_load_error_strings();
	ERR_load_BIO_strings();
	SSL_library_init();
	*/
	int port = 0;
	int posun = 0;
	string str, prefix1, prefix2, lomitko, str2;
	prefix1 = "http://";
	prefix2 = "https://";
	lomitko = "/";
	size_t found = parametr->adresaParamStr.find(prefix1);
	if (found!=string::npos)
	{
		port = 80;
		cout << "http nalezeno na " << found << " port: " << port << endl;
		posun = 7;
	}
	found = parametr->adresaParamStr.find(prefix2);
	if (found!=string::npos)
	{
		port = 443;
		cout << "https nalezeno na " << found << " port: " << port << endl;
		posun = 8;
	}
	
	// odstranění prefixu (http:// nebo https://)
	str.assign(parametr->adresaParamStr,posun,parametr->adresaParamStr.length());
	cout << "********************************************************************************" << endl;
	bool nalezeno = false;
	int index;
	for(unsigned int i = 0; i < str.length(); i++)
	{
		if (str[i] == '/')
		{
			nalezeno = true;
			index = i;
			break;
		}
	}
	cout << "index: " << index << endl;
	cout << "retezec: " << str << endl;
	string pozadavek;
	if (nalezeno == true)
	{
		str2.assign(str,0,index);
		pozadavek.assign(str,index,str.length());
	}
	cout << "********************************************************************************" << endl;
	
	cout << "retezec: " << str << endl;
	cout << "retezec2: " << str2 << endl;
	cout << "pozadavek: " << pozadavek << endl;
	//char *prip = new char[parametr->adresaParamStr.length() + 1];
	char *prip = new char[str2.length()+1];
	//strcpy(prip, "google.com:80" );
	//strcpy(prip, "tools.ietf.org:80" );
	//prip = parametr->adresaParamStr.c_str();
	//strcpy(prip, parametr->adresaParamStr);
	strcpy(prip,str2.c_str());
	//strcpy(prip,parametr->adresaParamStr.c_str());
	cout << "test: " << prip << endl;
	//char * prip = "google.com:80"; 
	
	BIO * bio;
	bio = BIO_new_connect(prip);
	if (bio == NULL)
	{
		cerr << "CHYBA" << endl;
		return 1;
	}

	if (BIO_do_connect(bio) <= 0)
	{
		cerr << "Chyba spojeni" << endl;
		return 2;
	}
	cout << "Spojeni ok" << endl;
	return 0;	
}
/**
 * Funkce pro výpis nápovědy
 */
void printHelp()
{
	const char *HELPMSG =
	  "Ctecka novinek ve formatu Atom s podporou SSL/TLS\n"
	  "Autor: Milan Gardas (xgarda04@stud.fit.vutbr.cz)\n"
	  "Program provádí...\n"
	  "Použití: proj1 --help\n"
	  "         proj1 -t  znamená ...\n"
	  "         proj1 -h  znamená ... \n"
	  "         ..."
	  "Popis parametrů:\n"
	// sem doplňte svůj popis
	  "...\n";
	  cout << HELPMSG;
}

/**
 * Funkce pro kontrolu zadaných parametrů
 * @param   int		počet parametrů
 * @param   char	pole zadaných argumentů
 * @param	Param	struktura pro uložený zadaných argumentů
 * @return  int
 */
int parseParameters(int argc, char* argv[], Param* parametr)
{
	for(int i = 1; i < argc; i++)
	{
		if (strcmp("-l", argv[i]) == 0)
		{
			parametr->lParam++; 
		}
		else if (strcmp("-T", argv[i]) == 0)
		{
			parametr->TParam++; 
		}
		else if (strcmp("-a", argv[i]) == 0)
		{
			parametr->aParam++; 
		}
		else if (strcmp("-u", argv[i]) == 0)
		{
			parametr->uParam++; 
		}
		else if (strcmp("-f", argv[i]) == 0)
		{
			parametr->fParam++; 
			parametr->fParamStr = argv[i+1];
			i++;
		}
		else if (strcmp("-c", argv[i]) == 0)
		{
			parametr->cParam++;
			parametr->cParamStr = argv[i+1];
			i++;
		}
		else if (strcmp("-C", argv[i]) == 0)
		{
			parametr->CParam++; 
			parametr->CParamStr = argv[i+1];
			i++;
		}
		else if (('h' == argv[i][0])&&('t' == argv[i][1])&&('t' == argv[i][2])&&('p' == argv[i][3]))
		{
			parametr->adresaParam++;
			parametr->adresaParamStr = argv[i];
		}
		else
		{
			cerr << "Zadan neznamy parametr" << endl;
			return -1;
		}
	}
	if (((parametr->adresaParam > 0) && (parametr->fParam > 0)) || ((parametr->adresaParam == 0) && (parametr->fParam == 0)) || 
	(parametr->lParam > 1) || (parametr->TParam > 1) || (parametr->aParam > 1) || (parametr->uParam > 1) || (parametr->adresaParam > 1) || 
	(parametr->fParam > 1) || (parametr->cParam > 1) || (parametr->CParam > 1))
	{
		cerr << "Zadana nepovolena kombinace parametru" << endl;
		return -1;
	}
	/*
	cout << "lParam: " << parametr->lParam << endl;
	cout << "TParam: " << parametr->TParam << endl;
	cout << "aParam: " << parametr->aParam << endl;
	cout << "uParam: " << parametr->uParam << endl;
	cout << "adresaParam: " << parametr->adresaParam << " " << "adresaParamStr: " << parametr->adresaParamStr << endl;
	cout << "fParam: " << parametr->fParam << " " << "fParamStr: " << parametr->fParamStr << endl;
	cout << "cParam: " << parametr->cParam << " " << "cParamStr: " << parametr->cParamStr << endl;
	cout << "CParam: " << parametr->CParam << " " << "CParamStr: " << parametr->CParamStr << endl;
	*/
	return 0;
}
