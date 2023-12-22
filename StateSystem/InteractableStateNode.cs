using Apin.Interact.Interactables;
using Apin.ZoneMove;
using System;
using System.Collections.Generic;
using UI;

namespace StateSystem
{
    public abstract class InteractableStateNode : IStateNode
    {
        private StateNode m_stateNode = new StateNode();

        protected bool m_finished = false;
        //static protected string m_classname = "InteractableStateNode"; // InteractableStateNode는 한 State에서 하나만 사용할 수 있다.
        protected enum EnumInteractable
        {
            FinishedSet_nF = IStateNode.EnumFunc.EnumFuncMax,
            //#SF_FuncDefineInsert
            EnumFuncMax
        }
        static private int ms_obj_id = -1;
        private int m_loc_id = 0;
        private StateDStructureValue m_localBase = null;

        public int m_objId
        {
            get { return ms_obj_id; }
            set { ms_obj_id = value; }
        }

        public int m_id
        {
            get { return m_loc_id; }
            set { m_loc_id = value; }
        }

        public StateDStructureValue m_baseLast
        {
            get { return m_localBase; }
            set { m_localBase = value; }
        }

        public string m_name
        {
            get { return ClassNameGet(); }
        }

        public abstract int SingleIDGet();
        public abstract int StateFuncRegistExt();
        public abstract int FunctionCallExt(StateFunction _func);
        public abstract string ClassNameGet();

        protected void StateStart_(string _createEvent, string _classname, int _max, ref List<int> _funcs, StateInterface.fnFuncProcessor _func)
        {
            IdGet();
            m_stateNode.set(this);
            StateFuncRegist(_classname, ref _funcs, _max, _func);
            
            IStateNode.event_create_post(_createEvent, this);
        }

        public void GroupIdSet(StateFunction _func)
        {
            _func.BaseGet().group_id_set(m_objId, m_id);
        }

        public static int FunctionProcessor(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState, List<int> _funcs, string _classname)
        {
            StateFunction func = new StateFunction();
            StateDStructureValue dsvBase = new StateDStructureValue(_pBase);
            int _func = dsvBase.state_function_hash_get();
            
            func.set(_func, dsvBase, _pEvent, _pContext, null, null);

            IStateNode node =
                IStateNode.FunctionProcessorHeader(func, _classname, ms_obj_id, _pBase, _pEvent, _pContext, _funcs);
            if (node == null)
                return 0;
            
            InteractableStateNode stateNode = node as InteractableStateNode;
            if (stateNode == null || node is not InteractableStateNode)
                return 0;
            
            node.m_baseLast = dsvBase;
            if (func.m_func == _funcs[(int)IStateNode.EnumFunc.Create])
                return 1;
            int ret = node.FunctionCall(func, _funcs);
            return ret;
        }
        int StateFuncRegist(string _class_name, ref List<int> _funcs, int _size, StateInterface.fnFuncProcessor _processor)
        {
            if (0 < IStateNode.StateFuncRegist(_class_name, ref _funcs, _size, _processor))
            {
                IStateNode.FunctionRegist(_class_name, _funcs, (int)EnumInteractable.FinishedSet_nF, "FinishedSet_nF", _processor);
				//#SF_FuncRegistInsert
                
                StateFuncRegistExt();
                return (int)EnumInteractable.EnumFuncMax;
            }
            return 0;
        }

        
        public int FunctionCall(StateFunction _func, List<int> _funcs)
        {
            int ret = m_stateNode.FunctionCall(_func, _funcs);
            if (ret != 0)
                return ret;
            //#SF_FuncCallStart
            if (_func.m_func == _funcs[(int)EnumInteractable.FinishedSet_nF]) return FinishedSet_nF(_func);
			//#SF_FuncCallInsert
            if (ret != 0)
                return ret;

            return FunctionCallExt(_func);
        }

        public int FinishedSet_nF(StateFunction _func)
        {
            m_finished = true;
            return 1;
        }

        public int IdGet()
        {
            if (m_objId == -1)
                m_objId = VLStateManager.hash(ClassNameGet());

            if (m_id == 0)
            {
                m_id = SingleIDGet();
            }
            return m_id;
        }
        //#SF_FuncInsert
        
        public void EventCast(string _event)
        {
            IStateNode.EventCast(this, _event);
        }
    }
}