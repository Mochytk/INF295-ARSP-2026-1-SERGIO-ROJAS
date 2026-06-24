#include <iostream>
#include "estructuras.h"

using namespace std;

int main() {
    int pistas;
    
    cout << "===============================================" << endl;
    cout << "   SISTEMA DE ASIGNACIÓN DE PISTAS (ARSP)      " << endl;
    cout << "===============================================" << endl;
    cout << "Ingrese la cantidad de pistas a simular: ";
    
    if (!(cin >> pistas)) {
        cerr << "Error: Entrada inválida." << endl;
        return 1;
    }

    if (pistas == 1) {
        cout << "\n>>> INICIANDO MOTOR UNIPISTA (Instancias Airland) <<<" << endl;
        // Llama a la lógica encapsulada de Unipista
        Uni::ejecutarPruebas(); 
    } 
    else if (pistas > 1) {
        cout << "\n>>> INICIANDO MOTOR MULTIPISTA (Instancias FPT) CON " << pistas << " PISTAS <<<" << endl;
        // Llama a la lógica encapsulada de Multipista
        Multi::ejecutarPruebas(pistas); 
    } 
    else {
        cout << "\nError: La cantidad de pistas debe ser al menos 1." << endl;
        return 1;
    }
    
    return 0;
}