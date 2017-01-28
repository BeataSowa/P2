#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <queue>
#include <vector>
#include <iostream>
#include <fstream>


using namespace std;

// żądanie
struct zadanie
{
	int program_id;
	int requester_track;
};

struct plik_caly
{
	int program_id;
	char* nazwa_pliku;
};


// Semafory
sem_t odpowiedz;

// Bufor (kolejka zadan)
vector<zadanie> lista_zadan;
	
void* programFun (void* t)
{
	printf("Startuje program\n");
	struct plik_caly* moj_plik = (struct plik_caly*) t;
	ifstream plik;

	plik.open(moj_plik->nazwa_pliku);
	
	if (plik.good())
	{
		int track;
		zadanie moje_zadanie;
		while (plik >> track)
		{
			moje_zadanie.program_id = moj_plik->program_id;
			moje_zadanie.requester_track = track;
			printf("requester %d track %d\n", moje_zadanie.program_id, moje_zadanie.requester_track);
			lista_zadan.push_back(moje_zadanie);
	
			// Czekaj na odpowiedz z glowicy
			sem_wait(&odpowiedz);
		}
	
	}
	
	pthread_exit(NULL);
}



void* glowicaFun (void* t)
{
	printf("Startuje glowica\n");
	//	int polozenieglowicy = 0;
	while (true)
	{
		if (!lista_zadan.empty())
		{
			zadanie obecnie_obslugiwanie_zadanie = lista_zadan.back();
			printf("service requester %d track %d\n", obecnie_obslugiwanie_zadanie.program_id, obecnie_obslugiwanie_zadanie.requester_track);
			lista_zadan.pop_back();
			// Poinformuj, że zadanie zostało obsłużone
			 sem_post(&odpowiedz);
		};
		
		
	}
	pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
	sem_init(&odpowiedz, 0, 0);
	
	int n=2;
	//Inicjuj wątki programów
	plik_caly moj_plik;
	vector <plik_caly> dane_wejsciowe;
	
	pthread_t programWatek[n];
	
	for (int i=0; i<n; i++)
	{
		
		moj_plik.program_id = i;
		moj_plik.nazwa_pliku = argv[i+1];
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
