/*
        Suponer un garaje de lavado de coches con dos zonas: una de espera y otra de lavado con 100 plazas.  
    Un coche entra en la zona de lavado del garaje sólo si hay (al menos) una plaza libre,si no, se queda en la cola de espera.
    Si un coche entra en la zona de lavado, busca una plaza libre y espera hasta que es atendido por un empleado del garaje. 
    Los coches no pueden volver a entrar al garaje hasta que el empleado que les atendió les cobre el servicio.
    Suponemos que hay más de 100 empleados que lavan coches, cobran, salen y vuelven a entrar al garaje.
    Cuando un empleado entra en el garaje comprueba si hay coches esperando ser atendidos (ya situados en su plaza de lavado),
    si no, aguarda a que haya alguno en esa situación.  Si hay al menos un coche esperando,
    recorre las plazas de la zona de lavado hasta que lo encuentra, entonces lo lava, le avisa de que puede salir y, por último,
    espera a que le pague. Puede suceder que varios empleados han terminado de lavar sus coches y 
    están todos esperando el pago de sus servicios,no se admite que un empleado cobre a un coche distinto del que ha lavado.  
    También hay que evitar que al entrar 2 ó más empleados a la zona de lavado,  habiendo comprobado que hay coches esperando,
    seleccionen a un mismo coche para lavarlo. 
    Se pide: programar una simulación de la actuación de los coches y 
    de los empleados del garaje utilizando un monitor con señales urgentes y los procedimientos que se dan a continuación. 
    Se valorará más una solución al problema anterior que utilice un número menor designal()s
*/
#include <iostream>
#include <iomanip>
#include <random>
#include "scd.h"

const int max_coches = 100;

class Garaje : public HoareMonitor
{
   private:
        mutex mtx;
        bool zona_lavado_ocupada[max_coches];
       CondVar zona_lavado[max_coches];
       CondVar empleado;
       CondVar zona_espera;
   public:
        Garaje() ; // constructor
        void  lavado_de_coche(int numero_coche);
        int procedure siguiente_coche();
        void termina_corte_de_pelo(int numero_coche);
} ;

Garaje::Garaje(){
   
}

/* lo llaman los procesos que simulan los coches, incluye la
 actuacion completa desde que entra en el garaje hasta que sale; 
 se supone que cuando un coche espera, deja a los otros coches que 
 se puedan mover dentro del garaje
*/
void Barberia::lavado_de_coche(int numero_coche){
    bool huecodisponible = false;
    int num_plaza ;  
    for( int i = 0; i < max_coches && !huecodisponible; i++){
        if(!zona_lavado[i].empty() && !zona_lavado_ocupada[i]){
            huecodisponible = true;
            num_plaza = i;
        }
    }
    if(huecodisponible ){
        mtx.lock()
        zona_lavado_ocupada[num_plaza]=true;
        if(!empleado.empty()){
            empleado.signal();
        }
        else{
            zona_lavado[num_plaza].wait();
        }
        mtx.unlock();
    }else{
        zona_espera.wait();
    }
}
/*
es llamado por los procesos que simulan los empleados;
 devuelve el numero de plaza donde hay un coche esperando ser lavado
*/
int Garaje::siguiente_coche()(){
    bool coche_esperando = false;
    int num_coche ;
    for(int i = 0; i < max_coches && !coche_esperando; i++){
        if(!zona_lavado[i].empty()){
            coche_esperando = true;
            num_coche = i;
        }
    }
    if(coche_esperando){
        return num_coche;
    }else{
        empleado.wait();
        for(int i = 0; i < max_coches && !coche_esperando; i++){
            if(!zona_lavado[i].empty()){
                coche_esperando = true;
                num_coche = i;
            }
        }
        return num_coche;
    }

}
/*es llamado por los empleados para avisar a un coche
 que ha terminado su lavado; terminara cuando se reciba 
 el pago del coche que ocupa la plaza cuyo identificador se paso como argumento

*/
void Garaje::terminar_y_cobrar(int numero_coche){
    cout << "Coche " << numero_coche << " ha sido lavado" << endl;
    
}