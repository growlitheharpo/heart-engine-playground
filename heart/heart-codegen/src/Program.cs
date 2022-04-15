/* Copyright (C) 2022 James Keats
*
* This file is part of Heart, a collection of game engine technologies.
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
*/
using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;

static class Program
{
    private static string GetParamater(string paramName, List<string> args)
    {
        var index = args.IndexOf(paramName);
        if (index < 0 || index + 1 >= args.Count)
            return null;

        return args[index + 1];
    }

    static int Main(string[] args)
    {
        Console.WriteLine("Invoking heart-codegen...");

        var listArgs = args.ToList();
        var scanDir = GetParamater("-in", listArgs);
        var heartDir = GetParamater("-heart", listArgs);

        if (string.IsNullOrEmpty(scanDir))
            Console.WriteLine("Invalid or missing \"-in\" argument - skipping heart-codegen");
        else if (string.IsNullOrEmpty(heartDir))
            Console.WriteLine("Invalid or missing \"-heart\" argument (should point to heart-core folder) - skipping heart-codegen");
        else
        {
            scanDir = Path.GetFullPath(scanDir);
            heartDir = Path.GetFullPath(heartDir);

            int serializationResult = Heart.Codegen.SerializationGen.ProcessSourceDirectory(scanDir, heartDir);
            if (serializationResult != 0)
                return 1;

            // Insert other systems here?

            Console.WriteLine("Completed heart-codegen");
        }

        return 0;
    }
}
