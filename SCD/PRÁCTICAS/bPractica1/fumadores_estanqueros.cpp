#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> 
#include <chrono> 
#include "scd.h"

using namespace std;
using namespace scd;

//Variables compartiadas
const int num_fumadores = 3, num_estanqueros = 3;

const int numRaciones = 4;

Semaphore ingr_disp[num_fumadores]={0,0,0};
Semaphore mostr_vacio[num_estanqueros]={4,4,4};

Semaphore exclusionMutua(1);

//=====================================================================

template<int min, int max> int aleatorio(){
    static default_random_engine generador( (random_device())() );
    static uniform_int_distribution<int> distribucion_uniforme(min, max);
    return distribucion_uniforme(generador);
}

int producir_ingrediente(){
    // calcular milisegundos aleatorios de duración de la acción de fumar)
    chrono::milliseconds duracion_produ( aleatorio<10,100>() );

    // informa de que comienza a producir
    cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;

    // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
    this_thread::sleep_for( duracion_produ );

    const int num_ingrediente = aleatorio<0,num_fumadores-1>();

    // informa de que ha terminado de producir
    cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;

    return num_ingrediente;
}

/**
 * @brief Función ejecutada por la hebra de estanquero
 */
void funcion_hebra_estanquero(int numEstanquero){
    int i;
    
    while(true){
        i = producir_ingrediente();
        
        //>>> INICIO SECCION CRITICA <<<
        mostr_vacio[i].sem_wait();
        
        exclusionMutua.sem_wait();
        cout << "[ESTANQUERO " << numEstanquero << "] Puesto ingr.: " << i << endl;
        exclusionMutua.sem_signal();
        
        ingr_disp[i].sem_signal();
        //>>> FIN SECCION CRITICA <<<
    }
}

/**
 * @brief Función que simula la acción de fumar, con un retardo aleatorio de hebra
 * @param num_fumador : Número de fumador que realiza la acción
 */
void fumar(int num_fumador){
    // calcular milisegundos aleatorios de duración de la acción de fumar)
    chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

    // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
         << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

    // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
    this_thread::sleep_for( duracion_fumar );

    // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;
}

/**
 * @brief Función ejecutada por la hebra del fumador
 */
void  funcion_hebra_fumador(int num_fumador){
    assert(0 <= num_fumador && num_fumador < num_fumadores);
    while(true){
        //>>> INICIO SECCION CRITICA <<<
        ingr_disp[num_fumador].sem_wait();
        cout << "    Retirado ingr.: " << num_fumador << endl;
        mostr_vacio[num_fumador].sem_signal();
        //>>> FIN SECCION CRITICA <<<
        
        fumar(num_fumador);
    }
}

//----------------------------------------------------------------------

int main(){
    //COMPROBAMOS CONDICION
    assert(num_fumadores > 0 && num_estanqueros > 0);
    
    //INICIALIZAR SEMAFOROS
    iniciarSemaforos();    
    
    //PARTE 0: DECLARACION DE HEBRAS
    thread estanqueros[num_estanqueros];  //HEBRA DEL ESTANQUERO
    thread fumadores[num_fumadores];    //VECTOR DE HEBRAS DE LOS FUMADORES
    
    string border = "===================================================================";
    cout << border << endl << "  PROBLEMA DE LOS FUMADORES " << endl << border << endl;
    
    //PARTE 1: LANZAMIENTO DE LAS HEBRAS
    for(int i=0; i<num_estanqueros; i++){
        estanqueros[i] = thread(funcion_hebra_estanquero,i);
    }
    
    for(int i=0; i<num_fumadores; i++){
        fumadores[i] = thread(funcion_hebra_fumador,i);
    }
    
    //PARTE 2: SINCRONIZACION ENTRE LAS HEBRAS
    for(int i=0; i<num_estanqueros; i++){
        estanqueros[i].join();
    }
    
    
    for(int i=0; i<num_fumadores;i++){
        fumadores[i].join();
    }
    
    return 0;
}
