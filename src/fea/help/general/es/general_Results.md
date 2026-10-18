# Resultados

Los **resultados** los produce el **solver** y están formados por **variables de resultado**.

## Variable de resultado

Una **variable de resultado** es un conjunto de valores calculados por el **solver**. Una **variable de resultado** se define por su **tipo** (por ejemplo, **temperatura** o **desplazamiento**) y un vector de valores. Según su **tipo**, una **variable de resultado** puede contener un vector de valores escalares o de valores vectoriales.


Por ejemplo, la variable de resultado **temperatura** contiene un vector de temperaturas, una por nodo, mientras que el **desplazamiento** contiene un vector de vectores de desplazamiento, uno por nodo.


Una **variable de resultado** se puede calcular a nivel de nodo o a nivel de elemento. Por ejemplo, el **desplazamiento** se calcula a nivel de nodo, mientras que la **tensión** se calcula a nivel de elemento. Toda variable calculada a nivel de elemento se puede convertir a nivel de nodo. Esta conversión es una operación única y no se puede deshacer; para volver atrás, el **solver** debe recalcular los resultados.
