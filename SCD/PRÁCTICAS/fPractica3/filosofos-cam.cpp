/**
 * @file funciones_MPI.cpp
 * @author Juan Valentín Guerrero Cano
 */
// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos-plantilla.cpp
// Implementación del problema de los filósofos (sin camarero).
// Plantilla para completar.
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------
#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   num_camareros = 1 ,
   id_camarero = 10,
   num_filosofos = 5 ,              // número de filósofos 
   num_filo_ten  = 2*num_filosofos, // número de filósofos y tenedores 
   num_procesos  = num_filo_ten + num_camareros;   // número de procesos total
const int
        tag_libre = 1,
        tag_ocupado = 2,
        tag_sentarse = 3,
        tag_levantarse = 4;
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

void funcion_filosofos( int id )
{
  int id_ten_izq = (id+1)              % num_filo_ten, //id. tenedor izq.
      id_ten_der = (id+num_filo_ten-1) % num_filo_ten; //id. tenedor der.4
  while ( true )
  {
    cout <<"Filósofo " <<id << " solicita SENTARSE " <<endl;
    MPI_Ssend(&id, 1,MPI_INT, id_camarero, tag_sentarse, MPI_COMM_WORLD);

    cout <<"Filósofo " <<id << " solicita ten. izq." <<id_ten_izq <<endl;
    // ... solicitar tenedor izquierdo (completar)
    MPI_Ssend(&id, 1, MPI_INT,id_ten_izq,tag_ocupado, MPI_COMM_WORLD);
    cout <<"Filósofo " <<id <<" solicita ten. der." <<id_ten_der <<endl;
    // ... solicitar tenedor derecho (completar)
    MPI_Ssend(&id, 1, MPI_INT,id_ten_der,tag_ocupado, MPI_COMM_WORLD);
    cout <<"Filósofo " <<id <<" comienza a comer" <<endl ;
    sleep_for( milliseconds( aleatorio<10,100>() ) );

    cout <<"Filósofo " <<id <<" suelta ten. izq. " <<id_ten_izq <<endl;
    // ... soltar el tenedor izquierdo (completar)
    MPI_Ssend(&id, 1, MPI_INT,id_ten_izq,tag_libre, MPI_COMM_WORLD);
    cout<< "Filósofo " <<id <<" suelta ten. der. " <<id_ten_der <<endl;
    // ... soltar el tenedor derecho (completar)
    MPI_Ssend(&id, 1, MPI_INT,id_ten_der,tag_libre, MPI_COMM_WORLD);

    cout <<"Filósofo " <<id << " solicita LEVANTARSE " <<endl;
    MPI_Ssend(&id, 1 , MPI_INT, id_camarero, tag_levantarse, MPI_COMM_WORLD);
    
    cout << "Filosofo " << id << " comienza a pensar" << endl;
    sleep_for( milliseconds( aleatorio<10,100>() ) );
 }
}
// ---------------------------------------------------------------------

void funcion_camarero(){
   int sentados = 0, tag, peticion;
   MPI_Status estado;

   while(true){
      if(sentados == 0){
         tag = tag_sentarse;
      }
      else if (sentados == 3){
         tag = tag_levantarse;
      }
      else{
         tag = MPI_ANY_TAG;
      }

      MPI_Recv(&peticion,1,MPI_INT, MPI_ANY_SOURCE, tag ,MPI_COMM_WORLD, &estado);

      if(estado.MPI_TAG == tag_sentarse){
         sentados++;
         cout << "### CAMARERO sienta a filosofo " << estado.MPI_SOURCE << endl;
      }
      else if(estado.MPI_TAG == tag_levantarse){
         sentados --;
         cout<< "### CAMARERO levanta a filosofo " << estado.MPI_SOURCE << endl;

      }
   }
}

// ---------------------------------------------------------------------

void funcion_tenedores( int id )
{
  int valor, id_filosofo ;  // valor recibido, identificador del filósofo
  MPI_Status estado ;       // metadatos de las dos recepciones

  while ( true )
  {
     
     // ...... recibir petición de cualquier filósofo (completar)
     MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, tag_ocupado, MPI_COMM_WORLD, &estado );
     // ...... guardar en 'id_filosofo' el id. del emisor (completar)
     id_filosofo = estado.MPI_SOURCE;

     cout <<"Ten. " <<id <<" ha sido cogido por filo. " <<id_filosofo <<endl; 

     // ...... recibir liberación de filósofo 'id_filosofo' (completar)
     MPI_Recv( &valor, 1 , MPI_INT, id_filosofo, tag_libre, MPI_COMM_WORLD, &estado);
     cout <<"Ten. "<< id<< " ha sido liberado por filo. " <<id_filosofo <<endl ;
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
      if ( id_propio % 2 == 0 && id_propio != 10)          // si es par
         funcion_filosofos( id_propio ); //   es un filósofo
      else if(id_propio % 2 != 0)                             // si es impar
         funcion_tenedores( id_propio ); //   es un tenedor
      else
         funcion_camarero();
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
