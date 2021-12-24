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

const int num_clientes = 10;


template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//*********************************************
// Monitor que representa la barbería

class Barberia : public HoareMonitor
{
private:
  bool gentePelandose;
  CondVar barbero;
  CondVar cliente;

public:
  Barberia(  );
  void avisarCliente();
  void avisarBarbero();
  void esperarBarbero();
  void esperarCliente();
} ;


Barberia::Barberia(  )
{
  gentePelandose = false;
  barbero = newCondVar();
  cliente = newCondVar();
}

void Barberia::avisarCliente() //Ejecuta Barbero
{
  cliente.signal();
  gentePelandose = false;
}

void Barberia::avisarBarbero() //Ejecuta cliente
{
  barbero.signal();
}

void Barberia::esperarBarbero //Ejecuta cliente
{
  if ( gentePelandose )
    cliente.wait();
}

void Barberia::esperarCliente() //Ejecuta barbero
{
  if ( cliente.empty() )
    barbero.wait();

  gentePelandose = true;
}
//-------------------------------------------------------------------------

//*************************************************************************
//función que simula la acción de pelar.

void cortarPelo(  )
{
  //Calcula cuando va a tardar en pelar
  chrono::milliseconds duracion_pelado( aleatorio<20,200>() );

  //Informa de las acciones del barbero

  mtx.lock();
  cout << "Barbero afeita cliente." << endl;
  mtx.unlock();

  this_thread::sleep_for( duracion_pelado );
}
//------------------------------------------------------------------------


//Función para poner a los clientes en bucle infinito
void crecerPelo ( int n_client )
{
  //Calcula cuando va a tardar en pelar
  chrono::milliseconds duracion_crecimiento( aleatorio<20,200>() );

  this_thread::sleep_for( duracion_crecimiento );

  mtx.lock();
  cout << "Cliente: " << n_client << " \"Ya me creció el pelo\"" << endl;
  mtx.unlock();
}

//************************************************************************
//Funciones para las hebras


void funcion_hebra_barbero( MRef<Barberia> monitor )
{
  while( true )
    {
      monitor->esperarCliente();
      cortarPelo();
      monitor->avisarCliente();
    }
}


void funcion_hebra_cliente( MRef<Barberia> monitor, int i )
{
  while(true){
    monitor->avisarBarbero();
    monitor->esperarBarbero();
    cout << "Cliente : " << i << " se ha cortado el pelo." << endl;
    crecerPelo(i);
  }
}



int main()
{
  cout << "--------------------------------------------------------" << endl
       << "Problema de la barbería" << endl
       << "--------------------------------------------------------" << endl
       << flush ;

  auto monitor = Create<Barberia>( );

  thread hebra_barbero( funcion_hebra_barbero, monitor );
  thread hebra_cliente[num_clientes];

  for ( int i = 0; i < num_clientes; ++i)
    hebra_cliente[i] = thread ( funcion_hebra_cliente, monitor, i );

  hebra_barbero.join();

  for ( int i = 0; i < num_clientes; ++i)
    hebra_cliente[i].join();

  cout << "-------------------------------------------------------" << endl
       << "FIN"
       << "-------------------------------------------------------" << endl;
}
