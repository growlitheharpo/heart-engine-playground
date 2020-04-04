using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Text.RegularExpressions;

using ClangSharp;
using ClangSharp.Interop;

namespace Heart.Codegen
{
	public class SerializationGen
	{
		// If this changes, update deserialization_fwd.h in heart-core!
		private const int SerializedDataPathSize = 64;

		public static int ProcessSourceDirectory(string dir, string heartLocation)
		{
			var heartDirs = new[] { heartLocation, heartLocation + "\\..\\heart-stl", heartLocation + "\\..\\heart-debug" };

			SerializationGen generator = new SerializationGen(dir, heartDirs);
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
		private string[] _heartDirectories;

		private string _directory;
		private HashSet<string> _includes = new HashSet<string>();
		private HashSet<int> _serializedStringSizes = new HashSet<int>();
		private HashSet<string> _serializedVectorTypes = new HashSet<string>();
		private Dictionary<string, List<FieldToken>> _typeFields = new Dictionary<string, List<FieldToken>>();

		private SerializationGen(string dir, string[] heartDirs)
		{
			_directory = dir;
			_heartDirectories = heartDirs;
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

			Encoding encoding = Encoding.ASCII;

			using (var stream = new MemoryStream())
			{
				using (var streamWriter = new StreamWriter(stream, encoding))
				using (var writer = new CodeWriter(streamWriter))
				{
					writer.WriteLine("/*\tWRITTEN BY HEART-CODEGEN\t*/");
					writer.WriteLine();

					WriteHeader(writer);

					using (var function = new FunctionBlock(writer, "void ReflectSerializedData()"))
					{
						ReflectBaseTypes(function);
						ReflectSerializedStrings(function);
						ReflectSerializedVectors(function);
						ReflectSerializedStructs(function);
					}

					writer.WriteLine();
					writer.WriteLine("/*\tWRITTEN BY HEART-CODEGEN\t*/");
				}

				bool commit = true;
				var streamContents = stream.ToArray();
				try
				{
					var current = File.ReadAllText($"{_directory}\\gen\\reflection.heartgen.cpp");

					string hash1;
					using(SHA1CryptoServiceProvider sha1 = new SHA1CryptoServiceProvider())
						hash1 = Convert.ToBase64String(sha1.ComputeHash(encoding.GetBytes(current)));

					string hash2;
					using(SHA1CryptoServiceProvider sha1 = new SHA1CryptoServiceProvider())
						hash2 = Convert.ToBase64String(sha1.ComputeHash(streamContents));

					if (string.Equals(hash1, hash2, StringComparison.OrdinalIgnoreCase))
						commit = false;
				}
				catch { }

				if (commit)
				{
					Console.WriteLine("heart-codegen: reflection.heartgen.cpp");
					File.WriteAllBytes($"{_directory}\\gen\\reflection.heartgen.cpp", streamContents);
				}
				else
				{
					Console.WriteLine("heart-codegen: reflection.heartgen.cpp - SKIPPED (nothing changed)");
				}
			}
		}

		private void WriteHeader(ICodeWriter writer)
		{
			writer.WriteLine("#include \"gen/gen.h\"");
			writer.WriteLine("#include <heart/deserialization.h>");
			writer.WriteLine();
			foreach (var incl in _includes.Distinct())
				writer.WriteLine($"#include \"{incl}\"");
			writer.WriteLine();

			writer.WriteLine("template <typename T>");
			writer.WriteLine("void ReflectionSet(T& prop, T value)");
			using (var f = new IndentLevel(writer, "{", "}"))
				f.WriteLine("prop = value;");
			writer.WriteLine();

			writer.WriteLine("template <typename T>");
			writer.WriteLine("T ReflectionGet(T& prop)");
			using (var f = new IndentLevel(writer, "{", "}"))
				f.WriteLine("return prop;");
			writer.WriteLine();
		}

		private void ReflectBaseTypes(ICodeWriter writer)
		{
			var baseTypes = new[] { "int8_t", "uint8_t", "int16_t", "uint16_t", "int32_t", "uint32_t", "int64_t", "uint64_t", "bool", "float", "double" };
			foreach (var type in baseTypes)
			{
				using (var baseType = new IndentLevel(writer, $"BEGIN_SERIALIZE_TYPE({type})", $"END_SERIALIZE_TYPE({type})"))
				{
					baseType.WriteLine($"SERIALIZE_SELF_ACCESS({type}, &ReflectionSet<{type}>, &ReflectionGet<{type}>)");

					if (type == "uint32_t")
					{
						baseType.WriteLine("SERIALIZE_CONVERSION(uint32_t, int32_t)");
						baseType.WriteLine("SERIALIZE_CONVERSION(uint32_t, int16_t)");
						baseType.WriteLine("SERIALIZE_CONVERSION(uint32_t, int8_t)");
					}
					else if (type == "int32_t")
					{
						baseType.WriteLine("SERIALIZE_CONVERSION(int32_t, uint32_t)");
						baseType.WriteLine("SERIALIZE_CONVERSION(int32_t, uint16_t)");
						baseType.WriteLine("SERIALIZE_CONVERSION(int32_t, uint8_t)");
					}
				}

				writer.WriteLine();
			}
		}

		private void ReflectSerializedStrings(ICodeWriter writer)
		{
			_serializedStringSizes.Add(SerializedDataPathSize);

			using (var baseType = new IndentLevel(writer, "BEGIN_SERIALIZE_TYPE_ADDITIVE(const char*)", "END_SERIALIZE_TYPE(const char*)"))
			{
				foreach (var size in _serializedStringSizes)
					baseType.WriteLine($"SERIALIZE_CONVERSION(const char*, &SerializedString<{size}>::CreateFromCString)");
			}
			writer.WriteLine();

			foreach (var size in _serializedStringSizes)
			{
				using (var block = new IndentLevel(writer, $"BEGIN_SERIALIZE_TYPE_ADDITIVE(SerializedString<{size}>)", $"END_SERIALIZE_TYPE(SerializedString<{size}>)"))
					block.WriteLine($"SERIALIZE_SELF_ACCESS(SerializedString<{size}>, &ReflectionSet<SerializedString<{size}>>, &ReflectionGet<SerializedString<{size}>>)");
				writer.WriteLine();
			}
		}

		private void ReflectSerializedVectors(ICodeWriter writer)
		{
			foreach (var vectorType in _serializedVectorTypes)
			{
				string type = $"hrt::vector<{vectorType}>";

				using (var vec = new IndentLevel(writer, $"BEGIN_SERIALIZE_TYPE({type})", $"END_SERIALIZE_TYPE({type})"))
				{
					vec.WriteLine($"SERIALIZE_FUNCTION_ALIAS({type}, emplace_back<>)");
					vec.WriteLine($"SERIALIZE_FUNCTION_ALIAS({type}, reserve)");
				}
				writer.WriteLine();
			}
		}

		private void ReflectSerializedStructs(ICodeWriter writer)
		{
			foreach (var typePair in _typeFields)
			{
				var type = typePair.Key;

				using (var typeBlock = new IndentLevel(writer, $"BEGIN_SERIALIZE_TYPE({type})", $"END_SERIALIZE_TYPE({type})"))
				{
					foreach (var field in typePair.Value)
					{
						if (field.asFunc)
							typeBlock.WriteLine($"SERIALIZE_FUNCTION({type}, {field.name})");
						else if (field.asRef)
							typeBlock.WriteLine($"SERIALIZE_FIELD_ALIAS({type}, {field.name})");
						else
							typeBlock.WriteLine($"SERIALIZE_FIELD({type}, {field.name})");
					}
				}

				writer.WriteLine();
			}
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
			}.Concat(_heartDirectories.Select(x => $"--include-directory={x}\\include"));

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
