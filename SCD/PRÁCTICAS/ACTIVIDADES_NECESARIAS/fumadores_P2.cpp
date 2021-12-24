/**
 * @file fumadoresMonitor.cpp
 * @author Daniel Pérez Ruiz
 * @brief PRÁCTICA P2 - SISTEMAS CONCURRENTES Y DISTRIBUIDOS
 */

#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.h"

using namespace std;
using namespace HM;

//=====================================================================
// VARIABLES COMPARTIDAS
//=====================================================================

const int num_fumadores = 3; //NUMERO DE FUMADORES TOTAL

//=====================================================================

/**
 * @brief Genera un entero aleatorio uniformemente distribuido entre dos
 * valores enteros, ambos incluidos
 * @param<T,U> : Números enteros min y max, respectivamente 
 * @return un numero aleatorio generado entre min,max
 */
template<int min, int max> int aleatorio(){
    static default_random_engine generador( (random_device())() );
    static uniform_int_distribution<int> distribucion_uniforme(min, max);
    return distribucion_uniforme(generador);
}

//=====================================================================
// MONITOR SU
//=====================================================================

/**
 * @brief Esta clase representa un monitor con las siguientes características
 * -> Semática SU
 * -> Num Fumadores : Múltiples
 * -> Num Estanqueros : 1
 */
class Estanco : public HoareMonitor{
private:
    //VARIABLES PERMANENTES
    int mostrador; //Ventanilla sobre la que se obtendra el ingrediente
    
    //COLAS DE CONDICION
    CondVar mostradorVacio,                     //Comprueba que podemos poner ingrediente
            ingredienteObtenido[num_fumadores]; //Comprueba qué ingrediente es el que está en mostrador
    
public:
    Estanco();                              //Constructor
    void obtenerIngrediente(int fumador);   //Proceso de obtener el ingrediente por el fumador
    void ponerIngrediente(int ingr);        //Proceso de colocar el ingrediente por el estanquero
    void esperarRecogidaIngrediente();      //Proceso de espera del estanquero a mostrador Vacio
};

/**
 * @brief Constructor por defecto del monitor
 */
Estanco::Estanco(){
    mostrador = -1;
    mostradorVacio = newCondVar();
    
    for(int i=0; i<num_fumadores; i++)
        ingredienteObtenido[i] = newCondVar();
}

/**
 * @brief El fumador obtiene el ingrediente colocado por el estanquero
 * @param fumador : Numero de fumador
 */
void Estanco::obtenerIngrediente(int fumador){
    //CONDICIÓN DE ESPERA: Hasta que el ingrediente no sea el que
    //necesita el fumador, se queda bloqueado
    if(mostrador != fumador)
        ingredienteObtenido[fumador].wait();
    
    //Comprobamos condición y actualizamos
    assert(0 <= mostrador && mostrador < num_fumadores);
    mostrador = -1; //Una vez se retira el ingrediente
    
    //Notificamos que el mostrador se encuentra vacio
    mostradorVacio.signal();
}

/**
 * @brief El estanquero coloca el ingrediente en el mostrador
 * @param ingr : Numero de ingrediente que coloca el estanquero
 */
void Estanco::ponerIngrediente(int ingr){
    //Comprobamos condición y actualizamos
    assert(0 <= ingr && ingr < num_fumadores);
    mostrador = ingr; //Se coloca ingrediente en el mostrador
    
    //Notificamos al fumador correspondiente que su ingrediente está disponible
    ingredienteObtenido[mostrador].signal();
}

/**
 * @brief El estanquero espera a que el mostrador esté vacío para
 * seguir colocando ingredientes
 */
void Estanco::esperarRecogidaIngrediente(){
    //CONDICIÓN DE ESPERA: Si el mostrador no está vacío se espera
    if(mostrador != -1)
        mostradorVacio.wait();
    
}

//----------------------------------------------------------------------

/**
 * @brief Simula la acción de producir un ingrediente, con un retardo aleatorio de hebra
 * @return Ingrediente producido
 */
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

//=====================================================================
// FUNCIONES DE LAS HEBRAS
//=====================================================================

/**
 * @brief Función que simula la acción de fumar, con un retardo aleatorio de hebra
 * @param num_fumador : Número de fumador que realiza la acción
 */
void fumar(int num_fumador){
    // calcular milisegundos aleatorios de duración de la acción de fumar)
    chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

    // informa de que comienza a fumar

    cout << "              Fumador " << num_fumador << "  :"
         << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

    // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
    this_thread::sleep_for( duracion_fumar );

    // informa de que ha terminado de fumar

    cout << "              Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;
}

/**
 * @brief Función ejecutada por la hebra de estanquero
 */
void funcion_hebra_estanquero(MRef<Estanco> monitor){
    int i;
    
    while(true){
        i = producir_ingrediente();
        monitor->ponerIngrediente(i);
        monitor->esperarRecogidaIngrediente();
    }
}

/**
 * @brief Función ejecutada por la hebra del fumador
 */
void  funcion_hebra_fumador(MRef<Estanco> monitor, int num_fumador){
    assert(0 <= num_fumador && num_fumador < num_fumadores);
    while(true){
        monitor->obtenerIngrediente(num_fumador);
        fumar(num_fumador);
    }
}

//----------------------------------------------------------------------

//=====================================================================
// FUNCION MAIN
//=====================================================================

int main(){
    //PARTE 0: DECLARACION DE HEBRAS
    thread estanquero;                  //HEBRA DEL ESTANQUERO
    thread fumadores[num_fumadores];    //VECTOR DE HEBRAS DE LOS FUMADORES
    MRef<Estanco> monitor = Create<Estanco>(); //MONITOR SU
    
    string border = "===================================================================";
    cout << border << endl << "  PROBLEMA DE LOS FUMADORES - MONITOR SU  " << endl << border << endl;
    
    //PARTE 1: LANZAMIENTO DE LAS HEBRAS
    estanquero = thread(funcion_hebra_estanquero, monitor);
    for(int i=0; i<num_fumadores; i++){
        fumadores[i] = thread(funcion_hebra_fumador,monitor,i);
    }
    
    //PARTE 2: SINCRONIZACION ENTRE LAS HEBRAS
    estanquero.join();
    for(int i=0; i<num_fumadores;i++){
        fumadores[i].join();
    }
    
    return 0;
}
