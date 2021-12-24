
#include <iostream>
#include <iomanip>
#include <random>
#include "scd.h"

using namespace std ;
using namespace scd ;

int const 
    num_gasoil = 4,
    num_gasolina = 6;


// *****************************************************************************
// clase para monitor Gasolinera,  semántica SU

class Gasolinera : public HoareMonitor
{
   private:
        int s_gasoil,       // contador de surtidores de gasoil ocupando gasolinera (0-2)
            s_gasolina,          // contador de surtidores de gasoilina (0-3)
            total_surtidores;   // número total de surtidores en uso
        CondVar  gasoil,        // cola de gasoil 
                 gasolina;     // cola de gasolina

   public:
      Gasolinera() ; // constructor
      void entra_coche_gasoil(int num_tipo);
      void sale_coche_gasoil(int num_tipo);
      void entra_coche_gasolina(int num_tipo);
      void sale_coche_gasolina(int num_tipo);

} ;
// -----------------------------------------------------------------------------

Gasolinera::Gasolinera()
{
    s_gasoil = 0;
    s_gasolina = 0;
    total_surtidores = 0;
    gasoil = newCondVar();
    gasolina = newCondVar();
}

void Gasolinera::entra_coche_gasoil(int num_tipo)
{
    if(s_gasoil >= 2)   // Espera si no hay surtidores de gasoil libres
        gasoil.wait();

    s_gasoil++;         // Indica que hay un surtidor de gasoil ocupado
    total_surtidores++;
    cout << "Coche de tipo gasoil nº: " << num_tipo << " entra a la gasolinera." << endl;
    cout << "\tNº de surtidores ocupados: " << total_surtidores << endl;
}

void Gasolinera::sale_coche_gasoil(int num_tipo)
{
    s_gasoil--;
    total_surtidores--;
    cout << "Coche de tipo gasoil nº: " << num_tipo << " sale de la gasolinera." << endl;
    cout << "\tNº de surtidores ocupados: " << total_surtidores << endl;
    gasoil.signal();
}

void Gasolinera::entra_coche_gasolina(int num_tipo)
{
    if(s_gasolina >= 3)   // Espera si no hay surtidores de gasolina libres
        gasoil.wait();

    s_gasolina++;         // Indica que hay un surtidor de gasolina ocupado
    total_surtidores++;
    cout << "Coche de tipo gasolina nº: " << num_tipo << " entra a la gasolinera." << endl;
    cout << "\tNº de surtidores ocupados: " << total_surtidores << endl;
}

void Gasolinera::sale_coche_gasolina(int num_tipo)
{
    s_gasolina--;
    total_surtidores--;
    cout << "Coche de tipo gasolina nº: " << num_tipo << " sale de la gasolinera." << endl;
    cout << "\tNº de surtidores ocupados: " << total_surtidores << endl;
    gasolina.signal();
}

void repostar(int num_tipo)
{
    // calcular milisegundos aleatorios de duración de la acción de repostar
    chrono::milliseconds duracion_repostar(aleatorio<20, 100>());

    // informa de que comienza a repostar
    cout << "Coche " << num_tipo << " comienza a repostar.." << endl;

    // espera bloqueada un tiempo igual a 'duracion_repostar' milisegundos
    this_thread::sleep_for( duracion_repostar );

    // informa de que ha terminado de repostar
    cout << "Coche " << num_tipo<< " termina de repostar. " << endl;
   
}

void esperar(int num_tipo)
{
    // calcular milisegundos aleatorios de duración de la acción de esperar
    chrono::milliseconds duracion_espera(aleatorio<20, 100>());

    // informa de que comienza a esperar
    cout << "Coche " << num_tipo << " comienza espera fuera de la gasolinera.." << endl;

    // espera bloqueada un tiempo igual a 'duracion_esperar' milisegundos
    this_thread::sleep_for( duracion_espera );

    // informa de que ha terminado de esperar
    cout << "Coche " << num_tipo<< " termina de esperar. " << endl;
   
}

void HebraCocheGasoil (MRef<Gasolinera> monitor, int num_tipo){
    while (true)
    {
        monitor->entra_coche_gasoil(num_tipo);
        repostar(num_tipo);
        monitor->sale_coche_gasoil(num_tipo);
        esperar(num_tipo);
    }
}

void HebraCocheGasolina (MRef<Gasolinera> monitor, int num_tipo){
    while (true)
    {
        monitor->entra_coche_gasolina(num_tipo);
        repostar(num_tipo);
        monitor->sale_coche_gasolina(num_tipo);
        esperar(num_tipo);
    }
}


int main()
{
    MRef<Gasolinera> monitor = Create<Gasolinera>();

   thread hebras_coches[num_gasoil + num_gasolina];

   int i = 0;

   for(i; i<num_gasoil; i++)
   {
      hebras_coches[i] = thread(HebraCocheGasoil, monitor, i) ;
   }
   for(i; i<num_gasolina + num_gasoil; i++)
   {
      hebras_coches[i] = thread(HebraCocheGasolina, monitor, i) ;
   }
   for(i=0; i<num_gasoil + num_gasolina; i++)
   {
      hebras_coches[i].join() ;
   }
}