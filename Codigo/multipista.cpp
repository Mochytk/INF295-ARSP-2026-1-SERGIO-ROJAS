/**
 * @file multipista.cpp
 * @brief Motor algorítmico para la resolución del Problema de Aterrizaje de Aeronaves (ALP) en Múltiples Pistas.
 * * Este archivo implementa una metaheurística Hill Climbing con Best Improvement (HC-BI).
 * Utiliza instancias basadas en tráfico real (FPT) y emplea una estrategia Greedy dentro de 
 * la función de evaluación para asignar cada aeronave de la secuencia a la pista que 
 * minimice su costo de penalización.
 */

#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <chrono>
#include "estructuras.h"

using namespace std;

namespace Multi {
    // --- Variables Globales del Espacio de Nombres ---
    int NUM_AVIONES;                    // Cantidad total de aeronaves en la instancia actual
    int NUM_PISTAS;                     // Número de pistas operativas para la simulación (K)
    vector<AvionReal> AVIONES;          // Lista con los datos de cada aeronave
    vector<vector<double>> COSTOS_S;    // Matriz asimétrica de tiempos de separación por turbulencia (S_ij)
    int NUM_PRUEBA;
    int SEED = 256;                     // Semilla base para el generador de números aleatorios
    int NUM_RESTARTS = 0;               // Contador actual de reinicios en el Hill Climbing
    int RESTARTS_MAXIMOS;               // Límite paramétrico de reinicios (ej. 10 o 20)
    double TIEMPO_TOTAL = 0;            // (No utilizada directamente, el tiempo se mide con chrono localmente)
    vector<int> MEJOR_SOLUCION;         // Arreglo de permutación que representa la mejor secuencia histórica
    double MEJOR_COSTO = 100000000.0;   // Valor de costo asociado a la mejor solución global encontrada

    /**
     * @brief Operador de movimiento elemental. Intercambia dos posiciones en una secuencia.
     */
    void swap(vector<int>& solucion, int i, int j) {
        int temp = solucion[i];
        solucion[i] = solucion[j];
        solucion[j] = temp;
    }

    /**
     * @brief Genera el vecindario completo a partir de una solución base utilizando el operador Swap.
     * @param solucion Secuencia de aterrizaje actual.
     * @return vector<vector<int>> Matriz con todas las combinaciones posibles de vecindarios.
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
     * @brief Función central de evaluación. Calcula el costo total de penalización de una secuencia dada.
     * Implementa una lógica Greedy: Para cada avión en la secuencia, revisa todas las pistas y lo 
     * asigna a aquella que le genere el menor costo o, en caso de empate, el aterrizaje más temprano.
     * @param solucion Permutación que representa el orden de llegada.
     * @return double Costo total de penalización de toda la secuencia.
     */
    double funcionEvaluacion(const vector<int>& solucion) {
        double costo_total = 0.0;
        
        // Historiales independientes para monitorear el estado y la estela de cada pista
        vector<vector<int>> historial_pistas(NUM_PISTAS);
        vector<vector<double>> tiempos_pistas(NUM_PISTAS);

        for (int i = 0; i < NUM_AVIONES; i++) {
            int avion_actual = solucion[i];
            
            int mejor_pista = -1;
            double mejor_tiempo_real = 9999999.0;
            double mejor_costo_avion = 9999999.0;
            
            // Evalúa el costo de insertar el avión actual en cada pista disponible
            for (int k = 0; k < NUM_PISTAS; k++) {
                double tiempo_liberacion = 0; // Se asume E_i = 0 (aterrizaje posible desde t=0)
                
                // Calcular el tiempo seguro basado en la estela de los aviones previamente asignados a esta pista (k)
                for (size_t p = 0; p < historial_pistas[k].size(); p++) {
                    int avion_previo = historial_pistas[k][p];
                    double tiempo_previo = tiempos_pistas[k][p];
                    tiempo_liberacion = max(tiempo_liberacion, tiempo_previo + COSTOS_S[avion_previo][avion_actual]);
                }
                
                // El tiempo real es el máximo entre el tiempo seguro de pista y el tiempo en que el avión desea aterrizar (T_i)
                double tiempo_real = max(tiempo_liberacion, AVIONES[avion_actual].tiempo_objetivo);
                
                // Cálculo de la penalidad absoluta y simétrica
                double costo_temp = abs(tiempo_real - AVIONES[avion_actual].tiempo_objetivo) * AVIONES[avion_actual].penalidad;
                
                // Política Greedy: Guardar la pista que resulte en menor costo o menor tiempo ante empates
                if (costo_temp < mejor_costo_avion || (costo_temp == mejor_costo_avion && tiempo_real < mejor_tiempo_real)) { 
                    mejor_tiempo_real = tiempo_real;
                    mejor_pista = k;
                    mejor_costo_avion = costo_temp;
                }
            }
            
            // Consolidar la asignación en la mejor pista encontrada
            historial_pistas[mejor_pista].push_back(avion_actual);
            tiempos_pistas[mejor_pista].push_back(mejor_tiempo_real);
            costo_total += mejor_costo_avion;
        }
        
        return costo_total;
    }

