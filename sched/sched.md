# sched

Lugar para respuestas en prosa, seguimientos con GDB y documentación del TP.

# Parte 1 - Cambio de contexto 
Utilizar GDB para visualizar el cambio de contexto. Realizar una captura donde se muestre claramente:
- el cambio de contexto
- el estado del stack al inicio de la llamada de context_switch
- cómo cambia el stack instrucción a instrucción
- cómo se modifican los registros luego de ejecutar iret

<img width="800" alt="1" src="https://github.com/user-attachments/assets/9508a498-c3f2-488c-a4fb-6ac05aba3b77">

<img width="800" alt="2" src="https://github.com/user-attachments/assets/980b0801-125f-4c2b-93e1-39d8b70520bc">

<img width="800" alt="3" src="https://github.com/user-attachments/assets/ec077b1b-b8b7-42cd-980e-55ee65697385">

<img width="800" alt="4" src="https://github.com/user-attachments/assets/53485819-c013-481a-937a-da5f270dcaee">

<img width="800" alt="5" src="https://github.com/user-attachments/assets/4f61222b-b706-40aa-82ad-2daeb2cc7d8b">

# Parte 3 - Priorities
El sistema de scheduling con prioridad que implementamos selecciona procesos basándose en su prioridad de la siguiente manera:

A cada proceso se le asigna un valor de prioridad, donde un número más bajo indica mayor prioridad.

En cada ciclo de scheduling, se selecciona el proceso RUNNABLE con el valor de prioridad más bajo. Este proceso es el "ganador" y será el próximo en ejecutarse.

Después de cada ejecución, la prioridad del proceso ganador se incrementa, reduciendo su prioridad para que otros procesos tengan oportunidad de ejecutarse en el próximo ciclo.

Cada cierto número de ciclos (BOOST_CYCLES), todos los procesos RUNNABLE reciben un “boost” que restablece su prioridad al valor inicial (DEFAULT_PRIORITY), para evitar que procesos de baja prioridad queden sin ejecutarse.
