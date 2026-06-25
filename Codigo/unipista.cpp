/**
 * @file unipista.cpp
 * @brief Motor algorítmico para la resolución del Problema de Aterrizaje de Aeronaves (ALP) en Pista Única.
 * * Implementa la metaheurística Hill Climbing (Mejor Mejora) para las instancias clásicas
 * de Beasley (OR-Library). Esta variante estricta maneja restricciones fuertes de viabilidad
 * (ventanas de tiempo límite L_i) que invalidan porciones sustanciales del espacio de búsqueda.
 */

#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include "estructuras.h"

using namespace std;

namespace Uni {
    // --- Variables Globales del Espacio de Nombres ---
    int NUM_AVIONES;                    // Número total de aviones en la instancia actual
    int HOLGURA;                        // Factor de holgura (no utilizado directamente en la evaluación)
    vector<Avion> AVIONES;              // Vector con la información de las restricciones y penalidades
    vector<vector<double>> COSTOS_S;    // Matriz de tiempos mínimos de separación entre naves (S_ij)
    int NUM_PRUEBA;
    int SEED;                           // Semilla base para el generador estocástico
    int NUM_RESTARTS = 0;               // Contador del número de reinicios actuales
    int RESTARTS_MAXIMOS;               // Límite paramétrico de reinicios (ej. 10 o 20)
    vector<int> MEJOR_SOLUCION;         // La mejor secuencia (permutación) válida encontrada globalmente
    double MEJOR_COSTO = 100000000;     // Costo mínimo asociado a MEJOR_SOLUCION

    /**
     * @brief Operador de movimiento: Intercambia la posición de dos aeronaves en la secuencia.
     */
    void swap(vector<int>& solucion, int i, int j) {
        int temp = solucion[i];
        solucion[i] = solucion[j];
        solucion[j] = temp;
    }

    /**
     * @brief Genera todo el espacio local de soluciones contiguas (vecindario) a partir de un arreglo base.
     * @param solucion Permutación origen.
     * @return vector<vector<int>> Lista de todos los arreglos permutados generados por Swap.
     */
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

    /**
     * @brief Evalúa la viabilidad y el costo de penalización de una secuencia dada.
     * Calcula los tiempos reales de aterrizaje empujando cada aeronave hacia su T_i, 
     * restringido por su ventana E_i y el colchón de seguridad de las naves anteriores.
     * @param solucion Permutación a evaluar.
     * @return double Costo total; devuelve -1 si la solución es estrictamente inviable (excede L_i).
     */
    double funcionEvaluacion(const vector<int>& solucion) {
        double costo_total = 0.0;
        vector<double> tiempos_asignados(NUM_AVIONES, 0.0);

        for (int i = 0; i < NUM_AVIONES; i++) {
            int avion_actual = solucion[i];
            
            // 1. Piso base: El avión no puede aterrizar antes de su Tiempo Mínimo (E_i)
            double tiempo_liberacion = AVIONES[avion_actual].tiempo_minimo; 

            // 2. Muro de separación: Evalúa contra todo el historial para asegurar 
            // que la estela de las naves previas se haya disipado.
            for (int j = 0; j < i; j++) {
                int avion_previo = solucion[j];
                tiempo_liberacion = max(tiempo_liberacion, tiempos_asignados[avion_previo] + COSTOS_S[AVIONES[avion_previo].id][AVIONES[avion_actual].id]);
            }

            // 3. Ajuste al objetivo: Se aterriza lo más cerca posible al Tiempo Objetivo (T_i)
            double tiempo_real = max(tiempo_liberacion, AVIONES[avion_actual].tiempo_objetivo);
            tiempos_asignados[avion_actual] = tiempo_real;

            // 4. Validar factibilidad estricta: Si se excede el Tiempo Máximo (L_i), la secuencia es inservible
            if (tiempo_real > AVIONES[avion_actual].tiempo_maximo) {
                return -1; 
            }

            // 5. Cálculo asimétrico de las penalizaciones económicas
            if (tiempo_real < AVIONES[avion_actual].tiempo_objetivo) {
                costo_total += (AVIONES[avion_actual].tiempo_objetivo - tiempo_real) * AVIONES[avion_actual].costo_segundo_temprano;
            } else if (tiempo_real > AVIONES[avion_actual].tiempo_objetivo) {
                costo_total += (tiempo_real - AVIONES[avion_actual].tiempo_objetivo) * AVIONES[avion_actual].costo_segundo_tarde;
            }
        }
        
        return costo_total;
    }

    /**
     * @brief Parsea los archivos de texto estructurados de las instancias de Beasley.
     * Carga las variables paramétricas de cada vuelo y la matriz de separación específica de esa instancia.
     */
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
            archivo >> AVIONES[i].tiempo_llegada
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

