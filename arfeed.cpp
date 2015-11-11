#include <iostream>
#include <string.h>
#include <cstring>
#include <cctype>
#include <fstream>
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
int pocetRadku (string cesta);
int zpracovaniSouboru(string cesta, string * radky);
string adresa(string* zdroj, string* pozadav, string* adre);
int port(string* zdroj, int* posun);
string portS(int index, string str, int portI, string* adre);
void parseStoryMy (xmlDocPtr doc, xmlNodePtr cur, xmlChar* text, int param);
void getReference (xmlNodePtr cur);
static void parseDoc(char *docname, int xmlVelikost, Param* parametr);


int main(int argc, char* argv[])
{
	ofstream errorFile( "EF" ); 
	cerr.rdbuf( errorFile.rdbuf() ); 
	// Zpracovani vstupnich parametru
	Param* parametr = new Param;
	int par = parseParameters(argc, argv, parametr);
	if (par == -1)
	{
		return -1;
	}

	// Zpracovani zdroju
	int ret = 0;
	string *novy;
	if (parametr->fParam == 1) // je zadan soubor se zdroji
	{
		int rad = pocetRadku(parametr->fParamStr);
		if (rad == -3)
		{
			return -3;
		}
		else if (rad == 0)
		{
			cerr << "Zadaný soubor neobsahuje zadny validni radek nebo je prazdny." << endl;
			return -4;
		}
		novy = new string[rad];
		ret = zpracovaniSouboru(parametr->fParamStr, novy);
		if (ret == -3)
		{
			return -3;
		}
	}
	else // Je zadan pouze jeden zdroj
	{
		novy = new string[1];
		ret = 1;
		novy[0].append(parametr->adresaParamStr);
	}
/*
	for(int i = 0; i < ret; i++)
    {
		cout << novy[i] << endl;
    }*/
	SSL_load_error_strings();
	ERR_load_BIO_strings();
	SSL_library_init();
    // -------------------------------------------------------------------------
    // Cyklus ktery ridi cely program
	for (int a = 0; a < ret; a++)
	{
		string pozadav;
		string adres;
		string adr = adresa(&novy[a], &pozadav, &adres);
		/*string adr = "www.tools.ietf.org:80";
		string adres = "/agenda/atom";
		string pozadav = "www.tools.ietf.org";*/
		char *prip = new char[adr.length()+1];
		strcpy(prip,adr.c_str());
		/*cout << "Adresa: " << prip << endl;
		cout << "PozadavekMain: " << pozadav << endl;
		cout << "Adresabezportu: " << adres << endl;*/
		//--------------------------------------------------------------------------------------------------
		// Pripojeni
		
		BIO * bio;
		bio = BIO_new_connect(prip);
		delete[] prip;
		if (bio == NULL)
		{
			cerr << "CHYBA" << endl;
			return -1;
		}

		if (BIO_do_connect(bio) <= 0)
		{
			cerr << "Chyba spojeni" << endl;
			return -2;
		}

		// skladani GET requestu 
		string s1, s2, s3, s4, s5, s6;
		s1 = "GET ";
		s2 = pozadav;//"/hardware/headlines.atom";//"/headlines.atom";
		s3 = " HTTP/1.0\r\nHost: ";
		s4 = adres;//"www.theregister.co.uk";//"www.theregister.co.uk";
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
	    /* Read in the response */
	    int p;
	    string stranka = "";
	    for(;;)
	    {
	        p = BIO_read(bio, r, 1023);
	        if(p <= 0) break;
	        r[p] = 0;
	        stranka.append(r);

	    }

		BIO_free_all(bio);
		delete[] request;
		
		//cout << stranka << endl;

		unsigned int delkaHlavicky = 0;
		for(unsigned int i = 0; i < stranka.length(); i++)
		{
			if ((stranka[i] == '<') && (stranka[i+1] == '?') && (stranka[i+2] == 'x') && 
			(stranka[i+3] == 'm') && (stranka[i+4] == 'l') && (stranka[i+5] == ' ') && 
			(stranka[i+6] == 'v') && (stranka[i+7] == 'e') && (stranka[i+8] == 'r') && 
			(stranka[i+9] == 's') && (stranka[i+10] == 'i') && (stranka[i+11] == 'o') && 
			(stranka[i+12] == 'n') && (stranka[i+13] == '='))
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
	}
	delete[] novy;
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
	  "Program provádí výpis novinek ze stranek dle zadaných parametru.\n"
	  "Použití: arfeed --help\t\tVypise napovedu\n"
	  "         arfeed URL -a -T -u\tVypise vsechny prispevky spolecne s autory, aktualizaci a odkazem ze zadane stranky\n"
	  "         arfeed -f example -l\tVypise nazev nejnovejsiho prispevku na vsech strankach ktere jsou v example\n"
	  "         arfeed URL -c example\tPokud je platny certifikat example pak vypise vsechny prispevky ze zadane URL\n\n"
	  "Popis parametrů:\n"
	  "URL\t\turl se zdrojem\n"
	  "-f example\tsoubor obsahujici adresy se zdroji\n"
	  "-c example\tsoubor s certifikáty\n"
	  "-C example\tsložka obsahující soubory s certifikáty\n"
	  "-a\t\tzajisti ze u kazde novinky bude vypsan i jeji autor\n"
	  "-T\t\tzajisti ze u kazde novinky bude vypsan i cas zmeny\n"
	  "-u\t\tzajisti ze u kazde novinky bude vypsan i odkaz na ni\n"
	  "-l\t\tzajisti ze se vypisi pouze nejnovejsi prispevek\n";
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
	if (parametr->CParam == 0)
	{
		parametr->CParamStr.append("/etc/ssl/certs");
	}
	return 0;
}


/**
 * Funkce pro zjištění počtu korektních řádků
 * @param   string	cesta k souboru
 * @return  int
 */
int pocetRadku (string cesta)
{
	string line;
	int radek = 0;
	ifstream myfile (cesta);
	if (myfile.is_open())
	{
		while (getline (myfile,line))
    	{
     	// cout << line << '\n';
			for(unsigned int i = 0; i < line.length(); i++)
			{
				if(line[i] == '#')
				{
					line.erase(line.begin()+i, line.end());
				}
			} 
			if (line[0] == 'h')
			{
				radek++;
			}
		//cout << line << endl;
    	}
    	//cout << "Pocet neprazdnych radku: " << radek << endl;
		myfile.close();
		return radek;
	} 
	else
	{
		cout << "Unable to open file" << endl;
		return -3;
	}
}


/**
 * Funkce pro získání korektních řádků ze souboru
 * @param   string	cesta k souboru
 * @param   string*	pole řetězců pro uložení obsahu souboru
 * @return  int
 */
int zpracovaniSouboru(string cesta, string * radky)
{
	string line;
	ifstream soubor (cesta);
	if (soubor.is_open())
	{
    	int pocet = 0;
    	while ( getline (soubor,line) )
    	{
			//cout << "TEST" << endl;
			for(unsigned int i = 0; i < line.length(); i++)
			{
				if(line[i] == '#')
				{
					line.erase(line.begin()+i, line.end());
				}
			}
				if (line[0] == 'h')
				{
					radky[pocet] = line;
					pocet++;
				}
    		
		}
    	soubor.close();
    	return pocet;
 	}
	else
	{
		cout << "Unable to open file" << endl;
		return -3; 
	}
}


/**
 * Funkce, která spojí adresu s portem
 * @param	Param	struktura pro uložený zadaných argumentů
 * @return  string
 */
string adresa(string* zdroj, string* pozadav, string* adre)
{
	int portI = 0;
	int posun = 0;
	string str, str2;
	string lomitko = "/";
	portI = port(zdroj, &posun);
	str.assign(*zdroj,posun,zdroj->length());
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
	//pozadav = new string[pozadavek.length()];
	pozadav->append(pozadavek);
	string adresaStr;
	adresaStr = portS(index, str2, portI, adre);
	/*cout << "-------------------------------------------------------------------" << endl;
	cout << "AdresaStr: " << adresaStr << endl;
	cout << "Pozadavek: " << pozadavek << endl;
	cout << "-------------------------------------------------------------------" << endl;*/
	return adresaStr;
}


/**
 * Funkce, která zjistí číslo portu z prefixu
 * @param	Param*	ukazatel na strukturu uložených argumentů
 * @param	int*	ukazatel na int pro uložení velikosti prefixu
 * @return  int
 */
int port(string* zdroj, int* posun)
{
	string prefix1 = "http://";
	string prefix2 = "https://";
	int portI = 0;
	size_t found = zdroj->find(prefix1);
	if (found!=string::npos)
	{
		portI = HTTP;
		//cout << "http nalezeno na " << found << " port: " << portI << endl;
		*posun = 7;
	}
	found = zdroj->find(prefix2);
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
string portS(int index, string str, int portI, string* adre)
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
	//cout << "Str: " << str << endl;
	adre->append(str);
	str.append(dvojteckaStr);
	str.append(portStr);
	/*cout << "-------------------------------------------------------------------" << endl;
	cout << "Str: " << str << endl;
	cout << "-------------------------------------------------------------------" << endl;*/
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
	int nalezl = 0;
	while (cur != NULL) {
	    if ((!xmlStrcmp(cur->name, text))) {
		    key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			switch(param)
		    {
		    case NAZEV :
		     	cout << "*** " << key << " ***" << endl;
		     	nalezl = 1; 
		    	break;
		  	case AUTOR :
		  		cout << "Autor: " << key << endl;
		  		nalezl = 1; 
		    	break;
		  	case AKTUALIZACE :
		    	cout << "Aktualizace: " << key << endl;
		    	nalezl = 1;
		    	break;
		   	case NAZEVNOVINKY :
		    	cout << key << endl;
		    	nalezl = 1;
		    	break;
		    case ATOMAUTOR :
		    	// ulozit do promenne
		    	nalezl = 1;
		    	break;
		   	default :
		    	cerr << "Spatny parametr funkce pro parsovani XML." << endl;
		   	}
		    //printf("%s: %s\n",text, key);
		    xmlFree(key);
 	    }
 	    
		cur = cur->next;
	}
	if (nalezl == 0)
 	{
 	   	switch(param)
	    {
	  	case AUTOR :
	  		cout << "Autor: " << "No author" << endl; 
	    	break;
	  	case AKTUALIZACE :
	    	cout << "Aktualizace: " << "Nebyla zadana" << endl;
	    	break;
	    case ATOMAUTOR :
	    	// ulozit do promenne
	    	break;
	 	default :
	    	cerr << "Spatny parametr funkce pro parsovani XML." << endl;
	   	}
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
	int nalezl = 0;
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
						nalezl = 1;
					}
					temp = temp->next; 
				}
				if (nalezl == 0)
				{
				cout << "Autor: " << "No author" << endl;
				nalezl = 1;
				}
			}
			// mezera mezi jednotlivými záznamy
			if ((parametr->aParam == 1) || (parametr->uParam == 1) || (parametr->TParam == 1))
			{
				cout << "" << endl; // mezera mezi jednotlivymi novinkami bez parametru tu nema co delat !!
			}
		}
		// pro zobrazeni pouze prvni novinky ("feedu")
		if ((parametr->lParam == 1) && (iterace == 1))
		{
			break;
		}
	nalezl = 0;	 
	cur = cur->next;
	}
	xmlFreeDoc(doc);
	return;
}