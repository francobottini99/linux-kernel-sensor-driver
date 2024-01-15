using System.Net.Sockets;
using Spectre.Console;
using AsciiChart.Sharp;
using AppSocket;

namespace AppGui
{
    class Gui
    {
        private static Server? _server;

        public static void Main()
        {            
            try
            {
                string ipAddress, port;

                Console.CancelKeyPress += Console_CancelKeyPress;
    
                while(true)
                {
                    AnsiConsole.Clear();

                    ShowTitle();
                    ShowExitShorcut();

                    AnsiConsole.Write("Please, enter the server IP address: ");
                    
                    ipAddress = Console.ReadLine()!;

                    if(string.IsNullOrEmpty(ipAddress))
                    {
                        AnsiConsole.WriteLine();
                        AnsiConsole.MarkupLine("[red]Error:[/] The IP address cannot be empty.");
                    }
                    else
                        break;

                    Thread.Sleep(1500);
                }

                AnsiConsole.WriteLine();
    
                while(true)
                {
                    AnsiConsole.Clear();

                    ShowTitle();
                    ShowExitShorcut();

                    AnsiConsole.Write("Please, enter the server port: ");
                    
                    port = Console.ReadLine()!;

                    if(string.IsNullOrEmpty(port))
                    {
                        AnsiConsole.WriteLine();
                        AnsiConsole.MarkupLine("[red]Error:[/] The port cannot be empty.");
                    }
                    else
                        break;
                    
                    Thread.Sleep(1500);
                }

                _server = new(ipAddress, int.Parse(port));

                MainMenu();

                Environment.Exit(0);
            }
            catch (SocketException)
            {
                AnsiConsole.Clear();
                AnsiConsole.Write(new Rule($"[red]Error:[/] The server is not available.{Environment.NewLine}"));

                Console.CursorVisible = true;

                Environment.Exit(1);
            }
            catch (Exception)
            {
                AnsiConsole.Clear();
                AnsiConsole.Write(new Rule($"[red]Error:[/] An unexpected error has occurred.{Environment.NewLine}"));

                Console.CursorVisible = true;

                Environment.Exit(1);
            }
        }

        private static void Console_CancelKeyPress(object? sender, ConsoleCancelEventArgs e)
        {
            if (e.SpecialKey == ConsoleSpecialKey.ControlC)
            {
                AnsiConsole.Clear();
                AnsiConsole.Write(new Rule($"[red]Bye!{Environment.NewLine}[/]"));

                Console.CursorVisible = true;

                Environment.Exit(0);
            }
        }

        private static void MainMenu()
        {
            if(_server == null)
                throw new NullReferenceException();

            while(true)
            {
                AnsiConsole.Clear();

                ShowTitle();
                ShowServerInfo();
                ShowExitShorcut();

                string[] sensorsList = _server.GetSensorsList();

                string selection = ShowPromtMenu(sensorsList);

                int samplingRate = GetSamplingRate();

                if(samplingRate == -1)
                    continue;

                GraphMenu(selection, samplingRate);
            }
        }

        private static void GraphMenu(string sensor, int samplingRate)
        {
            if(_server == null)
                throw new NullReferenceException();

            List<double> readsList = new();
            
            AnsiConsole.Clear();

            Console.CursorVisible = false;

            int consolePreviusHeight;
            int consolePreviusWidth;

            while (true)
            {
                Console.SetCursorPosition(0, 0);

                consolePreviusHeight = Console.WindowHeight;
                consolePreviusWidth = Console.WindowWidth;

                if (Console.KeyAvailable)
                {
                    ConsoleKeyInfo keyInfo = Console.ReadKey(true);

                    if ((keyInfo.Modifiers & ConsoleModifiers.Control) != 0 && keyInfo.Key == ConsoleKey.A)
                        break;
                }

                ShowTitle();
                ShowServerInfo();
                ShowSelecteSensorAndTime(sensor);
                ShowAllShorcut();

                int newRead = _server.GetSensorRead(sensor);

                if(newRead < 0)
                {
                    AnsiConsole.MarkupLine("[red]Error:[/] The server is not available.");
                    AnsiConsole.WriteLine();

                    Thread.Sleep(1500);

                    break;
                }

                readsList.Add(_server.GetSensorRead(sensor));

                while(readsList.Count > Console.WindowWidth - 8)
                    readsList.RemoveAt(0);

                ShowChart(readsList);

                while(Console.CursorTop < Console.WindowHeight - 2)
                    AnsiConsole.WriteLine(new string(' ', Console.WindowWidth - 5));

                Thread.Sleep(samplingRate);

                if(consolePreviusHeight != Console.WindowHeight || consolePreviusWidth != Console.WindowWidth)
                    AnsiConsole.Clear();
            }

            Console.CursorVisible = true;
        }

