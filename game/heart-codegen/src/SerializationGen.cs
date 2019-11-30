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
        private static string[] HeartDirectories;

        public static int ProcessSourceDirectory(string dir, string heartLocation)
        {
            HeartDirectories = new [] { heartLocation, heartLocation + "\\..\\heart-stl", heartLocation + "\\..\\heart-debug" };

            SerializationGen generator = new SerializationGen(dir);
            generator.Process();

            return generator.ErrorCode;
        }

        private struct FieldToken
        {
            public string name;
            public bool asRef;
            public bool asFunc;

            public FieldToken(string n, bool r, bool f)
            {
                name = n;
                asRef = r;
                asFunc = f;
            }
        }

        private int ErrorCode { get; set; }

        private string _currentFile;

        private string _directory;
        private HashSet<string> _includes = new HashSet<string>();
        private HashSet<int> _serializedStringSizes = new HashSet<int>();
        private HashSet<string> _serializedVectorTypes = new HashSet<string>();
        private Dictionary<string, List<FieldToken>> _typeFields = new Dictionary<string, List<FieldToken>>();

        private SerializationGen(string dir) 
        {
            _directory = dir;
        }

        private void EncounterError(string msg, CXSourceLocation location)
        {
            Console.Error.WriteLine($"heart-codegen error: {msg}");
            Console.Error.WriteLine($"\tEncountered at: {location.ToString()}");

            System.Diagnostics.Debug.WriteLine($"heart-codegen error: {msg}");
            System.Diagnostics.Debug.WriteLine($"\tEncountered at: {location.ToString()}");

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
                writer.WriteLine("\tentt::meta<int32_t>().conv<uint32_t>().conv<uint16_t>().conv<uint8_t>();");
                writer.WriteLine("\tentt::meta<uint32_t>().conv<int32_t>().conv<uint16_t>().conv<uint8_t>();");
                writer.WriteLine();
                if (_serializedStringSizes.Count > 0)
                {
                    foreach (var size in _serializedStringSizes)
                        writer.WriteLine($"\tentt::meta<const char*>().conv<&SerializedString<{size}>::CreateFromCString>();");
                }
                foreach (var vectorType in _serializedVectorTypes)
                {
                    writer.WriteLine();
                    string type = $"hrt::vector<{vectorType}>";
                    writer.WriteLine($"\tBEGIN_SERIALIZE_TYPE({type})");
                    writer.WriteLine($"\t\tSERIALIZE_FUNCTION_ALIAS({type}, emplace_back<>)");
                    writer.WriteLine($"\t\tSERIALIZE_FUNCTION_ALIAS({type}, reserve)");
                    writer.WriteLine($"\tEND_SERIALIZE_TYPE({type})");
                }
                foreach (var typePair in _typeFields)
                {
                    writer.WriteLine();
                    var type = typePair.Key;
                    writer.WriteLine($"\tBEGIN_SERIALIZE_TYPE({type})");

                    foreach (var field in typePair.Value)
                    {
                        if (field.asFunc)
                            writer.WriteLine($"\t\tSERIALIZE_FUNCTION({type}, {field.name})");
                        else if (field.asRef)
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

                // We don't parse generated files... duh
                if (file.Contains(".heartgen.cpp"))
                    continue;

                _currentFile = file;

                ProcessCurrentFile();
            }
        }

        private void ProcessCurrentFile()
        {
            _currentFile = Path.GetFullPath(_currentFile);

            _state = new CurrentState();

            var options = new string[]
            {
                "-x",
                "c++",
                "--comments",
                "--comments-in-macros",
                "-fparse-all-comments",
                "-D__HEART_CODEGEN_ACTIVE",
            }.Concat(HeartDirectories.Select(x => $"--include-directory={x}\\include"));

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
            SerializeCurrentParentNextAsFunc,
        }

        private struct CurrentState
        {
            public SerializeState state;
            public CXCursor? serializedParent;
        }

        CurrentState _state;

        private unsafe CXChildVisitResult VisitChildren(CXCursor cursor, CXCursor parent, void* client_data)
        {
            if (_state.serializedParent != null && _state.serializedParent != parent)
            {
                if (_state.state != SerializeState.SerializeCurrentParent)
                {
                    EncounterError("Unexpected end of declaration while parsing serialized struct!", cursor.Location);
                }

                _state.state = SerializeState.Scanning;
                _state.serializedParent = null;
            }

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

            if (cursor.Spelling.CString == "HEARTGEN___SERIALIZE_NEXT_SYMBOL_AS_MEMB_FUNCTION")
            {
                if (_state.state != SerializeState.SerializeCurrentParent)
                {
                    EncounterError("Unable to parse serialization directive; not inside serialized structure?", cursor.Location);
                }
                else
                {
                    _state.state = SerializeState.SerializeCurrentParentNextAsFunc;
                }

                return CXChildVisitResult.CXChildVisit_Continue;
            }

            if (_state.state == SerializeState.SerializeNextStruct)
            {
                if (cursor.CXXAccessSpecifier != CX_CXXAccessSpecifier.CX_CXXPublic && cursor.CXXAccessSpecifier != CX_CXXAccessSpecifier.CX_CXXInvalidAccessSpecifier)
                {
                    EncounterError($"Ignoring serialized struct {cursor.Spelling.CString} - cannot serialize non-public structures!", cursor.Location);
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

            bool shouldBeFunc = _state.state == SerializeState.SerializeCurrentParentNextAsFunc;

            bool valid = cursor.CXXAccessSpecifier == CX_CXXAccessSpecifier.CX_CXXPublic && (
                (!shouldBeFunc && cursor.Kind == CXCursorKind.CXCursor_FieldDecl) ||
                (shouldBeFunc && cursor.Kind == CXCursorKind.CXCursor_CXXMethod));

            // FINALLY we're inside a serialized struct! Let's look at the current cursor!
            if (valid)
            {
                var parentType = _state.serializedParent.GetValueOrDefault().Type.CanonicalType.Spelling.CString;
                var fieldName = cursor.Spelling.CString;

                if (!_typeFields.ContainsKey(parentType))
                    _typeFields[parentType] = new List<FieldToken>();

                bool asRef = _state.state == SerializeState.SerializeCurrentParentNextAsRef;
                bool asFunc = shouldBeFunc;
                _state.state = SerializeState.SerializeCurrentParent;

                // TODO: Don't do this linearly?
                if (!_typeFields[parentType].Any(x => x.name == fieldName))
                {
                    CXFile file;
                    uint line, column, row;
                    cursor.Location.GetFileLocation(out file, out line, out column, out row);

                    _includes.Add(GetRelativePath(file.Name.CString));
                    _typeFields[parentType].Add(new FieldToken(fieldName, asRef, asFunc));
                }

                var fieldType = cursor.Type.Spelling.CString;
                var serializedStringRgx = new Regex("(SerializedString<)(\\d+)(>)");
                if (serializedStringRgx.IsMatch(fieldType))
                {
                    int size = int.Parse(serializedStringRgx.Match(fieldType).Groups[2].Value);
                    _serializedStringSizes.Add(size);
                }
                var hrtVectorRgx = new Regex("(hrt::vector<)(.+)(>)");
                if (hrtVectorRgx.IsMatch(fieldType))
                {
                    string typeName = hrtVectorRgx.Match(fieldType).Groups[2].Value;
                    _serializedVectorTypes.Add(typeName);
                }

                return CXChildVisitResult.CXChildVisit_Continue;
            }
            else
            {
                if (_state.state != SerializeState.SerializeCurrentParent)
                    EncounterError("Invalid serialization directive preceding nonserializable cursor!", cursor.Location);

                _state.state = SerializeState.SerializeCurrentParent;
            }

            return CXChildVisitResult.CXChildVisit_Continue;
        }
    }
}
