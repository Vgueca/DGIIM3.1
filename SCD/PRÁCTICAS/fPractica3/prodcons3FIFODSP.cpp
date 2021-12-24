// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: prodcons2-mu.cpp
// Implementación del problema del productor-consumidor con
// un proceso intermedio que gestiona un buffer finito y recibe peticiones
// en orden arbitrario
// (versión con un multiples productores y multipoles consumidores)
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------

#include <iostream>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <cassert>
#include <cstdlib>
#include <mpi.h>

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

//=====================================================================
// VARIABLES CONSTANTES
//=====================================================================

//DEFINICION DEL NUMERO DE PRODUCTORES Y NUMERO DE CONSUMIDORES
static const int numProductores = 4,
                 numConsumidores = 5;

//DEFINICION DEL NUMERO DE PROCESOS QUE SE ESPERAN LANZAR
const int num_procesos_esperado = numProductores + numConsumidores + 1;

//ASIGNACION DE ROLES : BUFFER DE DATOS
const int id_buffer = numProductores;

//DEFINICION DE NUMERO DE ITEMS Y DIMENSION BUFFER
const int multiplo = 1;
const int num_items = multiplo * numProductores * numConsumidores, //[DEBE SER MULTIPLO DE NUMP Y NUMC]
          tam_vector = 10,
          //TOTAL DE ITEMS QUE PRODUCE UN PRODUCTOR
          itemsProductor = num_items / numProductores,
          //TOTAL DE ITEMS QUE CONSUME UN CONSUMIDOR
          itemsConsumidor = num_items / numConsumidores;

//DEFINICION DE ETIQUETAS PARA EL PASO DE MENSAJES
const int etiq_productor = 1,
          etiq_consumidor = 2;

//=====================================================================
// FUNCIONES COMUNES
//=====================================================================

/**
 * @brief Comprueba que los parametros correctos
 * @return True si todo se cumple | False en otro caso
 */
int checkParameters(){
    //1: COMPROBAR NUM_ITEMS Y TAM_VECTOR POSITIVOS
    if((num_items < 1) || (tam_vector < 1)){
        return 1;
    }

    //2: COMPROBAR QUE NUM_ITEMS SEA MULTIPLO DE NUMP Y NUMC
    if((num_items % numProductores != 0) || (num_items % numConsumidores != 0)){
        return 2;
    }

    //3: COMPROBAR QUE ROL BUFFER SEA NUMP
    if(id_buffer != numProductores){
        return 3;
    }

    //4: COMPROBAR QUE NUM_PROCESOS_ESPERADO SEA IGUAL A LA SUMA DE PROD Y CONS
    if(num_procesos_esperado != (numProductores + numConsumidores +1)){
        return 4;
    }

    //5: COMPROBR QUE LAS ETIQUETAS DEL PASO A MENSAJES SEAN NO NEGATIVAS
    if((etiq_productor < 0) || (etiq_consumidor < 0)){
        return 5;
    }
    
    return 0;
}

/**
 * @brief Muestra al usuario el tipo de error que se ha producido tras el lanzamiento
 * del programa (error en variables constantes)
 */
void notifyError(int errorCode){
    switch(errorCode){
        case 1:
            cout << "ERROR: NUM_ITEMS o TAM_VECTOR ES NULO "<< endl;
            break;
        
        case 2:
            cout << "ERROR: NUM_ITEMS NO ES MULTIPLO DE NUM_PROD o NUM_CONS" << endl;
            break;
        
        case 3:
            cout << "ERROR: EL IDENTIFICADO DEL BUFFER [" << id_buffer << "] DEBERIA SER [" << numProductores << "]" << endl;
            break;
            
        case 4:
            cout << "ERROR: EL NUMERO DE PROCESOS QUE SE ESPERA [" << num_procesos_esperado << "] NO ES CORRECTO." << endl;
            break;
            
        case 5:
            cout << "ERROR: LA ETIQUETA DE PRODUCTOR O DE CONSUMIDOR ES NEGATIVA" << endl;
            break;
            
    }
}

/**
 * @brief Genera un entero aleatorio uniformemente distribuido entre dos
 * valores enteros, ambos incluidos
 * @param<T,U> : Números enteros min y max, respectivamente 
 * @return un numero aleatorio generado entre min,max
 */
template< int min, int max > int aleatorio(){
    static default_random_engine generador((random_device())());
    static uniform_int_distribution<int> distribucion_uniforme(min, max);
    return distribucion_uniforme(generador);
}

/**
 * @brief Produce un valor (Ademas de incluir retardo de ejecucion)
 * @return valor producido
 */
