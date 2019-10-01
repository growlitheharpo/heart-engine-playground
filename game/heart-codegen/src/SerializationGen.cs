using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

using ClangSharp;
using ClangSharp.Interop;

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

        private CXChildVisitResult VisitChildren(CXCursor cursor, CXCursor parent, void* client_data)
        {
            return CXChildVisitResult.CXChildVisit_Continue;
        }

        private string ProcessCurrentFile()
        {
            unsafe
            {
                var index = clang.createIndex(0, 0);


                var file = Convert.ToSByte(_currentFile);

                var rawTranslationUnit = clang.parseTranslationUnit(index, &file, null, 0, null, 0, (uint)CXTranslationUnit_Flags.CXTranslationUnit_None);

                var indexCursor = clang.getTranslationUnitCursor(rawTranslationUnit);
                indexCursor.VisitChildren(VisitChildren, new CXClientData(IntPtr.Zero));

                clang.disposeTranslationUnit(rawTranslationUnit);
                clang.disposeIndex(index);
            }


            return "";
        }
    }
}