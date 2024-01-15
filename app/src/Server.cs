using System.Net;
using System.Net.Sockets;
using System.Text;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace AppSocket
{
    public class Server
    {
        private class ServerData 
        {
            public string? error { get; set; }
            public string? type { get; set; }
            public object? content { get; set; }
        }

        public string Ip { get; }
        public int Port { get; }

        private Socket _socket;

        public Server(string ipAddressString, int port)
        {
            IPAddress ipAddress = IPAddress.Parse(ipAddressString);
            IPEndPoint remoteEP = new(ipAddress, port);

            _socket = new(ipAddress.AddressFamily, SocketType.Stream, ProtocolType.Tcp);

            bool isConnected = ConnectWithTimeout(_socket, remoteEP, 2500).GetAwaiter().GetResult();

            if(!isConnected)
                throw new SocketException();

            Ip = ipAddressString;
            Port = port;
        }

        ~Server()
        {
            _socket.Shutdown(SocketShutdown.Both);
            _socket.Close();
        }

        private async Task<bool> ConnectWithTimeout(Socket socket, EndPoint remoteEP, int timeoutMilliseconds)
        {
            var connectTask = socket.ConnectAsync(remoteEP);
            var timeoutTask = Task.Delay(timeoutMilliseconds);
            var completedTask = await Task.WhenAny(connectTask, timeoutTask);

            if (completedTask == connectTask)
                return true;
            else
                return false;
        }

        public string[] GetSensorsList()
        {
            ServerData serverData = GetServerData("GET LIST")!;

            if(string.IsNullOrEmpty(serverData.error))
            {
                JArray jsonArray = (JArray)serverData.content!;

                string[] sensorsList = jsonArray.Select(j => (string)j!).ToArray();

                return sensorsList;
            }
            else
                throw new Exception(serverData.error);
        }

        public int GetSensorRead(string sensor)
        {
            ServerData serverData = GetServerData($"GET DATA {sensor}")!;

            if(string.IsNullOrEmpty(serverData.error))
                return int.Parse((string)serverData.content!);
            else
                return -1;
        }

        private ServerData? GetServerData(string request)
        {
            byte[] buffer = new byte[1024];

            _socket.Send(Encoding.ASCII.GetBytes(request));

            int bytesRead = _socket.Receive(buffer);

            string data = Encoding.ASCII.GetString(buffer, 0, bytesRead);

            return JsonConvert.DeserializeObject<ServerData>(data);
        }
    }
}
