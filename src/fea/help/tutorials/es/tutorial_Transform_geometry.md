# Transformar la geometría

Este tutorial es la continuación del tutorial **Dibujar un cubo** y muestra cómo escalar, rotar y trasladar toda la geometría o solo algunas de sus partes.

## 1. Cargar el modelo

Si el modelo **Cube** aún no está cargado, puede cargarse con la acción de menú **_Abrir modelo_**.

**Menú:** _Archivo -> Abrir modelo_

Se abrirá la ventana **Abrir modelo**. Seleccione el archivo **Cube.rbm** y haga clic en **Abrir**.

Una vez preparado el modelo, su geometría (completa o en parte) puede modificarse (transformarse).

## 2. Preparación

**Menú:** _Geometría -> Escalar, trasladar, rotar_

Una vez activado el elemento de menú, en el lado derecho de la ventana principal aparece el panel **Transformar geometría**, con tres pestañas: **Escala**, **Rotar** y **Trasladar**.

![Transformar geometría](image-Transform_geometry.png)

En este tutorial todas las transformaciones se aplicarán sucesivamente a una cara del cubo.

En este momento todas las caras del modelo **Cube** están agrupadas en la misma **entidad de superficie**. Por ello, una cara debe **marcarse** como **entidad de superficie** independiente, para que las transformaciones se apliquen solo a esa cara y no a las demás. Esto se hace con la acción de menú **_Marcar superficie_**. Pero antes hay que señalar la cara del cubo.

## 3. Señalar una cara

Mantenga pulsada la tecla **_Ctrl_** (**_⌘ Cmd_** en macOS) y haga clic con el **_botón izquierdo del ratón_** sobre una cara del cubo. El elemento señalado se resaltará. Como el modelo **Cube** está formado por 6 elementos rectangulares (cuadriláteros), un elemento corresponde a una cara del **cubo**.

## 4. Marcar una superficie

**Menú:** _Geometría -> Superficie -> Marcar superficie_

![Ventana Marcar superficie](image-Mark_surface_dialog.png)

Esta acción abre la ventana **Marcar entidad (superficie)**. Como en el paso anterior se **señaló** un elemento, la opción **_Marcar solo los elementos seleccionados y relacionados_** aparece preseleccionada. Al hacer clic en **Aceptar**, una cara del modelo **Cube** quedará marcada como **entidad de superficie** independiente.

_**Nota:** cualquier entidad se puede renombrar haciendo **doble clic** con el **botón izquierdo del ratón** sobre su nombre en el **árbol del modelo**, en el lado izquierdo de la ventana principal._

![Superficie marcada](image-Marked_surface.png)

## 5. Escalar la entidad señalada (cara del cubo)

Señale la superficie marcada (**_Ctrl_** / **_⌘ Cmd_** + **_botón izquierdo del ratón_**) y abra el panel **Transformar geometría**.

**Menú:** _Geometría -> Escalar, trasladar, rotar_

1. Marque **Misma escala en todas las direcciones**.
2. Ajuste el valor de **Escala** a 0.5 (use el separador decimal de su sistema, p. ej. 0,5).
3. Haga clic en el botón **Definir a partir del elemento/nodo señalado**.
4. En el grupo **Aplicar a**, seleccione **Entidades señaladas**.
5. Compruebe que **Incluir nodos compartidos** está marcado, así como **todas** sus casillas subordinadas.

La configuración correcta puede verse en la siguiente captura de pantalla.

![Transformar geometría – escala](image-Transform_geometry-scale.png)

Haga clic en **Aceptar**. El resultado puede verse en la siguiente captura de pantalla.

![Superficie escalada](image-Scaled_surface.png)

## 6. Rotar la entidad señalada (cara del cubo escalada)

Rotando la superficie señalada se puede crear una superficie extruida. Abra el panel **Transformar geometría**.

**Menú:** _Geometría -> Escalar, trasladar, rotar_

Cambie a la pestaña **Rotar** e introduzca los valores como en la siguiente captura de pantalla:

1. En **Ángulos de rotación**, ajuste **X** a -90.
2. Haga clic en el botón **Definir a partir del elemento/nodo señalado** y después cambie **Z** del **Centro de rotación** a 0.
3. En el grupo **Aplicar a**, seleccione **Entidades señaladas**.
4. Compruebe que **Incluir nodos compartidos**, **Separar/dividir nodos compartidos** y **Barrer nodos** están marcados y ajuste **Pasos de barrido** a 6.

_Nota: **Pasos de barrido: 6** produce seis segmentos nuevos._

![Transformar geometría – rotación](image-Transform_geometry-rotate.png)

Haga clic en **Aceptar**. El resultado puede verse en la siguiente captura de pantalla.

![Superficie rotada](image-Rotated_surface.png)
