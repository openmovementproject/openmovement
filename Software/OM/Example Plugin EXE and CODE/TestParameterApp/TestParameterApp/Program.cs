using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Diagnostics;

namespace TestParameterApp
{
    class Program
    {
        static void Main(string[] args)
        {
            int i = 0;

            Console.WriteLine("status: Getting Inputs...");

            string argsString = "";
            foreach(string arg in args)
                argsString += arg + " ";

            Console.WriteLine("args string: " + argsString);

            foreach(string arg in args)
            {
                Console.WriteLine("percentage: " + (100 / args.Length) * i);
                Console.WriteLine("arg" + i + ": " + arg);
                i++;

                Thread.Sleep(3000);
            }

            Console.WriteLine("percentage: 100");
            Console.WriteLine("status: Complete");
        }
    }
}
