#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "scd.h"

using namespace std ;
using namespace scd ;

//----------------------------------------------------------------------
// numero de coches

// const int num_coches = 10 ;
const int num_gasoil = 4 ; // [1-4]
const int num_gasolina = 6 ;  // [5-10]
Semaphore gasoil = 2;
Semaphore gasolina = 3;

//----------------------------------------------------------------------
//variable mutex para garantizar el acceso en exclusion mutua a la modificación del numero de sutidores en uso
mutex mtx;

//----------------------------------------------------------------------
//variable entera para contar el numero de contadores en uso
int surtidores = 0;

//-------------------------------------------------------------------------
// Función que simula la acción de repostar, como un retardo aleatoria de la hebra

void repostar( int num_coche )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a repostar

    cout << "Coche " << num_coche << "  :"
          << " empieza a repostar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de repostar

    cout << "Coche " << num_coche << "  : termina de repostar, sale de la gasolinera." << endl;

}

//-------------------------------------------------------------------------
// Función que simula la acción de esperar, como un retardo aleatoria de la hebra

void esperar( int num_coche )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a repostar

    cout << "Coche " << num_coche << "  :"
          << " empieza a esperar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de repostar

    cout << "Coche " << num_coche << "  : termina de esperar, entra a la gasolinera." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_coche( int num_coche )
{
   while( true )
   {
      if(num_coche<5){
         sem_wait(gasoil);
      }
      else{
         sem_wait(gasolina);
      }

      mtx.lock();
         surtidores++;
      mtx.unlock();

      repostar(num_coche);

      mtx.lock();
         surtidores--;
      mtx.unlock();

      if(num_coche<5){
         sem_signal(gasoil);
      }
      else{
         sem_signal(gasolina);
      }

      esperar(num_coche);
   }
}

//----------------------------------------------------------------------

int main()
{
   //Creamos las diferentes hebras a partir de la hebra 0 o hebra master
  thread coche1, coche2, coche3, coche4;
  thread coche5, coche6, coche7, coche8, coche9, coche10;

   //Les ordenamos a ca da hebra la función que deben ejecutar con sus argumentos correspondientes
  coche1 = thread(funcion_hebra_coche, 1); 
  coche2 = thread(funcion_hebra_coche, 2); 
  coche3 = thread(funcion_hebra_coche, 3); 
  coche4 = thread(funcion_hebra_coche, 4); 
  coche5 = thread(funcion_hebra_coche, 5); 
  coche6 = thread(funcion_hebra_coche, 6); 
  coche7 = thread(funcion_hebra_coche, 7); 
  coche8 = thread(funcion_hebra_coche, 8); 
  coche9 = thread(funcion_hebra_coche, 9); 
  coche10 = thread(funcion_hebra_coche, 10); 

   //Esperamos hasta que todas las hebras terminen para finalizar 
  coche1.join();
  coche2.join();
  coche3.join();
  coche4.join();
  coche5.join();
  coche6.join();
  coche7.join();
  coche8.join();
  coche9.join();
  coche10.join();
}
