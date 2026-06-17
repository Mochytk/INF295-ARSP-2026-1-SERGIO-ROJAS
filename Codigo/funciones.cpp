#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>
#include "estructuras.h"

using namespace std;

// Instanciación de variables globales
int NUM_AVIONES; 
int NUM_PISTAS; 
vector<Avion> AVIONES; 
vector<vector<double>> COSTOS_S; 
int NUM_PRUEBA;
int SEED = 21; 
int NUM_RESTARTS = 0; 
double TIEMPO_TOTAL = 0; 
vector<int> MEJOR_SOLUCION;
double MEJOR_COSTO = 100000000;

void leerArchivo(const string& filename){
    ifstream archivo(filename);
    if (!archivo.is_open()) {
        cerr << "Error al abrir el archivo: " << filename << endl;
    }
    archivo >> NUM_AVIONES >> NUM_PISTAS;
    AVIONES.resize(NUM_AVIONES);
    COSTOS_S.assign(NUM_AVIONES, vector<double>(NUM_AVIONES, 0.0));
    for (int i = 0; i < NUM_AVIONES; i++) {
        AVIONES[i].id = i;
        archivo >>  AVIONES[i].tiempo_llegada
                >> AVIONES[i].tiempo_minimo
                >> AVIONES[i].tiempo_objetivo
                >> AVIONES[i].tiempo_maximo
                >> AVIONES[i].costo_segundo_temprano
                >> AVIONES[i].costo_segundo_tarde;
        for (int j = 0; j < NUM_AVIONES; j++) {
            archivo >> COSTOS_S[i][j];
        }
        // Saltar matriz s_ij
        for (int j = 0; j < NUM_AVIONES; j++) {
            double ignorar;
            archivo >> ignorar; 
        }
    }
}

vector<int> generarSolucionInicial() {
    vector<int> solucion(NUM_AVIONES);
    for (int i = 0; i < NUM_AVIONES; i++) {
        solucion[i] = i; 
    }
    mt19937 g(SEED); 
    shuffle(solucion.begin(), solucion.end(), g);
    return solucion;
}

double funcionEvaluacion(const vector<int>& solucion) {
    double costo_total = 0.0;
    vector<double> tiempos_asignados(NUM_AVIONES, 0.0);

    for (int i = 0; i < NUM_AVIONES; i++) {
        int avion_actual = solucion[i];
        cout << "Evaluando avión " << avion_actual << " en posición " << i << "..." << endl;
        // 1. Piso base: Lo más temprano que físicamente puede aterrizar
        double tiempo_liberacion = AVIONES[avion_actual].tiempo_minimo; 
        cout << "  - Piso base (T_i mínimo): " << tiempo_liberacion << endl;
        // 2. Muro de separación: Revisar contra TODOS los aviones que ya aterrizaron antes en la secuencia
        for (int j = 0; j < i; j++) {
            int avion_previo = solucion[j];

            // Actualizamos el tiempo de liberación si la estela de turbulencia del avión previo nos obliga a esperar más
            tiempo_liberacion = max(tiempo_liberacion, tiempos_asignados[avion_previo] + COSTOS_S[AVIONES[avion_previo].id][AVIONES[avion_actual].id]);
            cout << "  - Revisando avión previo " << avion_previo << ": ";
            cout << "Tiempo asignado: " << tiempos_asignados[avion_previo] << ", Separación requerida: " << COSTOS_S[AVIONES[avion_previo].id][AVIONES[avion_actual].id] << ", Nuevo piso base: " << tiempo_liberacion << endl;
        }

        // 3. Ajuste al objetivo: Si la pista está libre antes de nuestro tiempo objetivo (T_i), 
        // retrasamos el aterrizaje hasta T_i para no pagar penalización por llegar temprano.
        double tiempo_real = max(tiempo_liberacion, AVIONES[avion_actual].tiempo_objetivo);
        cout << "  - Tiempo real de aterrizaje (ajustado al objetivo): " << tiempo_real << endl;
        cout << "  - Tiempo objetivo del avión: " << AVIONES[avion_actual].tiempo_objetivo << endl;
        tiempos_asignados[avion_actual] = tiempo_real;
        cout << "  - Tiempo asignado para el avión " << avion_actual << ": " << tiempos_asignados[avion_actual] << endl;
        // 4. Validar factibilidad estricta
        if (tiempo_real > AVIONES[avion_actual].tiempo_maximo) {
            cout << "  - Avión " << avion_actual << " excede el tiempo máximo permitido." << endl;
            cout << "  - Tiempo real: " << tiempo_real << ", Tiempo máximo: " << AVIONES[avion_actual].tiempo_maximo << endl;
            return -1; // Esta permutación es inválida, rompemos la evaluación de este vecino
        }

        // 5. Cálculo exacto de las penalizaciones
        if (tiempo_real < AVIONES[avion_actual].tiempo_objetivo) {
            costo_total += (AVIONES[avion_actual].tiempo_objetivo - tiempo_real) * AVIONES[avion_actual].costo_segundo_temprano;
            cout << "  - Penalización por llegar temprano: " << (AVIONES[avion_actual].tiempo_objetivo - tiempo_real) * AVIONES[avion_actual].costo_segundo_temprano << endl;
        } else if (tiempo_real > AVIONES[avion_actual].tiempo_objetivo) {
            costo_total += (tiempo_real - AVIONES[avion_actual].tiempo_objetivo) * AVIONES[avion_actual].costo_segundo_tarde;
            cout << "  - Penalización por llegar tarde: " << (tiempo_real - AVIONES[avion_actual].tiempo_objetivo) * AVIONES[avion_actual].costo_segundo_tarde << endl;
        }
    }
    
    return costo_total;
}

