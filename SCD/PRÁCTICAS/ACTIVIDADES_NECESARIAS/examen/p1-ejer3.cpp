// Pablo Olivares Martínez 24411228V
#include <cassert>
#include <chrono>  // duraciones (duration), unidades de tiempo
#include <iostream>
#include <mutex>
#include <random>  // dispositivos, generadores y distribuciones aleatorias
#include <thread>
#include <vector>

#include "scd.h"

using namespace std;
using namespace scd;

// numero de fumadores

const int num_fumadores = 6;
int contador = 0;
Semaphore mostr_vacio_A = Semaphore(1), mostr_vacio_B = Semaphore(1),
          sem_bin = Semaphore(1);
vector<Semaphore> ingr_disp;

void iniciarSemaforos() {
    assert(num_fumadores > 0);
    for (int i = 0; i < num_fumadores; i++)
        ingr_disp.push_back(Semaphore(0));
}

//-------------------------------------------------------------------------
// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)

int producir_ingrediente(int paridad) {
    // calcular milisegundos aleatorios de duración de la acción de fumar)
    chrono::milliseconds duracion_produ(aleatorio<10, 100>());

    // informa de que comienza a producir
    cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;

    // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
    this_thread::sleep_for(duracion_produ);

    int num_ingrediente = aleatorio<0, num_fumadores - 1>();
    // establecemos el valor par o impar
    num_ingrediente = (num_ingrediente / 2) * 2 + paridad;
    // informa de que ha terminado de producir
    cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;

    return num_ingrediente;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(int paridad) {
    int i;
    while (true) {
        i = producir_ingrediente(paridad);
        if (i % 2 == 0) {
            sem_wait(mostr_vacio_A);
            cout << "MOSTRADOR A (par): ";
        }
        else if (i % 2 == 1) {
            sem_wait(mostr_vacio_B);
            cout << "MOSTRADOR B (impar): ";
        }
        cout << "Puesto ingrediente " << i << endl;
        sem_signal(ingr_disp[i]);
    }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar(int num_fumador) {
    // calcular milisegundos aleatorios de duración de la acción de fumar)
    chrono::milliseconds duracion_fumar(aleatorio<20, 200>());

    // informa de que comienza a fumar
    cout << "Fumador " << num_fumador << "  :"
         << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

    // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
    this_thread::sleep_for(duracion_fumar);

    // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void funcion_hebra_fumador(int num_fumador) {
    int contador_local = 0;
    while (true) {
        sem_wait(ingr_disp[num_fumador]);
        cout << "Retirado ingrediente " << num_fumador << endl;
        if (num_fumador % 2 == 0)
            sem_signal(mostr_vacio_A);
        else if (num_fumador % 2 == 1)
            sem_signal(mostr_vacio_B);
        sem_wait(sem_bin);
        contador_local = ++contador;
        sem_signal(sem_bin);
        cout << "Fumadora " << num_fumador << ": comienzo cigarro " << contador_local << endl;
        fumar(num_fumador);
        cout << "Fumadora " << num_fumador << ": termina cigarro " << contador_local << endl;
    }
}

//----------------------------------------------------------------------

int main() {
    cout << "-----------------------------------------------------------------" << endl
         << "Problema de los fumadores." << endl
         << "------------------------------------------------------------------" << endl
         << flush;

    iniciarSemaforos();
    thread estanquero_A, estanquero_B, fumadores[num_fumadores];
    const int PAR = 0, IMPAR = 1;

    estanquero_A = thread(funcion_hebra_estanquero, PAR);
    estanquero_B = thread(funcion_hebra_estanquero, IMPAR);
    for (int i = 0; i < num_fumadores; i++) {
        fumadores[i] = thread(funcion_hebra_fumador, i);
    }

    estanquero_A.join();
    estanquero_B.join();
    for (int i = 0; i < num_fumadores; i++) {
        fumadores[i].join();
    }

    return 0;
}
