# Kernel Module for Sensor Reading via Serial Port Interface

Implementation of a kernel module for Linux that allows reading of [*DHT11*](https://www.mouser.com/datasheet/2/758/DHT11-Technical-Data-Sheet-Translated-Version-1143054.pdf) and [*TFMINI*](https://alicliimg.clewm.net/476/457/1457476/1502760306607344e88e353c263a4b22b3680f6a8be261502760289.pdf) sensors through a serial port interface with the [*CP210x*](https://www.sparkfun.com/datasheets/IC/cp2102.pdf) adapter. A user-level application that makes use of the driver is also implemented.

### Authors:
- **Bottini, Franco Nicolas**
- **Robledo, Valentin**
- **Lencina, Aquiles Benjamin**

## Summary

The project consists of three layers: the kernel module, the server application, and the client application.

The kernel module is responsible for mapping and supporting sensors connected via a USB-to-serial bridge, generating devices in the `/dev` directory that can be easily accessed in user-level applications through file operations.

The server runs at the user level on the device where the sensors are physically connected and uses the driver to obtain sensor readings. It then exposes the available readings through an IPv4 socket, allowing remote access to the data and supporting multiple clients.

Finally, the client application also runs at the user level and connects to the server application, retrieving the measurements and allowing the user to view the sensor data in real-time and in a simple manner.

C# was used for the client application and C for the server and kernel module.

Testing was performed on a [Raspberry Pi 4](https://datasheets.raspberrypi.com/rpi4/raspberry-pi-4-datasheet.pdf) running [Raspbian](https://www.raspbian.org/) without a graphical interface and [kernel 6.1.21-v7+](https://www.kernel.org/).

> [!NOTE]
> The [*DHT11*](https://www.mouser.com/datasheet/2/758/DHT11-Technical-Data-Sheet-Translated-Version-1143054.pdf) does not have a serial interface, so an [Arduino UNO](https://docs.arduino.cc/resources/datasheets/A000066-datasheet.pdf) was used to read the sensor's data and send it via a virtual serial port to the [*CP210x*](https://www.sparkfun.com/datasheets/IC/cp2102.pdf) as the serial interface.

![imgs](/imgs/WhatsApp%20Image%202023-06-23%20at%2019.09.41.jpeg)

## Compilation

To compile the project, the first step is to clone the repository:

```bash
$ git clone https://github.com/francobottini99/USBSERIAL-LINUX_KERNEL_MODULE-2023.git
$ git submodule update --init --recursive
```

Once the repository is cloned, navigate to the project folder:

```bash
$ cd USBSERIAL-LINUX_KERNEL_MODULE-2023
```

Once done, you can start compiling the different modules of the project.

### Kernel Module

To compile and load the kernel module, navigate to the `/driver` directory and run the makefile:

```bash
$ cd driver
$ make
```

The result is the `sensors_driver.ko` file, the compiled module, located in the `/bin` directory. After compilation, you can load the module into the kernel with the following command:

```bash
$ sudo insmod bin/sensors_driver.ko
```

If everything went well, the connected sensors should be detected, and their corresponding access paths will be generated in `/dev`. If new devices are connected after loading the module, they will be automatically detected and mapped; similarly, devices that are disconnected will be unmapped. You can verify the correct functioning of the module by analyzing the kernel messages:

```bash
$ dmesg
```

Finally, if you want to unload the kernel module, use the following command:

```bash
$ sudo rmmod sensors_driver
```

> [!NOTE]
> To compile the module, you need to have the kernel build tools installed. If you don’t have them, you can install them with the following command:

```bash
$ sudo apt-get install linux-headers-$(uname -r)
```

### Server Application

To compile the server application, navigate to the `/server` directory and run:

```bash
$ cd server
$ cmake .
$ make
```

The result is the `server` file located in the `/bin` directory. To run the application, execute:

```bash
$ sudo ./bin/server
```

> [!NOTE]
> The server application must be run with superuser privileges to access the devices mapped in `/dev`.

### Client Application

To compile the client application, navigate to the `/app` directory and run:

```bash
$ cd app
$ dotnet restore
$ dotnet build --configuration Release
```

The result is the `/bin` directory. To run the application, use:

```bash
$ dotnet run --configuration Release
```

or also:

```bash
$ ./bin/Release/net7.0/app
```

> [!NOTE]
> To compile the client application, you must have the .NET 7.0 SDK installed. If you don't have it, you can install it with the following command:

```bash
$ sudo apt-get install dotnet-sdk-7.0
```

## Usage

Once everything is compiled, you can start using the project. The first thing you need to do is load the kernel module on the device where the sensors are connected:

```bash
$ sudo insmod sensors_driver.ko
```

To verify that everything is working, execute the command:

```bash
$ dmesg
```

If everything went well, the connected sensors should have been detected, and you should see them in the kernel messages:

![dmegs](/imgs/Captura%20desde%202023-06-23%2015-26-16.png)

Next, run the server application on the same device:

```bash
$ sudo ./server
```

If everything is running properly, the server will start accepting connections on port 5000, and you should see the following message:

![server](/imgs/Captura%20desde%202023-06-23%2018-37-57.png)

This interface allows you to observe the data traffic between the server and the connected clients.

![server](/imgs/Captura%20desde%202023-06-23%2018-44-06.png)

With the server running, you can execute the client application on any device with network access to the server:

```bash
$ dotnet run --configuration Release
```

In the first screen of the application, you must enter the server's IP address and the port it is listening on:

![client](/imgs/Captura%20desde%202023-06-23%2018-46-01.png)

![client](/imgs/Captura%20desde%202023-06-23%2018-46-08.png)

Once connected to the server, the list of available sensors will be displayed, and you can select the sensor you want to view:

![client](/imgs/Captura%20desde%202023-06-23%2018-49-22.png)

After selecting a sensor, the user is asked to enter the desired sampling period:

![client](/imgs/Captura%20desde%202023-06-23%2018-51-36.png)

Finally, once the sampling period is entered, the data will begin to be graphed in real time:

![client](/imgs/Captura%20desde%202023-06-23%2018-18-36.png)
