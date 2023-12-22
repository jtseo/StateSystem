using UnityEngine;
using System.Collections;

namespace StateSystem
{
	public class UxUndo
	{
		public enum UndoType
		{
			None,
			Delete,
			Move,
			Cutter,
			Create,
			CreateLibrary,
			Update
		};

		protected UndoType m_undotype_n;

		public UndoType Type()
		{
			return m_undotype_n;
		}

		public virtual void Do()
		{
		}

		public virtual void Redo(int _serial = 0)
		{
		}

		public virtual void Undo(int _serial = 0)
		{
		}

		public virtual bool Merge(UxUndo _Undo)
		{
			return false;
		}

		public void DebugMsg(string _strPre)
		{

			switch (m_undotype_n)
			{
				case UndoType.None:
					VLStateManager.message(1, _strPre + " Stack:None");
					break;
				case UndoType.Delete:
					VLStateManager.message(1, _strPre + " Stack:Delete");
					break;
				case UndoType.Cutter:
					VLStateManager.message(1, _strPre + " Stack:Cutter");
					break;
			}
		}
	}
}