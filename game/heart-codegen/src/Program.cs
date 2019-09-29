using System;
using System.Runtime;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Linq;
using System.Text.RegularExpressions;

static class Program
{
    private static bool isErrored = false;
    private static List<string> necessaryIncludes = new List<string>();
    private static HashSet<int> serializedStringLengths = new HashSet<int>();

    struct CurrentContext
    {
        public string filename;
        public int linenumber;
    }

    struct FieldToken
    {
        public string name;
        public string annotation;
        public int linenumber;

        public FieldToken(string n, string a, int l)
        {
            name = n;
            annotation = a;
            linenumber = l;
        }
    }

    private static CurrentContext ctx;

    static int Main(string[] args)
    {
        Console.WriteLine("Invoking heart-codegen...");

        var location = args[1];
        var realLoc = $"{location}\\..\\..\\game\\sfml-demo\\src\\";

        WriteReflectionCodegen(realLoc);

        Console.WriteLine("Completed heart-codegen");

        return isErrored ? 1 : 0;
    }

    static void EncounterError(string msg)
    {
        Console.Error.WriteLine(msg);
        Console.Error.WriteLine($"Encountered at: {ctx.filename} ({ctx.linenumber})");
        isErrored = true;
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
            writer.WriteLine("\tentt::reflect<int32_t>().conv<uint32_t>().conv<uint16_t>().conv<uint8_t>();");
            writer.WriteLine("\tentt::reflect<uint32_t>().conv<int32_t>().conv<uint16_t>().conv<uint8_t>();");
            writer.WriteLine();
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
            ctx.filename = cpp;
            ctx.linenumber = 1;

            var result = ProcessFile(cpp);

            if (result.Length > 1)
                EncounterError("Cannot auto-serialize structs in cpp, please move to a header or remove SERIALIZE_STRUCT()!");
        }

        foreach (var h in Directory.GetFiles(dir, "*.h", SearchOption.AllDirectories))
        {
            ctx.filename = h;
            ctx.linenumber = 1;

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
                ++ctx.linenumber;

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

        var fields = new List<FieldToken>();

        {
            int next;

            do
            {
                next = reader.Peek();
                if (next == '{')
                {
                    reader.Read();
                    ++bracketlevel;
                    continue;
                }
                else if (next == '}')
                {
                    reader.Read();
                    --bracketlevel;
                    continue;
                }
                else if (bracketlevel > 1)
                {
                    reader.Read();
                    continue;
                }
                else if (bracketlevel == 0)
                    break;

                if (char.IsWhiteSpace((char)next))
                {
                    if (next == '\n')
                        ++ctx.linenumber;

                    reader.Read();
                    continue;
                }

                // we're at root level and we just hit something!
                // we can just consume the rest of the line now; we don't care what the type was, so
                // we don't care that we lost the first character of it. All we care about is the field name.
                // note: this will break if you do something stupid (like put the open parenthesis for a function on the next line,
                // or seperate the type and identifier.
                var line = reader.ReadLine();
                ++ctx.linenumber;

                if (line.Contains("(") || line.Contains(")")) // false alarm, this is a function
                {
                    if (line.Contains("{") && !line.Contains("}"))
                        ++bracketlevel;
                    continue;
                }

                var declarations = line.Split(';');

                foreach (var declaration in declarations)
                {
                    if (declaration.Length == 0)
                        continue;

                    var tokens = declaration.Split(' ');

                    string annotation = "";
                    string type;
                    string name;
                    if (tokens.Length == 2)
                    {
                        type = tokens[0];
                        name = tokens[1];
                    }
                    else if (tokens.Length == 3)
                    {
                        annotation = tokens[0];
                        type = tokens[1];
                        name = tokens[2];
                    }
                    else
                    {
                        EncounterError("Unable to parse - too many tokens in declaration!");
                        continue;
                    }

                    var nameArrayRegex = new Regex("(.+?)(\\[\\d+\\])");
                    if (nameArrayRegex.IsMatch(name))
                    {
                        name = nameArrayRegex.Match(name).Groups[1].Value;
                    }

                    fields.Add(new FieldToken(name, annotation, ctx.linenumber));

                    // Check for our special type that needs extra reflection
                    var serializedStringRegex = new Regex("(SerializedString<)(\\d+)(>)");
                    if (serializedStringRegex.IsMatch(type))
                    {
                        var stringSize = serializedStringRegex.Match(type).Groups[2].Value;
                        var realSize = int.Parse(stringSize);

                        serializedStringLengths.Add(realSize);
                    }
                }

            } while (true);
        }

        StringBuilder sb = new StringBuilder();
        sb.AppendLine($"\tBEGIN_SERIALIZE_TYPE({structname})");
        foreach (var f in fields)
        {
            if (string.IsNullOrEmpty(f.annotation))
                sb.AppendLine($"\t\tSERIALIZE_FIELD({structname}, {f.name})");
            else if (f.annotation == "SERIALIZE_AS_REF")
                sb.AppendLine($"\t\tSERIALIZE_FIELD_ALIAS({structname}, {f.name})");
            else
            {
                int curLine = ctx.linenumber;
                ctx.linenumber = f.linenumber;
                EncounterError($"Unknown annotation: {f.annotation}");
                ctx.linenumber = curLine;
            }
        }
        sb.AppendLine($"\tEND_SERIALIZE_TYPE({structname})");
        return sb.ToString();
    }
}
