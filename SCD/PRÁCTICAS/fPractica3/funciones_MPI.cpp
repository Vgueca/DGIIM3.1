/**
 * @file funciones_MPI.cpp
 * @author Juan Valentín Guerrero Cano
 */

/* Para compilar y ejecutar:

mpicxx -std=c++11 -o ejer ejer.cpp

mpirun -oversubscribe -np 11 ./ejer 

*/
#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

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


int main(int argc, char *argv[]){
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


    // EN LAS FUNCIONES FUERA DEL MAIN 
        //MPI SSEND --> ENVIO SINCRONO
            //-> void*        : valor_prod     = UBICACION DE BYTES EN MEMORIA [ENVIO]
            //-> int          : 1              = NUM DE ELEMENTOS ALMACENADOS
            //-> MPI_Datatype : MPI_INT        = TIPO DE DATO ENVIADO
            //-> int          : id_buffer      = IDENTIFICADOR DE PROCESO DESTINO [BUFFER]
            //-> int [tag]    : etiq_productor = ETIQUETA DEL MENSAJE
            //-> MPI_Comm     : MPI_COMM_WORLD = COMUNICADOR EN EL QUE SE REALIZA EL ENVIO
            MPI_Ssend(&valor_prod, 1, MPI_INT, id_buffer, etiq_productor, MPI_COMM_WORLD);
        //MPI SEND --> ENVIO ASÍNCRONO
            //-> void*        : valor_prod     = UBICACION DE BYTES EN MEMORIA [ENVIO]
            //-> int          : 1              = NUM DE ELEMENTOS ALMACENADOS
            //-> MPI_Datatype : MPI_INT        = TIPO DE DATO ENVIADO
            //-> int          : id_buffer      = IDENTIFICADOR DE PROCESO DESTINO [BUFFER]
            //-> int [tag]    : etiq_productor = ETIQUETA DEL MENSAJE
            //-> MPI_Comm     : MPI_COMM_WORLD = COMUNICADOR EN EL QUE SE REALIZA EL ENVIO
            MPI_Send(&valor_prod, 1, MPI_INT, id_buffer, etiq_productor, MPI_COMM_WORLD);

        //MPI RECV --> RECIBIMIENTO SINCRONO
            //-> void*        : valor_rec      = UBICACION DE MEMORIA DONDE SE RECIBEN LOS DATOS
            //-> int          : 1              = NUM DE ELEMENTOS RECIBIDOS
            //-> MPI_Datatype : MPI_INT        = TIPO DE DATO ENVIADO
            //-> int          : id_buffer      = IDENTIFICADOR DE PROCESO RECEPTOR
            //-> int [tag]    : etiq_productor = ETIQUETA DEL MENSAJE
            //-> MPI_Comm     : MPI_COMM_WORLD = COMUNICADOR EN EL QUE SE REALIZA LA RECEPCION
            //-> MPI_Status   : estado         = METADATO PARA OBTENER INFORMACION DEL EMISOR
            MPI_Recv (&valor_rec, 1, MPI_INT, id_buffer, etiq_consumidor, MPI_COMM_WORLD, &estado);


        //PASO INTERMEDIO: BLOQUEAR HASTA QUE HAYA ALGUN MENSAJE ESPERANDO
        //MPI PROBE --> ESPERA MENSAJE
            //-> int          : MPI_ANY_SOURCE           = IDENTIFICADOR DE EMISOR
            //-> int [tag]    : tag                      = ETIQUETA DEL MENSAJE
            //-> MPI_Comm     : MPI_COMM_WORLD           = COMUNICADOR EN EL QUE SE REALIZA LA RECEPCION
            //-> MPI_Status   : estado                   = METADATO PARA OBTENER INFORMACION DEL EMISOR
            MPI_Probe(MPI_ANY_SOURCE,tag,MPI_COMM_WORLD,&estado);   //Se  queda esperando mensaje
            //El flag>0 nos indica que se ha recibido algún mensaje
            MPI_Iprobe(MPI_ANY_SOURCE,tag,MPI_COMM_WORLD,flag, &estado);  //Solo comprueba que haya algún mensaje

        MPI_Get_count();


int maximo(int v[], int n){
    int max;
 
    for (int i = 0; i < n; i++)
        if (i == 0 || max < v[i])
            max = v[i];
 
    return max;
}
 
int minimo(int v[], int n){
    int min;
 
    for (int i = 0; i < n; i++)
        if (i == 0 || min > v[i])
            min = v[i];
 
    return min;
}
}