    /**
     * @brief Devuelve la separación mínima de seguridad en base a las categorías de turbulencia de estela.
     * @param prev Avión que aterrizó antes en la misma pista.
     * @param act Avión que intenta aterrizar.
     * @return double Tiempo de separación en segundos.
     */
    double obtenerSeparacionFija(const AvionReal& prev, const AvionReal& act) {
        if (prev.categoria_estela == 'H' && act.categoria_estela == 'H') return 90.0;
        if (prev.categoria_estela == 'H' && act.categoria_estela == 'M') return 120.0;
        if (prev.categoria_estela == 'H' && act.categoria_estela == 'L') return 150.0;
        if (prev.categoria_estela == 'M' && act.categoria_estela == 'H') return 60.0;
        if (prev.categoria_estela == 'M' && act.categoria_estela == 'M') return 80.0;
        if (prev.categoria_estela == 'M' && act.categoria_estela == 'L') return 100.0;
        if (prev.categoria_estela == 'L' && act.categoria_estela == 'H') return 60.0;
        if (prev.categoria_estela == 'L' && act.categoria_estela == 'M') return 60.0;
        if (prev.categoria_estela == 'L' && act.categoria_estela == 'L') return 60.0;
        return 60.0; // Fallback por defecto
    }

    /**
     * @brief Procesa el archivo de texto de la instancia y carga los datos en estructuras de memoria.
     * Calcula dinámicamente la matriz de separación estática COSTOS_S.
     */
    void leerArchivo(const string& filename_vuelos){
        ifstream arch_vuelos(filename_vuelos);
        if (!arch_vuelos.is_open()) {
            cerr << "Error al abrir vuelos: " << filename_vuelos << endl;
            exit(1);
        }
        
        arch_vuelos >> NUM_AVIONES;
        AVIONES.resize(NUM_AVIONES);
        
        for(int i = 0; i < NUM_AVIONES; i++) {
            AVIONES[i].id = i;
            arch_vuelos >> AVIONES[i].codigo_vuelo 
                        >> AVIONES[i].modelo 
                        >> AVIONES[i].categoria_estela 
                        >> AVIONES[i].operacion 
                        >> AVIONES[i].tiempo_objetivo 
                        >> AVIONES[i].penalidad;
        }
        arch_vuelos.close();

        // Generación de la matriz de turbulencia
        COSTOS_S.assign(NUM_AVIONES, vector<double>(NUM_AVIONES, 0.0));
        for(int i = 0; i < NUM_AVIONES; i++) {
            for(int j = 0; j < NUM_AVIONES; j++) {
                if (i == j) {
                    COSTOS_S[i][j] = 0.0; // Distancia de un avión consigo mismo
                } else {
                    COSTOS_S[i][j] = obtenerSeparacionFija(AVIONES[i], AVIONES[j]);
                }
            }
        }
    }

