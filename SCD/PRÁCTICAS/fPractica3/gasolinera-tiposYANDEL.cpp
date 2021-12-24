// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: gasolinera-tipos.cpp
// Implementación del problema de la gasolinera (práctica 2) con 3 tipos de combustible.
//
// Historial:
// Actualizado a C++11 en Diciembre de 2021
//
// Autor:
// Julián Garrido Arana
// -----------------------------------------------------------------------------

#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

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

const int
   num_coches = 15 ,                 // número de coches
   num_tipos = 3 ,                   // número de tipos de combustible
   num_surtidores  = 9 ,             // número de surtidores 
   num_procesos  = num_coches + 1 ,  // número de procesos total 
   id_gasolinera = 15 ,
   surtidores[num_tipos] = {4, 3, 2} ,
   etiq_entrar[num_tipos] = {0, 1, 2} ,
   etiq_salir = 3 ;
   //etiq_salir[num_tipos] = {3, 4, 5} ;
   /*
   // Inicializacion de las etiquetas: 
        //       valores 0 - (num_tipos-1) indican entrar para los respectivos tipos de combustibles
   for(int i=0; i<num_tipos; i++)
        etiq_entrar[i] = i;
        //       valores num_tipos - (2*num_tipos-1) indican salir para los respectivos tipos (en orden) de combustibles
   for(int i=0; i<num_tipos; i++)
        etiq_salir[i] = i + num_tipos;
    */
// *****************************************************************************

void entra_coche ( int id, int combustible )
{   
    int peticion;
    cout <<"Coche " <<id << " solicita repostar combustible tipo " << combustible <<endl;
    MPI_Ssend(&peticion, 1, MPI_INT, id_gasolinera, combustible, MPI_COMM_WORLD);

    cout <<"Coche" <<id << " entra a repostar" <<endl;
}
// ---------------------------------------------------------------------

void sale_coche ( int id, int combustible ) 
{
    //int peticion;
    int tipo = combustible;
    cout <<"Coche " <<id << " solicita salir de la gasolinera" <<endl;
    //MPI_Ssend(&peticion, 1, MPI_INT, id_gasolinera, combustible + num_tipos, MPI_COMM_WORLD);
    MPI_Ssend(&tipo, 1, MPI_INT, id_gasolinera, etiq_salir, MPI_COMM_WORLD);

    cout <<"Coche " <<id << " sale" <<endl;
}
// ---------------------------------------------------------------------

void repostar(int id)
{
    // informa de que comienza a repostar
    cout << "Coche " << id << " comienza a repostar.." << endl;

    // espera bloqueada un tiempo igual a 'milisegundos aleatorios'
    sleep_for( milliseconds( aleatorio<10,100>() ) );

    // informa de que ha terminado de repostar
    cout << "Coche " << id<< " termina de repostar. " << endl;
}
// ---------------------------------------------------------------------

void esperar(int id)
{
    // informa de que comienza a esperar
    cout << "Coche " << id << " comienza espera fuera de la gasolinera.." << endl;

    // espera bloqueada un tiempo aleatorio en milisegundos
    sleep_for( milliseconds( aleatorio<10,100>() ) );

    // informa de que ha terminado de esperar
    cout << "Coche " << id<< " termina de esperar. " << endl;
}
// ---------------------------------------------------------------------

void funcion_coches ( int id, int combustible )
{
    while ( true )
  {
    entra_coche (id, combustible);
    repostar (id);
    sale_coche (id, combustible);
    esperar (id);    
  }
}

void funcion_gasolinera ()
{
    int 
        surtidores_ocupados[num_tipos] = {0} ,
        id_coche_emisor ,
        valor ,
        flag ;

    MPI_Status estado ;      // metadatos de las recepciones
    //bool sigue;

    while ( true ) 
    {
        // Comprueba si hay mensajes de fin de repostar de algun tipo de surtidor
        MPI_Iprobe( MPI_ANY_SOURCE, etiq_salir, MPI_COMM_WORLD, &flag, &estado ) ;
        if( flag > 0 ){     // Indica que se ha encontrado un coche que quiere salir
            // Recibir el primero de los mensajes de salida (si hay)
            MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_salir, MPI_COMM_WORLD, &estado );
            
            // Procesar dicho mensaje
            id_coche_emisor = estado.MPI_SOURCE;    // identificar el emisor
            surtidores_ocupados[valor]--;
            //cout << "Gasolinera permite salida a coche " << id_coche_emisor << endl ;
        }
        /*
        //sigue = true;
        for(int i=0; i<num_tipos && sigue; i++){
            MPI_Iprobe( MPI_ANY_SOURCE, etiq_salir[i], MPI_COMM_WORLD, &flag, &estado ) ;
            if( flag > 0 ){
                // Indicar que se ha encontrado un coche que quiere salir
                sigue = false; 

                // Recibir el primero de los mensajes de salida (si hay)
                MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_salir[i], MPI_COMM_WORLD, &estado );
                
                // Procesar dicho mensaje
                id_coche_emisor = estado.MPI_SOURCE;    // identificar el emisor
                surtidores_ocupados[i]--;
                //cout << "Gasolinera permite salida a coche " << id_coche_emisor << endl ;
            }
        }
        */

        // Comprueba si hay mensajes de comienzo de repostar de los tipos de combustible
        for(int i=0; i<num_tipos; i++){
            MPI_Iprobe( MPI_ANY_SOURCE, etiq_entrar[i], MPI_COMM_WORLD, &flag, &estado ) ;
            if( flag > 0 && surtidores_ocupados[i] < surtidores[i] ){
                // Recibir el primero de los mensajes de comienzo de repostaje del tipo "i" (si hay y se puede)
                MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_entrar[i], MPI_COMM_WORLD, &estado );
                
                // Procesar dicho mensaje
                id_coche_emisor = estado.MPI_SOURCE;    // identificar el emisor
                surtidores_ocupados[i]++;
                //cout << "Gasolinera permite acceso a coche " << id_coche_emisor << endl ;
            }
        }

        // espera bloqueada un tiempo de 20 milisegundos
        sleep_for( milliseconds( 20 ) );
    }
}
// ---------------------------------------------------------------------

int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

   if ( num_procesos == num_procesos_actual )
   {
      // ejecutar la función correspondiente a 'id_propio'
      if (id_propio == id_gasolinera)     // si es el proceso num_procesos-1
         funcion_gasolinera();                // es el proceso gasolinera
      else{                               // otro caso, es un coche
         int combustible = aleatorio <0,num_tipos-1>() ;
         funcion_coches( id_propio, combustible );    
      }
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   MPI_Finalize( );
   return 0;
}