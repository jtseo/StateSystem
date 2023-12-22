using Google.Apis.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using TMPro;
using UI;

namespace StateSystem
{
    public class StateFunction
    {
        public int m_func; // hash for function name 'StateNode.EventCast'
        public IntPtr m_evt_p, m_context_p;
        private StateDStructureValue m_base = null;
        private StateDStructureValue m_evt = null;
        private StateDStructureValue m_variable = null;

        public void set(int _func, StateDStructureValue _base, IntPtr _evt_p, IntPtr _context, StateDStructureValue _variable, StateDStructureValue _evt)
        {
            m_func = _func;
            m_evt_p = _evt_p;
            m_context_p = _context;
            m_base = _base;
            m_evt = _evt;
            m_variable = _variable;
        }

        public StateDStructureValue BaseGet()
        {
            return m_base;
        }
        public StateDStructureValue EvtGet()
        {
            if (m_evt == null)
                m_evt = new StateDStructureValue(m_evt_p);
            return m_evt;
        }
        public StateDStructureValue VariableGet()
        {
            if (m_variable == null)
                m_variable = m_base.state_variable_get();
            return m_variable;
        }

        public bool ParamVariableGet(ref int _value)
        {
            int hash = 0;
            if (!m_base.get_int(m_func, ref hash))
                return false;

            if (!m_variable.get_int(hash, ref _value))
                return false;

            return true;
        }

        public bool ParamFallowGet(int _seq, ref int _value)
        {
            return m_base.param_get(m_evt_p, m_context_p, _seq, ref _value);
        }

        public bool ParamFallowGet(int _seq, ref string _value)
        {
            return m_base.param_get(m_evt_p, m_context_p, _seq, ref _value);
        }

        public int ParamIntGet()
        {
            int value = 0;
            if (!m_base.get_int(m_func, ref value))
                return -1;
            return value;
        }

        public string ParamStringGet()
        {
            string str = "";
            if (!m_base.get_string(m_func, ref str))
                return "";
            return str;
        }

        public bool ParamVariableSet(int _value)
        {
            int hash = 0;
            if (!m_base.get_int(m_func, ref hash))
                return false;

            if (!m_variable.set_variable(hash, _value))
                return false;
            return true;
        }

        public IStateNode ClassGet(string _classname)
        {
            IStateNode node = default;
            GCHandle handle = new GCHandle();
            if (!m_variable.get_ptr(_classname, ref handle))
                return node;
            node = (handle.Target as IStateNode);
            return node;
        }
    }

    public interface IStateNode
    {
        enum EnumFunc
        {
            EnumNull,
            Create,
            Delete,
            groupId_nIf,
            EventCast,
            IdGet_varF,
            EnumFuncMax
        };

        int m_id { get; set; }
        int m_objId { get; set; }
        string m_name { get; }
        StateDStructureValue m_baseLast { get; set; }
        
        static int StateFuncRegist(string _class_name, List<int> _funcs, int _size, StateInterface.fnFuncProcessor _processor)
        {
            FunctionRegist(_class_name, _funcs, (int)IStateNode.EnumFunc.Create, "Create", _processor);
            FunctionRegist(_class_name, _funcs, (int)IStateNode.EnumFunc.Delete, "Delete", _processor);
            FunctionRegist(_class_name, _funcs, (int)IStateNode.EnumFunc.EventCast, "EventCast", _processor);
            FunctionRegist(_class_name, _funcs, (int)IStateNode.EnumFunc.groupId_nIf, "groupId_nIf", _processor);
            FunctionRegist(_class_name, _funcs, (int)IStateNode.EnumFunc.IdGet_varF, "IdGet_varF", _processor);

            return (int)IStateNode.EnumFunc.EnumFuncMax;
        }
        static int StateFuncRegist(string _class_name, ref List<int> _funcs, int _size, StateInterface.fnFuncProcessor _processor)
        {
            if (_funcs == null || _funcs.Count() == 0)
            {
                _funcs = new List<int>(_size);
                for(int i=0; i<_size; i++)
                    _funcs.Add(0);
                StateFuncRegist(_class_name, _funcs, _size, _processor);
                return (int)IStateNode.EnumFunc.EnumFuncMax;
            }
            return 0;
        }