int producir(int numProductor){
    static int valor = numProductor * itemsProductor - 1;
    
    //REALIZACION DE ESPERA BLOQUEADA
    sleep_for(milliseconds(aleatorio<10,100>()));
    valor++;
    
    //INFORMA DE PRODUCCION DE VALOR
    cout << ">>> [P-" << numProductor << "] Productor ha producido valor " << valor << endl << flush;
    
    return valor;
}

// ---------------------------------------------------------------------

/**
 * @brief Consume el dato pasado como parametro
 * @param valor_cons : Valor a consumir
 */
void consumir(int numConsumidor, int valor_cons){
    //REALIZACION DE ESPERA BLOQUEADA
    sleep_for(milliseconds(aleatorio<110,200>()));
   
    //MOSTRAR CONSUMICION
    cout << "<<< [C-" << numConsumidor << "] Consumidor ha consumido valor " << valor_cons << endl << flush;
}

//=====================================================================
// FUNCIONES DE PROCESOS
//=====================================================================

/**
 * @brief Funcion que ejecuta el proceso productor. Envia datos
 * producidos por él al buffer
 */
void funcion_productor(int numProductor){
    unsigned int i;
    for(i=0; i<itemsProductor;i++){
        //PRODUCCION DEL VALOR
        int valor_prod = producir(numProductor);
        
        //ENVIO DEL VALOR AL BUFFER
        cout << ">>> [P-" << numProductor << "] Productor va a enviar valor " << valor_prod << endl << flush;
      
        //MPI SSEND --> ENVIO SINCRONO
        //-> void*        : valor_prod     = UBICACION DE BYTES EN MEMORIA [ENVIO]
        //-> int          : 1              = NUM DE ELEMENTOS ALMACENADOS
        //-> MPI_Datatype : MPI_INT        = TIPO DE DATO ENVIADO
        //-> int          : id_buffer      = IDENTIFICADOR DE PROCESO DESTINO [BUFFER]
        //-> int [tag]    : etiq_productor = ETIQUETA DEL MENSAJE
        //-> MPI_Comm     : MPI_COMM_WORLD = COMUNICADOR EN EL QUE SE REALIZA EL ENVIO
        MPI_Ssend(&valor_prod, 1, MPI_INT, id_buffer, etiq_productor, MPI_COMM_WORLD);
    }
}

// ---------------------------------------------------------------------

/**
 * @brief Funcion del proceso consmidor. Recibe los datos
 * enviados desde el proceso buffer y los consume
 */
void funcion_consumidor(int numConsumidor){
    int peticion,       //BUFFER DE ENVIO DE DATOS [INT EXPECTED]
        valor_rec = 1;  //BUFFER DE RECEPCION DE DATOS [INT EXPECTED]
   
    MPI_Status estado;  //METADATO PARA OBTENER REMITENTE DEL MENSAJE

    unsigned int i;
    for(i=0; i<itemsConsumidor; i++){
        //MPI SSEND --> ENVIO SINCRONO
        //-> void*        : peticion        = UBICACION DE BYTES EN MEMORIA [ENVIO]
        //-> int          : 1               = NUM DE ELEMENTOS ALMACENADOS
        //-> MPI_Datatype : MPI_INT         = TIPO DE DATO ENVIADO
        //-> int          : id_buffer       = IDENTIFICADOR DE PROCESO DESTINO [BUFFER]
        //-> int [tag]    : etiq_consumidor = ETIQUETA DEL MENSAJE
        //-> MPI_Comm     : MPI_COMM_WORLD  = COMUNICADOR EN EL QUE SE REALIZA EL ENVIO
        MPI_Ssend(&peticion, 1, MPI_INT, id_buffer, etiq_consumidor, MPI_COMM_WORLD);
        
        //MPI RECV --> RECIBIMIENTO SINCRONO
        //-> void*        : valor_rec      = UBICACION DE MEMORIA DONDE SE RECIBEN LOS DATOS
        //-> int          : 1              = NUM DE ELEMENTOS RECIBIDOS
        //-> MPI_Datatype : MPI_INT        = TIPO DE DATO ENVIADO
        //-> int          : id_buffer      = IDENTIFICADOR DE PROCESO RECEPTOR
        //-> int [tag]    : etiq_productor = ETIQUETA DEL MENSAJE
        //-> MPI_Comm     : MPI_COMM_WORLD = COMUNICADOR EN EL QUE SE REALIZA LA RECEPCION
        //-> MPI_Status   : estado         = METADATO PARA OBTENER INFORMACION DEL EMISOR
        MPI_Recv (&valor_rec, 1, MPI_INT, id_buffer, etiq_consumidor, MPI_COMM_WORLD, &estado);
        
        //INFORME DE QUE SE HA RECIBIDO EL VALOR A CONSUMIR
        cout << "<<< [C-" << numConsumidor << "] Consumidor ha recibido valor " << valor_rec << endl << flush;
        
        //CONSUMIR EL DATO RECIBIDO
        consumir(numConsumidor, valor_rec);
    }
}