void swap(vector<int>& solucion, int i, int j) {
    int temp = solucion[i];
    solucion[i] = solucion[j];
    solucion[j] = temp;
}

vector<vector<int>> generarVecindario(const vector<int>& solucion){
    vector<vector<int>> vecindario;
    for (int i = 0; i < NUM_AVIONES - 1; i++) {
        for (int j = i + 1; j < NUM_AVIONES; j++) {
            vector<int> nueva_solucion = solucion;
            swap(nueva_solucion, i, j);
            vecindario.push_back(nueva_solucion);
        }
    }
    return vecindario;
}

void HC(vector<int> solucion_actual) {
    while (NUM_RESTARTS < 10) {
        bool optimo_local = false;
        double costo_actual = funcionEvaluacion(solucion_actual);
        
        cout << "\n--- Iniciando Restart " << NUM_RESTARTS << " ---" << endl;
        cout << "Costo inicial de este intento: " << costo_actual << endl;

        while (!optimo_local) {
            vector<vector<int>> vecindario = generarVecindario(solucion_actual);
            
            vector<int> mejor_vecino = solucion_actual;
            double mejor_costo_vecino = costo_actual;
            bool mejora_encontrada = false;
            for (const auto& elemento : solucion_actual) {
                cout << elemento << " ";
            }
            cout << " -> Costo actual: " << costo_actual << endl;
            for (const auto& vecino : vecindario) {
                for (size_t idx = 0; idx < vecino.size(); idx++) {
                    cout << vecino[idx] << " ";
                }
                cout << " -> Costo: ";
                double costo_vecino = funcionEvaluacion(vecino);
                cout << costo_vecino << endl;
                if ((costo_vecino != -1) && (costo_vecino < mejor_costo_vecino)) {
                    mejor_vecino = vecino;
                    mejor_costo_vecino = costo_vecino;
                    mejora_encontrada = true;
                }
            }

            if (mejora_encontrada) {
                solucion_actual = mejor_vecino;
                costo_actual = mejor_costo_vecino;
            } else {
                optimo_local = true;
                cout << "Óptimo local alcanzado con costo: " << costo_actual << endl;
                if (costo_actual != -1 && costo_actual < MEJOR_COSTO) {
                    MEJOR_COSTO = costo_actual;
                    MEJOR_SOLUCION = solucion_actual;
                    cout << ">>> NUEVO MEJOR COSTO GLOBAL: " << MEJOR_COSTO << " <<<" << endl;
                }
            }
        }

        NUM_RESTARTS++;
        if (NUM_RESTARTS < 10) {
            SEED += 100; 
            cout << "Ejecutando restart..." << endl;
            solucion_actual = generarSolucionInicial(); 
        }
    } 
    cout << "\nNúmero máximo de restarts alcanzado. Terminando búsqueda." << endl;
    cout << "Mejor solución global encontrada con costo: " << MEJOR_COSTO << endl;
}

void escribirSalida(const string& filename, const vector<int>& solucion, const double& tiempo_total) {
    ofstream archivo(filename);
    if (!archivo.is_open()) {
        cerr << "Error al abrir el archivo para escribir: " << filename << endl;
        return;
    }
    archivo << fixed << setprecision(2);
    archivo << "Solución Final " << filename << ":" << endl;
    archivo << "Seed utilizada: " << SEED << endl;
    archivo << "Cantidad de Restarts: " << NUM_RESTARTS << endl;
    archivo << "Lista de Ordenes de Llegada:" << endl;
    
    for (size_t i = 0; i < solucion.size(); i++) {
        archivo << "Avión " << i << " asignado a Pista " << solucion[i] << endl;
    }
    archivo << "Tiempo Total de Ejecución: " << tiempo_total << endl;
    archivo.close();
}