    /**
     * @brief Método asistido para inicializar la heurística en un estado factible.
     * Debido a la agresividad de L_i, una secuencia 100% aleatoria suele ser inviable.
     * Este método pre-ordena los aviones y luego aplica perturbaciones leves (swaps controlados) 
     * garantizando un punto de partida válido.
     */
    vector<int> generarSolucionInicial() {
        vector<int> solucion(NUM_AVIONES);
        for (int i = 0; i < NUM_AVIONES; i++) {
            solucion[i] = i; 
        }
        
        // Ordenamiento Voraz Asistido: Da prioridad a los aviones con ventanas de tiempo más críticas (L_i corto)
        sort(solucion.begin(), solucion.end(), [](int a, int b) {
            return AVIONES[a].tiempo_maximo < AVIONES[b].tiempo_maximo;
        });

        double costo_inicial = -1;
        mt19937 g(SEED); 
        uniform_int_distribution<int> dist(0, NUM_AVIONES - 1);
        
        int intentos = 0;

        // Bucle de diversificación: Intenta encontrar una variación aleatoria factible de la lista ordenada
        while (costo_inicial == -1) {
            vector<int> candidata = solucion; 
            
            // Perturbación estocástica controlada (10% de swaps)
            int num_swaps = max(1, NUM_AVIONES / 10);
            
            // Retroceso heurístico: Si la aleatoriedad induce mucha infactibilidad, se reduce el ruido
            if (intentos > 500) num_swaps = 1; 
            if (intentos > 1000) num_swaps = 0; // Fallback: Retorna la secuencia ordenada de forma determinista

            for (int k = 0; k < num_swaps; k++) {
                int i = dist(g);
                int j = dist(g);
                int temp = candidata[i];
                candidata[i] = candidata[j];
                candidata[j] = temp;
            }
            
            costo_inicial = funcionEvaluacion(candidata);
            
            if (costo_inicial != -1) {
                return candidata; // Semilla validada y lista para la explotación
            }
            
            intentos++;
        }
        
        return solucion;
    }

    /**
     * @brief Motor iterativo de Búsqueda Local (Hill Climbing).
     * Navega los valles de la función objetivo aceptando siempre el mejor vecino generado,
     * siempre y cuando este último sea una solución matemáticamente factible (!= -1).
     */
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
                    // Control crítico: Solo evalúa vecinos que cumplan estrictamente con las ventanas de tiempo
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
                    
                    // Registro global de récords con control de spam para la salida de consola
                    if (costo_actual != -1 && costo_actual < MEJOR_COSTO) {
                        MEJOR_COSTO = costo_actual;
                        MEJOR_SOLUCION = solucion_actual;
                        cout << "    [Restart " << NUM_RESTARTS << "] Nuevo óptimo global encontrado: " << MEJOR_COSTO << endl;
                    }
                }
            }

            NUM_RESTARTS++;
            if (NUM_RESTARTS < 10) {
                SEED += 100; // Modifica la aleatoriedad para la siguiente solución inicial
                solucion_actual = generarSolucionInicial(); 
            }
        } 
    }

    /**
     * @brief Compila el resultado definitivo, regenerando los tiempos reales, y serializa 
     * los resultados en un reporte analítico por archivo.
     */
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

            double desviacion = 0.0;
            double penalizacion = 0.0;
            
            if (tiempo_real < AVIONES[avion_actual].tiempo_objetivo) {
                desviacion = AVIONES[avion_actual].tiempo_objetivo - tiempo_real;
                penalizacion = desviacion * AVIONES[avion_actual].costo_segundo_temprano;
            } else if (tiempo_real > AVIONES[avion_actual].tiempo_objetivo) {
                desviacion = tiempo_real - AVIONES[avion_actual].tiempo_objetivo;
                penalizacion = desviacion * AVIONES[avion_actual].costo_segundo_tarde;
            }

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
    
    /**
     * @brief Punto de entrada maestro. Orquesta las 12 iteraciones sobre las instancias
     * Unipista, administra la creación de los entornos estocásticos y controla la 
     * métrica de alto rendimiento basada en \texttt{<chrono>}.
     */
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
            
            leerArchivo(arch_vuelos); 
            
            for (size_t s = 0; s < lista_semillas.size(); s++) {
                int semilla_base = lista_semillas[s];
                
                SEED = semilla_base; 
                NUM_RESTARTS = 0; 
                MEJOR_COSTO = 100000000.0;
                MEJOR_SOLUCION.clear();
                
                cout << "  -> Ejecutando prueba con SEED " << semilla_base << "..." << endl;
                
                vector<int> solucion_inicial = generarSolucionInicial();
                
                auto inicio_tiempo = chrono::high_resolution_clock::now();            
                HC(solucion_inicial);
                auto fin_tiempo = chrono::high_resolution_clock::now();
                double tiempo_ejecucion_ms = chrono::duration<double, std::milli>(fin_tiempo - inicio_tiempo).count();

                string filename = "Output_Unipista/resultados_airland" + to_string(i) + "_SEED" + to_string(semilla_base) + ".txt";
                escribirSalida(filename, MEJOR_SOLUCION, tiempo_ejecucion_ms);
            }
        }
        cout << "\n¡Todas las instancias Unipista procesadas!" << endl;
    }
}