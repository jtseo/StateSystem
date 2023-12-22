using Apin.Routine;
using Apin.Scripts.Routine;
using Apin.Service;
using Apin.ZoneMove;
using Enums;
using Interact;
using ProtoSii;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UI;
using UnityEngine;
using UnityEngine.Rendering;

namespace StateSystem
{
    public class NpcStateNode : MonoBehaviour, IStateNode
    {
        private StateNode m_stateNode = new StateNode();
        private const string m_classname = "NpcStateNode";
        enum EnumNpc
        {
            MissionIdCheck_nIf = IStateNode.EnumFunc.EnumFuncMax,
            RoutineWalkCheck_nIf,
            RoutineArriveCheck_nIf,
            RoutineInDialogCheck_nIf,
            WalkArrivedCheck_nIf,
            WalkActivatedCheck_nIf,
            WalkWalkCheck_nIf,
            WalkTimelineCheck_nIf,
            CooltimerAdd_varF,
            CooltimerRepair_varF,
            CooltimerRequest_nF,
            AnimationSet_strF,
            //#SF_FuncEnum
            EnumFuncMax
        }
        static private int ms_obj_id = -1;
        private int m_loc_id = 0;
        private StateDStructureValue m_localBase = null;

        private NPC m_npc = default;
        private NpcRoutineControl m_npcRoutine = default;
        private WalkControl m_walk = default;

        public NPC NpcGet()
        {
            return m_npc;
        }
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
            m_npc = GetComponent<NPC>();
            m_npcRoutine = GetComponent<NpcRoutineControl>();
            m_walk = GetComponent<WalkControl>();

            IdGet();
            m_stateNode.set(this);
            StateFuncRegist(m_classname, ref ms_funcs, (int)EnumNpc.EnumFuncMax, FunctionProcessor);
            IStateNode.event_create_post("NPCCreate", this);
            //m_npc.CooltimerOnendSet(CooltimerOnend);
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
            
            NpcStateNode stateNode = node as NpcStateNode;
            if (stateNode == null || node is not NpcStateNode)
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
                IStateNode.FunctionRegist(_class_name, _funcs, (int)EnumNpc.MissionIdCheck_nIf, "MissionIdCheck_nIf", _processor);
                IStateNode.FunctionRegist(_class_name, _funcs, (int)EnumNpc.RoutineWalkCheck_nIf, "RoutineWalkCheck_nIf", _processor);
                IStateNode.FunctionRegist(_class_name, _funcs, (int)EnumNpc.RoutineArriveCheck_nIf, "RoutineArriveCheck_nIf", _processor);
                IStateNode.FunctionRegist(_class_name, _funcs, (int)EnumNpc.RoutineInDialogCheck_nIf, "RoutineInDialogCheck_nIf", _processor);
                IStateNode.FunctionRegist(_class_name, _funcs, (int)EnumNpc.WalkArrivedCheck_nIf, "WalkArrivedCheck_nIf", _processor);
                IStateNode.FunctionRegist(_class_name, _funcs, (int)EnumNpc.WalkActivatedCheck_nIf, "WalkActivatedCheck_nIf", _processor);
                IStateNode.FunctionRegist(_class_name, _funcs, (int)EnumNpc.WalkWalkCheck_nIf, "WalkWalkCheck_nIf", _processor);
                IStateNode.FunctionRegist(_class_name, _funcs, (int)EnumNpc.WalkTimelineCheck_nIf, "WalkTimelineCheck_nIf", _processor);
                IStateNode.FunctionRegist(_class_name, _funcs, (int)EnumNpc.CooltimerAdd_varF, "CooltimerAdd_varF", _processor);
                IStateNode.FunctionRegist(_class_name, _funcs, (int)EnumNpc.CooltimerRepair_varF, "CooltimerRepair_varF", _processor);
                IStateNode.FunctionRegist(_class_name, _funcs, (int)EnumNpc.CooltimerRequest_nF, "CooltimerRequest_nF", _processor);
                IStateNode.FunctionRegist(_class_name, _funcs, (int)EnumNpc.AnimationSet_strF, "AnimationSet_strF", _processor);
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
            if (_func.m_func == _funcs[(int)EnumNpc.MissionIdCheck_nIf]) return MissionIdCheck_nIf(_func);
            else if (_func.m_func == _funcs[(int)EnumNpc.RoutineWalkCheck_nIf]) return RoutineWalkCheck_nIf(_func);
            else if (_func.m_func == _funcs[(int)EnumNpc.RoutineArriveCheck_nIf]) return RoutineArriveCheck_nIf(_func);
            else if (_func.m_func == _funcs[(int)EnumNpc.RoutineInDialogCheck_nIf]) return RoutineInDialogCheck_nIf(_func);
            else if (_func.m_func == _funcs[(int)EnumNpc.WalkArrivedCheck_nIf]) return WalkArrivedCheck_nIf(_func);
            else if (_func.m_func == _funcs[(int)EnumNpc.WalkActivatedCheck_nIf]) return WalkActivatedCheck_nIf(_func);
            else if (_func.m_func == _funcs[(int)EnumNpc.WalkWalkCheck_nIf]) return WalkWalkCheck_nIf(_func);
            else if (_func.m_func == _funcs[(int)EnumNpc.WalkTimelineCheck_nIf]) return WalkTimelineCheck_nIf(_func);
            else if (_func.m_func == _funcs[(int)EnumNpc.CooltimerAdd_varF]) return CooltimerAdd_varF(_func);
            else if (_func.m_func == _funcs[(int)EnumNpc.CooltimerRepair_varF]) return CooltimerRepair_varF(_func);
            else if (_func.m_func == _funcs[(int)EnumNpc.CooltimerRequest_nF]) return CooltimerRequest_nF(_func);
            else if (_func.m_func == _funcs[(int)EnumNpc.AnimationSet_strF]) return AnimationSet_strF(_func);
            //#SF_FuncCallInsert
            return 0;
        }

