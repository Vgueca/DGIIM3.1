// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: gasolinera-impr.cpp
// Implementación del problema de la gasolinera con tipos de combustible + proceso impresor
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
#include <string>

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
   num_procesos  = num_coches + 2 ,  // número de procesos total 
   id_gasolinera = 15 ,
   id_impresor = 16 ,
   surtidores[num_tipos] = {4, 3, 2} ,
   etiq_entrar[num_tipos] = {0, 1, 2} ,
   //etiq_salir[num_tipos] = {3, 4, 5} ,
   etiq_salir = 3 ,
   etiq_imprimir = 6 ;
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

void imprimir ( const std::string & cadena )
{
    const char *mensaje = cadena.c_str();
    // strlen () se usa para calcular la longitud de la cadena especificada s, excluyendo el carácter final "\ 0".
    MPI_Ssend(mensaje, strlen(mensaje), MPI_CHAR, id_impresor, etiq_imprimir, MPI_COMM_WORLD);
}
// ---------------------------------------------------------------------

void entra_coche ( int id, int combustible )
{   
    int peticion;
    const string cadena = "Coche " + to_string(id) + " solicita repostar combustible tipo " + to_string(combustible) + "";
    imprimir(cadena);
    MPI_Ssend(&peticion, 1, MPI_INT, id_gasolinera, combustible, MPI_COMM_WORLD);

    const string mensaje = "Coche " + to_string(id) + " entr a repostar";
    imprimir(mensaje);
}
// ---------------------------------------------------------------------

void sale_coche ( int id, int combustible ) 
{
    int tipo = combustible;
    const string cadena = "Coche " + to_string(id) + " solicita salir de la gasolinera";
    imprimir(cadena);
    MPI_Ssend(&tipo, 1, MPI_INT, id_gasolinera, combustible + num_tipos, MPI_COMM_WORLD);

    const string mensaje = "Coche " + to_string(id) + " sale";
    imprimir(mensaje);
}
// ---------------------------------------------------------------------

void repostar(int id)
{
    // informa de que comienza a repostar
    const string mensaje_i = "Coche " + to_string(id) + " comienza a repostar..";
    imprimir(mensaje_i);

    // espera bloqueada un tiempo igual a 'milisegundos aleatorios'
    sleep_for( milliseconds( aleatorio<10,100>() ) );

    // informa de que ha terminado de repostar
    const string mensaje_f = "Coche " + to_string(id) + " termina de repostar. ";
    imprimir(mensaje_f);
}
// ---------------------------------------------------------------------

void esperar(int id)
{
    // informa de que comienza a esperar
    const string men_i = "Coche " + to_string(id) + " comienza espera fuera de la gasolinera..";
    imprimir(men_i);

    // espera bloqueada un tiempo aleatorio en milisegundos
    sleep_for( milliseconds( aleatorio<10,100>() ) );

    // informa de que ha terminado de esperar
    const string men_f = "Coche " + to_string(id) + " termina de esperar. ";
    imprimir(men_f);
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
        // Comprueba si hay mensajes de fin de repostar
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
        sigue = true;
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
                // Recibir el primero de los mensajes de comienzo de repostaje del tipo "i" (si hay)
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

void funcion_impresor ()
{
    int long_cadena;
    int id_emisor;
    MPI_Status estado ;      // metadatos de las recepciones

    while (true)
    {
        // esperar mensaje y leer la cuenta de caracteres (sin recibirlo)
        MPI_Probe( MPI_ANY_SOURCE, etiq_imprimir, MPI_COMM_WORLD, &estado );

        // leer longitud del mensaje
        MPI_Get_count( &estado, MPI_CHAR, &long_cadena );

        // identificar al emisor
        id_emisor = estado.MPI_SOURCE;

        // reservar memoria para el mensaje y recibirlo
        char * buffer = new char[long_cadena+1] ;
        //char * buffer = new char[long_cadena] ;

        // recibir el mensaje en el buffer añadiendo el 0 al final del mismo
        MPI_Recv( buffer, long_cadena, MPI_CHAR, id_emisor, etiq_imprimir, MPI_COMM_WORLD, &estado );
        buffer[long_cadena] = 0 ; // añadir un cero al final 
        
        // imprimir el mensaje y liberar la memoria que ocupa
        cout << buffer << endl ;

        // liberar la memoria ocupada por el buffer
        delete [] buffer ;
    } // Alternativa para no usar MPI_Probe: Pasar unn mensaje INT previo con etiq_imprimir con la longitud de chars.
}

int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

   if ( num_procesos == num_procesos_actual )
   {
      // ejecutar la función correspondiente a 'id_propio'
      if (id_propio == id_gasolinera)     // si es el proceso num_procesos-2
         funcion_gasolinera();                // es el proceso gasolinera
      else if (id_propio == id_impresor)  // si es el proceso num_procesos-1
         funcion_impresor();                  // es el proceso impresor
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

// LINKS DE INTERÉS (Ayuda/Apoyo):
    // Longitud y uso de Chars
        // http://www.datsi.fi.upm.es/~dopico/MPI/MPI.pdf -> pág. 22
    // Función strlen ()
        // https://programmerclick.com/article/87161785109/