        static IStateNode FunctionProcessorHeader(StateFunction _func, string _class_name, int _obj_id, IntPtr _base, IntPtr _event,
            IntPtr _context, List<int> _funcs)
        {
            StateDStructureValue variable = _func.VariableGet();

            IStateNode node = null;
            bool ret = false;
            if (_func.m_func != _funcs[(int)IStateNode.EnumFunc.Create])
            {
                GCHandle handle = new GCHandle();
                ret = variable.get_ptr(_class_name, ref handle);
                if (ret)
                {
                    node = (handle.Target as IStateNode);
                }
            }

            StateDStructureValue dsvEvt = null;
            if (node == null)
            {
                dsvEvt = _func.EvtGet();
                if (dsvEvt == null)
                    return null;
                GCHandle handle = new GCHandle();
                ret = dsvEvt.get_ptr("ParamInt64", ref handle);
                if (ret)
                    node = (handle.Target as IStateNode);

                if (node != null)
                {
                    node.GroupIdSet(_func);
                    variable.set_variable(_class_name, handle);
                }
            }
            
            return node;
        }

        static void event_create_post(string _event, IStateNode _node)
        {
            StateDStructureValue evt = VLStateManager.event_state_make(_event);
            set_ptr(evt, "ParamInt64", _node);
            VLStateManager.event_post(evt);
        }
        static void set_ptr(StateDStructureValue _dsv, string _key, IStateNode _node)
        {
            GCHandle handle = GCHandle.Alloc(_node);
            _dsv.set_variable(_key, handle);
        }
        
        static void FunctionRegist(string _class_name, List<int> _funcs, int _idx, string _funName, StateInterface.fnFuncProcessor _processor)
        {
            string func_str = _class_name + ".";
            func_str += _funName;
            _funcs[_idx] = VLStateManager.hash(func_str);
            VLStateManager.ProcessReg(func_str, _processor, func_str, 0);
        }

        int FunctionCall(StateFunction _func, List<int> _funcs);
        void GroupIdSet(StateFunction _func);
        int IdGet();
        
        static void EventCast(IStateNode _node, string _event)
        {
            StateDStructureValue evt = VLStateManager.create_event_group(VLStateManager.hash(_event), _node.m_objId, _node.m_id);
            if (evt == null)
                return;
            VLStateManager.event_post(evt);
        }
    }
    public class StateNode
    {
        private IStateNode m_istate;

        public void set(IStateNode _istate)
        {
            m_istate = _istate;
        }
        public int FunctionCall(StateFunction _func, List<int> _funcs)
        {
            //#SF_FuncCallStart
            if (_func.m_func == _funcs[(int)IStateNode.EnumFunc.EventCast]) return EventCast(_func);
            else if (_func.m_func == _funcs[(int)IStateNode.EnumFunc.groupId_nIf]) return groupId_nIf(_func);
            else if (_func.m_func == _funcs[(int)IStateNode.EnumFunc.IdGet_varF]) return IdGet_varF(_func);
            else if (_func.m_func == _funcs[(int)IStateNode.EnumFunc.Delete]) return Delete(_func);
            //#SF_FuncCallInsert
            return 0;
        }

        public StateDStructureValue EventMake(int _evt)
        {
            if (m_istate.m_baseLast == null)
                return null;
            StateDStructureValue evt = m_istate.m_baseLast.event_group_make(_evt, m_istate.m_objId, m_istate.m_id);
            return evt;
        }

        public void EventCast(StateDStructureValue _evt)
        {
            VLStateManager.event_post(_evt);
            m_istate.m_baseLast.event_cast_reset();
        }
        public int EventCast(StateFunction _func)
        {
            int evtHash = 0;
            evtHash = _func.ParamIntGet();
            StateDStructureValue evt = _func.BaseGet().event_group_make(evtHash, m_istate.m_objId, -1);// if id is -1, this event will be for a group. 
            // mean that the other groups can't receive this event.
            EventCast(evt);
            return 0;
        }

        int groupId_nIf(StateFunction _func)
        {
            if (_func.ParamIntGet() != m_istate.m_objId)
                return 0;
            return 1;
        }

        int IdGet_varF(StateFunction _func)
        {
            if (!_func.ParamVariableSet(m_istate.m_id))
                return 0;
            return 1;
        }

        int Delete(StateFunction _func)
        {
            m_istate.m_baseLast = null;
            GCHandle handle = new GCHandle();
            if (!_func.VariableGet().get_ptr(m_istate.m_name, ref handle))
                return 0;
            handle.Free();
            return 1;
        }

        public bool EventFunc(int _hash, ref int _ret)
        {
            StateDStructureValue evt = EventMake(_hash);
            if (evt == null)
                return false;
            return VLStateManager.event_func(evt, _hash, ref _ret);
        }
    }
}