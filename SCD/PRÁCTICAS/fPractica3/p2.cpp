// 24411228V - Pablo Olivares Martínez

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

#include <chrono>  // duraciones (duration), unidades de tiempo
#include <iostream>
#include <random>  // dispositivos, generadores y distribuciones aleatorias
#include <thread>  // this_thread::sleep_for

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

const int num_filosofos = 7,           // número de filósofos
    num_filo_ten = 2 * num_filosofos,  // número de filósofos y tenedores
    num_procesos = num_filo_ten + 1,   // número de procesos total
    id_camarero = num_filo_ten;

const int etiq_reservar_tenedor = 1, etiq_soltar_tenedor = 2,
          etiq_reservar_sitio = 3, etiq_soltar_sitio = 4;

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template <int min, int max>
int aleatorio() {
    static default_random_engine generador((random_device())());
    static uniform_int_distribution<int> distribucion_uniforme(min, max);
    return distribucion_uniforme(generador);
}

// ---------------------------------------------------------------------

void funcion_filosofos(int id) {
    int id_ten_izq = (id + 1) % num_filo_ten,  // id. tenedor izq.
        id_ten_der =
            (id + num_filo_ten - 1) % num_filo_ten;  // id. tenedor der.

    while (true) {
        cout << "Filósofo " << id << " solicita sentarse." << endl;
        // ... solicitar sentarse
        MPI_Ssend(&id, 1, MPI_INT, id_camarero, etiq_reservar_sitio,
                  MPI_COMM_WORLD);

        cout << "Filósofo " << id << " solicita ten. izq." << id_ten_izq
             << endl;
        // ... solicitar tenedor izquierdo (completar)
        MPI_Ssend(&id, 1, MPI_INT, id_ten_izq, etiq_reservar_tenedor,
                  MPI_COMM_WORLD);
        cout << "Filósofo " << id << " solicita ten. der." << id_ten_der
             << endl;
        // ... solicitar tenedor derecho (completar)
        MPI_Ssend(&id, 1, MPI_INT, id_ten_der, etiq_reservar_tenedor,
                  MPI_COMM_WORLD);
        cout << "Filósofo " << id << " comienza a comer" << endl;
        sleep_for(milliseconds(aleatorio<10, 100>()));

        cout << "Filósofo " << id << " suelta ten. izq. " << id_ten_izq << endl;
        // ... soltar el tenedor izquierdo (completar)
        MPI_Ssend(&id, 1, MPI_INT, id_ten_izq, etiq_soltar_tenedor,
                  MPI_COMM_WORLD);
        cout << "Filósofo " << id << " suelta ten. der. " << id_ten_der << endl;
        // ... soltar el tenedor derecho (completar)
        MPI_Ssend(&id, 1, MPI_INT, id_ten_der, etiq_soltar_tenedor,
                  MPI_COMM_WORLD);

        cout << "Filósofo " << id << " solicita irse." << endl;
        // ... solicitar irse
        MPI_Ssend(&id, 1, MPI_INT, id_camarero, etiq_soltar_sitio,
                  MPI_COMM_WORLD);

        cout << "Filosofo " << id << " comienza a pensar" << endl;
        sleep_for(milliseconds(aleatorio<10, 100>()));
    }
}
// ---------------------------------------------------------------------

void funcion_tenedores(int id) {
    int valor, id_filosofo;  // valor recibido, identificador del filósofo
    MPI_Status estado;       // metadatos de las dos recepciones

    while (true) {
        // ...... recibir petición de cualquier filósofo (completar)
        MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_reservar_tenedor,
                 MPI_COMM_WORLD, &estado);
        // ...... guardar en 'id_filosofo' el id. del emisor (completar)
        id_filosofo = estado.MPI_SOURCE;
        cout << "Ten. " << id << " ha sido cogido por filo. " << id_filosofo
             << endl;

        // ...... recibir liberación de filósofo 'id_filosofo' (completar)
        MPI_Recv(&valor, 1, MPI_INT, id_filosofo, etiq_soltar_tenedor,
                 MPI_COMM_WORLD, &estado);
        cout << "Ten. " << id << " ha sido liberado por filo. " << id_filosofo
             << endl;
    }
}
// ---------------------------------------------------------------------

void funcion_camarero() {
    static int contador = 0;
    int valor, etiq_recibida, etiq_aceptable, id_filosofo;
    MPI_Status estado;  // metadatos de las dos recepciones

    while (true) {
        if (contador == 0)
            etiq_aceptable = etiq_reservar_sitio;
        else if (contador == num_filosofos - 1)
            etiq_aceptable == etiq_soltar_sitio;
        else
            etiq_aceptable = MPI_ANY_TAG;

        MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_aceptable,
                 MPI_COMM_WORLD, &estado);
        etiq_recibida = estado.MPI_TAG;
        id_filosofo = estado.MPI_SOURCE;

        if (etiq_recibida == etiq_reservar_sitio) {
            cout << "El filósofo " << id_filosofo << " ha cogido sitio. "
                 << id_filosofo << endl;
            contador++;
        } else if (etiq_recibida == etiq_soltar_sitio) {
            cout << "El filósofo " << id_filosofo << " ha soltado su sitio. "
                 << id_filosofo << endl;
            contador--;
        }
    }
}
// ---------------------------------------------------------------------

int main(int argc, char** argv) {
    int id_propio, num_procesos_actual;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_propio);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procesos_actual);

    if (num_procesos == num_procesos_actual) {
        // ejecutar la función correspondiente a 'id_propio'
        if (id_propio % 2 == 0 && id_propio != num_filo_ten)  // si es par
            funcion_filosofos(id_propio);  //   es un filósofo
        else if (id_propio == num_filo_ten)
            funcion_camarero();
        else                               // si es impar
            funcion_tenedores(id_propio);  //   es un tenedor
    } else {
        if (id_propio == 0)  // solo el primero escribe error, indep. del rol
        {
            cout << "el número de procesos esperados es:    " << num_procesos
                 << endl
                 << "el número de procesos en ejecución es: "
                 << num_procesos_actual << endl
                 << "(programa abortado)" << endl;
        }
    }

    MPI_Finalize();
    return 0;
}

// ---------------------------------------------------------------------
