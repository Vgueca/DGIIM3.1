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
const int num_fumadores = 3;

// Monitor SU Estanco
class Estanco : public HoareMonitor {
   private:
    int ingrediente;
    CondVar mostr_vacio, ingr_disp[num_fumadores];

   public:
    Estanco();
    void ponerIngrediente(int i);
    void esperarRecogidaIngrediente();
    void obtenerIngrediente(int i);
};

void fumar(int i);
int producir_ingrediente();
void funcion_hebra_estanquero(MRef<Estanco> monitor);
void funcion_hebra_fumador(MRef<Estanco> monitor, int num_fumador);
void fumar(int num_fumador);

//------------------------------------------------------------------------------

int main() {
    cout << "-----------------------------------------------------------------" << endl
         << "Problema de los fumadores con monitor SU." << endl
         << "------------------------------------------------------------------" << endl
         << flush;

    MRef<Estanco> monitor = Create<Estanco>();
    thread estanquero, fumadores[num_fumadores];

    estanquero = thread(funcion_hebra_estanquero, monitor);
    for (int i = 0; i < num_fumadores; i++) {
        fumadores[i] = thread(funcion_hebra_fumador, monitor, i);
    }

    estanquero.join();
    for (int i = 0; i < num_fumadores; i++) {
        fumadores[i].join();
    }

    return 0;
}

// -----------------------------------------------------------------------------

Estanco::Estanco() {
    ingrediente = -1;
    mostr_vacio = newCondVar();
    assert(num_fumadores > 0);
    for (int i = 0; i < num_fumadores; i++)
        ingr_disp[i] = newCondVar();
}

// -----------------------------------------------------------------------------

void Estanco::ponerIngrediente(int i) {
    ingrediente = i;
    cout << "Puesto ingrediente " << ingrediente << endl;
    ingr_disp[ingrediente].signal();
}

// -----------------------------------------------------------------------------

void Estanco::esperarRecogidaIngrediente() {
    if (ingrediente != -1)
        mostr_vacio.wait();
}

// -----------------------------------------------------------------------------

void Estanco::obtenerIngrediente(int i) {
    if (ingrediente != i)
        ingr_disp[i].wait();
    cout << "Retirado ingrediente " << ingrediente << endl;
    ingrediente = -1;
    mostr_vacio.signal();
}

//------------------------------------------------------------------------------

// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)
int producir_ingrediente() {
    // calcular milisegundos aleatorios de duración de la acción de fumar)
    chrono::milliseconds duracion_produ(aleatorio<10, 100>());

    // informa de que comienza a producir
    cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;

    // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
    this_thread::sleep_for(duracion_produ);

    const int num_ingrediente = aleatorio<0, num_fumadores - 1>();

    // informa de que ha terminado de producir
    cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;

    return num_ingrediente;
}

//------------------------------------------------------------------------------

// función que ejecuta la hebra del estanquero
void funcion_hebra_estanquero(MRef<Estanco> monitor) {
    int i;
    while (true) {
        i = producir_ingrediente();
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
        monitor->obtenerIngrediente(num_fumador);
        fumar(num_fumador);
    }
}
