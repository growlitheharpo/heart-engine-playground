using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

namespace Heart.Codegen
{
    public class SerializationGen
    {
        public static int ProcessSourceDirectory(string dir)
        {
            SerializationGen generator = new SerializationGen(dir);
            generator.Process();

            return generator.ErrorCode;
        }

        private class CountingStreamReader : StreamReader
        {
            public int LineNumber { get; private set; }

            public CountingStreamReader(Stream s) : base(s) 
            {
                LineNumber = 1;
            }

            public override int Read()
            {
                int r = base.Read();
                if (r == '\n')
                    ++LineNumber;
                return r;
            }

            public override string ReadLine()
            {
                ++LineNumber;
                return base.ReadLine();
            }
        }

        private struct FieldToken
        {
            public string name;
            public string annotation;

            public FieldToken(string n, string a)
            {
                name = n;
                annotation = a;
            }
        }

        private int ErrorCode { get; set; }

        private string _currentFile;
        private bool _mayCurrentlySerialize;

        private string _directory;
        private List<string> _includes = new List<string>();
        private HashSet<int> _serializedStringSizes = new HashSet<int>();

        private readonly Dictionary<string, string> _annotations = new Dictionary<string, string>()
        {
            { "", "SERIALIZE_FIELD" },
            { "SERIALIZE_AS_REF", "SERIALIZE_FIELD_ALIAS" }
        };

        private SerializationGen(string dir) 
        {
            _directory = dir;
        }

        private void EncounterError(string msg, CountingStreamReader reader, bool processingPrevious = true)
        {
            Console.Error.WriteLine($"heart-codegen error: {msg}");
            Console.Error.WriteLine($"\tEncountered at: {_currentFile} ({reader.LineNumber + (processingPrevious ? -1 : 0)})");
            ErrorCode = 1;
        }

