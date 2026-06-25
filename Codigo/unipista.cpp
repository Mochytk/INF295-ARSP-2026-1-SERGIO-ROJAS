#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include "estructuras.h"

using namespace std;

namespace Uni {
    // Instanciación de variables globales
    int NUM_AVIONES; 
    int HOLGURA; 
    vector<Avion> AVIONES; 
    vector<vector<double>> COSTOS_S; 
    int NUM_PRUEBA;
    int SEED; 
    int NUM_RESTARTS = 0; 
    int RESTARTS_MAXIMOS;
    vector<int> MEJOR_SOLUCION;
    double MEJOR_COSTO = 100000000;

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

    double funcionEvaluacion(const vector<int>& solucion) {
        double costo_total = 0.0;
        vector<double> tiempos_asignados(NUM_AVIONES, 0.0);

        for (int i = 0; i < NUM_AVIONES; i++) {
            int avion_actual = solucion[i];
            
            // 1. Piso base
            double tiempo_liberacion = AVIONES[avion_actual].tiempo_minimo; 

            // 2. Muro de separación (historial completo)
            for (int j = 0; j < i; j++) {
                int avion_previo = solucion[j];
                tiempo_liberacion = max(tiempo_liberacion, tiempos_asignados[avion_previo] + COSTOS_S[AVIONES[avion_previo].id][AVIONES[avion_actual].id]);
            }

            // 3. Ajuste al objetivo
            double tiempo_real = max(tiempo_liberacion, AVIONES[avion_actual].tiempo_objetivo);
            tiempos_asignados[avion_actual] = tiempo_real;

            // 4. Validar factibilidad estricta
            if (tiempo_real > AVIONES[avion_actual].tiempo_maximo) {
                return -1; 
            }

            // 5. Cálculo exacto de las penalizaciones
            if (tiempo_real < AVIONES[avion_actual].tiempo_objetivo) {
                costo_total += (AVIONES[avion_actual].tiempo_objetivo - tiempo_real) * AVIONES[avion_actual].costo_segundo_temprano;
            } else if (tiempo_real > AVIONES[avion_actual].tiempo_objetivo) {
                costo_total += (tiempo_real - AVIONES[avion_actual].tiempo_objetivo) * AVIONES[avion_actual].costo_segundo_tarde;
            }
        }
        
        return costo_total;
    }

