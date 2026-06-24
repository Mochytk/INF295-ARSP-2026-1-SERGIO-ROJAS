#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>
#include <cmath>
#include "estructuras.h"

using namespace std;

namespace Multi {
    // Instanciación de variables globales
    int NUM_AVIONES; 
    int NUM_PISTAS; 
    vector<AvionReal> AVIONES; 
    vector<vector<double>> COSTOS_S; 
    int NUM_PRUEBA;
    int SEED = 256; 
    int NUM_RESTARTS = 0; 
    double TIEMPO_TOTAL = 0; 
    vector<int> MEJOR_SOLUCION;
    double MEJOR_COSTO = 100000000.0;

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
        
        // Historiales independientes para cada pista
        vector<vector<int>> historial_pistas(NUM_PISTAS);
        vector<vector<double>> tiempos_pistas(NUM_PISTAS);

        for (int i = 0; i < NUM_AVIONES; i++) {
            int avion_actual = solucion[i];
            
            int mejor_pista = -1;
            double mejor_tiempo_real = 9999999.0;
            double mejor_costo_avion = 9999999.0;
            
            // El algoritmo evalúa todas las pistas para tomar la decisión más barata
            for (int k = 0; k < NUM_PISTAS; k++) {
                double tiempo_liberacion = 0; // Al no tener E_i, asumimos que puede aterrizar desde t=0
                
                // Revisar la estela SOLO contra los aviones asignados a ESTA pista (k)
                for (size_t p = 0; p < historial_pistas[k].size(); p++) {
                    int avion_previo = historial_pistas[k][p];
                    double tiempo_previo = tiempos_pistas[k][p];
                    tiempo_liberacion = max(tiempo_liberacion, tiempo_previo + COSTOS_S[avion_previo][avion_actual]);
                }
                
                double tiempo_real = max(tiempo_liberacion, AVIONES[avion_actual].tiempo_objetivo);
                
                // Al no tener L_i, la permutación siempre es factible. Calculamos penalidad simétrica.
                double costo_temp = abs(tiempo_real - AVIONES[avion_actual].tiempo_objetivo) * AVIONES[avion_actual].penalidad;
                
                // Nos quedamos con la pista que nos genere el menor costo (o si empatan costo, la que nos deje aterrizar antes)
                if (costo_temp < mejor_costo_avion || (costo_temp == mejor_costo_avion && tiempo_real < mejor_tiempo_real)) { 
                    mejor_tiempo_real = tiempo_real;
                    mejor_pista = k;
                    mejor_costo_avion = costo_temp;
                }
            }
            
            historial_pistas[mejor_pista].push_back(avion_actual);
            tiempos_pistas[mejor_pista].push_back(mejor_tiempo_real);
            costo_total += mejor_costo_avion;
        }
        
        return costo_total;
    }

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
    }

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

        // Generamos la matriz internamente usando la regla fija del ayudante
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

    vector<int> generarSolucionInicial() {
        vector<int> solucion(NUM_AVIONES);
        for (int i = 0; i < NUM_AVIONES; i++) {
            solucion[i] = i; 
        }
        
        double costo_inicial = -1;
        
        while (costo_inicial == -1) {
            mt19937 g(SEED); 
            shuffle(solucion.begin(), solucion.end(), g);
            
            costo_inicial = funcionEvaluacion(solucion);
            if (costo_inicial == -1) {
                SEED += 13; 
            }
        }
        return solucion;
    }
    
    void HC(vector<int> solucion_actual) {
        while (NUM_RESTARTS < 10) {
            bool optimo_local = false;
            double costo_actual = funcionEvaluacion(solucion_actual);
            
            cout << "\n--- Iniciando Restart " << NUM_RESTARTS << " ---" << endl;
            cout << "Costo inicial: " << costo_actual << endl;

            while (!optimo_local) {
                vector<vector<int>> vecindario = generarVecindario(solucion_actual);
                
                vector<int> mejor_vecino = solucion_actual;
                double mejor_costo_vecino = costo_actual;
                bool mejora_encontrada = false;

                for (const auto& vecino : vecindario) {
                    double costo_vecino = funcionEvaluacion(vecino);
                    if (costo_vecino < mejor_costo_vecino) {
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
                    if (costo_actual < MEJOR_COSTO) {
                        MEJOR_COSTO = costo_actual;
                        MEJOR_SOLUCION = solucion_actual;
                        cout << ">>> NUEVO MEJOR COSTO GLOBAL: " << MEJOR_COSTO << " <<<" << endl;
                    }
                }
            }

            NUM_RESTARTS++;
            if (NUM_RESTARTS < 10) {
                SEED += 100; 
                solucion_actual = generarSolucionInicial(); 
            }
        } 
        cout << "\nBúsqueda Terminada. Mejor costo global: " << MEJOR_COSTO << endl;
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
        archivo << "Costo Total: " << MEJOR_COSTO << endl;
        archivo << "\n--- ASIGNACIÓN POR PISTAS ---" << endl;

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

        // Escribimos los resultados estructurados
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
        
        archivo << "\nTiempo Total de Ejecución: " << tiempo_total << endl;
        archivo.close();
    }

    void ejecutarPruebas(int pistas_ingresadas) {
        NUM_PISTAS = pistas_ingresadas; 
        cout << fixed << setprecision(2); 
        int cant_semillas;
        cout << "Ingrese el número de semillas: ";
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
                
                string filename = "Output_Multipista/resultados_FPT" + num_str + "_SEED" + to_string(semilla_base) + ".txt";
                escribirSalida(filename, MEJOR_SOLUCION, tiempo_final - tiempo_inicial);
            }
        }
        cout << "\n¡Todas las instancias Multipista procesadas!" << endl;
    }
}