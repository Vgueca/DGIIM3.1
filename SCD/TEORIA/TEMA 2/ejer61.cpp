/*
    JUAN VALENTÍN GUERRERO CANO         45338112Y
*/
#include <iostream>
#include <iomanip>
#include <random>
#include "scd.h"

const int max_clientes = 20;
const int max_barberos = 10;

class Barberia : public HoareMonitor
{
   private:
    int contador_Barberos;
    mutex mtx;
    CondVar cliente[max_clientes];            
    CondVar barbero[max_barberos];      
   public:
      Barberia() ; // constructor
      void  corte_de_pelo(int numero_cliente);
      int siguiente_cliente();
      void termina_corte_de_pelo(int numero_cliente);
} ;

Barberia::Barberia(){
    for(int i =0; i < max_clientes; i++){
        cliente[i] = newCondvar();
    }
     for(int i =0; i < max_barberos; i++){
        barbero[i] = newCondvar();
    }
    contador_Barberos = 0;
}

/*lo llaman los procesos que simulan los clientes de la barberia, este
metodo incluye la actuacion completa del cliente desde que entra hasta
que sale; se supone que cuando se espera en alguna silla del tipo que
sea, deja que otros procesos puedan moverse (de uno en uno) dentro de la
barberia
*/
void Barberia::corte_de_pelo(int numero_cliente){
    bool vacio = true;
    int indice_barbero = 1000;

    for(int i = 0; i < max_barberos && vacio; i++){
        if(!barbero[i].empty()){
            vacio = false;
            indice_barbero = i;
        }
    }
    //si no hay ningún barbero disponible el cliente[numero_cliente] se espera a que hayan
    if(vacio){
        mtx.lock();
        cout << "Cliente " << numero_cliente << " espera en silla A" << endl;
        cliente[numero_cliente].wait(); 
        mtx.unlock();
    }
    //En caso de que haya algún barbero disponible el cliente se mueve hacia la silla B) y despierta al barbero concreto
    mtx.lock();
    cout << "Cliente " << numero_cliente << " se sienta en silla B" << endl;
    mtx.unlock(); 
    barbero[indice_barbero].signal(); 
   
}

/*es llamado por los procesos que simulan a los barberos; devuelve el
numero de cliente seleccionado; el criterio de seleccion consiste en
revisar el estado de cada cliente, en orden de menor a mayor indice
hasta encontrar uno que espera ser atendido; el indice de este cliente
se pasara como argumento al metodo termina_corte_de_pelo(i:
numero_de_cliente), llamado por el mismo barbero
*/
int Barberia::siguiente_cliente(){
    

    bool vacio = true;
    int indice_cliente = 1000;
    for(int i = 0; i < max_clientes && vacio; i++){
        if(!clientes[i].empty()){
            vacio = false;
            indice_cliente = i;
        }
    }


    if(vacio){   //Comprueba que no haya clientes esperando
        mtx.lock();
        cout << "Barbero esperando cliente " << endl;
        mtx.unlock();
        barbero[contador_Barberos].wait();     //En dicho caso, el barbero espera a que entre algún cliente 
    }

    mtx.lock();
    contador_Barberos = (contador_Barberos+1)%max_barberos;
    mtx.unlock();

    bool fin = false;
    int indice_cliente;
    for(int i = 0; i < max_clientes && !fin; i++){
        if(!clientes[i].empty()){
            indice = i;
            fin = true;
        }
    }
    return indice_cliente;
    
    }


}
/*es llamado por los barberos para levantar a un cliente de la silla tipo-b
y cobrarle antes de que salga de la barberia; el argumentoi sirve, por
tanto, para que el barbero sepa a que cliente tiene que cobrar
*/
void Barberia::termina_corte_de_pelo(int numero_cliente){
    mtx.lock();
    cout << "Cliente " << numero_cliente << " ha sido pelado" << endl;

    cout << "Cliente " << numero_cliente << " es cobrado" << endl;
    mtx.unlock();
}