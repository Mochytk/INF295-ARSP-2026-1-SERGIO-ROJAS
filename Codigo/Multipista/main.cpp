#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "estructuras.h"

using namespace std;

int main(){
    cout << fixed << setprecision(2); 
    
    int cant_semillas;
    cin >> NUM_PISTAS;
    cin >> cant_semillas;

    vector<int> lista_semillas;

    for (int i = 8; i < 8+cant_semillas; i++) {
        lista_semillas.push_back(pow(2, i));
    }
    for (int i = 1; i <= 12; i++) {
        string num_str = (i < 10) ? "0" + to_string(i) : to_string(i);
        string arch_vuelos = "Múltiples Pistas/FPT" + num_str + ".txt";
        
        cout << "\n=============================================" << endl;
        cout << "Cargando FPT" << num_str << "..." << endl;
        
        leerArchivo(arch_vuelos); 
        
        for (size_t s = 0; s < lista_semillas.size(); s++) {
            int semilla_base = lista_semillas[s];
            
            SEED = semilla_base; 
            NUM_RESTARTS = 0; 
            MEJOR_COSTO = 100000000.0;
            MEJOR_SOLUCION.clear();
            
            cout << "  -> Ejecutando prueba con SEED " << semilla_base << "..." << endl;
            
            vector<int> solucion_inicial = generarSolucionInicial();
            double tiempo_inicial = static_cast<double>(clock()) / CLOCKS_PER_SEC;
            HC(solucion_inicial);
            double tiempo_final = static_cast<double>(clock()) / CLOCKS_PER_SEC;
            
            SEED = semilla_base; 
            
            string filename = "Output/resultados_FPT" + num_str + "_SEED" + to_string(semilla_base) + ".txt";
            escribirSalida(filename, MEJOR_SOLUCION, tiempo_final - tiempo_inicial);
        }
    }
    
    cout << "\n¡Todas las instancias y semillas han sido procesadas!" << endl;
    return 0;
}