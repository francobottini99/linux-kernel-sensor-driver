using Newtonsoft.Json;

namespace AppUtils
{
    public class SensorData
    {
        public string? NumSensors { get; set; }
        public string? Sensors { get; set; }
    }
    
    public class Utils
    {
        public static void UpdateArray(double value, List<double> data)
        {
            if (data.Count >= 100)
                data.RemoveAt(0);

            data.Add(value);
        }

        public static (string, string) GetServerConnectionInfo()
        {
            string ipAddress, port;

            do
            {
                Console.WriteLine("Please, enter the server IP address: ");

                ipAddress = Console.ReadLine()!;
            } while (ipAddress == "");

            do{
                Console.WriteLine("Please, enter the server port: ");

                port = Console.ReadLine()!;
            } while (port == "");

            return (ipAddress, port);
        }

        public static (string, string[]) JsonDecode(string json)
        {
            SensorData sensorData = JsonConvert.DeserializeObject<SensorData>(json)!;

            return (sensorData.NumSensors, sensorData.Sensors!.Split(' '))!;
        }
    }
}