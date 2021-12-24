// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Practica 2. SU LIFO PRODCONS
//
// archivo: prod_cons_n_su.cpp
//
// Autor: Julián Garrido Arana
//
// -----------------------------------------------------------------------------

#include <iostream>
#include <iomanip>
#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>
#include "scd.h"

using namespace std ;
using namespace scd ;

constexpr int
   num_items  = 40 ;     // número de items a producir/consumir

mutex
   mtx ;                 // mutex de escritura en pantalla
unsigned
   cont_prod[num_items], // contadores de verificación: producidos
   cont_cons[num_items]; // contadores de verificación: consumidos

constexpr int 
    num_productores = 4,   // nº de productores
    num_consumidores = 10, // nº de consumidores
    p = num_items / num_productores, // El nº de items que produce cada productor
    c = num_items / num_consumidores; // El nº de items que consume cada consumidor
int    
    items_producidos[num_productores] = {0}, // Array para almacenar cuantos items produce cada productor
    items_consumidos[num_consumidores] = {0} ; // Array para almacenar cuantos items consume cada consumidor

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato(int num_productor)
{
   int contador = items_producidos[num_productor] + num_productor*p ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   //mtx.lock();
   //cout << "producido: " << contador << " (" << num_productor << ")" << endl << flush ;
   //mtx.unlock();
   cont_prod[contador] ++ ;
   items_producidos[num_productor]++;
   return contador;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato, int num_consumidor)
{
   if ( num_items <= dato )
   {
      cout << " dato === " << dato << ", num_items == " << num_items << endl ;
      assert( dato < num_items );
   }
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   //mtx.lock();
   //cout << "                  consumido: " << dato << " (" << num_consumidor << ")" << endl ;
   //mtx.unlock();
   items_consumidos[num_consumidor]++;
}
//----------------------------------------------------------------------

void ini_contadores()
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  cont_prod[i] = 0 ;
      cont_cons[i] = 0 ;
   }
}

//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." << flush ;

   for( unsigned i = 0 ; i < num_items && ok; i++ )
   {
      if ( cont_prod[i] != 1 )
      {
         cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {
         cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

// *****************************************************************************
// clase para monitor buffer, version LIFO, semántica SU, multiples prod. y multiples cons.

class ProdConsNSU : public HoareMonitor
{
 private:
 static const int           // constantes:
   num_celdas_total = 10;   //  núm. de entradas del buffer
 int                        // variables permanentes
   buffer[num_celdas_total],//  buffer de tamaño fijo, con los datos
   primera_libre ;          //  indice de celda de la próxima inserción

 CondVar ocupadas, libres;

 public:                    // constructor y métodos públicos
   ProdConsNSU(  ) ;           // constructor
   int  leer();                // extraer un valor (sentencia L) (consumidor)
   void escribir( int valor ); // insertar un valor (sentencia E) (productor)
} ;
// -----------------------------------------------------------------------------

ProdConsNSU::ProdConsNSU(  )
{
   primera_libre = 0 ;
   libres = newCondVar();
   ocupadas = newCondVar();
}
// -----------------------------------------------------------------------------
// función llamada por el consumidor para extraer un dato

int ProdConsNSU::leer()
{
/*
   // ganar la exclusión mutua del monitor con una guarda
   unique_lock<mutex> guarda( cerrojo_monitor );

   // esperar bloqueado hasta que 0 < num_celdas_ocupadas
   if ( primera_libre == 0 )
      ocupadas.wait( guarda );

   // hacer la operación de lectura, actualizando estado del monitor
   assert( 0 < primera_libre  );
   primera_libre-- ;
   const int valor = buffer[primera_libre] ;


   // señalar al productor que hay un hueco libre, por si está esperando
   libres.notify_one();

   // devolver valor
   return valor ;
*/
    if ( primera_libre == 0 )
        ocupadas.wait();
    assert( 0 < primera_libre  );
    primera_libre-- ;
    const int valor = buffer[primera_libre];
    cout << "                  consumido: " << valor << endl ;
    if(primera_libre == 0)
      libres.signal();
    return valor;
}
// -----------------------------------------------------------------------------

void ProdConsNSU::escribir( int valor )
{
/*
   // ganar la exclusión mutua del monitor con una guarda
   unique_lock<mutex> guarda( cerrojo_monitor );

   // esperar bloqueado hasta que num_celdas_ocupadas < num_celdas_total
   if ( primera_libre == num_celdas_total )
      libres.wait( guarda );

   //cout << "escribir: ocup == " << num_celdas_ocupadas << ", total == " << num_celdas_total << endl ;
   assert( primera_libre < num_celdas_total );

   // hacer la operación de inserción, actualizando estado del monitor
   buffer[primera_libre] = valor ;
   primera_libre++ ;

   // señalar al consumidor que ya hay una celda ocupada (por si esta esperando)
   ocupadas.notify_one();
*/
    if ( primera_libre == num_celdas_total )
      libres.wait();
    assert( primera_libre < num_celdas_total );
    buffer[primera_libre] = valor;
    primera_libre++;
    cout << "producido: " << valor << endl << flush ;
    if(primera_libre==num_celdas_total)
      ocupadas.signal();
}
// *****************************************************************************
// funciones de hebras

void funcion_hebra_productora(MRef <ProdConsNSU> monitor, int num_productor)
{
   for( unsigned i = 0 ; i < p ; i++ )
   {
      int valor = producir_dato(num_productor) ;
      monitor->escribir( valor );
   }
}
// -----------------------------------------------------------------------------

void funcion_hebra_consumidora(MRef <ProdConsNSU> monitor, int num_consumidor)
{
   for( unsigned i = 0 ; i < c ; i++ )
   {
      int valor = monitor->leer();
      consumir_dato( valor, num_consumidor) ;
   }
}
// -----------------------------------------------------------------------------

int main()
{
   cout << "-------------------------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (1 prod/cons, Monitor SC, buffer FIFO). " << endl
        << "-------------------------------------------------------------------------------" << endl
        << flush ;

   //Creamos el Monitor
   MRef<ProdConsNSU> monitor = Create<ProdConsNSU>();

   thread hebras_productoras[num_productores];
   thread hebras_consumidoras[num_consumidores];

   for(int i=0; i<num_productores; i++)
   {
      hebras_productoras[i] = thread(funcion_hebra_productora, monitor, i) ;
   }
   for(int i=0; i<num_consumidores; i++)
   {
      hebras_consumidoras[i] = thread(funcion_hebra_consumidora, monitor, i) ;
   }
   for(int i=0; i<num_productores; i++)
   {
      hebras_productoras[i].join() ;
   }
   for(int i=0; i<num_consumidores; i++)
   {
      hebras_consumidoras[i].join() ;
   }
   

   // comprobar que cada item se ha producido y consumido exactamente una vez
   test_contadores() ;

   cout << endl;

   // comprobar el numero de veces que cada hebra hace su trabajo
   for(int i=0; i<num_productores; i++)
   {
      cout << "Items producidos por el productor nº: " << i << " ---- " << items_producidos[i] << endl ;
   }
   cout << endl;
   for(int i=0; i<num_consumidores; i++)
   {
      cout << "Items consumidos por el consumidor nº: " << i << " ---- " << items_consumidos[i] << endl ;
   }
}