    /**
     * @brief Crea una permutación inicial factible mezclando de forma aleatoria los IDs de los aviones.
     * (Al no haber ventanas L_i, cualquier secuencia generada aleatoriamente es factible por defecto).
     */
    vector<int> generarSolucionInicial() {
        vector<int> solucion(NUM_AVIONES);
        for (int i = 0; i < NUM_AVIONES; i++) {
            solucion[i] = i; 
        }
        
        double costo_inicial = -1;
        
        while (costo_inicial == -1) {
            mt19937 g(SEED); 
            shuffle(solucion.begin(), solucion.end(), g); // Diversificación inicial
            
            costo_inicial = funcionEvaluacion(solucion);
            if (costo_inicial == -1) {
                SEED += 13; 
            }
        }
        return solucion;
    }
    
    /**
     * @brief Motor principal de búsqueda metaheurística: Hill Climbing - Best Improvement.
     * Explora exhaustivamente todo el vecindario y adopta el vecino que brinde la mayor mejora.
     * Continúa iterando hasta estancarse en un óptimo local, tras lo cual reinicia el proceso
     * alterando la semilla para explorar nuevas regiones.
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

                // Evaluación exhaustiva de todos los vecinos generados por Swap
                for (const auto& vecino : vecindario) {
                    double costo_vecino = funcionEvaluacion(vecino);
                    if (costo_vecino < mejor_costo_vecino) {
                        mejor_vecino = vecino;
                        mejor_costo_vecino = costo_vecino;
                        mejora_encontrada = true;
                    }
                }

                if (mejora_encontrada) {
                    // Actualización de estado si se detecta una trayectoria descendente (mejora)
                    solucion_actual = mejor_vecino;
                    costo_actual = mejor_costo_vecino;
                } else {
                    // Se alcanza el fondo de un valle (óptimo local)
                    optimo_local = true;
                    // Verificación contra la memoria histórica global
                    if (costo_actual < MEJOR_COSTO) {
                        MEJOR_COSTO = costo_actual;
                        MEJOR_SOLUCION = solucion_actual;
                        cout << ">>> " << "[Restart " << NUM_RESTARTS<<"] NUEVO MEJOR COSTO GLOBAL: " << MEJOR_COSTO << " <<<" << endl;
                    }
                }
            }

            // Gestión paramétrica de Reinicios
            NUM_RESTARTS++;
            if (NUM_RESTARTS < RESTARTS_MAXIMOS) {
                SEED += 100; // Alteración del estado del generador para asegurar diversidad
                solucion_actual = generarSolucionInicial(); 
            }
        } 
        cout << "\nBúsqueda Terminada. Mejor costo global: " << MEJOR_COSTO << endl;
    }

    /**
     * @brief Formatea e imprime los resultados, incluyendo métricas paramétricas y un desglose 
     * detallado de las operaciones aeronáuticas asignadas a cada pista.
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
        archivo << "Costo Total: " << MEJOR_COSTO << endl;
        archivo << "\n--- ASIGNACIÓN POR PISTAS ---" << endl;

        // Se re-evalúa la solución para extraer la logística de separación final a imprimir
        vector<vector<int>> historial_pistas(NUM_PISTAS);
        vector<vector<double>> tiempos_pistas(NUM_PISTAS);

        for (int i = 0; i < NUM_AVIONES; i++) {
            int avion_actual = solucion[i];
            int mejor_pista = -1;
            double mejor_tiempo_real = 9999999.0;
            double mejor_costo_avion = 9999999.0;
            
            for (int k = 0; k < NUM_PISTAS; k++) {
                double tiempo_liberacion = 0; 
                for (size_t p = 0; p < historial_pistas[k].size(); p++) {
                    int avion_previo = historial_pistas[k][p];
                    tiempo_liberacion = max(tiempo_liberacion, tiempos_pistas[k][p] + COSTOS_S[avion_previo][avion_actual]);
                }
                double tiempo_real = max(tiempo_liberacion, AVIONES[avion_actual].tiempo_objetivo);
                double costo_temp = abs(tiempo_real - AVIONES[avion_actual].tiempo_objetivo) * AVIONES[avion_actual].penalidad;
                
                if (costo_temp < mejor_costo_avion || (costo_temp == mejor_costo_avion && tiempo_real < mejor_tiempo_real)) { 
                    mejor_tiempo_real = tiempo_real;
                    mejor_pista = k;
                    mejor_costo_avion = costo_temp;
                }
            }
            historial_pistas[mejor_pista].push_back(avion_actual);
            tiempos_pistas[mejor_pista].push_back(mejor_tiempo_real);
        }

        // Escribimos los resultados estructurados segmentados por pista de aterrizaje
        for (int k = 0; k < NUM_PISTAS; k++) {
            archivo << "\n>> PISTA " << k + 1 << " <<" << endl;
            for (size_t p = 0; p < historial_pistas[k].size(); p++) {
                int id = historial_pistas[k][p];
                archivo << "Vuelo: " << AVIONES[id].codigo_vuelo 
                        << " \t| Mod: " << AVIONES[id].modelo
                        << " \t| T.Obj: " << AVIONES[id].tiempo_objetivo
                        << " \t| T.Real: " << tiempos_pistas[k][p] 
                        << " \t| Desvío: " << abs(tiempos_pistas[k][p] - AVIONES[id].tiempo_objetivo) << "s" << endl;
            }
        }
        
        archivo << "\nTiempo Total de Ejecución: " << tiempo_total << " ms" << endl;
        archivo.close();
    }

    /**
     * @brief Orquestador central de las pruebas. Itera sistemáticamente a través de las 12
     * instancias FPT aplicando variaciones paramétricas estocásticas e inyectando temporizadores 
     * de alta resolución.
     */
    void ejecutarPruebas(int pistas_ingresadas) {
        NUM_PISTAS = pistas_ingresadas; 
        cout << fixed << setprecision(2); 
        
        // Petición interactiva de hiperparámetros
        cout << "Ingrese la cantidad de Restarts: ";
        cin >> RESTARTS_MAXIMOS;
        int cant_semillas;
        cout << "Ingrese el número de semillas: ";
        cin >> cant_semillas;

        // Generación del conjunto robusto de semillas basado en potencias de 2
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
                
                // Reset crítico del estado global por cada ensayo experimental
                SEED = semilla_base; 
                NUM_RESTARTS = 0; 
                MEJOR_COSTO = 100000000.0;
                MEJOR_SOLUCION.clear();
                
                cout << "  -> Ejecutando prueba con SEED " << semilla_base << "..." << endl;
                
                vector<int> solucion_inicial = generarSolucionInicial();
                
                // Encapsulamiento con reloj de alta precisión (ms)
                auto inicio_tiempo = chrono::high_resolution_clock::now();
                HC(solucion_inicial);
                auto fin_tiempo = chrono::high_resolution_clock::now();
                
                double tiempo_ejecucion_ms = chrono::duration<double, std::milli>(fin_tiempo - inicio_tiempo).count();
                
                // Serialización dinámica de salidas para facilitar el post-procesamiento con scripts
                string filename = "Output_Multipista/resultados_FPT" + num_str + "_SEED" + to_string(SEED) + "_PISTAS=" + to_string(NUM_PISTAS) + ".txt";
                escribirSalida(filename, MEJOR_SOLUCION, tiempo_ejecucion_ms);
            }
        }
        cout << "\n¡Todas las instancias Multipista procesadas!" << endl;
    }
}