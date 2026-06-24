#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <vector>
#include <string>

using namespace std;

// Struct para los datos reales del aeropuerto
struct AvionReal {
    int id;          
    string codigo_vuelo;     
    string modelo;           
    char categoria_estela;   // Ej: 'M', 'H'
    char operacion;          // Ej: 'A', 'D'
    double tiempo_objetivo;  
    double penalidad;        
};

// Declaración de Variables globales (extern)
extern int NUM_AVIONES; 
extern int NUM_PISTAS; 
extern vector<AvionReal> AVIONES; 
extern vector<vector<double>> COSTOS_S; 
extern int NUM_PRUEBA;
extern int SEED; 
extern int NUM_RESTARTS; 
extern double TIEMPO_TOTAL; 
extern vector<int> MEJOR_SOLUCION;
extern double MEJOR_COSTO;

void leerArchivo(const string& filename_vuelos);

vector<int> generarSolucionInicial();
double funcionEvaluacion(const vector<int>& solucion);
void swap(vector<int>& solucion, int i, int j);
vector<vector<int>> generarVecindario(const vector<int>& solucion);
void HC(vector<int> solucion_actual);
void escribirSalida(const string& filename, const vector<int>& solucion, const double& tiempo_total);

#endif