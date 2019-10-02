using System;
using System.Runtime;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Linq;
using System.Text.RegularExpressions;

static class Program
{
    static int Main(string[] args)
    {
        Console.WriteLine("Invoking heart-codegen...");

        var location = args[1];
        var realLoc = $"{location}\\..\\..\\game\\sfml-demo\\src\\";

        int serializationResult = Heart.Codegen.SerializationGen.ProcessSourceDirectory(realLoc);
        if (serializationResult != 0)
            return 1;

        // Insert other systems here?

        Console.WriteLine("Completed heart-codegen");
        return 0;
    }
}
