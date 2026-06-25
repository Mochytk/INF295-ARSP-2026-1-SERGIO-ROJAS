import os
import re
import csv

# Detecta automáticamente la carpeta exacta donde este script está guardado
base_dir = os.path.dirname(os.path.abspath(__file__))

# Expresiones regulares para capturar los datos
regex_seed = re.compile(r"Seed utilizada:\s*(\d+)")
regex_restarts = re.compile(r"Cantidad de Restarts:\s*(\d+)")
regex_costo = re.compile(r"Costo Total.*:\s*([\d.]+)")
regex_tiempo = re.compile(r"Tiempo Total de Ejecución:\s*([\d.]+)")

datos = []

print(f"Iniciando la extracción en la carpeta: {base_dir}...")

# os.walk recorre automáticamente todas las subcarpetas sin importar su nombre
for root, dirs, files in os.walk(base_dir):
    for filename in files:
        if filename.endswith('.txt') and filename.startswith('resultados_'):
            filepath = os.path.join(root, filename)
            
            # 1. Extraer contexto de la ruta (Pistas y Configuración)
            ruta_limpia = filepath.replace('\\', '/')
            partes_ruta = ruta_limpia.split('/')
            
            num_pistas = "1"
            configuracion = "Desconocida"
            
            for parte in partes_ruta:
                if "pista" in parte.lower():
                    m = re.search(r'(\d+)\s*[Pp]ista', parte)
                    if m: num_pistas = m.group(1)
                if "semilla" in parte.lower() and "restart" in parte.lower():
                    configuracion = parte
            
            tipo_instancia = 'Unipista' if num_pistas == "1" else 'Multipista'
            
            # 2. Extraer el nombre de la instancia
            match_instancia = re.search(r'resultados_(.*)_SEED', filename)
            instancia = match_instancia.group(1) if match_instancia else "Desconocida"
            
            seed = ""
            restarts = ""
            costo = ""
            tiempo = ""
            
            # 3. Leer el contenido del archivo
            with open(filepath, 'r', encoding='utf-8') as f:
                for line in f:
                    if not seed:
                        m = regex_seed.search(line)
                        if m: seed = m.group(1)
                    if not restarts:
                        m = regex_restarts.search(line)
                        if m: restarts = m.group(1)
                    if not costo:
                        m = regex_costo.search(line)
                        if m: costo = m.group(1)
                    if not tiempo:
                        m = regex_tiempo.search(line)
                        if m: tiempo = m.group(1)
            
            
            datos.append({
                'Tipo': tipo_instancia,
                'Num_Pistas': num_pistas,
                'Configuracion': configuracion,
                'Instancia': instancia,
                'Semilla': seed,
                'Restarts_Reales': restarts,
                'Costo_Total': costo,
                'Tiempo': tiempo
            })

# Función para ordenar lógicamente
def sort_key(x):
    match = re.match(r"([a-zA-Z]+)(\d+)", x['Instancia'])
    inst_name = match.group(1) if match else x['Instancia']
    inst_num = int(match.group(2)) if match else 0
    
    return (
        x['Tipo'], 
        int(x['Num_Pistas']), 
        x['Configuracion'], 
        inst_name, 
        inst_num, 
        int(x['Semilla'] if x['Semilla'] else 0)
    )

datos.sort(key=sort_key)

# Generar el archivo CSV exactamente en la misma carpeta del script
csv_file = os.path.join(base_dir, 'Resultados_Experimentos_Full.csv')

with open(csv_file, 'w', newline='', encoding='utf-8') as f:
    columnas = ['Tipo', 'Num_Pistas', 'Configuracion', 'Instancia', 'Semilla', 'Restarts_Reales', 'Costo_Total', 'Tiempo']
    writer = csv.DictWriter(f, fieldnames=columnas, delimiter=';')
    writer.writeheader()
    writer.writerows(datos)

print(f"\n¡Éxito total! Se procesaron {len(datos)} archivos.")
print(f"El Excel consolidado te espera en: {csv_file}")