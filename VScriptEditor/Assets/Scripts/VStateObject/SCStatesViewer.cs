using System;
using System.Collections.Generic;
using AOT;
using UnityEngine;


namespace StateSystem
{
	public class SCStatesViewer : MonoBehaviour, IStateNode
	{
		#region StateSystemMacro
		private StateNode m_stateNode = new StateNode();
		private const string m_classname = "SCStatesViewer";
		
		enum EState
		{
			EStart = IStateNode.EnumFunc.EnumFuncMax,
			StatesSelectedSave_strV,
			StatesSelectedLoad_strV,
			StateRename_varF,
			//#SF_FuncEnum
			EStateMax
		}
		static private int ms_obj_id = -1;
		public int m_loc_id = 0;
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
			get { return m_classname; }
		}

		void Awake()
		{
			IdGet();
			m_stateNode.set(this);
		}

		public void GroupIdSet(StateFunction _func)
		{
			_func.BaseGet().group_id_set(m_objId, m_id);
		}
		
		static List<int>  ms_funcs;
		[MonoPInvokeCallback(typeof(StateInterface.fnFuncProcessor))]
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
			
			if (node == null || node is not SCStatesViewer)
				return 0;
			
			node.m_baseLast = dsvBase;
			if (func.m_func == ms_funcs[(int)IStateNode.EnumFunc.Create])
				return 1;
			int ret = node.FunctionCall(func, ms_funcs);
			return ret;
		}
		
		public static void StateFuncRegist()
		{
			StateFuncRegist(m_classname, ref ms_funcs, (int)EState.EStateMax, FunctionProcessor);    
		}
		
		static int StateFuncRegist(string _class_name, ref List<int> _funcs, int _size, StateInterface.fnFuncProcessor _processor)
		{
			if (0 < IStateNode.StateFuncRegist(_class_name, ref _funcs, _size, _processor))
			{
				IStateNode.FunctionRegist(_class_name, _funcs, (int)EState.StatesSelectedSave_strV, "StatesSelectedSave_strV", _processor);
				IStateNode.FunctionRegist(_class_name, _funcs, (int)EState.StatesSelectedLoad_strV, "StatesSelectedLoad_strV", _processor);
				IStateNode.FunctionRegist(_class_name, _funcs, (int)EState.StateRename_varF, "StateRename_varF", _processor);
				//#SF_FuncRegistInsert
				return (int)EState.EStateMax;
			}
			return 0;
		}

		
		public int FunctionCall(StateFunction _func, List<int> _funcs)
		{
			int ret = m_stateNode.FunctionCall(_func, _funcs);
			if (ret != 0)
				return ret;
			//#SF_FuncCallStart
			if (_func.m_func == 0) return 0;
			else if (_func.m_func == _funcs[(int)EState.StatesSelectedSave_strV]) return StatesSelectedSave_strV(_func);
			else if (_func.m_func == _funcs[(int)EState.StatesSelectedLoad_strV]) return StatesSelectedLoad_strV(_func);
			else if (_func.m_func == _funcs[(int)EState.StateRename_varF]) return StateRename_varF(_func);
			//#SF_FuncCallInsert
			return 0;
		}

		private static int ms_sequence = 800000;
		public int IdGet()
		{
			if (m_objId == -1)
				m_objId = VLStateManager.hash(m_classname);

			if (m_id == 0)
			{
				m_id = ++ms_sequence;
			}
			return m_id;
		}

		public void EventCast(string _event)
		{
			IStateNode.EventCast(this, _event);
		}
		#endregion

		#region StateCustomFunction

		public UxViewStateContent m_stateContext;
		
		int StatesSelectedSave_strV(StateFunction _func)
		{
			string filename = _func.ParamStringGet();

			m_stateContext.stateActivesSave(filename);

			return 1;
		}	
		int StatesSelectedLoad_strV(StateFunction _func)
		{
            string filename = _func.ParamStringGet();

            m_stateContext.stateActivesLoad(filename);

            return 0;
		}	
		int StateRename_varF(StateFunction _func)
		{
			string nameTo = "";
			if(!_func.ParamVariableGet(ref nameTo))
					return 0;
			string nameFrom = "";
			if (!_func.ParamFallowGet(0, ref nameFrom))
				return 0;

			m_stateContext.state_rename(nameTo, nameFrom);
			return 1;
		}	
		//#SF_FuncScript
		
		#endregion
	}
}