#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <vector>
#include <string>

using namespace std;

/**
 * @brief Estructura de datos para las instancias clásicas de Beasley (Unipista).
 * Modela las restricciones temporales y económicas de cada aeronave.
 */
struct Avion {
    int id;                         // Identificador único de la aeronave.
    double tiempo_llegada;          // Tiempo en que el avión entra al área de control (radar).
    double tiempo_minimo;           // Tiempo más temprano posible de aterrizaje (E_i).
    double tiempo_objetivo;         // Tiempo preferido o ideal de aterrizaje (T_i).
    double tiempo_maximo;           // Tiempo límite para aterrizar antes de quedarse sin combustible (L_i).
    double costo_segundo_temprano;  // Penalización económica por unidad de tiempo al aterrizar antes (c_i^-).
    double costo_segundo_tarde;     // Penalización económica por unidad de tiempo al aterrizar después (c_i^+).
};

/**
 * @brief Estructura de datos para las instancias basadas en tráfico real (FPT - Multipista).
 * Incorpora atributos físicos y comerciales para evaluar la estela de turbulencia y prioridades.
 */
struct AvionReal {
    int id;                  // Identificador numérico de la aeronave en la secuencia.
    string codigo_vuelo;     // Código comercial alfanumérico del vuelo (ej. LATAM123).
    string modelo;           // Modelo físico de la aeronave (ej. A320, B737).
    char categoria_estela;   // Categoría de turbulencia de estela (L=Light, M=Medium, H=Heavy).
    char operacion;          // Tipo de maniobra (A = Aterrizaje, D = Despegue).
    double tiempo_objetivo;  // Tiempo preferido o ideal para realizar la operación (T_i).
    double penalidad;        // Factor de costo o penalización base por desvíos del tiempo objetivo.
};

/**
 * @namespace Uni
 * @brief Encapsula la lógica de ejecución y evaluación para los experimentos de Pista Única (Beasley).
 */
namespace Uni {
    /**
     * @brief Inicia la batería de pruebas para las 12 instancias de Beasley, 
     * iterando sobre las configuraciones de semillas y restarts.
     */
    void ejecutarPruebas();
}

/**
 * @namespace Multi
 * @brief Encapsula la lógica de ejecución y evaluación para los experimentos de Múltiples Pistas (FPT).
 */
namespace Multi {
    /**
     * @brief Inicia la batería de pruebas para las instancias de tráfico real.
     * @param pistas_ingresadas Cantidad de pistas a simular (K = 2, 3, 4, 5) para evaluar escalabilidad.
     */
    void ejecutarPruebas(int pistas_ingresadas);
}

#endif