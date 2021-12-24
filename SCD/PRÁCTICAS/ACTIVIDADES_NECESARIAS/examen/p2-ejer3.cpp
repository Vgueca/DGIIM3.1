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
const int num_fumadores = 7;

// Monitor SU Estanco
class Estanco : public HoareMonitor {
   private:
    int ingrediente, contador, cont_A, cont_B;
    CondVar mostr_vacio_A, mostr_vacio_B, sem_bin, ingr_disp[num_fumadores];

   public:
    Estanco();
    void ponerIngrediente(int i);
    void esperarRecogidaIngrediente();
    int obtenerIngrediente(int i);
};

void fumar(int i);
int producir_ingrediente(int paridad);
void funcion_hebra_estanquero(MRef<Estanco> monitor, int paridad);
void funcion_hebra_fumador(MRef<Estanco> monitor, int num_fumador);
void fumar(int num_fumador);

//------------------------------------------------------------------------------

int main() {
    cout << "-----------------------------------------------------------------" << endl
         << "Problema de los fumadores con monitor SU." << endl
         << "------------------------------------------------------------------" << endl
         << flush;

    MRef<Estanco> monitor = Create<Estanco>();
    thread estanquero_A, estanquero_B, fumadores[num_fumadores];
    const int PAR = 0, IMPAR = 1;

    estanquero_A = thread(funcion_hebra_estanquero, monitor, PAR);
    estanquero_B = thread(funcion_hebra_estanquero, monitor, IMPAR);
    for (int i = 0; i < num_fumadores; i++) {
        fumadores[i] = thread(funcion_hebra_fumador, monitor, i);
    }

    estanquero_A.join();
    estanquero_B.join();
    for (int i = 0; i < num_fumadores; i++) {
        fumadores[i].join();
    }

    return 0;
}

// -----------------------------------------------------------------------------

Estanco::Estanco() {
    contador = 0;
    cont_A = 0;
    cont_B = 0;
    ingrediente = -1;
    mostr_vacio_A = newCondVar();
    mostr_vacio_B = newCondVar();
    sem_bin = newCondVar();
    assert(num_fumadores > 0);
    for (int i = 0; i < num_fumadores; i++)
        ingr_disp[i] = newCondVar();
}

// -----------------------------------------------------------------------------

void Estanco::ponerIngrediente(int i) {
    ingrediente = i;
    if (i % 2 == 0) {
        cout << "MOSTRADOR A (par): ";
        cont_A++;
    } else if (i % 2 == 1) {
        cout << "MOSTRADOR B (impar): ";
        cont_B++;
        if (cont_B - cont_A >= 5)
            mostr_vacio_B.wait();
        }
    cout << "Puesto ingrediente " << ingrediente << endl;
    ingr_disp[ingrediente].signal();

}

// -----------------------------------------------------------------------------

void Estanco::esperarRecogidaIngrediente() {
    if (ingrediente != -1 && ingrediente % 2 == 0)
        mostr_vacio_A.wait();
    else if (ingrediente != -1 && ingrediente % 2 == 1)
        mostr_vacio_B.wait();
}

// -----------------------------------------------------------------------------

int Estanco::obtenerIngrediente(int i) {
    int contador_local = 0;
    if (ingrediente != i)
        ingr_disp[i].wait();
    cout << "Retirado ingrediente " << ingrediente << endl;
    ingrediente = -1;

    if (i % 2 == 0)
        mostr_vacio_A.signal();
    else if (i % 2 == 1)
        mostr_vacio_B.signal();

    contador_local = ++contador;
    return contador_local;
}

//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------

// función que ejecuta la hebra del estanquero
void funcion_hebra_estanquero(MRef<Estanco> monitor, int paridad) {
    int i;
    while (true) {
        i = producir_ingrediente(paridad);
        monitor->ponerIngrediente(i);
        monitor->esperarRecogidaIngrediente();
    }
}

//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------

// función que ejecuta la hebra del fumador
void funcion_hebra_fumador(MRef<Estanco> monitor, int num_fumador) {
    while (true) {
        int n_cigarro = monitor->obtenerIngrediente(num_fumador);
        cout << "Fumadora " << num_fumador << ": comienzo cigarro " << n_cigarro << endl;
        fumar(num_fumador);
        cout << "Fumadora " << num_fumador << ": termina cigarro " << n_cigarro << endl;
    }
}
