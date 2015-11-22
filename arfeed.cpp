/*
 * @Projekt:    Ctecka novinek ve formatu Atom s podporou SSL/TLS 
 * @Autor:      Milan Gardas <xgarda04@stud.fit.vutbr.cz>
 * @file:       arfeed.cpp
 * @date:       22.11.2014
 */


#include "arfeed.h"

using namespace std;

int main(int argc, char* argv[])
{
	// Zpracování vstupních parametrů
	Param* parametr = new Param;
	int par = zpracujParametry(argc, argv, parametr);
	if (par == -1)
	{
		return -1;
	}

	// Zpracování zdrojů
	int ret = 0;
	string *novy;
	if (parametr->fParam == 1) // Je zadán soubor se zdroji
	{
		int rad = pocetRadku(parametr->fParamStr);
		if (rad == -3)
		{
			delete parametr;
			return -3;
		}
		else if (rad == 0)
		{
			cerr << "Zadany soubor neobsahuje zadny validni radek nebo je prazdny." << endl;
			delete parametr;
			return -4;
		}
		novy = new string[rad];
		ret = zpracovaniSouboru(parametr->fParamStr, novy);
		if (ret == -3)
		{
			delete parametr;
			return -3;
		}
	}
	else // Je zadán pouze jeden zdroj tj. URL
	{
		novy = new string[1];
		ret = 1;
		novy[0].append(parametr->adresaParamStr);
	}

	// Inicializace openSSL
	SSL_load_error_strings();
	ERR_load_BIO_strings();
	SSL_library_init();

    // Cyklus, který řídí celý program
	for (int a = 0; a < ret; a++)
	{
		// Příprava adresy pro připojení
		string pozadav;
		string adres;
		int portC;
		string adr = adresa(&novy[a], &pozadav, &adres, &portC);
		char *prip = new char[adr.length()+1];
		strcpy(prip,adr.c_str());

		// Připojení
		if (portC == HTTP)
		{
			connectHTTP(prip, pozadav, adres, parametr);
		}
		else if (portC == HTTPS)
		{
			connectHTTPS(prip, pozadav, adres, parametr);
		}
		else // Nebyl zadán port 80 nebo 443
		{
			cerr << "Byl zadan spatny port." << endl;
		}
		delete[] prip;

		// Řešení mezery mezi zdroji
		if (a+1 == ret)
		{
			
		}
		else
		{
			cout << "" << endl;
		}
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
	  "Program provadi vypis novinek ze stranek dle zadanych parametru.\n"
	  "Použití: arfeed --help\t\tVypise napovedu\n"
	  "         arfeed URL -a -T -u\tVypise vsechny prispevky spolecne s autory, aktualizaci a odkazem ze zadane stranky\n"
	  "         arfeed -f example -l\tVypise nazev nejnovejsiho prispevku na vsech strankach ktere jsou v example\n"
	  "         arfeed URL -c example\tPokud je platny certifikat example pak vypise vsechny prispevky ze zadane URL\n\n"
	  "Popis parametrů:\n"
	  "URL\t\turl se zdrojem\n"
	  "-f example\tsoubor obsahujici adresy se zdroji\n"
	  "-c example\tsoubor s certifikaty\n"
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
 * @param	Param	struktura pro uložení zadaných argumentů
 * @return  int 	0 při OK
 					-1 při chybě
 */
int zpracujParametry(int argc, char* argv[], Param* parametr)
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
		parametr->CParam++;
		parametr->CParamStr.append("/etc/ssl/certs");
	}
	return 0;
}


/**
 * Funkce pro zjištění počtu korektních řádků
 * @param   string	cesta k souboru
 * @return  int 	počet řádků při OK
 					-3 při chybě
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
    	}
		myfile.close();
		return radek;
	} 
	else
	{
		cerr << "Nepodarilo se otevrit soubor" << endl;
		return -3;
	}
}


/**
 * Funkce pro získání korektních řádků ze souboru
 * @param   string	cesta k souboru
 * @param   string*	pole řetězců pro uložení obsahu souboru
 * @return  int 	počet řádků při OK
 					-3 při chybě
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
		cerr << "Nepodařilo se otevřít soubor" << endl;
		return -3; 
	}
}


/**
 * Funkce, která rozdělí adresu zdroje na adresa:port/pozadavek
 * @param	string*	adresa zdroje
 * @param 	string* ukazatel pro uložení požadavku
 * @param 	string* ukazatel pro uložení adresy
 * @param 	int* 	ukazatel pro uložení čísla portu 
 * @return  string 	adresa ve tvaru adresa:port
 */
