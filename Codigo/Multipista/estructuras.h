#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <vector>
#include <string>

using namespace std;

// Struct para los datos de cada avión
struct Avion {
    int id;
    double tiempo_llegada;
    double tiempo_minimo;
    double tiempo_objetivo;
    double tiempo_maximo;
    double costo_segundo_temprano;
    double costo_segundo_tarde;
};

// Declaración de Variables globales (extern)
extern int NUM_AVIONES; 
extern int HOLGURA; 
extern vector<Avion> AVIONES; 
extern vector<vector<double>> COSTOS_S; 
extern int NUM_PRUEBA;
extern int SEED; 
extern int NUM_RESTARTS; 
extern double TIEMPO_TOTAL; 
extern vector<int> MEJOR_SOLUCION;
extern double MEJOR_COSTO;

// Declaración de las funciones para que el main las conozca
void leerArchivo(const string& filename);
vector<int> generarSolucionInicial();
double funcionEvaluacion(const vector<int>& solucion);
void swap(vector<int>& solucion, int i, int j);
vector<vector<int>> generarVecindario(const vector<int>& solucion);
void HC(vector<int> solucion_actual);
void escribirSalida(const string& filename, const vector<int>& solucion, const double& tiempo_total);

#endif