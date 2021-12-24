/*
@author:    Juan Valentín Guerrero Cano         45338112Y

Responde en tu portafolios a estas cuestiones:  
    A)¿ cual es el mínimo tiempo de espera que queda al final de las
    iteraciones del ciclo secundario con tu solución ?
SOLUCIÓN: En la iteración número 2, 10 milisegundos.

    B)¿ sería planificable si la tarea D tuviese un tiempo cómputo de
    250 ms ?
SOLUCIÓN: Sí sería planificable según el código propuesto, aunque en este caso no habría tiempo de espera
          en el ciclo secundario concretamente en la segunda iteración del ciclo, donde 
          se consumiría la duración total del ciclo secundario lo que podría llevar 
          a ligeros retrasos en las posteriores iteraciones si no se tiene adecuadamente controlado.

*/
// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 4. Implementación de sistemas de tiempo real.
//
// Archivo: ejecutivo2.cpp
// Implementación de lal segunda actividad de ejecutivo cíclico:
//
//   Datos de las tareas:
//   ------------
//   Ta.  T    C
//   ------------
//   A  500   100
//   B  500   150
//   C  1000  200
//   D  2000  240
//  -------------
//
//  Planificación (con Ts == 500 ms)
//  *---------*----------*---------*--------*
//  | A B C   | A B D   | A B C   | A B   |
//  *---------*----------*---------*--------*
//
//
// Historial:
// Creado en Diciembre de 2021
// -----------------------------------------------------------------------------

#include <string>
#include <iostream> // cout, cerr
#include <thread>
#include <chrono>   // utilidades de tiempo
#include <ratio>    // std::ratio_divide

using namespace std ;
using namespace std::chrono ;
using namespace std::this_thread ;

// tipo para duraciones en segundos y milisegundos, en coma flotante:
//typedef duration<float,ratio<1,1>>    seconds_f ;
typedef duration<float,ratio<1,1000>> milliseconds_f ;

// -----------------------------------------------------------------------------
// tarea genérica: duerme durante un intervalo de tiempo (de determinada duración)

void Tarea( const std::string & nombre, milliseconds tcomputo )
{
   cout << "   Comienza tarea " << nombre << " (C == " << tcomputo.count() << " ms.) ... " ;
   sleep_for( tcomputo );
   cout << "fin." << endl ;
}

// -----------------------------------------------------------------------------
// tareas concretas del problema:

void TareaA() { Tarea( "A", milliseconds(100) );  }
void TareaB() { Tarea( "B", milliseconds( 150) );  }
void TareaC() { Tarea( "C", milliseconds( 200) );  }
void TareaD() { Tarea( "D", milliseconds( 240) );  }

// -----------------------------------------------------------------------------
// implementación del ejecutivo cíclico:

int main( int argc, char *argv[] )
{
   // Ts = duración del ciclo secundario (en unidades de milisegundos, enteros)
   const milliseconds Ts_ms( 500 ); //Ahora pasa a ser 500, el menor valor de la columna T

   // ini_sec = instante de inicio de la iteración actual del ciclo secundario
   time_point<steady_clock> ini_sec = steady_clock::now();

   while( true ) // ciclo principal
   {
      cout << endl
           << "---------------------------------------" << endl
           << "Comienza iteración del ciclo principal." << endl ;

      for( int i = 1 ; i <= 4 ; i++ ) // ciclo secundario (4 iteraciones)
      {
         cout << endl << "Comienza iteración " << i << " del ciclo secundario." << endl ;

         switch( i )
         {
            case 1 : TareaA(); TareaB(); TareaC();           break ;    //50 milliseconds
            case 2 : TareaA(); TareaB(); TareaD();           break ;    //10 milliseconds
            case 3 : TareaA(); TareaB(); TareaC();           break ;    //50 milliseconds
            case 4 : TareaA(); TareaB();                     break ;    //250 milliseconds
         }

         // calcular el siguiente instante de inicio del ciclo secundario
         ini_sec += Ts_ms ;

         // esperar hasta el inicio de la siguiente iteración del ciclo secundario
         sleep_until( ini_sec );

         time_point<steady_clock> instante_actual = steady_clock::now();
         float tiempo_ms_f = milliseconds_f( instante_actual - ini_sec ).count();
        cout << "La diferencia entre el instante real en que termina el ciclo secundario y el instante esperado (milisegundos)"<< endl << "[--->" << tiempo_ms_f << "<---]" << endl;
      
      }
   }

}