        private static int GetSamplingRate()
        {
            int samplingRate;

            while(true)
            {
                AnsiConsole.Clear();

                ShowTitle();
                ShowServerInfo();
                ShowBackCmd();
                ShowExitShorcut();

                AnsiConsole.Write("Please, enter the sampling rate for signal (in miliseconds) [range: 10 - 1000]: ");

                if(int.TryParse(Console.ReadLine(), out samplingRate))
                {
                    if(samplingRate == -1 || samplingRate >= 10 && samplingRate <= 1000)
                        break;
                    else
                    {
                        AnsiConsole.WriteLine();
                        AnsiConsole.MarkupLine("[red]Error:[/] The sampling rate must be between 10 and 1000. Or -1 to go back.");
                    }
                }
                else
                {
                    AnsiConsole.WriteLine();
                    AnsiConsole.MarkupLine("[red]Error:[/] The sampling rate must be a number. Or -1 to go back.");
                }

                Thread.Sleep(1500);
            }

            return samplingRate;
        }

        private static string ShowPromtMenu(string[] sensorList)
        {
            return AnsiConsole.Prompt(
                new SelectionPrompt<string>()
                    .PageSize(sensorList.Count())
                    .Title("What [green]signal[/] do you want to see?")
                    .AddChoices(sensorList));
        }

        private static void ShowChart(List<double> values)
        {
            string dataChart = AsciiChart.Sharp.AsciiChart.Plot(
                values.ToArray(), new Options 
                {   
                    Height = Console.WindowHeight - Console.CursorTop - 2,
                    AxisLabelLeftMargin = 1,
                    AxisLabelRightMargin = 1, 
                    AxisLabelFormat = "0",
                });

            AnsiConsole.Write(dataChart);
        }

        private static void ShowTitle()
        {
            AnsiConsole.Write(new FigletText("Sensor App").Centered().Color(Color.DarkCyan));
            AnsiConsole.WriteLine();
        }

        private static void ShowServerInfo()
        {
            AnsiConsole.Write(new Rule($"Server: [green]{_server!.Ip}:{_server!.Port}[/]"));
            AnsiConsole.WriteLine();
        }

        private static void ShowBackShortcut()
        {
            AnsiConsole.Write(new Rule("Press [yellow]Ctrl + A[/] to go back."));
            AnsiConsole.WriteLine();
        }

        private static void ShowBackCmd()
        {
            AnsiConsole.Write(new Rule("[yellow]Write -1[/] to go back."));
            AnsiConsole.WriteLine();
        }

        private static void ShowExitShorcut()
        {
            AnsiConsole.Write(new Rule("Press [yellow]Ctrl + C[/] to exit."));
            AnsiConsole.WriteLine();
        }

        private static void ShowAllShorcut()
        {
            AnsiConsole.Write(new Rule("Press [yellow]Ctrl + A[/] to go back - Press [yellow]Ctrl + C[/] to exit."));
            AnsiConsole.WriteLine();
        }

        private static void ShowSelectedSensor(string sensor)
        {
            AnsiConsole.Write(new Rule($"[green]{sensor}[/]"));
            AnsiConsole.WriteLine();
        }

        private static void ShowSelecteSensorAndTime(string sensor)
        {
            AnsiConsole.Write(new Rule($"[green]{sensor}[/] - [green]{DateTime.Now:HH:mm:ss}[/]"));
            AnsiConsole.WriteLine();
        }
    }
}