        private void Process()
        {
            string output = TraverseCodebase();

            using (var writer = File.CreateText($"{_directory}\\gen\\reflection.heartgen.cpp"))
            {
                writer.WriteLine("/*   WRITTEN BY HEART-CODEGEN   */");
                writer.WriteLine();
                writer.WriteLine("#include \"gen/gen.h\"");
                writer.WriteLine("#include <heart/deserialization.h>");
                writer.WriteLine();
                foreach (var incl in _includes.Distinct())
                    writer.WriteLine($"#include \"{incl}\"");
                writer.WriteLine();

                writer.WriteLine("void ReflectSerializedData()");
                writer.WriteLine("{");
                writer.WriteLine("\tentt::reflect<int32_t>().conv<uint32_t>().conv<uint16_t>().conv<uint8_t>();");
                writer.WriteLine("\tentt::reflect<uint32_t>().conv<int32_t>().conv<uint16_t>().conv<uint8_t>();");
                writer.WriteLine();
                if (_serializedStringSizes.Count > 0)
                {
                    foreach (var size in _serializedStringSizes)
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

        private string TraverseCodebase()
        {
            StringBuilder sb = new StringBuilder();
            var dirUri = new Uri(_directory);
            var codeExtensions = new[] { ".cpp", ".c", ".h", ".hpp" };
            var allowedExtensions = new[] { ".h", ".hpp" };

            foreach (var file in Directory.GetFiles(_directory, "*.*", SearchOption.AllDirectories))
            {
                var ext = Path.GetExtension(file);
                if (!codeExtensions.Contains(ext))
                    continue;

                _mayCurrentlySerialize = allowedExtensions.Contains(ext);
                _currentFile = file;

                string serializations = ProcessCurrentFile();
                if (serializations.Length > 0)
                {
                    sb.Append(serializations);
                    _includes.Add(dirUri.MakeRelativeUri(new Uri(file)).OriginalString);
                }
            }

            return sb.ToString();
        }

        private string ProcessCurrentFile()
        {
            StringBuilder sb = new StringBuilder();

            using (var filestream = File.OpenRead(_currentFile))
            using (var stream = new CountingStreamReader(filestream))
            {
                while (!stream.EndOfStream)
                {
                    var line = stream.ReadLine();

                    if (line == "SERIALIZE_STRUCT()")
                    {
                        if (!_mayCurrentlySerialize)
                        {
                            EncounterError("Cannot auto-serialize structs in cpp, please move to a header or remove SERIALIZE_STRUCT()!", stream, false);
                            continue;
                        }
                        else
                        {
                            sb.Append(ProcessCurrentStruct(stream));
                            sb.AppendLine();
                        }
                    }
                }
            }

            return sb.ToString();
        }

        private string ProcessCurrentStruct(CountingStreamReader stream)
        {
            int bracketLevel = 0;
            var structName = ExtractTypeName(stream, ref bracketLevel);
            if (string.IsNullOrEmpty(structName))
            {
                EncounterError("Unable to parse structure name!", stream);
                return string.Empty;
            }

            var tokens = ProcessStructFields(stream, bracketLevel);

            StringBuilder sb = new StringBuilder();
            sb.AppendLine($"\tBEGIN_SERIALIZE_TYPE({structName})");
            foreach (var t in tokens)
            {
                var macro = _annotations[t.annotation];
                sb.AppendLine($"\t\t{macro}({structName}, {t.name})");
            }
            sb.AppendLine($"\tEND_SERIALIZE_TYPE({structName})");
            return sb.ToString();
        }

        private List<FieldToken> ProcessStructFields(CountingStreamReader stream, int bracketLevel)
        {
            // TODO: This is a very dumb way of reading that can easily be broken by coding styles.
            // A better idea would be to use libclang or some other actual C++ parser

            var result = new List<FieldToken>();
            
            while (true)
            {
                int next = stream.Peek();
                
                if (next == '{')
                {
                    stream.Read();
                    ++bracketLevel;
                    continue;
                }
                else if (next == '}')
                {
                    stream.Read();
                    --bracketLevel;
                    continue;
                }

                if (bracketLevel > 1)
                {
                    stream.Read();
                    continue;
                }
                else if (bracketLevel == 0)
                {
                    break;
                }

                if (char.IsWhiteSpace((char)next))
                {
                    stream.Read();
                    continue;
                }

                // we're at root level and we just hit something!
                // we can just consume the rest of the line now.
                // note: this will break if you do something stupid (like put the open parenthesis for a function on the next line,
                // or seperate the type and identifier.
                var line = stream.ReadLine();
                if (line.Contains("(")) // false alarm, this is a function
                {
                    if (line.Contains("{") && !line.Contains("}"))
                        ++bracketLevel;
                    continue;
                }

                var declarations = line.Split(';');
                foreach (var d in declarations)
                {
                    if (d.Length == 0)
                        continue;

                    var tokens = d.Split(' ');

                    string annotation = "", type, identifier;
                    if (tokens.Length == 2)
                    {
                        type = tokens[0];
                        identifier = tokens[1];
                    }
                    else if (tokens.Length == 3)
                    {
                        annotation = tokens[0];
                        type = tokens[1];
                        identifier = tokens[2];
                    }
                    else
                    {
                        EncounterError("Unable to parse - too many tokens in declaration!", stream);
                        continue;
                    }

                    // Fixup for array declarations (eg. int x[5])
                    var arrayRegex = new Regex("(.+?)(\\[\\d+\\])");
                    if (arrayRegex.IsMatch(identifier))
                    {
                        identifier = arrayRegex.Match(identifier).Groups[1].Value;
                    }

                    if (!_annotations.ContainsKey(annotation))
                    {
                        EncounterError($"Unknown annotation: \"{annotation}\"", stream);
                        continue;
                    }

                    result.Add(new FieldToken(identifier, annotation));

                    // Check for our special type that needs extra reflection
                    var serializedStringRegex = new Regex("(SerializedString<)(\\d+)(>)");
                    if (serializedStringRegex.IsMatch(type))
                    {
                        var stringSize = serializedStringRegex.Match(type).Groups[2].Value;
                        _serializedStringSizes.Add(int.Parse(stringSize));
                    }
                }
            }

            return result;
        }

        private string ExtractTypeName(CountingStreamReader stream, ref int bracketLevel)
        {
            var nextLine = stream.ReadLine();
            if (nextLine.Contains("{") && !nextLine.Contains("}"))
                ++bracketLevel;

            var tokens = nextLine.Split(' ');
            if (tokens[0] == "struct" || tokens[1] == "class")
                return tokens[1];

            return string.Empty;
        }
    }
}