        public int IdGet()
        {
            if (m_objId == -1)
                m_objId = VLStateManager.hash(m_classname);

            if (m_id == 0)
            {
                m_id = m_npc.SingleID;
            }
            return m_id;
        }

        int MissionIdCheck_nIf(StateFunction _func)
        {
            return 1;
        }

        int RoutineWalkCheck_nIf(StateFunction _func) {
            if (m_npcRoutine.VariableWalkingGet())
                return 1;
            return 0;
        }
        int RoutineArriveCheck_nIf(StateFunction _func)
        {
            if (m_npcRoutine.VariableArriveGet())
                return 1;
            return 0;
        }
        int RoutineInDialogCheck_nIf(StateFunction _func)
        {
            if (m_npcRoutine.VariableInDialogGet())
                return 1;
            return 0;
        }

        int WalkArrivedCheck_nIf(StateFunction _func)
        {
            if (m_walk.VariableArrivedGet())
                return 1;
            return 0;
        }
        int WalkActivatedCheck_nIf(StateFunction _func)
        {
            if (m_walk.VariableActivatedGet())
                return 1;
            return 0;
        }

        int WalkWalkCheck_nIf(StateFunction _func)
        {
            if (m_walk.VariableWalkGet())
                return 1;
            return 0;
        }

        int WalkTimelineCheck_nIf(StateFunction _func)
        {
            if (m_walk.VariableWalkTimelineGet())
                return 1;
            return 0;
        }

        int CooltimerAdd_varF(StateFunction _func)
        {
            // zone id, Indexabletype NPC, SingleID id, step(variable paramater)
            int param = 0;
            _func.ParamVariableGet(ref param);
            int time = 0;
            if (!_func.ParamFallowGet(0, ref time))
                return 0;
            //m_npc.CooltimerAdd(time, ZoneMoveManager.CurrentZoneID, param);
            return 1;
        }
        
        int CooltimerRepair_varF(StateFunction _func)
        {
            // zone id, Indexabletype NPC, SingleID id, step(variable paramater)
            int param = 0;
            _func.ParamVariableGet(ref param);
            int time = 0;
            if (!_func.ParamFallowGet(0, ref time))
                return 0;
            //m_npc.CooltimerRepair(time, ZoneMoveManager.CurrentZoneID, param);
            return 1;
        }

        public void CooltimerOnend(int _step)
        {
            StateDStructureValue evt = VLStateManager.create_event_group(VLStateManager.hash("CooltimerEnd"), m_objId, m_id);
            if (evt == null)
                return;
            evt.set_variable("CooltimerStep_nV", _step);
            VLStateManager.event_post(evt);
        }

        int CooltimerRequest_nF(StateFunction _func)
        {// cast "CooltimeResponse"
            // m_npc.CooltimerRequest(ZoneMoveManager.CurrentZoneID, (step, endTime) =>
            // {
            //     double remainD = (endTime - DateTime.Now).TotalSeconds;
            //     if (step > 0 && remainD > 0)
            //     {
            //         StateDStructureValue evt = VLStateManager.create_event_group(VLStateManager.hash("CooltimerResponse"), m_objId, m_id);
            //         if (evt == null)
            //             return;
            //         evt.set_variable("CooltimerStep_nV", step);
            //         VLStateManager.event_post(evt);
            //         int remain = (int)remainD;
            //         m_npc.CooltimerRepair(remain, ZoneMoveManager.CurrentZoneID, step);
            //     }
            // });
            return 1;
        }

        int AnimationSet_strF(StateFunction _func)
        {
            string ani = _func.ParamStringGet();
            if (ani == "default")
            {
                m_npc.NpcRoutineControl.PlayDefaultAnimation();
            }
            else
            {
                m_npc.TalkAnimatorControl.PlayTrigger(ani);
            }
            return 1;
        }

        public void EventCast(string _event)
        {
            IStateNode.EventCast(this, _event);
        }
    }
}