string adresa(string* zdroj, string* pozadav, string* adre, int* portC)
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
	pozadav->append(pozadavek);
	string adresaStr;
	adresaStr = portS(index, str2, portI, adre, portC);
	return adresaStr;
}


/**
 * Funkce, která zjistí číslo portu z prefixu
 * @param	string*	adresa zdroje
 * @param	int*	ukazatel pro uložení velikosti prefixu
 * @return  int 	číslo portu
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
		*posun = 7;
	}
	found = zdroj->find(prefix2);
	if (found!=string::npos)
	{
		portI = HTTPS;
		*posun = 8;
	}
	return portI;
}


/**
 * Funkce, která zjistí číslo portu z adresy a spojí adresu s číslem portu
 * @param	int		hodnota obsahující index lomítka v adrese
 * @param 	string 	zadaná adresa
 * @param	int		port zjistěný z prefixu
 * @param 	string* ukazatel pro uložení adresy
 * @param 	int* 	ukazatel pro uložení čísla portu
 * @return  string 	adresa ve tvaru adresa:port
 */
string portS(int index, string str, int portI, string* adre, int* portC)
{
	int delka = 0;
	bool dvojtecka = false;
	// zjisteni zda adresa obsahuje cislo portu
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
	// priprava portu pro pridani do adresy
	if (dvojtecka == true)
	{
		portStr.assign(str,(index-delka),index-1);
		str.erase(index-delka-1,delka+1);
	}
	else
	{
		portStr = to_string(portI);
	}
	string::size_type vel;
	*portC = stoi(portStr, &vel);
	string dvojteckaStr = ":";
	adre->append(str);
	// spojeni adresy s cislem portu
	str.append(dvojteckaStr);
	str.append(portStr);
	return str;
}


//******************************************************************************
/*
	Funkce pro zpracování XML byly vytvořeny za pomoci Libxml Tutorial dostupného z adresy: http://xmlsoft.org/tutorial/
	Autor: John Fleck
*/
/**
 * Funkce, která vypíše hodnotu xml elementu
 * @param	xmlDocPtr	xml strom
 * @param	xmlNodePtr	aktuální uzel xml stromu
 * @param	xmlChar*	element který vyhledáváme
 * @param	int			hodnota sloužící k vypsání hodnoty elementu se správným řetězcem
 */
