# shell

### Búsqueda en $PATH
#### Responder: ¿cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?

Las diferencias de la syscall execve(2) con los wrappers de exec(3) es que execve(2) realiza una llamada directa al sistema para reemplazar el proceso actual con uno nuevo, tomando los siguientes argumentos:

pathname: Especifica la ruta al archivo ejecutable que deseas ejecutar. Puede ser una ruta absoluta (comenzando desde el directorio raíz) o una ruta relativa (relativa al directorio actual).

argv[] : Este parámetro es un array de cadenas de strings que representan los argumentos de línea de comandos pasados al nuevo programa. Convencionalmente, se termina con un puntero NULL. El primer elemento (argv[0]) generalmente representa el nombre del programa que se está ejecutando. Los elementos subsiguientes serían argv[1], argv[2], etc.

envp[]: Este parámetro es un array de cadenas de strings que representan las variables de entorno pasadas al nuevo programa. Al igual que argv, se termina con un puntero NULL. Cada cadena en este array tiene el formato "key=value", donde variable es el nombre de la variable de entorno y valor es su valor.

execve(2) es más bajo nivel y flexible, pero requiere más trabajo manual para configurar todos los argumentos y el entorno correctamente.
En cambio,las funciones de la familia exec(3) proporcionan una interfaz más fácil de usar pues simplifica el uso de execve(2), por ejemplo, proporcionando variaciones más fáciles de usar para pasar argumentos y manejar entornos, además de realizar la búsqueda del programa ejecutable en el PATH. 


#### Responder: ¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso? 

La llamada exec(3) puede fallar por varias razones, como la incapacidad de encontrar el archivo ejecutable especificado, permisos insuficientes para ejecutar el archivo, o errores relacionados con el archivo referido por el pathname porque está siendo escrito por otro proceso. Cuando una llamada a exec(3) falla, la función devuelve -1 y establece la variable errno para indicar el tipo de error que ocurrió. En nuestro caso, la shell ante un caso de error, realiza un exit(-1) e imprime el error por pantalla y espera el proximo comando.

---

### Procesos en segundo plano

#### Explicar detalladamente el mecanismo completo utilizado. 

Para implementar un manejo y liberación de recursos en procesos de segundo plano, se creo la función sigchld_handler. Esta función se encarga de notificar cuando un proceso hijo ha finalizado, mostrando un mensaje en la pantalla. Posteriormente, se vinculo esta función a una estructura sigaction y se utilizo la syscall sigaction(2) con la señal SIGCHLD, que notifica al proceso padre sobre la terminación de sus procesos hijos. Además, en los casos donde no se están ejecutando comandos en segundo plano, se modifico el grupo de procesos (pgid) del proceso hijo para evitar que el manejador se active innecesariamente.


#### Responder: ¿Por qué es necesario el uso de señales?

En primer lugar, permiten que un proceso padre, sea notificado cuando un proceso hijo concluye su ejecución. Esta funcionalidad es esencial para mantener un control efectivo sobre los procesos en segundo plano y su estado. Sin el uso de señales, la shell tendría que esperar activamente para comprobar si los procesos hijos han terminado, lo que sería ineficiente.

Además, al finalizar un proceso en segundo plano, es necesario liberar su entrada, salida estándar y cualquier recurso asignado. Las señales permiten gestionar esta liberación de manera oportuna, justo después de que el proceso ha terminado. Gracias al uso de señales, la shell puede configurar un manejador personalizado que gestione específicamente las señales recibidas al concluir los procesos hijos, garantizando así un manejo eficiente de los recursos y una mejor administración del sistema.

---

### Flujo estándar

#### Responder: Investigar el significado de 2>&1, explicar cómo funciona su forma general. 
- Investigar el significado de 2>&1, explicar cómo funciona su forma general
"2>&1" es una forma de redirigir el flujo de salida de error estándar (stderr) hacia el mismo destino que la salida estándar (stdout). El "2" hace referencia al canal "stderr" y el "&1" indica que la salida de error estándar debe redirigirse al mismo destino que la salida estándar.

