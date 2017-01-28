#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <queue>
#include <vector>
#include <iostream>


using namespace std;

// żądanie
struct zadanie
{
	int program_id;
	int requester_track;
};

// Semafory
//sem_t pojawilo_sie_nowe_zadanie;
//sem_t zadanie_obsluzone;
sem_t odpowiedz;

/*/ Program
struct program 
{
	int id;
	queue <int> zadania;
	
};*/

//Bufor (kolejka zadan)
vector<zadanie> lista_zadan;

// Zmienne

//program programy[1]; 
	
void* programFun (void* t)
{
	printf("Startuje program\n");
	queue<int> zadania;
	zadania.push(233);
	zadania.push(239);
	zadania.push(533);
	zadania.push(33);
	zadania.push(23);
	
	zadanie moje_zadanie;
	while (!zadania.empty())
	{
		moje_zadanie.program_id = 1;
		moje_zadanie.requester_track = zadania.front();
		zadania.pop();
		printf("requester %d track %d\n", moje_zadanie.program_id, moje_zadanie.requester_track);
		lista_zadan.push_back(moje_zadanie);
	
		// Czekaj na odpowiedz z glowicy
		sem_wait(&odpowiedz);
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
	/*	while (1)
	{
		// czekaj na zadanie do osluzenia
		printf("Głowica czeka na zadanie");
		sem_wait(& pojawilo_sie_nowe_zadanie);
		
		// wyslij informację że nowe zadanie bedzie obsłużone
		printf("zadanie ...");
		
		// obsłuż zadanie
		printf("Zadanie cos tam mialo wypisywac ;"));
		sem_post (&zadanie_obsluzone);
		
		// wyslij informacje zwotna o obsluzeniu programu
		// ustal miejsce dalszego polozenia glowicy/ oblicz gdzie ma się udać glowica
		
		//
		
	}
	*/	
}

int main()
{
	sem_init(&odpowiedz, 0, 0);
	pthread_t programWatek;
	pthread_create (&programWatek, NULL, programFun, NULL);
	
	pthread_t glowicaWatek;
	pthread_create (&glowicaWatek, NULL, glowicaFun, NULL);
	
	pthread_join(programWatek, NULL);
	pthread_join(glowicaWatek, NULL);
	
	return 0;
}
