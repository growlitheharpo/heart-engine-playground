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

        private struct FieldToken
        {
            public string name;
            public bool asRef;

            public FieldToken(string n, bool r)
            {
                name = n;
                asRef = r;
            }
        }

        private int ErrorCode { get; set; }

        private string _currentFile;

        private string _directory;
        private HashSet<string> _includes = new HashSet<string>();
        private HashSet<int> _serializedStringSizes = new HashSet<int>();
        private Dictionary<string, List<FieldToken>> _typeFields = new Dictionary<string, List<FieldToken>>();

        private SerializationGen(string dir) 
        {
            _directory = dir;
        }

        private void EncounterError(string msg, CXSourceLocation location)
        {
            Console.Error.WriteLine($"heart-codegen error: {msg}");
            Console.Error.WriteLine($"\tEncountered at: {location.ToString()}");
            ErrorCode = 1;
        }

        private string GetRelativePath(string target)
        {
            var dirUri = new Uri(_directory);
            return dirUri.MakeRelativeUri(new Uri(target)).OriginalString;
        }

        private void Process()
        {
            TraverseCodebase();

            var dirUri = new Uri(_directory);

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
                foreach (var typePair in _typeFields)
                {
                    var type = typePair.Key;
                    writer.WriteLine($"\tBEGIN_SERIALIZE_TYPE({type})");

                    foreach (var field in typePair.Value)
                    {
                        if (field.asRef)
                            writer.WriteLine($"\t\tSERIALIZE_FIELD_ALIAS({type}, {field.name})");
                        else
                            writer.WriteLine($"\t\tSERIALIZE_FIELD({type}, {field.name})");
                    }
                    writer.WriteLine($"\tEND_SERIALIZE_TYPE({type})");
                }
                writer.WriteLine("}");
                writer.WriteLine();
                writer.WriteLine("/*   WRITTEN BY HEART-CODEGEN   */");
            }

            Console.WriteLine("heart-codegen: reflection.heartgen.cpp");
        }

        private void TraverseCodebase()
        {
            var codeExtensions = new[] { ".cpp", ".c", ".h", ".hpp" };

            foreach (var file in Directory.GetFiles(_directory, "*.*", SearchOption.AllDirectories))
            {
                var ext = Path.GetExtension(file);
                if (!codeExtensions.Contains(ext))
                    continue;

                _currentFile = file;

                ProcessCurrentFile();
            }
        }

        private void ProcessCurrentFile()
        {
            _currentFile = Path.GetFullPath(_currentFile);

            _state = new CurrentState();

            var options = new[]
            {
                "-x",
                "c++",
                "--comments",
                "--comments-in-macros",
                "-fparse-all-comments",
                "-D__HEART_CODEGEN_ACTIVE",
                "--include-directory=C:\\Users\\James\\source\\repos\\cpp-fun-with-sfml\\game\\heart-core\\include",
            };

            var optionBytes = options.Select(x => Encoding.ASCII.GetBytes(x)).ToArray();

            unsafe
            {
                var index = clang.createIndex(0, 0);

                var bytes = Encoding.ASCII.GetBytes(_currentFile);

                sbyte* file;
                fixed (byte* p = bytes)
                    file = (sbyte*)p;

                var optionSb = new sbyte*[optionBytes.Length];
                for (int i = 0; i < optionBytes.Length; ++i)
                {
                    fixed (byte* p = optionBytes[i])
                        optionSb[i] = (sbyte*)p;
                }

                CXTranslationUnitImpl* rawTranslationUnit;

                fixed (sbyte** sb = optionSb)
                    rawTranslationUnit = clang.parseTranslationUnit(index, file, sb, optionBytes.Length, null, 0, (uint)CXTranslationUnit_Flags.CXTranslationUnit_None);

                var indexCursor = clang.getTranslationUnitCursor(rawTranslationUnit);
                var clientData = new CXClientData(new IntPtr(rawTranslationUnit));
                indexCursor.VisitChildren(VisitChildren, clientData);

                clang.disposeTranslationUnit(rawTranslationUnit);
                clang.disposeIndex(index);
            }

            _currentFile = "";
        }

        private enum SerializeState
        {
            Scanning,
            SerializeNextStruct,
            SerializeCurrentParent,
            SerializeCurrentParentNextAsRef,
        }

        private struct CurrentState
        {
            public SerializeState state;
            public CXCursor? serializedParent;
        }

        CurrentState _state;

        private unsafe CXChildVisitResult VisitChildren(CXCursor cursor, CXCursor parent, void* client_data)
        {
            if (cursor.Spelling.CString == "HEARTGEN___SERIALIZE_NEXT_SYMBOL_STRUCT")
            {
                if (_state.state != SerializeState.Scanning)
                {
                    EncounterError("Cannot parse nested serialized structures!", cursor.Location);
                }
                else
                {
                    _state.state = SerializeState.SerializeNextStruct;
                }

                return CXChildVisitResult.CXChildVisit_Continue;
            }

            if (cursor.Spelling.CString == "HEARTGEN___SERIALIZE_NEXT_SYMBOL_AS_REF")
            {
                if (_state.state != SerializeState.SerializeCurrentParent)
                {
                    EncounterError("Unable to parse serialization directive; not inside serialized structure?", cursor.Location);
                }
                else
                {
                    _state.state = SerializeState.SerializeCurrentParentNextAsRef;
                }

                return CXChildVisitResult.CXChildVisit_Continue;
            }

            if (_state.state == SerializeState.SerializeNextStruct)
            {
                if (cursor.CXXAccessSpecifier != CX_CXXAccessSpecifier.CX_CXXPublic)
                {
                    EncounterError("Ignoring serialized struct - cannot serialize non-public structures!", cursor.Location);
                    _state.serializedParent = null;
                    _state.state = SerializeState.Scanning;
                    return CXChildVisitResult.CXChildVisit_Continue;
                }
                else
                {
                    _state.serializedParent = cursor;
                    _state.state = SerializeState.SerializeCurrentParent;
                    return CXChildVisitResult.CXChildVisit_Recurse;
                }
            }

            if (_state.state == SerializeState.Scanning)
                return CXChildVisitResult.CXChildVisit_Recurse;

            if (parent != _state.serializedParent)
            {
                _state.state = SerializeState.Scanning;
                _state.serializedParent = null;
                return CXChildVisitResult.CXChildVisit_Recurse;
            }

            // FINALLY we're inside a serialized struct! Let's look at the current cursor!
            if (cursor.Kind == CXCursorKind.CXCursor_FieldDecl && cursor.CXXAccessSpecifier == CX_CXXAccessSpecifier.CX_CXXPublic)
            {
                var parentType = _state.serializedParent.GetValueOrDefault().Type.CanonicalType.Spelling.CString;
                var fieldName = cursor.Spelling.CString;

                if (!_typeFields.ContainsKey(parentType))
                    _typeFields[parentType] = new List<FieldToken>();

                bool asRef = _state.state == SerializeState.SerializeCurrentParentNextAsRef;
                _state.state = SerializeState.SerializeCurrentParent;

                // TODO: Don't do this linearly?
                if (!_typeFields[parentType].Any(x => x.name == fieldName))
                {
                    CXFile file;
                    uint line, column, row;
                    cursor.Location.GetFileLocation(out file, out line, out column, out row);

                    _includes.Add(GetRelativePath(file.Name.CString));
                    _typeFields[parentType].Add(new FieldToken(fieldName, asRef));
                }

                return CXChildVisitResult.CXChildVisit_Continue;
            }

            return CXChildVisitResult.CXChildVisit_Continue;
        }
    }
}