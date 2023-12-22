using Apin.Interact.Interactables;
using Apin.Routine;
using Apin.ZoneMove;
using System;
using System.Collections.Generic;
using UnityEngine;

// Interactable Object Assets/Scripts/Apin/Interactables/FunctionalObject.cs

namespace StateSystem
{
    public class FunctionalObjectStateNode : MonoBehaviour, IStateNode
    {
        private StateNode m_stateNode = new StateNode();
        private const string m_classname = "FunctionalObjectStateNode";
        enum EnumNpc
        {
            CooltimerAdd_varF = IStateNode.EnumFunc.EnumFuncMax,
			InteractableIconSet_nF,
            //#SF_FuncDefineInsert
            EnumFuncMax
        }
        static private int ms_obj_id = -1;
        private int m_loc_id = 0;
        private StateDStructureValue m_localBase = null;

        private FunctionalObject m_fnObject = default;

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
            get { return m_classname; }
        }

        void Start()
        {
            m_fnObject = GetComponent<FunctionalObject>();

            IdGet();
            m_stateNode.set(this);
            StateFuncRegist(m_classname, ref ms_funcs, (int)EnumNpc.EnumFuncMax, FunctionProcessor);
            IStateNode.event_create_post("FunctionalObjectCreate", this);
            //m_fnObject.CooltimerOnendSet(CooltimerOnend);
        }

        public void GroupIdSet(StateFunction _func)
        {
            _func.BaseGet().group_id_set(m_objId, m_id);
        }
        
        static List<int>  ms_funcs;

        public static int FunctionProcessor(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateFunction func = new StateFunction();
            StateDStructureValue dsvBase = new StateDStructureValue(_pBase);
            int _func = dsvBase.state_function_hash_get();
            
            func.set(_func, dsvBase, _pEvent, _pContext, null, null);

            IStateNode node =
                IStateNode.FunctionProcessorHeader(func, m_classname, ms_obj_id, _pBase, _pEvent, _pContext, ms_funcs);
            if (node == null)
                return 0;
            
            FunctionalObjectStateNode stateNode = node as FunctionalObjectStateNode;
            if (stateNode == null || node is not FunctionalObjectStateNode)
                return 0;
            
            node.m_baseLast = dsvBase;
            if (func.m_func == ms_funcs[(int)IStateNode.EnumFunc.Create])
                return 1;
            int ret = node.FunctionCall(func, ms_funcs);
            return ret;
        }
        static int StateFuncRegist(string _class_name, ref List<int> _funcs, int _size, StateInterface.fnFuncProcessor _processor)
        {
            if (0 < IStateNode.StateFuncRegist(_class_name, ref _funcs, _size, _processor))
            {
                IStateNode.FunctionRegist(_class_name, _funcs, (int)EnumNpc.CooltimerAdd_varF, "CooltimerAdd_varF", _processor);
				IStateNode.FunctionRegist(_class_name, _funcs, (int)EnumNpc.InteractableIconSet_nF, "InteractableIconSet_nF", _processor);
                //#SF_FuncRegistInsert
                return (int)EnumNpc.EnumFuncMax;
            }
            return 0;
        }

        
        public int FunctionCall(StateFunction _func, List<int> _funcs)
        {
            int ret = m_stateNode.FunctionCall(_func, _funcs);
            if (ret != 0)
                return ret;
            //#SF_FuncCallStart
            if (_func.m_func == _funcs[(int)EnumNpc.CooltimerAdd_varF]) return CooltimerAdd_varF(_func);
			if (_func.m_func == _funcs[(int)EnumNpc.InteractableIconSet_nF]) return InteractableIconSet_nF(_func);
            //#SF_FuncCallInsert
            return 0;
        }

        public int IdGet()
        {
            if (m_objId == -1)
                m_objId = VLStateManager.hash(m_classname);

            if (m_id == 0)
            {
                m_id = m_fnObject.GroupID;
            }
            return m_id;
        }

        int CooltimerAdd_varF(StateFunction _func)
        {
            // zone id, Indexabletype NPC, SingleID id, step(variable paramater)
            int param = 0;
            _func.ParamVariableGet(ref param);
            int time = 0;
            if (!_func.ParamFallowGet(0, ref time))
                return 0;
            //m_fnObject.CooltimerRepair(time, ZoneMoveManager.CurrentZoneID, param);
            m_fnObject.TypeSet(FunctionalObject.eFunctionalObject.LunaMaking);
            return 1;
        }

		int InteractableIconSet_nF(StateFunction _func)
        {
            int t = 0;
            if (!_func.ParamVariableGet(ref t))
                return 0;
            FunctionalObject.eFunctionalObject type = (FunctionalObject.eFunctionalObject)t;
            
            m_fnObject.TypeSet(type);
			return 1;
		}
        //#SF_FuncInsert
        
        public void CooltimerOnend(int _step)
        {
            StateDStructureValue evt = VLStateManager.create_event_group(VLStateManager.hash("CooltimerEnd"), m_objId, m_id);
            if (evt == null)
                return;
            evt.set_variable("CooltimerStep_nV", _step);
            VLStateManager.event_post(evt);
        }

        public void EventCast(string _event)
        {
            IStateNode.EventCast(this, _event);
        }
    }
}