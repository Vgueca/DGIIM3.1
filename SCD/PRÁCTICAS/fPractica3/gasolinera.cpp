/**
 * @file funciones_MPI.cpp
 * @author Juan Valentín Guerrero Cano
 */

#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   num_surtidores = 5 ,
   num_coches = 10,
   num_procesos = num_coches+1 ;
const int
        id_gasolinera = num_coches,
        tag_entrar = 1,
        tag_salir = 2;

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

// ---------------------------------------------------------------------

void funcion_coche( int id )
{
  while ( true )
  {
    cout <<"Coche " << id << " solicita entrar a repostar " <<endl;
    MPI_Ssend(&id, 1,MPI_INT, id_gasolinera, tag_entrar, MPI_COMM_WORLD);

    cout <<"Coche " <<id << " entra a repostar " << endl;

    sleep_for( milliseconds( aleatorio<10,100>() ) );


    cout <<"Coche " <<id << " solicita salir de la gasolinera " <<endl;
    MPI_Ssend(&id, 1 , MPI_INT, id_gasolinera, tag_salir, MPI_COMM_WORLD);
    
    cout <<"Coche " << id << " sale de la gasolinera" << endl;
 }
}
// ---------------------------------------------------------------------

void funcion_gasolinera(){
   int surtidores_ocupados = 0, tag, peticion;
   MPI_Status estado;

   while(true){
        if(surtidores_ocupados == 0){
            tag = tag_entrar;
        }
        else if (surtidores_ocupados == num_surtidores){
            tag = tag_salir;
        }
        else{
             tag = MPI_ANY_TAG;
        }

      MPI_Recv(&peticion,1,MPI_INT, MPI_ANY_SOURCE, tag ,MPI_COMM_WORLD, &estado);

      if(estado.MPI_TAG == tag_entrar){
         surtidores_ocupados++;
         cout << "### Surtidor " << surtidores_ocupados << " ocupado por coche " << estado.MPI_SOURCE << endl;
      }
      else if(estado.MPI_TAG == tag_salir){
         surtidores_ocupados --;
         cout << "### Surtidor " << surtidores_ocupados << " libre " << endl;

      }
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
      if ( id_propio != id_gasolinera)          
         funcion_coche( id_propio ); 
      else
         funcion_gasolinera();
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

// ---------------------------------------------------------------------
