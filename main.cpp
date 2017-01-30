#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <queue>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdlib.h> //atoi
#include <math.h>

using namespace std;

// żądanie
struct zadanie
{
	int program_id;
	int requester_track;
	sem_t* odpowiedz_czy_zostalo_wykonane_zadanie;
};

struct plik_caly
{
	int program_id;
	char* nazwa_pliku;
};

// Bufor (kolejka zadan)
vector<zadanie> lista_zadan;

//Semafory dodane
sem_t dostep_do_listy_zadan;
sem_t dostep_do_listy_zadan_dla_okreslonej_liczby_programow;

void* programFun (void* t)
{
	struct plik_caly* moj_plik = (struct plik_caly*) t;
	sem_t nowy;
	ifstream plik;

	//printf("Startuje program %d\n", moj_plik->program_id);
    
	sem_init(&nowy, 0, 0);
	plik.open(moj_plik->nazwa_pliku);

	if (plik.good())
	{
		int track;
		zadanie moje_zadanie;

		moje_zadanie.program_id = moj_plik->program_id;
		moje_zadanie.odpowiedz_czy_zostalo_wykonane_zadanie = &nowy;

		while (plik >> track)
		{
            // Otwarcie określonej liczby programów
            sem_wait(&dostep_do_listy_zadan_dla_okreslonej_liczby_programow);
    
            // Czekaj na możliwość umieszczenia zadania w liscie zadan(wektorze) 
            //printf("Program %d czeka na możliwosc umieszczenia żądania do listy żądań\n", moje_zadanie.program_id);
            sem_wait(&dostep_do_listy_zadan);
            
			moje_zadanie.requester_track = track;
            cout << "requester " << moje_zadanie.program_id << " track "  << moje_zadanie.requester_track << endl;
			//printf("requester %d track %d\n", moje_zadanie.program_id, moje_zadanie.requester_track);
			lista_zadan.push_back(moje_zadanie);

            // Zwolnij dostęp do listy żądań 
            sem_post(&dostep_do_listy_zadan);
            
			// Czekaj na odpowiedz z glowicy 
			sem_wait(&nowy);
		}
	}

	pthread_exit(NULL);
}



void* glowicaFun (void* t)
{
	//printf("Startuje glowica\n");
	int polozenieglowicy = 0;
    
	while (true)
	{
        // Czekaj na dostęp do listy zadań 
        sem_wait(&dostep_do_listy_zadan);
        
		if (!lista_zadan.empty())
		{
            vector<zadanie>::iterator it = lista_zadan.begin();
            vector<zadanie>::iterator obecnie_obslugiwane_zadanie = it;
            
            int roznica_aktualnego_a_elementem = abs(polozenieglowicy - (*obecnie_obslugiwane_zadanie).requester_track);
            it++;
            
            // Obliczenie najmniejszej odleglosci pomiedzy obecnym polozeniem glowicy a jednym z n programow
            while(it != lista_zadan.end())
            {
                int temp;
                temp = (abs(polozenieglowicy - (*it).requester_track));
                
                if (roznica_aktualnego_a_elementem > temp)
                {
                    roznica_aktualnego_a_elementem=temp;
                    obecnie_obslugiwane_zadanie= it;
                }
                it++;
            }

            polozenieglowicy= (*obecnie_obslugiwane_zadanie).requester_track;
            
            cout << "service requester " << (*obecnie_obslugiwane_zadanie).program_id << " track "  << (*obecnie_obslugiwane_zadanie).requester_track << endl;
            
			//printf("service requester %d track %d\n", (*obecnie_obslugiwane_zadanie).program_id, (*obecnie_obslugiwane_zadanie).requester_track);
            
            // Poinformuj, że zadanie zostało obsłużone
            sem_post((*obecnie_obslugiwane_zadanie).odpowiedz_czy_zostalo_wykonane_zadanie);
              
			lista_zadan.erase(obecnie_obslugiwane_zadanie, obecnie_obslugiwane_zadanie+1);
            
            // Poinformuj, że zwolniło się miejsce w liście żądań
            sem_post(&dostep_do_listy_zadan_dla_okreslonej_liczby_programow);
            
			
		};
        
        // Zwolnij dostęp do listy żądań 
        sem_post(&dostep_do_listy_zadan);
	}
    
	pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    sem_init(&dostep_do_listy_zadan, 0, 1);
    
    
	// wczytanie z poziomu konsoli liczby zadan do obsluzenia 
	//char* liczba_zadan= argv[1];
	//int liczba_zadan_w_kolejce = ;
	//printf("liczba zadan w kolejce %d\n", liczba_zadan_w_kolejce);
    sem_init(&dostep_do_listy_zadan_dla_okreslonej_liczby_programow, 0, atoi (argv[1]));

	int n = argc - 2;
	//Inicjuj wątki programów
	plik_caly moj_plik;
	vector <plik_caly> dane_wejsciowe;

	pthread_t programWatek[n];

	for (int i=0; i<n; i++)
	{

		moj_plik.program_id = i;
		moj_plik.nazwa_pliku = argv[i+2];
		dane_wejsciowe.push_back(moj_plik);
		pthread_create (&programWatek[i], NULL, programFun, &dane_wejsciowe.back());
	}

	pthread_t glowicaWatek;
	pthread_create (&glowicaWatek, NULL, glowicaFun, NULL);
	for (int i=0; i<n; i++)
	{
		pthread_join(programWatek[i], NULL);
	}

	pthread_join(glowicaWatek, NULL);

	return 0;
}