// ---------------------------------------------------------------------

/**
 * @brief Funcion del proceso buffer. Recibe los datos producidos
 * por el proceso productor y envia los datos que almacena en el vector
 * al consumidor
 */
void funcion_buffer(){
    int buffer[tam_vector],      // buffer con celdas ocupadas y vacías
        valor,                   // valor recibido o enviado
        primera_libre       = 0, // índice de primera celda libre
        primera_ocupada     = 0, // índice de primera celda ocupada
        num_celdas_ocupadas = 0, // número de celdas ocupadas
        id_emisor_aceptable;     // identificador de emisor aceptable
   
    MPI_Status estado;           // metadatos del mensaje recibido
    int tag = MPI_ANY_TAG;                

    //BUCLE DE GESTION DE PETICIONES EN EL BUFFER
    //-> TOTAL DE ITERACIONES: 2* NUM_ITEMS [ATIENDE A PETICIONES CONSUMIDOR + PRODUCTOR]
    for(unsigned int i=0; i<num_items*2; i++){
        //PASO 1 : DETERMINAR SI PUEDE ENVIAR [PROD], [CONS], [ANY_SOURCE]
        if (num_celdas_ocupadas == 0){              // si buffer vacío
            tag = etiq_productor;                   // $~~~$ solo prod.
        }
        
        else if (num_celdas_ocupadas == tam_vector){ // si buffer lleno
            tag = etiq_consumidor;                   // $~~~$ solo cons.
        }
        
        else{                                       // si no vacío ni lleno
            //PASO INTERMEDIO: BLOQUEAR HASTA QUE HAYA ALGUN MENSAJE ESPERANDO
            tag = MPI_ANY_TAG;
            
            //MPI PROBE --> ESPERA MENSAJE
            //-> int          : MPI_ANY_SOURCE           = IDENTIFICADOR DE EMISOR
            //-> int [tag]    : tag                      = ETIQUETA DEL MENSAJE
            //-> MPI_Comm     : MPI_COMM_WORLD           = COMUNICADOR EN EL QUE SE REALIZA LA RECEPCION
            //-> MPI_Status   : estado                   = METADATO PARA OBTENER INFORMACION DEL EMISOR
            MPI_Probe(MPI_ANY_SOURCE,tag,MPI_COMM_WORLD,&estado);
            
            //OBTENER PROCEDENCIA DE LA PETICION DEL MENSAJE
            tag = estado.MPI_TAG;
        }
      
        //PASO 2 : RECIBIR EL MENSAJE DEL EMISOR O EMISORES ACEPTABLES
        
        //MPI RECV --> RECIBIMIENTO SINCRONO
        //-> void*        : valor                    = UBICACION DE MEMORIA DONDE SE RECIBEN LOS DATOS
        //-> int          : 1                        = NUM DE ELEMENTOS RECIBIDOS
        //-> MPI_Datatype : MPI_INT                  = TIPO DE DATO ENVIADO
        //-> int          : MPI_ANY_SOURCE           = IDENTIFICADOR DE PROCESO RECEPTOR
        //-> int [tag]    : tag                      = ETIQUETA DEL MENSAJE
        //-> MPI_Comm     : MPI_COMM_WORLD           = COMUNICADOR EN EL QUE SE REALIZA LA RECEPCION
        //-> MPI_Status   : estado                   = METADATO PARA OBTENER INFORMACION DEL EMISOR
        MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &estado);
      
        //PASO 3 : PROCESAR EL MENSAJE RECIBIDO
      
        //SI EL RECIBO DE LA PETICION FUE DESDE EL PRODUCTOR
        if(estado.MPI_TAG == etiq_productor){
            //INSERCION EN VECTOR
            buffer[primera_libre] = valor;
            //MODIFICACION DE APUNTADOR
            primera_libre = (primera_libre+1) % tam_vector;
            //MODIFICACION DE CONTADOR DE OCUPADAS
            num_celdas_ocupadas++;
            //MOSTRAR MENSAJE DE RECEPCION
            cout << "----------> Buffer ha recibido valor " << valor << endl;
        }
        //SI EL RECIBO DE LA PETICION FUE DEL CONSUMIDOR
        else{
            //EXTRACCION DEL DATO DEL VECTOR
            valor = buffer[primera_ocupada];
            //MODIFICACION DEL APUNTADOR
            primera_ocupada = (primera_ocupada+1) % tam_vector;
            //MODIFICACION DEL CONTADOR DE OCUPADAS
            num_celdas_ocupadas--;
            //MOSTRAR MENSAJE DE ENVIO
            cout << "<---------- Buffer va a enviar valor " << valor << endl;

            //MPI SSEND --> ENVIO SINCRONO
            //-> void*        : valor_prod     = UBICACION DE BYTES EN MEMORIA [ENVIO]
            //-> int          : 1              = NUM DE ELEMENTOS ALMACENADOS
            //-> MPI_Datatype : MPI_INT        = TIPO DE DATO ENVIADO
            //-> int          : id_buffer      = IDENTIFICADOR DE PROCESO DESTINO [BUFFER]
            //-> int [tag]    : etiq_buffer    = ETIQUETA DEL MENSAJE
            //-> MPI_Comm     : MPI_COMM_WORLD = COMUNICADOR EN EL QUE SE REALIZA EL ENVIO
            MPI_Ssend(&valor, 1, MPI_INT, estado.MPI_SOURCE, etiq_consumidor, MPI_COMM_WORLD);
        }
    }
}

