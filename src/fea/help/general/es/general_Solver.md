# Solver

El **solver** es una herramienta que procesa la configuración del **problema** y calcula los **resultados** sobre un **modelo** dado.

Según el tamaño del **modelo**, el **tipo de problema**, la configuración de convergencia y la configuración transitoria, el tiempo necesario para resolver el **problema** puede variar desde segundos hasta horas o más.

## Inicio del solver

**Menú:** _Solución -> Iniciar solver_

El modelo debe estar guardado antes de que el solver pueda iniciarse. Si aún no se ha guardado, se muestra primero el diálogo **Guardar modelo**. A continuación se muestra el diálogo **Iniciar solver**.

![Iniciar solver](image-Start_solver.png)

Antes de iniciar el solver se comprueba la configuración del problema. Los **Errores** (p. ej., una entidad sin material) y las **Advertencias** encontrados se enumeran en el diálogo **Iniciar solver**. Si el modelo ya contiene resultados, se puede marcar **Reiniciar el solver / continuar** para continuar a partir de esos resultados en lugar de empezar desde el principio.

El solver se ejecuta en segundo plano como un proceso independiente, por lo que se puede seguir trabajando mientras calcula. Toda su salida se muestra en la pestaña **Salida del proceso**, debajo del área 3D. Cuando el solver termina, los **resultados** calculados se cargan automáticamente en el modelo.

## Detención del solver

- _Solución -> Detener solver_ – pide a todos los solvers en ejecución que se detengan.
- _Solución -> Forzar la terminación del solver_ – termina inmediatamente el proceso del solver.

## Seguimiento del solver

- _Informe -> Convergencia del solver_ – muestra un gráfico de la convergencia del solver.
- _Informe -> Puntos de monitorización_ – muestra los valores calculados en los puntos de monitorización definidos mediante _Problema -> Definir puntos de monitorización_.
- _Informe -> Archivo de registro del solver_ – muestra el archivo de registro del solver.
