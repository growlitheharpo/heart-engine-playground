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
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace Heart.Codegen
{
	public interface ICodeWriter
	{
		void Dispose();

		void WriteLine(string line);
		void WriteLine();

		ICodeWriter GetParent();
		int GetIndentLevel();
	}

	public abstract class BaseCodeWriter : ICodeWriter, IDisposable
	{
		private ICodeWriter _parent;

		protected List<string> _lines = new List<string>();
		protected int _indentLevel = 0;

		protected BaseCodeWriter(ICodeWriter parent)
		{
			_parent = parent;
		}

		public void WriteLine(string line)
		{
			StringBuilder sb = new StringBuilder();
			for (int i = 0; i < _indentLevel; ++i)
				sb.Append("\t");
			sb.Append(line);
			_lines.Add(sb.ToString());
		}

		public void WriteLine()
		{
			_lines.Add(string.Empty);
		}

		public ICodeWriter GetParent()
		{
			return _parent;
		}

		public int GetIndentLevel()
		{
			return _indentLevel;
		}

		public abstract void Dispose();
	}

	public class CodeWriter : BaseCodeWriter
	{
		StreamWriter _stream;

		public CodeWriter(StreamWriter stream) : base(null)
		{
			_stream = stream;
		}

		public override void Dispose()
		{
			foreach (var l in _lines)
				_stream.WriteLine(l);
		}
	}

	public class IndentLevel : BaseCodeWriter
	{
		private string _endLine;

		public IndentLevel(ICodeWriter parent, string firstLine = "", string endLine = "") : base(parent)
		{
			if (!string.IsNullOrEmpty(firstLine))
				WriteLine(firstLine);

			_endLine = endLine;

			++_indentLevel;
		}

		public override void Dispose()
		{
			--_indentLevel;
			if (!string.IsNullOrEmpty(_endLine))
			{
				WriteLine(_endLine);
			}

			foreach (var l in _lines)
				GetParent().WriteLine(l);
		}
	}

	public class FunctionBlock : BaseCodeWriter
	{
		public FunctionBlock(ICodeWriter parent, string functionProto) : base(parent)
		{
			WriteLine(functionProto);
			WriteLine("{");
			++_indentLevel;
		}

		public override void Dispose()
		{
			--_indentLevel;
			WriteLine("}");

			foreach (var l in _lines)
				GetParent().WriteLine(l);
		}
	}
}
