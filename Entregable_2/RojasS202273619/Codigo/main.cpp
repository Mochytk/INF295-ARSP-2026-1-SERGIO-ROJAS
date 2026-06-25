/**
 * @file main.cpp
 * @brief Punto de entrada principal para el Sistema de Asignación de Pistas (ARSP).
 * * Este archivo gestiona la interacción inicial con el usuario para determinar 
 * el tipo de simulación a ejecutar (pista única o múltiples pistas) y enruta 
 * la ejecución al motor algorítmico correspondiente.
 */

#include <iostream>
#include "estructuras.h"

using namespace std;

/**
 * @brief Función principal del programa.
 * * Solicita al usuario la cantidad de pistas operativas y, basándose en la entrada,
 * delega la ejecución de las pruebas a los espacios de nombres Uni (Beasley) o 
 * Multi (FPT). Incluye manejo básico de excepciones para la entrada estándar.
 * * @return int Código de estado de salida (0 si la ejecución es exitosa, 1 en caso de error).
 */
int main() {
    int pistas;
    
    // Interfaz de consola inicial
    cout << "===============================================" << endl;
    cout << "   SISTEMA DE ASIGNACIÓN DE PISTAS (ARSP)      " << endl;
    cout << "===============================================" << endl;
    cout << "Ingrese la cantidad de pistas a simular: ";
    
    // Validación de entrada para evitar que el programa colapse si se ingresan caracteres no numéricos
    if (!(cin >> pistas)) {
        cerr << "Error: Entrada inválida." << endl;
        return 1;
    }

    // Enrutamiento de la ejecución según la infraestructura simulada
    if (pistas == 1) {
        // Escenario clásico de Beasley (optimización estricta en una sola línea de aterrizaje)
        cout << "\n>>> INICIANDO MOTOR UNIPISTA (Instancias Airland) <<<" << endl;
        // Llama a la lógica encapsulada en el namespace Uni
        Uni::ejecutarPruebas(); 
    } 
    else if (pistas > 1) {
        // Escenario escalable FPT (asignación Greedy y balanceo de carga en K pistas)
        cout << "\n>>> INICIANDO MOTOR MULTIPISTA (Instancias FPT) CON " << pistas << " PISTAS <<<" << endl;
        // Llama a la lógica encapsulada en el namespace Multi pasando el número de pistas como parámetro
        Multi::ejecutarPruebas(pistas); 
    } 
    else {
        // Manejo de casos lógicamente inviables (0 o pistas negativas)
        cout << "\nError: La cantidad de pistas debe ser al menos 1." << endl;
        return 1;
    }
    
    return 0; // Ejecución finalizada con éxito
}