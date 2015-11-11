#include <iostream>
#include <string.h>
#include <cstring>
#include <cctype>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#define HTTP 80
#define HTTPS 443
#define NAZEV 1
#define AUTOR 2
#define AKTUALIZACE 3
#define NAZEVNOVINKY 4
#define ATOMAUTOR 5

using namespace std;

// struktura pro uložení parametrů příkazové řádky
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

// deklarace funkčních prototypů
void printHelp();
int parseParameters(int argc, char* argv[], Param* parametr);
string adresa(Param* parametr);
int port(Param* parametr, int* posun);
string portS(int index, string str, int portI);
void parseStoryMy (xmlDocPtr doc, xmlNodePtr cur, xmlChar* text, int param);
void getReference (xmlNodePtr cur);
static void parseDoc(char *docname, int xmlVelikost, Param* parametr);


int main(int argc, char* argv[])
{
	Param* parametr = new Param;
	int par = parseParameters(argc, argv, parametr);
	if (par == -1)
	{
		return -1;
	}

		
	string adr = adresa(parametr);

	char *prip = new char[adr.length()+1];
	strcpy(prip,adr.c_str());
	//cout << "Adresa: " << prip << endl;
	//--------------------------------------------------------------------------------------------------
	// Pripojeni

	//funkce pro openSSL
	SSL_load_error_strings();
	ERR_load_BIO_strings();
	SSL_library_init();

	BIO * bio;
	bio = BIO_new_connect(prip);
	delete[] prip;
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
	//cout << "Spojeni ok" << endl;
	//----------------------------------------------------------------
	//char const * request = "GET / HTTP/1.1\x0D\x0AHost: www.verisign.com\x0D\x0A\x43onnection: Close\x0D\x0A\x0D\x0A";
	//char const * request = "GET / HTTP/1.1\x0D\x0AHost:http://tools.ietf.org/agenda/atom\x0D\x0A\x43onnection: Close\x0D\x0A\x0D\x0A";
	//char const * request = "GET / HTTP/1.1\x0D\x0AHost: www.w3.org/pub/WWW/TheProject";
	//char const * request = "GET / HTTP/1.1\r\nHost: www.google.com\r\nConnection: close\r\n\r\n"; // relativne funkcni
	//char const * request = "GET /agenda/atom HTTP/1.1\r\nHost: tools.ietf.org\r\nConnection: close\r\n\r\n"; // 403 forbidden
	//char const * request = "GET /headlines.atom HTTP/1.1\r\nHost: www.theregister.co.uk\r\nConnection: close\r\n\r\n"; // SPRAVNY GET REQUEST
	//////////////////
	// skladani GET requestu https://www.theregister.co.uk/hardware/headlines.atom
	string s1, s2, s3, s4, s5, s6;
	s1 = "GET ";
	s2 = "/hardware/headlines.atom";//"/headlines.atom";
	s3 = " HTTP/1.0\r\nHost: ";
	s4 = "www.theregister.co.uk";//"www.theregister.co.uk";
	s5 = "\r\nConnection: close\r\n\r\n";
	s6 = s1;
	s6.append(s2);
	s6.append(s3);
	s6.append(s4);
	s6.append(s5);
	char *request = new char[s6.length()+1];
	strcpy(request,s6.c_str());
	//////////////////
    char r[1024];
	/* Send the request */

    BIO_write(bio, request, strlen(request));
    //int velikost = 0;
    /* Read in the response */
    int p;
    string stranka = "";
    for(;;)
    {
        p = BIO_read(bio, r, 1023);
        if(p <= 0) break;
        /*cout << endl << "--------------------------------------------------" << endl;
        cout << "Hodnota promenne p: " << p << endl;
        cout << "--------------------------------------------------" << endl;*/
        //velikost = velikost + p;
        r[p] = 0;
        stranka.append(r);
        //printf("%s", r);
    }
    //cout << endl << "Velikost je: " << velikost << endl;
    //cout << "--------------------------------------------------" << endl;
	//cout << stranka << endl; // premenna stranka obsahuje obsah stránky získaný přes GET request
	//cout << "Prijato: " << buf << endl; (http://www.theregister.co.uk/headlines.atom)
	//----------------------------------------------------------------
	BIO_free_all(bio);
	delete[] request;
	
	/*string xml = "<?xml version=";
	size_t foundd = stranka.find(xml);
  	if (foundd!=std::string::npos)
    std::cout << "Nalezeno: " << foundd << " delka: " << xml.length() << " Act: " << foundd - xml.length() << endl;
	stranka.erase(stranka.begin(), stranka.begin()+(foundd-xml.length()));
	cout << "---------------------------------------------------------" << endl;
	cout << stranka << endl;*/
	//cout << "--------------------------------------------------" << endl;
	unsigned int delkaHlavicky = 0;
	for(unsigned int i = 0; i < stranka.length(); i++)
	{
		if ((stranka[i] == '<') && (stranka[i+1] == '?') && (stranka[i+2] == 'x') && (stranka[i+3] == 'm') && (stranka[i+4] == 'l') && (stranka[i+5] == ' ') && (stranka[i+6] == 'v') && (stranka[i+7] == 'e') && (stranka[i+8] == 'r') && (stranka[i+9] == 's') && (stranka[i+10] == 'i') && (stranka[i+11] == 'o') && (stranka[i+12] == 'n') && (stranka[i+13] == '='))
		{
			delkaHlavicky = i;
			break;
		}
	}
	stranka.erase(stranka.begin(), stranka.begin()+delkaHlavicky);
	int xmlVelikost = stranka.length() + 1;
	char *xmldoc = new char[xmlVelikost];
	strcpy(xmldoc,stranka.c_str());
	//cout << stranka << endl;
	//cout << xmldoc << endl;
	parseDoc (xmldoc, xmlVelikost, parametr);
	delete[] xmldoc;
	delete parametr;
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
		else if (((strcmp("-h", argv[i]) == 0) || (strcmp("--help", argv[i]) == 0)) && (argc != 2))
		{
			cerr << "Parametr napovedy nebyl zadan samostatne" << endl;
			delete parametr;
			return -1;
		}
		else if ((strcmp("-h", argv[i]) == 0) || (strcmp("--help", argv[i]) == 0))
		{
			printHelp();
			delete parametr;
			return -1;
		}
		else if (('h' == argv[i][0])&&('t' == argv[i][1])&&('t' == argv[i][2])&&('p' == argv[i][3]))
		{
			parametr->adresaParam++;
			parametr->adresaParamStr = argv[i];
		}
		else
		{
			cerr << "Zadan neznamy parametr" << endl;
			delete parametr;
			return -1;
		}
	}
	if (((parametr->adresaParam > 0) && (parametr->fParam > 0)) || ((parametr->adresaParam == 0) && (parametr->fParam == 0)) || 
	(parametr->lParam > 1) || (parametr->TParam > 1) || (parametr->aParam > 1) || (parametr->uParam > 1) || (parametr->adresaParam > 1) || 
	(parametr->fParam > 1) || (parametr->cParam > 1) || (parametr->CParam > 1))
	{
		cerr << "Zadana nepovolena kombinace parametru" << endl;
		delete parametr;
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


/**
 * Funkce, která spojí adresu s portem
 * @param	Param	struktura pro uložený zadaných argumentů
 * @return  string
 */
string adresa(Param* parametr)
{
	int portI = 0;
	int posun = 0;
	string str, str2;
	string lomitko = "/";
	portI = port(parametr, &posun);
	str.assign(parametr->adresaParamStr,posun,parametr->adresaParamStr.length());
	int index;
	str.append(lomitko); // pridani lomitka na konec adresy => zajisti ze lomitko bude vzdy nalezeno 
	for(unsigned int i = 0; i < str.length(); i++)
	{
		if (str[i] == '/')
		{
			index = i;
			break;
		}
	}

	string pozadavek;
	str2.assign(str,0,index); // <host>:<port>
	pozadavek.assign(str,index,str.length()); // <path>?<searchpart>
	pozadavek.pop_back(); // odebrani lomitka na konci adresy
	string adresaStr;
	adresaStr = portS(index, str2, portI);

	return adresaStr;
}


/**
 * Funkce, která zjistí číslo portu z prefixu
 * @param	Param*	ukazatel na strukturu uložených argumentů
 * @param	int*	ukazatel na int pro uložení velikosti prefixu
 * @return  int
 */
int port(Param* parametr, int* posun)
{
	string prefix1 = "http://";
	string prefix2 = "https://";
	int portI = 0;
	size_t found = parametr->adresaParamStr.find(prefix1);
	if (found!=string::npos)
	{
		portI = HTTP;
		//cout << "http nalezeno na " << found << " port: " << portI << endl;
		*posun = 7;
	}
	found = parametr->adresaParamStr.find(prefix2);
	if (found!=string::npos)
	{
		portI = HTTPS;
		//cout << "https nalezeno na " << found << " port: " << portI << endl;
		*posun = 8;
	}
	return portI;
}


/**
 * Funkce, která zjistí číslo portu z adresy
 * @param	int		hodnota obsahující index lomítka v adrese
 * @param 	string 	zadaná adresa
 * @param	int		port zjistěný z prefixu
 * @return  string
 */
string portS(int index, string str, int portI)
{
	int delka = 0;
	bool dvojtecka = false;
	for(int i = index-1; i > 0; i--)
	{
		if (isdigit(str[i]) == true)
		{
			delka++;
			continue;
		}
		else if (str[i] == ':')
		{
			dvojtecka = true;
			break;
		}
		else
		{
			break;
		}
	}

	string portStr;
	if (dvojtecka == true)
	{
		portStr.assign(str,(index-delka),index-1);
		str.erase(index-delka-1,delka+1);
	}
	else
	{
		portStr = to_string(portI);
	}
	string dvojteckaStr = ":";
	str.append(dvojteckaStr);
	str.append(portStr);
	return str;
}


/**
 * Funkce, která vypíše hodnotu xml elementu
 * @param	xmlDocPtr	xml strom
 * @param	xmlNodePtr	aktuální uzel xml stromu
 * @param	xmlChar*	element který vyhledáváme
 * @param	int			hodnota sloužící k vypsání hodnoty elementu se správným řetězcem
 */
void parseStoryMy (xmlDocPtr doc, xmlNodePtr cur, xmlChar* text, int param)
{
	xmlChar *key;
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
	    if ((!xmlStrcmp(cur->name, text))) {
		    key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			switch(param)
		    {
		    case NAZEV :
		     	cout << "*** " << key << " ***" << endl; 
		    	break;
		  	case AUTOR :
		  		cout << "Autor: " << key << endl; 
		    	break;
		  	case AKTUALIZACE :
		    	cout << "Aktualizace: " << key << endl;
		    	break;
		   	case NAZEVNOVINKY :
		    	cout << key << endl;
		    	break;
		    case ATOMAUTOR :
		    	// ulozit do promenne
		    	break;
		   	default :
		    	cerr << "Spatny parametr funkce pro parsovani XML." << endl;
		   	}
		    //printf("%s: %s\n",text, key);
		    xmlFree(key);
 	    }
	cur = cur->next;
	}
    return;
}


