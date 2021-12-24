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
   combustible_A = 0,
   combustible_B = 1,
   combustible_C = 2,
   num_coches = 10,
   num_tipos = 3,
   num_surtidores[num_tipos]={2,2,2} ,
   num_procesos = num_coches+1 ;
const int
        id_gasolinera = num_coches,
        tag_repostar[3]={0,1,2},
        tag_salir = 3;

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
// 
void funcion_coche( int id, int combustible )
{
  while ( true )
  { 
    cout <<"Coche " << id << " solicita entrar a repostar combustible " <<endl;
    MPI_Ssend(&id, 1,MPI_INT, id_gasolinera, tag_repostar[combustible], MPI_COMM_WORLD);

    cout <<"Coche " <<id << " entra a repostar combustible  " << endl;

    sleep_for( milliseconds( aleatorio<30,200>() ) );

    cout <<"Coche " <<id << " solicita salir de la gasolinera " <<endl;
    MPI_Ssend(&combustible, 1 , MPI_INT, id_gasolinera, tag_salir, MPI_COMM_WORLD);
    
    cout <<"Coche " << id << " sale de la gasolinera" << endl;
 }
}
// ---------------------------------------------------------------------

void funcion_gasolinera(){
  int tag, flag, peticion;
  bool repostar = false;
  int surtidores_ocupados[3]={0,0,0};
  MPI_Status estado;

  while(true){
    /*Primero comprobamos si hay algún mensaje que refiera a salir del repostaje*/

    MPI_Iprobe(MPI_ANY_SOURCE,tag_salir,MPI_COMM_WORLD,&flag,&estado);
    if(flag > 0){   //Se ha enviado algún mensaje
      MPI_Recv(&peticion,1,MPI_INT, MPI_ANY_SOURCE, estado.MPI_TAG ,MPI_COMM_WORLD, &estado);
      
      //En el mensaje hemos recibido en la variable "peticion" el tipo de combustible que utiliza el coche que desea salir
      surtidores_ocupados[peticion]--;
      cout << "********** Surtidor de tipo "<< peticion << " liberado  " << endl;
    }

    /*Comprobamos por cada tipo de combustible si hemos recibido una petición de repostar de dicho coche
    y si tenemos surtidores disponibles para este tipo recibimos el mensaje determinado y el coche pasa a 
    repostar
    */
    for(int i = 0; i < num_tipos; i++){
      MPI_Iprobe(MPI_ANY_SOURCE,tag_repostar[i],MPI_COMM_WORLD,&flag,&estado);
      if(flag > 0 && surtidores_ocupados[i] < num_surtidores[i]){
        MPI_Recv(&peticion,1,MPI_INT, MPI_ANY_SOURCE, tag_repostar[i] ,MPI_COMM_WORLD, &estado);
        surtidores_ocupados[i]++;
        cout << "### Surtidor de tipo " << tag_repostar[i] << " ocupado por coche " << estado.MPI_SOURCE << endl;
      }
    }

    sleep_for( milliseconds(20));
    
  }
}


// ---------------------------------------------------------------------


int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual, combustible ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


    if ( num_procesos == num_procesos_actual )
    {
      // ejecutar la función correspondiente a 'id_propio'
      if ( id_propio == id_gasolinera){
          funcion_gasolinera();   
      }
      else{
          combustible = aleatorio<0,num_tipos-1>();
          funcion_coche(id_propio, combustible);
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

// ---------------------------------------------------------------------
