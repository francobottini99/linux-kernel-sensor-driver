# Trabajo final de la materia de Sistemas de Computación 2023 

Implementación de un modulo para el kernel de Linux que permite la lectura de sensores [*DHT11*](https://www.mouser.com/datasheet/2/758/DHT11-Technical-Data-Sheet-Translated-Version-1143054.pdf) y [*TFMINI*](https://alicliimg.clewm.net/476/457/1457476/1502760306607344e88e353c263a4b22b3680f6a8be261502760289.pdf) por medio de una interfaz de puerto serie con el adaptador [*CP210x*](https://www.sparkfun.com/datasheets/IC/cp2102.pdf). También se implementa una aplicación a nivel de usuario que hace uso del driver.

### Autores:
- **Bottini, Franco Nicolas**
- **Robledo, Valentin**
- **Lencina, Aquiles Benjamin**

## Resumen

El proyecto consta de tres capas: el modulo del kernel, la aplicación servidor y la aplicación cliente.

El modulo del kernel se encarga de mapear y dar soporte a los sensores conectados por medio de un puente usb-serial, generando dispositivos en el directorio `/dev` que son fáciles de leer en aplicaciones de nivel de usuario por medio de operaciones con ficheros. 

El server corre a nivel de usuario sobre el dispositivo donde se encuentran conectados físicamente los sensores y hace uso del driver para obtener las lecturas de los mismos. Luego, expone las lecturas disponibles a través de un sockect ipv4 que permite el acceso a los datos de manera remota soportando múltiples clientes. 

Finalmente, la aplicación cliente también se ejecuta a nivel usuario y se conecta a la aplicación servidor, obteniendo las mediciones y permitiendo al usuario visualizar los datos de los sensores en tiempo real y de manera sencilla.

Para la realización de las aplicaciones se utilizo el lenguaje de programación C# para el cliente y C para el servidor y el modulo del kernel.

Las pruebas se realizaron sobre una [Raspberry Pi 4](https://datasheets.raspberrypi.com/rpi4/raspberry-pi-4-datasheet.pdf) con el sistema operativo [Raspbian](https://www.raspbian.org/) sin interfaz grafica y el [kernel 6.1.21-v7+](https://www.kernel.org/).

> [!NOTE]
> El [*DHT11*](https://www.mouser.com/datasheet/2/758/DHT11-Technical-Data-Sheet-Translated-Version-1143054.pdf) no posee una interfaz serie, por lo tanto se utilizo un [Arduino UNO](https://docs.arduino.cc/resources/datasheets/A000066-datasheet.pdf) para leer los datos del sensor y enviarlos por medio de un puerto serie virtual al [*CP210x*](https://www.sparkfun.com/datasheets/IC/cp2102.pdf) a modo de interfaz serie.

![imgs](/imgs/WhatsApp%20Image%202023-06-23%20at%2019.09.41.jpeg)

## Compilación

Para compilar el proyecto lo primero es clonar el repositorio:

```bash
$ git clone https://github.com/francobottini99/USBSERIAL-LINUX_KERNEL_MODULE-2023.git
$ git submodule update --init --recursive
```

Con el repositorio ya clonado, se debe ingresar a la carpeta del proyecto:

```bash
$ cd USBSERIAL-LINUX_KERNEL_MODULE-2023
```

Una vez hecho esto, se pueden comenzar a compilar los distintos módulos del proyecto.

### Modulo del kernel

Para compilar y cargar el modulo del kernel se debe acceder al directorio `/driver` y ejecutar el makefile:

```bash
$ cd driver
$ make
```

Como resultado obtenemos el archivo `sensors_driver.ko` que es el modulo compilado en el directorio `/bin`. Una vez compilado el modulo, se puede cargar al kernel con el siguiente comando:

```bash
$ sudo insmod bin/sensors_driver.ko
```

Si todo salio bien, se deben haber detectado los sensores conectados al dispositivo y generado sus correspondientes rutas de acceso en `/dev`. Si se conectan nueves dispositivos posterior a la carga del modulo, estos serán detectados y mapeados automáticamente, del mismo modo se des-mapean los dispositivos que son desconectados. Se puede verificar el correcto funcionamiento del modulo analizando los mensajes del kernel:

```bash
$ dmesg
```

Finalmente, si se desea descargar el modulo del kernel se debe ejecutar el siguiente comando:

```bash
$ sudo rmmod sensors_driver
```

> [!NOTE]
> Para compilar el modulo es necesario tener instaladas las herramientas de compilación del kernel. En caso de no tenerlas, se pueden instalar con el siguiente comando:

```bash
$ sudo apt-get install linux-headers-$(uname -r)
```

### Aplicacion servidor

Para compilar la aplicación servidor se debe acceder al directorio `/server` y ejecutar:

```bash
$ cd server
$ cmake .
$ make
```

Como resultado obtenemos el archivo `server` en el directorio `/bin`. Para ejecutar la aplicación se debe ejecutar:

```bash
$ sudo ./bin/server
```

> [!NOTE]
> La aplicación servidor debe ejecutarse con permisos de superusuario para poder acceder a los dispositivos mapeados en `/dev`.

### Aplicacion cliente

Para compilar la aplicación cliente se debe acceder al directorio `/app` y ejecutar:

```bash
$ cd app
$ dotnet restore
$ dotnet build --configuration Release
```

Como resultado obtenemos el directorio `/bin`. Para ejecutar la aplicación se debe ejecutar:

```bash
$ dotnet run --configuration Release
```

o tambien:

```bash
$ ./bin/Release/net7.0/app
```

> [!NOTE]
> Para poder compilar la aplicación cliente es necesario tener instalado el SDK de .NET 7.0. En caso de no tenerlo, se puede instalar con el siguiente comando:

```bash
$ sudo apt-get install dotnet-sdk-7.0
```

## Uso

Una vez que tenemos todo compilado, podemos comenzar a utilizar el proyecto. Lo primero que debemos hacer es cargar el modulo del kernel en el dispositivo donde se encuentran conectados los sensores:

```bash
$ sudo insmod sensors_driver.ko
```

Para verificar que todo este bien ejecutamos el comando:

```bash
$ dmesg
```

Si todo salio bien, se deben haber detectado los sensores conectados al dispositivo y deberíamos verlos en los mensajes del kernel:

![dmegs](/imgs/Captura%20desde%202023-06-23%2015-26-16.png)

Luego, ejecutamos la aplicación servidor sobre el mismo dispositivo:

```bash
$ sudo ./server
```

Si todo salio bien, el servidor comienza a aceptar conexiones a través del puerto 5000 y deberíamos ver el siguiente mensaje:

![server](/imgs/Captura%20desde%202023-06-23%2018-37-57.png)

Desde esta interfaz se puede observar el trafico de datos entre el servidor y los clientes conectados.

![server](/imgs/Captura%20desde%202023-06-23%2018-44-06.png)

Con el server ya corriendo, podemos ejecutar la aplicación cliente en cualquier dispositivo con acceso por red al servidor:

```bash
$ dotnet run --configuration Release
```

En la primer pantalla de la aplicación se debe ingresar la dirección IP del servidor y el puerto donde se encuentra escuchando:

![client](/imgs/Captura%20desde%202023-06-23%2018-46-01.png)

![client](/imgs/Captura%20desde%202023-06-23%2018-46-08.png)

Una vez conectados al servidor, se muestra el listado de los sensores disponibles y se puede seleccionar el sensor que se desea visualizar:

![client](/imgs/Captura%20desde%202023-06-23%2018-49-22.png)

Luego de seleccionar un sensor, se solicita al usuario ingresar el periodo de muestreo deseado:

![client](/imgs/Captura%20desde%202023-06-23%2018-51-36.png)

Finalmente, una vez ingresado el periodo de muestreo, se comienzan a graficar los datos en tiempo real:

![client](/imgs/Captura%20desde%202023-06-23%2018-18-36.png)