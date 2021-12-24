#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.hpp"
#include <vector>


using namespace std ;
using namespace HM ;

mutex mtx; //candado de escritura en pantalla

const int num_fum = 3;

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

//----------------------------------------------------------------------
// Monitor semántica SU que implementa el Estanco

class Estanco : public HoareMonitor
{
private:
  int suministro;
  CondVar fumador[num_fum] ;
  CondVar estanquero ;

public:                       // constructor y métodos públicos
  Estanco(  ) ;            // constructor
  void  ponerIngrediente( int ingrediente );
  void  obtenerIngrediente( int n_fumador );
  void  esperarRecogida();
} ;


Estanco::Estanco(  )
{
  suministro = -1;

  estanquero = newCondVar();

  for (int i = 0; i<num_fum; ++i)
    fumador[i] = newCondVar();
}

void Estanco::ponerIngrediente( int ingrediente )
{
  suministro = ingrediente;
  fumador[ingrediente].signal();
}

void Estanco::esperarRecogida()
{
  if( suministro != -1 )
    estanquero.wait();
}

void Estanco::obtenerIngrediente( int n_fumador )
{
  if( suministro != n_fumador )
    fumador[n_fumador].wait();
  suministro = -1;
  estanquero.signal();
}


int producir ()
{
  int producto = aleatorio<0,num_fum-1>();
  mtx.lock();
  cout << "Estanquero: pone ingrediente " << producto << endl;
  mtx.unlock();
  return producto;

}


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



void funcion_hebra_estanquero( MRef<Estanco> monitor )
{
  while ( true )
    {
      int ingrediente = producir();
      monitor->ponerIngrediente(ingrediente);
      monitor->esperarRecogida();
    }
}


void  funcion_hebra_fumador(  MRef<Estanco> monitor,  int num_fumador )
{
  while( true )
    {
      monitor->obtenerIngrediente(  num_fumador );
      cout << "Fumador " << num_fumador << " retira el producto." << endl;
      fumar(num_fumador);
    }
}

//----------------------------------------------------------------------

int main()
{
  cout << "--------------------------------------------------------" << endl
       << "Problema de los fumadores." << endl
       << "--------------------------------------------------------" << endl
       << flush ;

  auto monitor = Create<Estanco>( );

  thread hebra_fumador[num_fum];

  for ( int i = 0; i < num_fum; ++i )
    hebra_fumador[i] = thread ( funcion_hebra_fumador, monitor, i );


  thread hebra_estanquero( funcion_hebra_estanquero , monitor );

  for ( int i = 0; i < num_fum; ++i )
    hebra_fumador[i].join();

  hebra_estanquero.join();

  cout << "-------------------------------------------------------" << endl
       << "FIN"
       << "-------------------------------------------------------" << endl;
}
