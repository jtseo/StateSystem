using UnityEngine;
using System.Collections;
using System.Collections.Generic;

using System;
using System.Runtime.InteropServices;
using System.IO;
using UnityEngine.UI;
using System.Text;

namespace StateSystem
{
    public class UndoStackDLLAction : UxUndo
    {
        IntPtr m_ptr_dll;

        public UndoStackDLLAction()
        {
        }

        ~UndoStackDLLAction()
        {
            //StateInterface.action_del(m_ptr_dll);
        }

        public void init(IntPtr _action)
        {
            m_ptr_dll = _action;
        }

        public override bool Merge(UxUndo _Undo)
        {
            // true를 리턴할 경우 parameter로 들어온 Undo는  this에 값을 적용하고 삭제된다.
            return false;
        }

        public override void Do()
        {

        }

        public override void Undo(int _serial = 0)
        {
            //StateInterface.action_undo(m_ptr_dll, _serial);
        }

        public override void Redo(int _serial = 0)
        {
            //StateInterface.action_redo(m_ptr_dll, _serial);
        }
    }

    public class UndoStack : MonoBehaviour
    {
        public static List<UxUndo> m_stlVDoStack;
        public static List<UxUndo> m_stlVUndoStack;

        public UndoStack()
        {
            m_stlVDoStack = new List<UxUndo>();
            m_stlVUndoStack = new List<UxUndo>();
        }

        static public UxUndo undo_top_get()
        {
            if (m_stlVDoStack.Count == 0)
                return null;
            return m_stlVDoStack[m_stlVDoStack.Count - 1];
        }

        static public void Clear()
        {
            m_stlVDoStack.Clear();
            m_stlVUndoStack.Clear();
        }

        public bool IsEmptyDo()
        {
            if (m_stlVDoStack.Count.Equals(0))
                return true;
            return false;
        }

        public void copy_undo(List<UxUndo> _undoStack)
        {
            for (int i = 0; i < m_stlVUndoStack.Count; i++)
            {
                _undoStack.Add(m_stlVUndoStack[i]);
            }
        }

        public void replace_undo(List<UxUndo> _undoStack)
        {
            m_stlVUndoStack = _undoStack;
        }

        static public void DoCutter(UxUndo _actor)
        {
            for (int i = 0; i < m_stlVDoStack.Count; i++)
            {
                UxUndo actor = m_stlVDoStack[i];
                if (actor.Type() == UxUndo.UndoType.Cutter)
                {
                    m_stlVDoStack.RemoveAt(i);
                    break;
                }
            }

            _actor.Do();
            _actor.DebugMsg("DoCuttor:");
            m_stlVDoStack.Add(_actor);
        }

        static public void CutterLastRemove()
        {
            int nLast = m_stlVDoStack.Count - 1;
            UxUndo actor = m_stlVDoStack[nLast];
            if (actor.Type() == UxUndo.UndoType.Cutter)
                m_stlVDoStack.RemoveAt(nLast);
        }

        static public void Do(UxUndo _actor)
        {
            VLStateManager.event_state_post("UxUndoStackPush");
            bool bMerged = false;
            _actor.Do();
            if (m_stlVDoStack.Count > 0)
            {
                UxUndo actor = m_stlVDoStack[m_stlVDoStack.Count - 1];
                if (actor.Merge(_actor))
                    bMerged = true;
            }

            if (!bMerged)
            {
                m_stlVDoStack.Add(_actor);
                _actor.DebugMsg("Do:");
            }

            m_stlVUndoStack.Clear();
        }

        static public void Cancel()
        {
            UxUndo actor;

            if (m_stlVDoStack.Count == 0)
                return;

            bool bFound = false;
            for (int i = 0; i < m_stlVDoStack.Count; i++)
            {
                actor = m_stlVDoStack[i];
                if (actor.Type() == UxUndo.UndoType.Cutter)
                {
                    bFound = true;
                    break;
                }
            }

            if (!bFound) // without Cuttor, don't try to cancel stack.
                return;

            do
            {
                actor = m_stlVDoStack[m_stlVDoStack.Count - 1];
                m_stlVDoStack.RemoveAt(m_stlVDoStack.Count - 1);
                actor.Undo();
                actor.DebugMsg("Cancel:");
            } while (actor.Type() != UxUndo.UndoType.Cutter &&
                     m_stlVDoStack.Count > 0);
            //m_stlVUndoStack.Add (actor);
        }

        static public bool Undo(int _serial)
        {
            if (m_stlVDoStack.Count == 0)
                return false;

            UxUndo actor;
            do
            {
                actor = m_stlVDoStack[m_stlVDoStack.Count - 1];
                m_stlVDoStack.RemoveAt(m_stlVDoStack.Count - 1);
                if (actor.Type() != UxUndo.UndoType.Cutter)
                {
                    VLStateManager.event_state_post("UxUndoStackPop");
                    actor.Undo(_serial);
                    m_stlVUndoStack.Add(actor);
                }
            } while (actor.Type() == UxUndo.UndoType.Cutter &&
                     m_stlVDoStack.Count > 0);

            if (m_stlVDoStack.Count == 0)
                VLStateManager.event_state_post("UxUndoStackEmpty");

            return true;
        }

        static public bool Redo(int _serial)
        {
            if (m_stlVUndoStack.Count == 0)
                return false;

            UxUndo actor;
            actor = m_stlVUndoStack[m_stlVUndoStack.Count - 1];
            m_stlVUndoStack.RemoveAt(m_stlVUndoStack.Count - 1);
            actor.Redo(_serial);
            m_stlVDoStack.Add(actor);

            return true;
        }

        private void Awake()
        {

        }

        private void Start()
        {
        }

        public static void StateFuncRegist()
        {
            VLStateManager.ProcessReg("UxUndo_nF", UxUndo_nF);
            VLStateManager.ProcessReg("UxRedo_nF", UxRedo_nF);
            VLStateManager.ProcessReg("UxUndoStackClear_nF", UxUndoStackClear_nF);
            VLStateManager.ProcessReg("BaseUndoStackDLLPush", BaseUndoStackDLLPush);
        }

        public static int BaseUndoStackDLLPush(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            UndoStackDLLAction action = new UndoStackDLLAction();

            action.init(_pBase);
            UndoStack.Do(action);
            return 1;
        }

        public static int UxUndo_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            int serial = stBase.serial_get();

            UndoStack.Undo(serial);
            return 1;
        }

        public static int UxRedo_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            int serial = stBase.serial_get();

            UndoStack.Redo(serial);
            return 1;
        }

        public static int UxUndoStackClear_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            UndoStack.Clear();
            return 1;
        }
    }
}