//=====================================================================
// FUNCION PRINCIPAL
//=====================================================================

/**
 * @brief Funcion principal
 * @param argc : Numero de argumentos
 * @param argv : Total de argumentos
 * @return 0 [EX COMPLETADA]
 */
int main(int argc, char *argv[]){
    //ID PROPIO : IDENTIFICADOR DE PROCESO
    //NUM_PROCESOS_ACTUAL : TOTAL DE PROCESOS EJECUTADOS
    int id_propio, num_procesos_actual;
    
    //PASO 1: INICIALIZAR MPI, LEER ID_PROPIO Y NUM_PROCESOS_ACTUAL
   
    //MPI INIT --> INICIO DEL PROCEDMIENTO MPI
    //-> int*         : argc = NUMERO DE ARGUMENTOS
    //-> char***      : argv = TOTAL DE ARGUMENTOS
    MPI_Init(&argc, &argv);
    
    //MPI_COMM_RANK --> OBTIENE LOS VALORES DE CADA PROCESO
    //-> MPI_Comm  : MPI_COMM_WORLD = COMUNICADOR MPI
    //-> int*      : id_propio      = ALMACENA EL IDENTIFICADOR DEL PROCESO [AUTO-INVOKE]
    MPI_Comm_rank(MPI_COMM_WORLD, &id_propio);
    
    //MPI_COMM_SIZE --> OBTIENE EL TOTAL DE PROCESOS
    //-> MPI_Comm  : MPI_COMM_WORLD      = COMUNICADOR MPI 
    //-> int*      : num_procesos_actual = ALMACENA EL TOTAL DE PROCESOS ACTUALES 
    MPI_Comm_size(MPI_COMM_WORLD, &num_procesos_actual);
    

    //COMPROBACION DE QUE EL LANZAMIENTO DE PROCESOS ES EL ESPERADO
    if ((num_procesos_esperado == num_procesos_actual)){
        //COMPROBAMOS SI EL PROGRAMA ES CORRECTO
        int guarda = checkParameters();
        
        //SI LAS CONSTANTES SON CORRECTAS LANZAMOS APLICACION
        if(guarda == 0){
            //ASIGNACION DE ROL: PRODUCTOR
            if (id_propio < id_buffer){
                funcion_productor(id_propio);
            }
            //ASIGNACION DE ROL : BUFFER
            else if (id_propio == id_buffer)
                funcion_buffer();
            //ASIGNACION DE ROL : CONSUMIDOR
            else
                funcion_consumidor(id_propio);
        }
        //LAS CONSTANTES NO TIENE PARAMETROS VALIDOS
        else{
            if(id_propio == 0)
                notifyError(guarda);
        }
    }
    //NO SE CUMPLE LOS PARAMETROS ESPERADOS
    else{
        //ERROR LANZADO DESDE EL PROCESO 0
        if (id_propio == 0){
            cout << "el número de procesos esperados es:    " << num_procesos_esperado << endl
                 << "el número de procesos en ejecución es: " << num_procesos_actual << endl
                 << "(programa abortado)" << endl;
        }
    }

   //MPI_FINALIZE --> Finaliza el procedimiento MPI
   MPI_Finalize();
   
   return 0;
}
