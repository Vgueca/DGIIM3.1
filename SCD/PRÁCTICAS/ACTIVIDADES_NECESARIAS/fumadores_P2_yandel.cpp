
#include <iostream>
#include <iomanip>
#include <random>
#include "scd.h"

using namespace std ;
using namespace scd ;

// numero de fumadores 
const int num_fumadores = 3 ;

// *****************************************************************************
// clase para monitor Estanco,  semántica SU

class Estanco : public HoareMonitor
{
   private:
      int      ingrediente;             // contador de ingredientes en mostrador (-1 -> vacío, 1-3 ->ingredientes)
      CondVar  mostradorVacio, ingredienteEsperado[num_fumadores];                   // cola de hebras esperando en cita

   public:
      Estanco() ; // constructor
      void obtenerIngrediente(int i);
      void ponerIngrediente(int i);
      void esperarRecogidaIngrediente();
   
      //void Estanquero( int num_hebra );               // método del Estanquero
      //void Fumador(int num_hebra);              // método del Fumadores
} ;
// -----------------------------------------------------------------------------

Estanco::Estanco()
{
   ingrediente = -1;
   mostradorVacio = newCondVar();
   for(int i=0; i<num_fumadores; i++)
      ingredienteEsperado[i] = newCondVar();
}
// -----------------------------------------------------------------------------

void Estanco::obtenerIngrediente ( int num_ingrediente )
{
   if(ingrediente != num_ingrediente){
      ingredienteEsperado[num_ingrediente].wait(); // Espera hasta que le mandan un
                                                   // signal y luego sigue ejec. codigo
   }

   ingrediente = -1;
   mostradorVacio.signal();      
   

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
// *****************************************************************************

void funcion_hebra_Fumador( MRef<Estanco> monitor, int num_fumador )
{
   while( true )
   {
      monitor->obtenerIngrediente(num_fumador);
      fumar(num_fumador);
   }

}
// *****************************************************************************

void Estanco::ponerIngrediente ( int num_ingrediente )
{
   if(ingrediente != -1){
      mostradorVacio.wait();
   }
   
   ingrediente = num_ingrediente;
   ingredienteEsperado[num_ingrediente].signal();      
   

}
// *****************************************************************************

void Estanco::esperarRecogidaIngrediente ()
{
   if (ingrediente != -1)
      mostradorVacio.wait();

}

// *****************************************************************************

int ProducirIngrediente(){
   //Generar y devolver un numero aleatorio del 1-3
   
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10,100>() );

   // informa de que comienza a producir
   cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   const int num_ingrediente = aleatorio<0,num_fumadores-1>() ;

   // informa de que ha terminado de producir
   cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;

   return num_ingrediente ;

}
// *****************************************************************************

void funcion_hebra_Estanquero( MRef<Estanco> monitor )
{
   int ingre;
   while( true )
   {
      ingre = ProducirIngrediente();
      monitor -> ponerIngrediente(ingre);
      monitor -> esperarRecogidaIngrediente();
   }

}

// *****************************************************************************

int main()
{
   //Creamos las diferentes hebras a partir de la hebra 0 o hebra master
  thread estanquero;
  thread fumadores[num_fumadores];
  
  //Creamos el Monitor
  MRef<Estanco> monitor = Create<Estanco>();

   //Le ordenamos a cada hebra la función que debe ejecutar con sus argumentos correspondientes
  estanquero = thread(funcion_hebra_Estanquero, monitor);
  for(int i=0; i<num_fumadores; i++){
     fumadores[i] = thread(funcion_hebra_Fumador, monitor, i);
  }

   //Esperamos hasta que todas las hebras terminen para finalizar 
  estanquero.join();
  for(int i=0; i<num_fumadores; i++){
     fumadores[i].join();
  }
}
