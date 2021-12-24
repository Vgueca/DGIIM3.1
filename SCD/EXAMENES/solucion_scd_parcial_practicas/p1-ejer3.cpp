/*
    JUAN VALENTÍN GUERRERO CANO         45338112Y
*/
#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <vector>
#include "scd.h"


using namespace std ;
using namespace scd ;

// numero de fumadores 
const int num_fumadores = 10 ;
const int num_proveedores = 2;
Semaphore mostrador_vacio = 1;
vector<Semaphore> ingr_disp;
vector<Semaphore> boquillas_disponibles;
Semaphore habitaciones[2]={4,3};
Semaphore notificacion[2]={0,0};

mutex mtx;
/*  Declaración menos práctica dado el elevado número de fumadores
Semaphore ingr_disp[num_fumadores]={0,0,0,0,0,0,0,0,0,0};
Semaphore boquillas_disponibles[2]={0,0};
*/


// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)

int producir_ingrediente()
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10,100>() );

   // informa de que comienza a producir  
     mtx.lock();

   cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;
    mtx.unlock();

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   const int num_ingrediente = aleatorio<0,num_fumadores-1>() ;
    mtx.lock();

   cout << "Estanquero:  produce ingrediente " << num_ingrediente << endl;
       mtx.unlock();

   return num_ingrediente ;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
   int a; 
   while (true) {
      
      a = producir_ingrediente();
      sem_wait(mostrador_vacio);
      cout << "Estanquero:  coloca ingrediente " << a << endl;
      sem_signal(ingr_disp[a]);

   }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar
    mtx.lock();
    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;
    mtx.unlock();
   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar
    mtx.lock();
    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;
    mtx.unlock();
}

void proveer(int num_proveedor){

    // calcular milisegundos aleatorios de duración de la acción de proveer)
    chrono::milliseconds duracion_proveer( aleatorio<30,200>() );

    // informa de que comienza a proveer
    mtx.lock();

    cout << "Proveedor " << num_proveedor << "  :"
          << " empieza a crear la boquilla durante  (" << duracion_proveer.count() << " milisegundos)" << endl;
    mtx.unlock();
    // espera bloqueada un tiempo igual a ''duracion_proveer' milisegundos 
    this_thread::sleep_for( duracion_proveer );
    
    // informa de que ha terminado de proveer
        mtx.lock();
    cout << "Proveedor " << num_proveedor << "  : termina de proveer." << endl;
    mtx.unlock();
}

void funcion_hebra_proveedora ( int num_proveedor){

    while(true){
        proveer(num_proveedor);
        sem_signal(boquillas_disponibles[num_proveedor]);
        sem_wait(notificacion[num_proveedor]);

    }

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
    int num_proveedor;
    if(num_fumador % num_proveedores == 0){
        num_proveedor = 0;
    }
    else{
        num_proveedor = 1;
    }
     
   while( true )
   {
       
      sem_wait(ingr_disp[num_fumador]);
      cout << "El fumador " << num_fumador << " retira su ingrediente" << endl;
      sem_signal(mostrador_vacio);
    
      sem_wait(boquillas_disponibles[num_proveedor]);

      if(num_fumador != 0){
          sem_signal(notificacion[num_proveedor]);
      }
      
      sem_wait(habitaciones[num_proveedor]);

      fumar(num_fumador);

      if(num_fumador == 0){
          sem_signal(notificacion[num_proveedor]);
      }
      
   }
}

//----------------------------------------------------------------------

int main()
{
    assert(num_fumadores%num_proveedores == 0 && num_fumadores >= 10);

    //Inicializamos semáforos
    for(int i = 0; i < num_fumadores; i++){
        ingr_disp.push_back(Semaphore(0));
    }
    for(int i = 0; i < num_proveedores; i++){
        boquillas_disponibles.push_back(Semaphore(0));
    }
   //Creamos las diferentes hebras a partir de la hebra 0 o hebra master
  thread estanquero;
  thread proveedora[num_proveedores];
  thread fumadores[num_fumadores];

   //Les ordenamos a cada hebra la función que deben ejecutar con sus argumentos correspondientes
  estanquero = thread(funcion_hebra_estanquero);
  for(int i=0; i<num_fumadores; i++)
   fumadores[i] = thread(funcion_hebra_fumador, i);

    for(int i=0; i<num_proveedores; i++)
        proveedora[i] = thread(funcion_hebra_proveedora, i);
   //Esperamos hasta que todas las hebras terminen para finalizar 
  estanquero.join();
  for(int i=0; i<num_fumadores; i++)
   fumadores[i].join();

   for(int i=0; i<num_proveedores; i++)
   proveedora[i].join();
}