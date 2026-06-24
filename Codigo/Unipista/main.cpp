#include <iostream>
#include <iomanip>
#include "estructuras.h"

using namespace std;

int main(){
    cout << fixed << setprecision(2); 
    
    for (int i = 0; i < 2; i++) {
        NUM_RESTARTS = 0; 
        MEJOR_COSTO = 100000000;
        MEJOR_SOLUCION.clear();
        NUM_PRUEBA = i + 1;
        
        cout << "Ejecutando prueba " << NUM_PRUEBA << "..." << endl;
        string archivo = "Una pista/airland" + to_string(NUM_PRUEBA) + ".txt";
        
        leerArchivo(archivo);
        vector<int> solucion_inicial = generarSolucionInicial();
        HC(solucion_inicial);
    }
    return 0;
}