    void leerArchivo(const string& filename){
        ifstream archivo(filename);
        if (!archivo.is_open()) {
            cerr << "Error al abrir el archivo: " << filename << endl;
        }
        archivo >> NUM_AVIONES >> HOLGURA;
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
        }
    }

    vector<int> generarSolucionInicial() {
        vector<int> solucion(NUM_AVIONES);
        for (int i = 0; i < NUM_AVIONES; i++) {
            solucion[i] = i; 
        }
        
        // Ordenar por Tiempo Máximo (L_i) ascendente. 
        // Esto ordena a los aviones por quién se queda sin combustible primero.
        sort(solucion.begin(), solucion.end(), [](int a, int b) {
            return AVIONES[a].tiempo_maximo < AVIONES[b].tiempo_maximo;
        });

        double costo_inicial = -1;
        mt19937 g(SEED); 
        uniform_int_distribution<int> dist(0, NUM_AVIONES - 1);
        
        int intentos = 0;

        while (costo_inicial == -1) {
            vector<int> candidata = solucion; // Partimos de la base
            
            // Intercambiamos solo un par de aviones aleatoriamente.
            int num_swaps = max(1, NUM_AVIONES / 10);
            
            // Si la perturbación rompe mucho la factibilidad, la reducimos.
            if (intentos > 500) num_swaps = 1; 
            if (intentos > 1000) num_swaps = 0; // Se rinde y devuelve la base pura ordenada

            for (int k = 0; k < num_swaps; k++) {
                int i = dist(g);
                int j = dist(g);
                // Aplicar el swap aleatorio leve
                int temp = candidata[i];
                candidata[i] = candidata[j];
                candidata[j] = temp;
            }
            
            costo_inicial = funcionEvaluacion(candidata);
            
            if (costo_inicial != -1) {
                return candidata; // Encontramos una inicial válida y estocástica
            }
            
            intentos++;
        }
        
        return solucion;
    }

    void HC(vector<int> solucion_actual) {
        while (NUM_RESTARTS < RESTARTS_MAXIMOS) {
            bool optimo_local = false;
            double costo_actual = funcionEvaluacion(solucion_actual);

            while (!optimo_local) {
                vector<vector<int>> vecindario = generarVecindario(solucion_actual);
                
                vector<int> mejor_vecino = solucion_actual;
                double mejor_costo_vecino = costo_actual;
                bool mejora_encontrada = false;

                for (const auto& vecino : vecindario) {
                    double costo_vecino = funcionEvaluacion(vecino);
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
                    
                    // SOLO IMPRIMIR SI ES UN RÉCORD GLOBAL (Evita el spam en consola)
                    if (costo_actual != -1 && costo_actual < MEJOR_COSTO) {
                        MEJOR_COSTO = costo_actual;
                        MEJOR_SOLUCION = solucion_actual;
                        cout << "    [Restart " << NUM_RESTARTS << "] Nuevo óptimo global encontrado: " << MEJOR_COSTO << endl;
                    }
                }
            }

            NUM_RESTARTS++;
            if (NUM_RESTARTS < 10) {
                SEED += 100; 
                solucion_actual = generarSolucionInicial(); 
            }
        } 
    }

    void escribirSalida(const string& filename, const vector<int>& solucion, const double& tiempo_total) {
        ofstream archivo(filename);
        if (!archivo.is_open()) {
            cerr << "Error al escribir: " << filename << endl;
            return;
        }
        
        archivo << fixed << setprecision(2);
        archivo << "Solución Final " << filename << ":" << endl;
        archivo << "Seed utilizada: " << SEED << endl;
        archivo << "Cantidad de Restarts: " << NUM_RESTARTS << endl;
        archivo << "Costo Total (Penalidad): " << MEJOR_COSTO << endl;
        archivo << "\n--- ORDEN Y TIEMPOS DE ATERRIZAJE (PISTA ÚNICA) ---" << endl;

        // Reconstruimos los tiempos reales para imprimirlos
        vector<double> tiempos_asignados(NUM_AVIONES, 0.0);

        for (int i = 0; i < NUM_AVIONES; i++) {
            int avion_actual = solucion[i];
            double tiempo_liberacion = AVIONES[avion_actual].tiempo_minimo;

            for (int j = 0; j < i; j++) {
                int avion_previo = solucion[j];
                tiempo_liberacion = max(tiempo_liberacion, tiempos_asignados[avion_previo] + COSTOS_S[AVIONES[avion_previo].id][AVIONES[avion_actual].id]);
            }

            double tiempo_real = max(tiempo_liberacion, AVIONES[avion_actual].tiempo_objetivo);
            tiempos_asignados[avion_actual] = tiempo_real;

            // Calculamos la desviación y penalidad específica para mostrarla en el reporte
            double desviacion = 0.0;
            double penalizacion = 0.0;
            
            if (tiempo_real < AVIONES[avion_actual].tiempo_objetivo) {
                desviacion = AVIONES[avion_actual].tiempo_objetivo - tiempo_real;
                penalizacion = desviacion * AVIONES[avion_actual].costo_segundo_temprano;
            } else if (tiempo_real > AVIONES[avion_actual].tiempo_objetivo) {
                desviacion = tiempo_real - AVIONES[avion_actual].tiempo_objetivo;
                penalizacion = desviacion * AVIONES[avion_actual].costo_segundo_tarde;
            }

            // Formato tabulado para fácil lectura
            archivo << "Orden " << setw(2) << i + 1 
                    << " | Avión: " << setw(2) << AVIONES[avion_actual].id 
                    << " | T. Objetivo: " << setw(7) << AVIONES[avion_actual].tiempo_objetivo
                    << " | T. Real: " << setw(7) << tiempo_real 
                    << " | Desvío: " << setw(6) << desviacion << "s"
                    << " | Penalidad: " << penalizacion << endl;
        }

        archivo << "\nTiempo Total de Ejecución: " << tiempo_total << " ms" << endl;        
        archivo.close();
    }
    
    void ejecutarPruebas() {
        cout << fixed << setprecision(2); 
        cout << "Ingrese la cantidad de Restarts: ";
        cin >> RESTARTS_MAXIMOS;
        int cant_semillas;
        cout << "Ingrese el número de semillas: ";
        cin >> cant_semillas;
        vector<int> lista_semillas;

        for (int i = 8; i < 8+cant_semillas; i++) {
            lista_semillas.push_back(pow(2, i));
        }
        
        for (int i = 1; i <= 12; i++) {
        string arch_vuelos = "Una pista/airland" + to_string(i) + ".txt";
        
        cout << "\n=============================================" << endl;
        cout << "Cargando airland" << i << "..." << endl;
        
        // Leemos el archivo de la instancia una sola vez
        leerArchivo(arch_vuelos); 
        
        // Bucle interno: Itera por cada una de las semillas
        for (size_t s = 0; s < lista_semillas.size(); s++) {
            int semilla_base = lista_semillas[s];
            
            // 1. Reiniciar el entorno global para una ejecución limpia
            SEED = semilla_base; 
            NUM_RESTARTS = 0; 
            MEJOR_COSTO = 100000000.0;
            MEJOR_SOLUCION.clear();
            
            cout << "  -> Ejecutando prueba con SEED " << semilla_base << "..." << endl;
            
            // 2. Generar solución inicial e iniciar la búsqueda
            vector<int> solucion_inicial = generarSolucionInicial();
            
            auto inicio_tiempo = chrono::high_resolution_clock::now();            
            HC(solucion_inicial);
            auto fin_tiempo = chrono::high_resolution_clock::now();
            double tiempo_ejecucion_ms = chrono::duration<double, std::milli>(fin_tiempo - inicio_tiempo).count();

            // 4. Formatear el nombre del archivo y escribir los resultados
            string filename = "Output_Unipista/resultados_airland" + to_string(i) + "_SEED" + to_string(semilla_base) + ".txt";
            escribirSalida(filename, MEJOR_SOLUCION, tiempo_ejecucion_ms);
        }
    }
        cout << "\n¡Todas las instancias Unipista procesadas!" << endl;
    }
}