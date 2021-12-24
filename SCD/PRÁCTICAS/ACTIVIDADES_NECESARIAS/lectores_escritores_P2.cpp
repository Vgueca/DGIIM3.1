
#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.h"

using namespace std;
using namespace HM;

//=====================================================================
// VARIABLES COMPARTIDAS
//=====================================================================

const int num_lectores = 3,   //NUMERO DE LECTORES
          num_escritores = 3; //NUMERO DE ESCRITORES

mutex mtx;

//=====================================================================

/**
 * @brief Genera un entero aleatorio uniformemente distribuido entre dos
 * valores enteros, ambos incluidos
 * @param<T,U> : Números enteros min y max, respectivamente 
 * @return un numero aleatorio generado entre min,max
 */
template<int min, int max> int aleatorio(){
    static default_random_engine generador( (random_device())() );
    static uniform_int_distribution<int> distribucion_uniforme(min, max);
    return distribucion_uniforme(generador);
}

//=====================================================================

//----------------------------------------------------------------------

//=====================================================================
// FUNCIONES SIMULADAS
//=====================================================================

/**
 * @brief Funcion de escribir simulada
 * @param numEscritor : Número de escritor que realiza la acción
 */
void escribir(int numEscritor){
    chrono::milliseconds duracion_escritura(aleatorio<20,200>());
    
    mtx.lock();
    cout << "[Escritor " << numEscritor << "]: Escribiendo datos... ("
            << duracion_escritura.count() << " milisegundos)" << endl;
    mtx.unlock();
    
    this_thread::sleep_for(duracion_escritura);
}

/**
 * @brief Funcion de leer simulada
 * @param numLector : Número de lector que realiza la acción
 */
void leer(int numLector){
    chrono::milliseconds duracion_lectura(aleatorio<20,200>());
    
    mtx.lock();
    cout << "[Lector " << numLector << "]: Leyendo datos... ("
            << duracion_lectura.count() << " milisegundos)" << endl;
    mtx.unlock();
    
    this_thread::sleep_for(duracion_lectura);
}

//----------------------------------------------------------------------

//=====================================================================
// MONITOR SU
//=====================================================================

/**
 * @brief Esta clase representa un monitor con las siguientes características
 * -> Semática SU
 * -> Num Lectores : Múltiples
 * -> Num Escritores : Múltiples
 */
class Lec_Esc : public HoareMonitor{
private:
    //VARIABLES PERMANENTES
    int n_lec;     //Numero de lectores
    bool escrib;   //Comprueba que hay un escritor activo
    
    //VARIABLES DE CONDICION
    CondVar lectura,    //Cola de lectores
            escritura;  //Cola de escritores
    
public:
    Lec_Esc();              //Constructor por defecto
    void ini_lectura();     //Función de inicio de lectura
    void fin_lectura();     //Función de fin de lectura
    
    void ini_escritura();   //Función de inicio de escritura
    void fin_escritura();   //Función de fin de escritura
};

/**
 * @brief Constructor del monitor
 */
Lec_Esc::Lec_Esc(){
    n_lec = 0;
    escrib = false;
    
    lectura = newCondVar();
    escritura = newCondVar();
}

/**
 * @brief Función de inicio de lectura
 */
void Lec_Esc::ini_lectura(){
    //COMPROBACIÓN DE SI HAY ESCRITOR
    if(escrib)
        lectura.wait(); //ESPERAMOS A LECTURA
    
    //REGISTRAR UN LECTOR MÁS
    n_lec++;
    
    //DESBLOQUEO EN CADENA DE POSIBLES LECTORES
    lectura.signal();
}

/**
 * @brief Función de fin de lectura 
 */
void Lec_Esc::fin_lectura(){
    //REGISTRAR UN LECTOR MENOS
    n_lec--;
    
    //SI ES EL ÚLTIMO LECTOR, DESBLOQUEAR
    //UN ESCRITOR
    if(n_lec == 0)
        escritura.signal();
}

/**
 * @brief Función de inicio de escritura
 */
void Lec_Esc::ini_escritura(){
    //
    if((n_lec > 0) or escrib)
        escritura.wait();
    
    escrib = true;
}

/**
 * @brief Funcion de fin de escritura
 */
void Lec_Esc::fin_escritura(){
    //REGISTRAR QUE YA NO HAY ESCRITOR
    escrib = false;
    
    //SI HAY LECTORES, DESPERTAR UNO
    if(!lectura.empty())
        lectura.signal();
    //SI NO HAY LECTORES, DESPERTAR UN ESCRITOR
    else
        escritura.signal();
}

//=====================================================================
// FUNCIONES DE LAS HEBRAS
//=====================================================================

/**
 * @brief Funcion de la hebra lectora
 * @param monitor : Puntero a monitor
 * @param numLector : Numero de lector
 */
void funcion_hebra_lector(MRef<Lec_Esc> monitor, int numLector){
    while(true){
        //RETARDO ALEATORIO
        chrono::milliseconds retardo(aleatorio<20,200>());
        this_thread::sleep_for(retardo);
        
        //PROCEDIMIENTO
        monitor->ini_lectura();
        leer(numLector);
        monitor->fin_lectura();
    }
}

/**
 * @brief Funcion de la hebra escritora
 * @param monitor : Puntero a monitor
 * @param numEscritor : Numero de escritor
 */
void funcion_hebra_escritor(MRef<Lec_Esc> monitor, int numEscritor){
    while(true){
        //RETARDO ALEATORIO
        chrono::milliseconds retardo(aleatorio<20,200>());
        this_thread::sleep_for(retardo);
        
        //PROCEDIMIENTO
        monitor->ini_escritura();
        escribir(numEscritor);
        monitor->fin_escritura();
    }
}


//----------------------------------------------------------------------

//=====================================================================
// FUNCION MAIN
//=====================================================================

int main(){
    //PARTE 0: DECLARACION DE HEBRAS
    assert(0 < num_lectores && 0 < num_escritores);
    thread lectores[num_lectores];
    thread escritores[num_escritores];
    MRef<Lec_Esc> monitor = Create<Lec_Esc>();
            
    string border = "===================================================================";
    cout << border << endl << "  LECTORES / ESCRITORES - MONITOR SU  " << endl << border << endl;
    
    //PARTE 1: LANZAMIENTO DE LAS HEBRAS
    for(int i=0; i<num_lectores; i++){
        lectores[i] = thread(funcion_hebra_lector, monitor, i);
    }
    
    for(int i=0; i<num_escritores; i++){
        escritores[i] = thread(funcion_hebra_escritor, monitor, i);
    }
    
    //PARTE 2: SINCRONIZACION ENTRE LAS HEBRAS
    for(int i=0; i<num_lectores; i++){
        lectores[i].join();
    }
    
    for(int i=0; i<num_escritores; i++){
        escritores[i].join();
    }
    
    return 0;
}