void obsahElementu(xmlDocPtr doc, xmlNodePtr cur, xmlChar* text, int param)
{
	xmlChar *key;
	cur = cur->xmlChildrenNode;
	while (cur != NULL) 
	{
	    if ((!xmlStrcmp(cur->name, text))) 
	    {
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
			   	default :
			    	cerr << "Spatny parametr funkce pro parsovani XML." << endl;
		   	}
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
void odkaz(xmlNodePtr cur)
{
	xmlChar *uri;
	cur = cur->xmlChildrenNode;
	while (cur != NULL) 
	{
	    if ((!xmlStrcmp(cur->name, (const xmlChar *)"link"))) 
	    {
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
static void zpracujXML(char *docname, int xmlVelikost, Param* parametr)
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlNodePtr temp;
	xmlNodePtr pocitadlo;

	doc = xmlParseMemory(docname, xmlVelikost);
	if (doc == NULL )
	{
		cerr << "Chyba, nepodarilo se zpracovat xml" << endl;
		return;
	}
	
	cur = xmlDocGetRootElement(doc);
	
	if (cur == NULL)
	{
		cerr << "Chyba, prazdne xml" << endl;
		xmlFreeDoc(doc);
		return;
	}
	
	if (xmlStrcmp(cur->name, (const xmlChar *) "feed"))
	{
		cerr << "Chyba, korenovy element xml se nejmenuje feed" << endl;
		xmlFreeDoc(doc);
		return;
	}

	if ((!xmlStrcmp(cur->name, (const xmlChar *)"feed")))
	{
		obsahElementu(doc, cur, (xmlChar *)"title", NAZEV);
	}
	cur = cur->xmlChildrenNode;
	pocitadlo = cur;
	int s = 0;
	// Zjištění počtu feedů v xml pro pozdější výpis mezer
	while(pocitadlo != NULL)
	{
		if ((!xmlStrcmp(pocitadlo->name, (const xmlChar *)"entry")))
		{
			s++;
		}
		pocitadlo = pocitadlo->next;
	}

	int iterace = 0;
	int nalezlA = 0;
	//int nalezlU = 0, nalezlT = 0;
	while(cur != NULL)
	{
		// Vyhledani elementu "entry" v kořenovém elementu
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"entry")))
		{
			iterace++;
			// Volani funkce pro vypis nazvu clanku
			obsahElementu(doc, cur, (xmlChar *)"title", NAZEVNOVINKY);
			if (parametr->aParam == 1)
			{
				temp = cur->xmlChildrenNode;
				while(temp != NULL)
				{
					// Vyhledani elementu author v elementu entry
					if ((!xmlStrcmp(temp->name, (const xmlChar *)"author")))
					{
						// Volani funkce pro vypis autora, pouze pokud je zadan parametr -a
						obsahElementu(doc, temp, (xmlChar *)"name", AUTOR); // Parametr -a
						nalezlA = 1;
					}
					temp = temp->next; 
				}
				if (nalezlA == 0)
				{
				cout << "Autor: " << "No author" << endl;
				nalezlA = 1;
				}
			}
			// Volani funkce pro vypis datumu aktualizace, pouze pokud je zadan parametr -T
			if (parametr->TParam == 1)
			{
				obsahElementu(doc, cur, (xmlChar *)"updated", AKTUALIZACE); // Parametr -T
				//nalezlT = 1;
			}
			// Volani funkce pro ziskani parametru z xml tagu a jeho nasledne vypsani, pouze pokud je zadan parametr -u
			if (parametr->uParam == 1)
			{
				odkaz(cur); // Parametr -u
				//nalezlU = 1;
			}
			if (parametr->lParam == 1) // Nevypisuji mezeru protoze je pouze jeden zaznam
			{
				;
			}
			else if (iterace == s) // Posledni zaznam nevypisovat mezeru
			{
				; 
			}
			else if ((parametr->aParam == 1) || (parametr->uParam == 1) || (parametr->TParam == 1))
			{
				cout << "" << endl;
			}
		}
		// Pro zobrazeni pouze prvni novinky ("feedu")
		if ((parametr->lParam == 1) && (iterace == 1))
		{
			break;
		}
		nalezlA = 0;
		//nalezlU = 0;
		//nalezlT = 0;	 
		cur = cur->next;
	}
	xmlFreeDoc(doc);
	return;
}
//******************************************************************************


//******************************************************************************
/*
	Funkce pro připojení k HTTP/HTTPS serveru byly vytvořeny za pomoci návodu a souborů na www stránce: Secure programming with the OpenSSL API, Part 1: Overview of the API, dostupné z adresy: http://www.ibm.com/developerworks/library/l-openssl/
	Autor: Kenneth Ballard
*/
/**
 * Funkce, která se připojí k HTTP serveru a získá z něj data
 * @param	char*	adresa ve tvaru adresa:port
 * @param	string 	požadavek pro GET request
 * @param 	string 	adresa pro GET request
 * @param	Param*	struktura se zadanými parametry
 * @return 	int 	0 při OK
 					-1 při chybě vytvoření BIO socketu
 					-2 při chybě připojení
 					-7 pokud navratovy kod GET request není 200 nebo 301
 */
int connectHTTP(char* prip, string pozadav, string adres, Param* parametr)
{
	BIO * bio;
	bio = BIO_new_connect(prip);
	if (bio == NULL)
	{
		cerr << "CHYBA" << endl;
		BIO_free_all(bio);
		return -1;
	}
	if (BIO_do_connect(bio) <= 0)
	{
		cerr << "Chyba spojeni" << endl;
		BIO_free_all(bio);
		return -2;
	}

	// Skládání GET requestu 
	string s1, s2, s3, s4, s5, s6;
	s1 = "GET ";
	s2 = pozadav;
	s3 = " HTTP/1.0\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:40.0) Gecko/20100101 Firefox/40.1\r\nHost: ";
	s4 = adres;
	s5 = "\r\nConnection: close\r\n\r\n";
	s6.clear();
	s6.append(s1);
	s6.append(s2);
	s6.append(s3);
	s6.append(s4);
	s6.append(s5); 
	char *request = new char[s6.length()+1];
	strcpy(request,s6.c_str());
    
	// Zaslání GET requestu
    BIO_write(bio, request, strlen(request));

    // Čtení odpovědi serveru na GET request 
    string stranka = "";
    int p;
    char r[1024];
    for(;;)
    {
        p = BIO_read(bio, r, 1023);
        if(p <= 0) break;
        r[p] = 0;
       	stranka.append(r);
	}

	// Uzavření spojení
	BIO_free_all(bio);
	delete[] request;

    // Zjištění návratového kódu odpovědi na GET request
    int kod = navratovyKod(stranka, parametr);
	if (kod == 1) // přesměrování
	{
		return 0;
	}
	else if (kod == -7) // chyba
	{
		return -7; 
	}

	// Nalezení začátku xml z GET požadavku
	unsigned int delkaHlavicky = 0;
	for(unsigned int i = 0; i < stranka.length(); i++)
	{
		if ((stranka[i] == '<') && (stranka[i+1] == 'f') && (stranka[i+2] == 'e') && 
        (stranka[i+3] == 'e') && (stranka[i+4] == 'd') && (stranka[i+5] == ' ') && 
        (stranka[i+6] == 'x') && (stranka[i+7] == 'm') && (stranka[i+8] == 'l') && 
        (stranka[i+9] == 'n') && (stranka[i+10] == 's') && (stranka[i+11] == '='))
		{
			delkaHlavicky = i;
			break;
		}
	}

	stranka.erase(stranka.begin(), stranka.begin()+delkaHlavicky);
	int xmlVelikost = stranka.length() + 1;
	char *xmldoc = new char[xmlVelikost];
	strcpy(xmldoc,stranka.c_str());

	// Zpracování xml
	zpracujXML(xmldoc, xmlVelikost, parametr);
	delete[] xmldoc;
	return 0;
}


/**
 * Funkce, která se připojí k HTTPS serveru a získá z něj data
 * @param	char*	adresa ve tvaru adresa:port
 * @param	string 	požadavek pro GET request
 * @param 	string 	adresa pro GET request
 * @param	Param*	struktura se zadanými parametry
 * @return 	int 	0 při OK
 					-1 při chybě vytvoření BIO socketu
 					-2 při chybě připojení
 					-5 při otevírání certifikátu
 					-6 při chybě ověřování certifikátu
 					-7 pokud navratovy kod GET request není 200 nebo 301
 */
int connectHTTPS(char* prip, string pozadav, string adres, Param* parametr)
{
    BIO * bio;
    SSL * ssl;
    SSL_CTX * ctx;
    ctx = SSL_CTX_new(SSLv23_client_method());

    // Načtení certifikátu z -c parametru 
    if (parametr->cParam == 1)
    {
    	char *cFile = new char[parametr->cParamStr.length()+1];
		strcpy(cFile,parametr->cParamStr.c_str());
        if(! SSL_CTX_load_verify_locations(ctx, cFile, NULL))
        {
        	cerr << "Chyba pri otevirani certifikatu." << endl;
            SSL_CTX_free(ctx);
            delete[] cFile;
            return -5;
        }
        delete[] cFile;
    }

    // načtení certifikátů ze složky z -C parametru
    char *CDirectory = new char[parametr->CParamStr.length()+1];
	strcpy(CDirectory,parametr->CParamStr.c_str());
    if(! SSL_CTX_load_verify_locations(ctx, NULL, CDirectory))
    {
    	cerr << "Chyba pri otevirani certifikatu ze zadane slozky." << endl;
        SSL_CTX_free(ctx);
        delete[] CDirectory;
        return -5;
    }
    delete[] CDirectory;

    // Nastavení spojení 
    bio = BIO_new_ssl_connect(ctx);

    // Nastavení SSL
    BIO_get_ssl(bio, & ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    // Vytvoření spojení 
    BIO_set_conn_hostname(bio, prip);

    if(BIO_do_connect(bio) <= 0)
    {
    	cerr << "Nepodařilo se připojit k serveru: " << adres << endl;
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        return -2;
    }

    // Kontrola certifikátu 
    if(SSL_get_verify_result(ssl) != X509_V_OK)
    {
    	string zdroj(prip); 
    	zdroj.erase(zdroj.end()-4, zdroj.end());
    	cerr << "Chyba: nepodarilo se overit platnost certifikatu serveru " << zdroj << endl;
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        return -6;
    }

    // Skládání GET requestu
    string s1, s2, s3, s4, s5, s6;
    s1 = "GET ";
    s2 = pozadav;
    s3 = " HTTP/1.0\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:40.0) Gecko/20100101 Firefox/40.1\r\nHost: ";
    s4 = adres;
    s5 = "\r\nConnection: close\r\n\r\n";
    s6 = s1;
    s6.append(s2);
    s6.append(s3);
    s6.append(s4);
    s6.append(s5);
    char *request = new char[s6.length()+1];
    strcpy(request,s6.c_str());

    // Zaslání GET requestu 
    BIO_write(bio, request, strlen(request));

    // Čtení odpovědi serveru na GET request 
    string stranka = "";
    int p;
	char r[1024];
    for(;;)
    {
        p = BIO_read(bio, r, 1023);
        if(p <= 0) break;
        r[p] = 0;
        //printf("%s", r);
        stranka.append(r);
    }

    // Uzavření spojení 
    BIO_free_all(bio);
    SSL_CTX_free(ctx);
    delete[] request;

    // Zjištění návratového kódu odpovědi na GET request
    int kod = navratovyKod(stranka, parametr);
	if (kod == 1) // přesměrování
	{
		return 0;
	}
	else if (kod == -7) // chyba
	{
		return -7; 
	}

    // Nalezení začátku xml z GET požadavku
    unsigned int delkaHlavicky = 0;
    for(unsigned int i = 0; i < stranka.length(); i++)
    {
        if ((stranka[i] == '<') && (stranka[i+1] == 'f') && (stranka[i+2] == 'e') && 
        (stranka[i+3] == 'e') && (stranka[i+4] == 'd') && (stranka[i+5] == ' ') && 
        (stranka[i+6] == 'x') && (stranka[i+7] == 'm') && (stranka[i+8] == 'l') && 
        (stranka[i+9] == 'n') && (stranka[i+10] == 's') && (stranka[i+11] == '='))
        {
            delkaHlavicky = i;
            break;
        }
    }

    stranka.erase(stranka.begin(), stranka.begin()+delkaHlavicky);
    int xmlVelikost = stranka.length() + 1;
    char *xmldoc = new char[xmlVelikost];
    strcpy(xmldoc,stranka.c_str());

    // Zpracování xml
    zpracujXML(xmldoc, xmlVelikost, parametr);
    delete[] xmldoc;
    return 0;
}
//******************************************************************************


/**
 * Funkce, zjistí kód odpovědi serveru
 * @param	string 	odpověď na GET request
 * @param	Param*	struktura se zadanými parametry
 * @return 	int 	0 při 200 OK
 					1 při 301 Moved Permanently
 					-7 pokud odpověď není 200 nebo 301
 */
int navratovyKod(string stranka, Param* parametr)
{
	string ok = "200 OK";
	string moved = "301 Moved Permanently";
	string location = "Location: ";
	string radek = "\n";
	size_t found;

	// Vyhledání řetězce 200 OK
	found = stranka.find(ok);
	if (found != string::npos)
	{
		return 0;
	}

	// Vyhledání řetězce 301 Moved Pernamently
	found = stranka.find(moved);
	if (found != string::npos)
	{
		// Vyhledání řetězce Location: 
		found = stranka.find(location);
		if (found != string::npos)
		{
			// Mazání obsahu odpovědi do konce řetezce Location: 
			stranka.erase(stranka.begin(), stranka.begin()+found+location.length());
			// Hledání adresy na ktrou byla stránka přesunuta
			found = stranka.find(radek);
			if (found != string::npos)
			{
				// smazání obsahu odpovědi tak, aby zbyla pouze adresa kam se přesuneme
				stranka.erase(stranka.begin()+found-1, stranka.end());

				// Příprava adresy pro připojení
				string* novy = new string[1]; 
				novy->append(stranka);
				string pozadav;
				string adres;
				int portC;
				string adr = adresa(&novy[0], &pozadav, &adres, &portC);
				char *prip = new char[adr.length()+1];
				strcpy(prip,adr.c_str());

				// Připojení
				if (portC == HTTP)
				{
					connectHTTP(prip, pozadav, adres, parametr);
				}
				else if (portC == HTTPS)
				{
					connectHTTPS(prip, pozadav, adres, parametr);
				}
				delete[] prip;
				delete[] novy;
				return 1;
			}
			return -7;
		}
		return -7;
	}
	return -7;
}