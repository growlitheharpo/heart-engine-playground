using System;
using System.Runtime;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Linq;
using System.Text.RegularExpressions;

static class Program
{
    private static List<string> necessaryIncludes = new List<string>();
    private static HashSet<int> serializedStringLengths = new HashSet<int>();

    static int Main(string[] args)
    {
        Console.WriteLine("Invoking heart-codegen...");

        var location = args[1];
        var realLoc = $"{location}\\..\\..\\game\\sfml-demo\\src\\";

        WriteReflectionCodegen(realLoc);

        Console.WriteLine("Completed heart-codegen");

        return 0;
    }

    static void WriteReflectionCodegen(string realLoc)
    {
        string output = TraverseCodebase(realLoc);

        using (var writer = File.CreateText($"{realLoc}\\gen\\reflection.heartgen.cpp"))
        {
            writer.WriteLine("/*   WRITTEN BY HEART-CODEGEN   */");
            writer.WriteLine();
            writer.WriteLine("#include \"gen/gen.h\"");
            writer.WriteLine("#include <heart/deserialization.h>");
            writer.WriteLine();
            foreach (var incl in necessaryIncludes.Distinct())
                writer.WriteLine($"#include \"{incl}\"");
            writer.WriteLine();

            writer.WriteLine("void ReflectSerializedData()");
            writer.WriteLine("{");
            if (serializedStringLengths.Count > 0)
            {
                foreach (var size in serializedStringLengths)
                    writer.WriteLine($"\tentt::reflect<const char*>().conv<&SerializedString<{size}>::CreateFromCString>();");
                writer.WriteLine();
            }
            writer.Write(output);
            writer.WriteLine("}");
            writer.WriteLine();
            writer.WriteLine("/*   WRITTEN BY HEART-CODEGEN   */");
        }

        Console.WriteLine("heart-codegen: reflection.heartgen.cpp");
    }

    static string TraverseCodebase(string dir)
    {
        StringBuilder sb = new StringBuilder();

        var dirUri = new Uri(dir);

        foreach (var cpp in Directory.GetFiles(dir, "*.cpp", SearchOption.AllDirectories))
        {
            var result = ProcessFile(cpp);
            if (result.Length > 1)
                throw new InvalidOperationException("Cannot auto-serialize structs in cpp, please move to a header or remove SERIALIZE_STRUCT()!");
        }

        foreach (var h in Directory.GetFiles(dir, "*.h", SearchOption.AllDirectories))
        {
            var result = ProcessFile(h);
            if (result.Length == 0)
                continue;

            sb.Append(result);

            var headerUri = new Uri(h);
            var diff = dirUri.MakeRelativeUri(headerUri);
            var path = diff.OriginalString;
            necessaryIncludes.Add(path);
        }

        return sb.ToString();
    }

    static string ProcessFile(string filepath)
    {
        StringBuilder sb = new StringBuilder();

        using (var fileStream = File.OpenRead(filepath))
        using (var stream = new StreamReader(fileStream))
        {
            while (!stream.EndOfStream)
            {
                var line = stream.ReadLine();
                if (line == "SERIALIZE_STRUCT()")
                {
                    sb.Append(ProcessStruct(stream));
                    sb.AppendLine();
                }
            }
        }

        return sb.ToString();
    }

    static string ProcessStruct(StreamReader reader)
    {
        // This is called immediately after hitting "SERIALIZE_STRUCT()\n"
        int bracketlevel = 0;

        var structname = "";
        {
            var nextline = reader.ReadLine();
            var tokens = nextline.Split(' ');

            if (tokens[0] == "struct" || tokens[0] == "class")
                structname = tokens[1];

            if (nextline.Contains("{"))
                ++bracketlevel;
        }

        var fields = new List<string>();

        {
            int next;

            do
            {
                next = reader.Read();
                if (next == '{')
                {
                    ++bracketlevel;
                    continue;
                }
                else if (next == '}')
                {
                    --bracketlevel;
                    continue;
                }
                else if (bracketlevel > 1)
                {
                    continue;
                }
                else if (bracketlevel == 0)
                    break;

                if (char.IsWhiteSpace((char)next))
                    continue;

                // we're at root level and we just hit something!
                // we can just consume the rest of the line now; we don't care what the type was, so
                // we don't care that we lost the first character of it. All we care about is the field name.
                // note: this will break if you do something stupid (like put the open parenthesis for a function on the next line,
                // or seperate the type and identifier.
                var line = reader.ReadLine();
                if (line.Contains("(") || line.Contains(")")) // false alarm, this is a function
                {
                    if (line.Contains("{") && !line.Contains("}"))
                        ++bracketlevel;
                    continue;
                }

                var tokens = line.Split(' ', ';');

                // Add every other token; that's probably safe, right? i.e struct x { int val; float val2; bool flag; };
                for (int i = 1; i < tokens.Length; i += 2)
                {
                    fields.Add(tokens[i]);
                    var r = new Regex("(erializedString<)(\\d+)(>)");
                    if (r.IsMatch(tokens[i - 1]))
                    {
                        var match = r.Match(tokens[i - 1]);
                        var size = match.Groups[2].Value;
                        var realSize = int.Parse(size);

                        if (!serializedStringLengths.Contains(realSize))
                            serializedStringLengths.Add(realSize);
                    }
                }

            } while (true);
        }

        StringBuilder sb = new StringBuilder();
        sb.AppendLine($"\tBEGIN_SERIALIZE_TYPE({structname})");
        foreach (var f in fields)
        {
            sb.AppendLine($"\t\tSERIALIZE_FIELD({structname}, {f})");
        }
        sb.AppendLine($"\tEND_SERIALIZE_TYPE({structname})");
        return sb.ToString();
    }
}
