#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <queue>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdlib.h> //atoi

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

void* programFun (void* t)
{
	struct plik_caly* moj_plik = (struct plik_caly*) t;
	sem_t nowy;
	ifstream plik;

	printf("Startuje program %d\n", moj_plik->program_id);

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
            // Czekaj na możliwość umieszczenia zadania w liscie zadan(wektorze) dodane
            printf("Program %d czeka na możliwosc umieszczenia żądania do listy żądań\n", moje_zadanie.program_id);
            sem_wait(&dostep_do_listy_zadan);
            
			moje_zadanie.requester_track = track;
			printf("requester %d track %d\n", moje_zadanie.program_id, moje_zadanie.requester_track);
			lista_zadan.push_back(moje_zadanie);

            // Zwolnij dostęp do listy żądań dodane
            sem_post(&dostep_do_listy_zadan);
            
			// Czekaj na odpowiedz z glowicy 
			sem_wait(&nowy);
            
            
            
		}

	}

	pthread_exit(NULL);
}



void* glowicaFun (void* t)
{
	printf("Startuje glowica\n");
	// int polozenieglowicy = 0;
	while (true)
	{
        // Czekaj na dostęp do listy zadań dodane
        sem_wait(&dostep_do_listy_zadan);
        
		if (!lista_zadan.empty())
		{
			zadanie obecnie_obslugiwanie_zadanie = lista_zadan.back();
			printf("service requester %d track %d\n", obecnie_obslugiwanie_zadanie.program_id, obecnie_obslugiwanie_zadanie.requester_track);
			lista_zadan.pop_back();
			// Poinformuj, że zadanie zostało obsłużone
			sem_post(obecnie_obslugiwanie_zadanie.odpowiedz_czy_zostalo_wykonane_zadanie);
		};
        
        // Zwolnij dostęp do listy żądań dodane
         sem_post(&dostep_do_listy_zadan);

	}
	pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    sem_init(&dostep_do_listy_zadan, 0, 1);

	// wczytanie z poziomu konsoli liczby zadan do obsluzenia 
	//char* liczba_zadan= argv[1];
	int liczba_zadan_w_kolejce = atoi (argv[1]);
	printf("liczba zadan w kolejce %d\n", liczba_zadan_w_kolejce);


	int n=2;
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