- Mostrar qué sucede con la salida de cat out.txt en el ejemplo.
![cat_out_2_&1.png](https://github.com/fiubatps/sisop_2024b_g34/blob/entrega_shell/shell/cat_out_2_%261.png)

- Luego repetirlo, invirtiendo el orden de las redirecciones (es decir, 2>&1 >out.txt). ¿Cambió algo? Compararlo con el comportamiento en bash(1).
En el comando 2>&1 >out.txt, primero se redirige la salida de error estándar (stderr) al mismo lugar que la salida estándar (stdout). Luego, se redirige la salida estándar al archivo out.txt.
![cat_out_2_&1_invertido.png](https://github.com/fiubatps/sisop_2024b_g34/blob/entrega_shell/shell/cat_out_2_%261_invertido.png)

En nuestra implementación, en la shell no cambió nada. La razón por la cual la inversión de los redireccionamientos no afecta la salida es porque tanto la salida estándar (stdout) como la salida de error estándar (stderr) están siendo redirigidas al mismo archivo en el primer comando, donde primero se redirige la salida estándar al archivo "out.txt" y luego se redirige la salida de error estándar a la misma dirección que la salida estándar (es decir, al archivo "out.txt"). Cuando se invierte el orden, la salida estándar se redirige primero a la salida de error estándar, y luego la salida de error estándar y la salida estándar redirigen ambos a "out.txt". Como resultado, el archivo "out.txt" contiene la misma información en ambas situaciones.

Ejecutando en bash:

![cat_out_2_&1_bash.png](https://github.com/fiubatps/sisop_2024b_g34/blob/entrega_shell/shell/cat_out_2_%261_bash.png)

En este caso es diferente porque lo que ocurre es que al invertir el orden de las redirecciones con 2>&1, se va a imprimir el error por la salida estandar y esto ocurre antes de que se redireccione la salida estandar a out.txt, entonces en out.txt queda solo la salida de ls y no el error.


---

### Tuberías múltiples

#### Responder: Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe
- ¿Cambia en algo?
- ¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con su implementación.

Cuando se ejecuta un pipe en la shell, el código de salida que se reporta es el exit code de la última de la tarea en la secuencia del pipe. Cada comando dentro del pipe se ejecuta como un proceso separado, y el resultado de cada comando se pasa al siguiente mediante la tubería. Si un comando falla dentro del pipe, la shell constesta el error de dicho comando. Cuando se ejecuta correctamente el pipe se devuelve un 0, si en el ultimo comando es inexistente devuelve el exit code correspondiente a ese comando pero si se invierte el orden, pasando primero el comando inexistente, el exit code sigue siendo el del último comando que es el exitoso.

![pipe_correcto_bash.png](https://github.com/fiubatps/sisop_2024b_g34/blob/entrega_shell/shell/pipe_correcto_bash.png)

![pipe_error_bash.png](https://github.com/fiubatps/sisop_2024b_g34/blob/entrega_shell/shell/pipe_error_bash.png)

![pipe_invertido_bash.png](https://github.com/fiubatps/sisop_2024b_g34/blob/entrega_shell/shell/pipe_invertido_bash.png)

En nuestra implementación, el exit code de nuestra shell siempre es exitoso porque no soporta la característica mencionada anteriormente

![pipe_correcto_shell.png](https://github.com/fiubatps/sisop_2024b_g34/blob/entrega_shell/shell/pipe_correcto_shell.png)

![pipe_error_shell.png](https://github.com/fiubatps/sisop_2024b_g34/blob/entrega_shell/shell/pipe_error_shell.png)

![pipe_invertido_shell.png](https://github.com/fiubatps/sisop_2024b_g34/blob/entrega_shell/shell/pipe_invertido_shell.png)

---

### Variables de entorno temporarias

#### Responder: ¿Por qué es necesario hacerlo luego de la llamada a fork(2)?

Es necesario hacerlo después de la llamada a `fork()` porque cuando un proceso se clona, el hijo hereda una copia del espacio de memoria del padre, incluidas sus variables de entorno. Si se establecen variables de entorno antes de `fork()`, estas serán accesibles tanto en el proceso padre como en el hijo. 

#### Responder: En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un arreglo y se lo coloca en el tercer argumento de una de las funciones de exec(3).
- ¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.
- Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.

No es el mismo comportamiento, si se pasan las nuevas variables de entorno como un tercer argumento en las funciones de exec(3) que terminan en "e", solo esas variables se aplicarán al proceso que se va a ejecutar. Porque estas variables se crean directamente en el espacio de memoria del nuevo proceso que se ejecutará. Además las funciones `exec()` que no poseen la letra "e" en su final utilizan la variable global `environ` del proceso actual para definir el entorno del nuevo proceso. Esto implica que las variables del entorno del proceso actual se copian directamente al proceso hijo.

Una posible implementación para que el comportamiento sea el mismo puede ser copiar el contenido de la variable global environ (que contiene el entorno del proceso actual) a un nuevo arreglo, agregar o modificar las variables necesarias, y luego pasar este nuevo arreglo completo a la función exec(3). Así, el proceso hijo heredará tanto las variables originales del entorno como las nuevas variables, logrando el mismo comportamiento que si se usara setenv(3) en el proceso padre antes de llamar a exec().


---

### Pseudo-variables

#### Responder: Investigar al menos otras tres variables mágicas estándar, y describir su propósito.
- Incluir un ejemplo de su uso en bash (u otra terminal similar).

El propósito de la variable `$?` es que almacena el valor de retorno del ultimo proceso ejecutado. 

Otras tres variables mágicas a parte de `$?`, son:
- `$!` retorna el PID del ultimo proceso ejecutado en segundo plano 
   ```bash
    bash$ sleep 100 &
    [1] 12345
    bash$ echo $!
    12345

- `$_` contiene el último argumento del último comando ejecutado.
    ```bash 
    bash$ echo Hello World! 
    Hello World!
    bash$ echo $_
    World!

- `$$` contiene el PID del proceso actual, es decir, el ID del proceso del shell en el que se está ejecutando el comando.
    ```bash 
    bash$ echo $$
    12345  # Este es el PID del shell actual

---

### Comandos built-in

#### Responder: ¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false)

pwd puede implementarse como un comando externo porque solo muestra el directorio actual sin modificar el entorno de la shell. En cambio, cd debe ser built-in, ya que cambia el directorio de trabajo y, si fuera externo, solo afectaría a un proceso hijo.

---

### Historial

#### Responder: ¿Cuál es la función de los parámetros MIN y TIME del modo no canónico? ¿Qué se logra en el ejemplo dado al establecer a MIN en 1 y a TIME en 0?

---