/**
 * Funkce, která vypíše hodnotu parametru xml elementu
 * @param	xmlNodePtr	aktuální uzel xml stromu
 */
void getReference (xmlNodePtr cur)
{
	xmlChar *uri;
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
	    if ((!xmlStrcmp(cur->name, (const xmlChar *)"link"))) {
		    uri = xmlGetProp(cur, (xmlChar *)"href");
		    cout << "URL: " << uri << endl;
		    xmlFree(uri);
	    }
	    cur = cur->next;
	}
	return;
}


/**
 * Funkce, která zpracuje xml získané ze zadané stránky
 * @param	char*	uložení obsahu stránky
 * @param	int		velikost obsahu stránky
 * @param	Param*	struktura se zadanými parametry
 */
static void parseDoc(char *docname, int xmlVelikost, Param* parametr)
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlNodePtr temp;

	//doc = xmlParseFile(docname); // pravděpodobně bude změna na xmlParseMemory
	doc = xmlParseMemory(docname, xmlVelikost);
	if (doc == NULL )
	{
		//fprintf(stderr,"Document not parsed successfully. \n");
		cerr << "Document not parsed successfully." << endl;
		return;
	}
	
	cur = xmlDocGetRootElement(doc);
	
	if (cur == NULL)
	{
		//fprintf(stderr,"empty document\n");
		cerr << "empty document" << endl;
		xmlFreeDoc(doc);
		return;
	}
	
	if (xmlStrcmp(cur->name, (const xmlChar *) "feed"))
	{
		//fprintf(stderr,"document of the wrong type, root node != feed");
		cerr << "document of the wrong type, root node != feed" << endl;
		xmlFreeDoc(doc);
		return;
	}
	////////////////////////
	// uroven zanoreni 1
	if ((!xmlStrcmp(cur->name, (const xmlChar *)"feed")))
	{
		parseStoryMy (doc, cur, (xmlChar *)"title", NAZEV);
	}
	//vyhledavani elementu "author" v rodicovskem uzlu
	//temp = cur;
	cur = cur->xmlChildrenNode;
	//printf("jmeno cur elementu: %s\n", cur->name);
	int iterace = 0;
	while(cur != NULL)
	{
		//iterace++;
		// vyhledavani elementu "author v kořenovém elementu"
		if (((!xmlStrcmp(cur->name, (const xmlChar *)"author"))) && (parametr->aParam == 1))
		{
			// volani funkce pro ulozeni jmeno autor celeho atom dokumentu
			parseStoryMy (doc, cur, (xmlChar *)"name", ATOMAUTOR);
		}
		// vyhledani elementu "entry" v kořenovém elementu
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"entry")))
		{
			iterace++;
			// volani funkce pro vypis nazvu clanku
			parseStoryMy (doc, cur, (xmlChar *)"title", NAZEVNOVINKY);
			// volani funkce pro vypis datumu aktualizace, pouze pokud je zadan parametr -T
			if (parametr->TParam == 1)
			{
				parseStoryMy (doc, cur, (xmlChar *)"updated", AKTUALIZACE); // Parametr -T
			}
			if (parametr->uParam == 1)
			{
			// volani funkce pro ziskani parametru z xml tagu a jeho nasledne vypsani
			getReference (cur);
			}
			if (parametr->aParam == 1)
			{
				temp = cur->xmlChildrenNode;
				while(temp != NULL)
				{
					// vyhledani elementu author v elementu entry
					if ((!xmlStrcmp(temp->name, (const xmlChar *)"author")))
					{
						// volani funkce pro vypis autora, pouze pokud je zadan parametr -a
						parseStoryMy(doc, temp, (xmlChar *)"name", AUTOR); // Parametr -a
					}
					temp = temp->next; 
				}
			}
			if ((parametr->aParam == 1) || (parametr->uParam == 1) || (parametr->TParam == 1))
			{
				cout << "" << endl; // mezera mezi jednotlivymi novinkami bez parametru tu nema co delat !!
			}
		}
		if ((parametr->lParam == 1) && (iterace == 1))
		{
			cout << "BREAK" << endl;
			break;
		}	 
	cur = cur->next;
	}
	xmlFreeDoc(doc);
	return;
}