#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;


//Variables compartidas necesarias
Semaphore ingredientes[3] = {0,0,0};
Semaphore mostrador_vacio = 1; //El mostrador empieza vacío



//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

// Función que produce un elemento con un cierto retraso
int Producir(){
  // calcular milisegundos aleatorios de duración de producir)
  chrono::milliseconds dur_producir( aleatorio<20,200>() );
  this_thread::sleep_for( dur_producir ); //Esperar
  int resultado = aleatorio<0,2>();
  cout << "Estanquero produce ingrediente " << resultado << endl;
  return resultado;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
  while (true) {
    int ingrediente = Producir();
    sem_wait(mostrador_vacio);
    cout << "Puesto ingrediente " << ingrediente << endl;
    sem_signal(ingredientes[ingrediente]);
  }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

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

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true )
   {
     sem_wait(ingredientes[num_fumador]);
     cout << "          Retirado ingrediente " << num_fumador << endl;
     sem_signal(mostrador_vacio);
     fumar(num_fumador);
   }
}

//----------------------------------------------------------------------

int main()
{
  thread hebra_fumador1 ( funcion_hebra_fumador, 0 ),
    hebra_fumador2 ( funcion_hebra_fumador, 1 ),
    hebra_fumador3( funcion_hebra_fumador, 2 ),
    hebra_estanquero( funcion_hebra_estanquero );

  hebra_fumador1.join() ;
  hebra_fumador2.join() ;
  hebra_fumador3.join() ;
  hebra_estanquero.join() ;

  cout << endl << "fin" << endl;
}
