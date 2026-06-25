# INF295-ARSP-HCBI-2026-1-SERGIO-ROJAS
**Autor:** Sergio Rojas Gutiérrez  
**Asignatura:** Inteligencia Artificial (INF295)  
**Semestre:** 2026-1

## Descripción del Proyecto
Este proyecto implementa un motor algorítmico en C++ para resolver el Aircraft Runway Scheduling Problem. 

Utiliza una metaheurística de **Búsqueda Local (Hill Climbing con Best Improvement y Restarts paramétricos)** para optimizar la secuencia de aterrizaje y minimizar las penalizaciones económicas por desvíos temporales.

El sistema está diseñado para operar bajo dos paradigmas:
1. **Pista Única (Instancias Airland de Beasley):** Evaluación estricta de factibilidad con restricciones duras de tiempo límite de vuelo ($L_i$).
2. **Múltiples Pistas (Instancias FPT Reales):** Escalabilidad de 2 a 5 pistas mediante una política de inserción *Greedy* para balanceo de carga aeroportuaria.

---

## Requisitos del Sistema
Para compilar y ejecutar el proyecto correctamente, el entorno debe contar con:
* **Sistema Operativo:** Linux / Ubuntu o Windows Subsystem for Linux (WSL).
* **Compilador:** `g++` con soporte para el estándar C++11 o superior.
* **Herramientas:** `make` (para compilación automatizada).
* **Python 3:** Exclusivamente para ejecutar el script de extracción y tabulación de datos (`extractor.py`).

---

## Estructura de Directorios
Es fundamental mantener la siguiente jerarquía de carpetas para que las rutas relativas del código funcionen correctamente:

```text
Codigo/
├── main.cpp                  # Punto de entrada y enrutador del sistema
├── estructuras.h             # Definición de structs (Avion, AvionReal) y namespaces
├── unipista.cpp              # Lógica algorítmica para Pista Única
├── multipista.cpp            # Lógica algorítmica para Múltiples Pistas
├── Makefile                  # Script de compilación automatizada
├── extractor_datos_anidados.py # Script de Python para consolidar resultados
├── Una pista/                # Carpeta con las 12 instancias de Beasley (.txt)
├── Múltiples Pistas/         # Carpeta con las 12 instancias FPT (.txt)
├── Output_Unipista/          # Directorio autogenerado para salidas de 1 pista
├── Output_Multipista/        # Directorio autogenerado para salidas de K pistas
└── Resultados Analizados/    # Carpeta destino para el archivo CSV final
```
## Instrucciones de Ejecución
1. Abrir una terminal en la carpeta de Código donde están los archivos `.cpp` y `.h`.
2. Escribir `make` y luego apretar Enter para que se compilen los archivos y se genere un ejecutable.
3. Luego hay que usar el comando `make run` para ejecutar el programa.
4. La terminar irá guiando y pidiendo los datos necesarios para crear las pruebas (Pistas a simular, número máximo de Restarts y número de semillas a evaluar).
5. El programa irá dando feedback por consola sobre el descubrimiento de óptimos locales.
6. Cuando la ejecución termine tendrá los archivos en la Carpeta Output correspondiente, dentro de los archivos se encontrará con los detalles de la ejecución, los tiempos asignados, las penalizaciones, los ordenes por pista, la semilla, la cantidad de restarts y el tiempo total de ejecución de cada prueba.