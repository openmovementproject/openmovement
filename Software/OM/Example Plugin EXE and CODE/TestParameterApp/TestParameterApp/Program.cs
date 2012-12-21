using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace TestParameterApp
{
    class Program
    {
        static void Main(string[] args)
        {
            int i = 0;
            foreach(string arg in args)
            {
                Console.WriteLine("arg" + i + ": " + arg);
                i++;
            }

            string x = Console.ReadLine();
        }
    }
}
