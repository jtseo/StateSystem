using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using System;
using TMPro;

namespace StateSystem
{
    public class VLogLink
    {
        public int count = 0;
        public int serial = 0;
    }

    public class VScriptMListLogHistory
    {
        public List<StateDStructureValue> m_log_history_a;
        public Hashtable m_log_link_hash_cnt_h;
        public List<VLogLink> m_log_link_a;

        public VScriptMListLogHistory()
        {
            m_log_history_a = new List<StateDStructureValue>();
            m_log_link_hash_cnt_h = new Hashtable();
            m_log_link_a = new List<VLogLink>();
        }
    }

    public class VScriptLogHistory : MonoBehaviour
    {
        public bool m_mode_history = false;
        public bool m_mode_logger = true;
        public bool m_filter = false;
        bool m_filter_link_fix = false;
        int m_filter_link_serial = 0;

        int m_log_index_cur = 0;
        int m_key_main = 0;
        int m_hash_mainkey = 0;

        Hashtable m_MListLog_h;

        static public VScriptLogHistory ms_instance = null;

        // Start is called before the first frame update
        void Start()
        {
            ms_instance = this;
            m_MListLog_h = new Hashtable();
            m_hash_mainkey = VLStateManager.hash("VSLoggerMainKey_nV");
        }

        // Update is called once per frame
        void Update()
        {

        }

        public int log_push(StateDStructureValue _log, ref int _cnt)
        {
            _log.set_ptr(VLStateManager.hash("nTimeToProcess"), IntPtr.Zero, 0);

            int main_key = 0;
            _log.get_int(m_hash_mainkey, ref main_key);
            VScriptMListLogHistory mlistLogHistory = (VScriptMListLogHistory)m_MListLog_h[main_key];
            if (mlistLogHistory == null)
            {
                mlistLogHistory = new VScriptMListLogHistory();
                m_MListLog_h[main_key] = mlistLogHistory;
            }

            mlistLogHistory.m_log_history_a.Add(_log);

            int key = 0;
            if (_log.get_int("VSLoggerLinkKey_nV", ref key))
            {
                VLogLink link = null;
                if (!mlistLogHistory.m_log_link_hash_cnt_h.Contains(key))
                {
                    link = new VLogLink();
                    mlistLogHistory.m_log_link_hash_cnt_h[key] = link;
                    mlistLogHistory.m_log_link_a.Add(link);
                }
                else
                {
                    link = (VLogLink)mlistLogHistory.m_log_link_hash_cnt_h[key];
                }

                int serial = 0;
                _log.get_int("VSLoggerSerial_nV", ref serial);

                if (!m_filter)
                {
                    link.count++;
                    link.serial = serial;
                }
                else
                {
                    int[] groupid_a = _log.get_int_arry("VSLoggerGroup_anV");
                    if (groupid_a != null && UxGroupID.ms_instance.group_filter_check(groupid_a))
                    {
                        link.count++;
                        link.serial = serial;
                    }
                }

                _cnt = link.count;
            }

            return mlistLogHistory.m_log_history_a.Count - 1;
        }

        public void key_main_set(int _key)
        {
            m_key_main = _key;
        }

        public int log_link_cnt(int _key, ref int _serial)
        {
            if (m_key_main == 0)
            {
                m_key_main = UxViewStateContent.ms_Instance.main_key_get();
            }

            VScriptMListLogHistory mlistLogHistory = (VScriptMListLogHistory)m_MListLog_h[m_key_main];
            if (mlistLogHistory == null)
            {
                return 0;
            }

            if (mlistLogHistory.m_log_link_hash_cnt_h.Contains(_key))
            {
                VLogLink link = (VLogLink)mlistLogHistory.m_log_link_hash_cnt_h[_key];
                _serial = link.serial;
                return link.count;
            }

            return 0;
        }

        public bool log_link_old_remove(int _link, int _hold_cnt)
        {
            VScriptMListLogHistory mlistLogHistory = (VScriptMListLogHistory)m_MListLog_h[m_key_main];
            if (mlistLogHistory == null)
                return false;

            int cnt = 0;
            int key = 0;
            int len = mlistLogHistory.m_log_history_a.Count;
            if (mlistLogHistory.m_log_history_a == null)
                return false;
            for (int i = len - 1; i >= 0; i--)
            {
                StateDStructureValue log;
                log = mlistLogHistory.m_log_history_a[i];

                if (!log.get_int("VSLoggerLinkKey_nV", ref key))
                    continue;

                if (key != _link)
                    continue;

                cnt++;
                if (cnt <= _hold_cnt) // keep until hold count
                    continue;

                log.delete();
                mlistLogHistory.m_log_history_a.RemoveAt(i);
                i++;

                VLogLink log_link = (VLogLink)mlistLogHistory.m_log_link_hash_cnt_h[key];
                if (log_link == null)
                    continue;

                log_link.count--;
            } // end of for

            return true;
        }

        public bool log_link_recount(int _key_main)
        {
            if (_key_main != 0)
                m_key_main = _key_main;

            VScriptMListLogHistory mlistLogHistory = (VScriptMListLogHistory)m_MListLog_h[m_key_main];

            foreach (VLogLink link in mlistLogHistory.m_log_link_a)
            {
                link.count = 0;
                link.serial = 0;
            }

            /*
            for (int i = 0; i < m_log_history_a.Count; i++)
            {
                StateDStructureValue log = m_log_history_a[i];
                if(m_filter)
                {
                    int key = 0;
                    if (!log.get_int("VSLoggerStateKey_nV", ref key))
                        continue;
                    int[] groupid_a = log.get_int_arry("VSLoggerGroup_anV");
                    if (groupid_a == null)
                        continue;
    
                    //if (UxGroupID.ms_instance.group_filter_check(groupid_a))
                    //    log_link_cnt(key);
                }
            }
            //*/
            return true;
        }

        public StateDStructureValue log_get(int _index)
        {
            VScriptMListLogHistory mlistLogHistory = (VScriptMListLogHistory)m_MListLog_h[m_key_main];

            if (mlistLogHistory == null)
                return null;

            if (_index < 0 || _index >= mlistLogHistory.m_log_history_a.Count)
                return null;

            return mlistLogHistory.m_log_history_a[_index];
        }

        public StateDStructureValue log_restart()
        {
            m_log_index_cur = 0;
            return log_next_get();
        }

        public int log_index_cur_get()
        {
            return m_log_index_cur;
        }

        public void log_index_cur_set(int _index)
        {
            m_log_index_cur = _index;
        }

        public void log_reset()
        {
            VScriptMListLogHistory mlistLogHistory = (VScriptMListLogHistory)m_MListLog_h[m_key_main];

            if (mlistLogHistory == null)
                return;
            
            foreach (StateDStructureValue log in mlistLogHistory.m_log_history_a)
            {
                log.delete();
            }

            mlistLogHistory.m_log_history_a.Clear();
            mlistLogHistory.m_log_link_hash_cnt_h.Clear();
            mlistLogHistory.m_log_link_a.Clear();
        }

        public StateDStructureValue log_next_get(int _state_key)
        {
            //VSLoggerStateKey_nV
            StateDStructureValue log = log_next_get();
            if (log == null)
                return null;

            StateDStructureValue evt = log.copy_get();

            int[] groupid = new int[2];
            groupid[0] = VLStateManager.hash("LoggerHistory");
            groupid[1] = 999999;
            evt.add_int_arry("BaseTransitionGoalIdentifier", groupid); // 999999
            VLStateManager.event_post(evt);

            int key = 0;
            if (!log.get_int("VSLoggerStateKey_nV", ref key))
                return log_next_get(_state_key);

            if (_state_key == key)
                return log;

            return log_next_get(_state_key);
        }

        public int log_next_link_get(int _link_key)
        {
            StateDStructureValue log = log_next_get();
            if (log == null)
                return -1;

            int key = 0;
            if (!log.get_int("VSLoggerLinkKey_nV", ref key))
                return log_next_link_get(_link_key);

            if (_link_key == key)
                return m_log_index_cur;

            return log_next_link_get(_link_key);
        }

        int m_log_direction = 1;

        public void log_direction_set(int _dir)
        {
            m_log_direction = _dir;
        }

        public StateDStructureValue log_next_get()
        {
            // originaly this is return next log.
            // but return a filtered log when filter or link fix is enabled.

            int log_index_backup = m_log_index_cur;

            m_log_index_cur += m_log_direction;
            if (m_log_index_cur < 0)
            {
                StateDStructureValue evt = VLStateManager.event_state_make("UxNotifierAuto");
                evt.set_string("TempString_strV", "Logger first!");
                VLStateManager.event_post(evt);

                m_log_index_cur = 0;
                return null;
            }

            VScriptMListLogHistory mlistLogHistory = (VScriptMListLogHistory)m_MListLog_h[m_key_main];

            if (m_log_index_cur >= mlistLogHistory.m_log_history_a.Count)
            {
                StateDStructureValue evt = VLStateManager.event_state_make("UxNotifierAuto");
                evt.set_string("TempString_strV", "Logger end!, go first");
                VLStateManager.event_post(evt);

                m_log_index_cur = 0;
                return null;
            }

            StateDStructureValue log = mlistLogHistory.m_log_history_a[m_log_index_cur];

            if (m_filter)
            {
                int[] groupid_a = log.get_int_arry("VSLoggerGroup_anV");
                if (groupid_a == null)
                    return log_next_get();

                if (!UxGroupID.ms_instance.group_filter_check(groupid_a))
                    return log_next_get();
            }

            if (m_filter_link_fix)
            {
                int serial = 0;
                if (!log.get_int("VSLoggerStateSerial_nV", ref serial))
                    return log_next_get();

                if (m_filter_link_serial != serial)
                    return log_next_get();
            }

            return log;
        }

        public static int VSLogHistoryFlagOn_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue base_p = new StateDStructureValue(_pBase);

            int flag = 0;
            if (!base_p.get_int("VSLogHistoryFlagOn_nF", ref flag))
                return 0;

            if (flag == VLStateManager.hash("VSLogger_filter"))
                VScriptLogHistory.ms_instance.m_filter = true;

            if (flag == VLStateManager.hash("VSLogger_link_fix"))
                VScriptLogHistory.ms_instance.m_filter_link_fix = true;

            if (flag == VLStateManager.hash("VSLogger_mode_history"))
                VScriptLogHistory.ms_instance.m_mode_history = true;

            if (flag == VLStateManager.hash("VSLogger_mode_logger"))
                VScriptLogHistory.ms_instance.m_mode_logger = true;

            return 1;
        }


        public static int VSLogHistoryFlagOff_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue base_p = new StateDStructureValue(_pBase);

            int flag = 0;
            if (!base_p.get_int("VSLogHistoryFlagOff_nF", ref flag))
                return 0;

            if (flag == VLStateManager.hash("VSLogger_filter"))
                VScriptLogHistory.ms_instance.m_filter = false;

            if (flag == VLStateManager.hash("VSLogger_link_fix"))
                VScriptLogHistory.ms_instance.m_filter_link_fix = false;

            if (flag == VLStateManager.hash("VSLogger_mode_history"))
                VScriptLogHistory.ms_instance.m_mode_history = false;

            if (flag == VLStateManager.hash("VSLogger_mode_logger"))
                VScriptLogHistory.ms_instance.m_mode_logger = false;

            return 1;
        }

        public static int VSLogHistoryNextGo_nIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue base_p = new StateDStructureValue(_pBase);
            StateDStructureValue state_variable = base_p.state_variable_get();

            StateDStructureValue log = ms_instance.log_next_get();
            if (log == null)
                return 0;

            state_variable.set_int("TempInt_nV", ms_instance.m_log_index_cur);
            return 1;
        }

        public static int VSLogHistoryNextStateGo_varIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue base_p = new StateDStructureValue(_pBase);
            StateDStructureValue state_variable = base_p.state_variable_get();

            int hash = 0;
            if (!base_p.get_int("VSLogHistoryNextStateGo_varIf", ref hash))
                return 0;

            int state_key = 0;
            if (!state_variable.get_int(hash, ref state_key))
                return 0;

            StateDStructureValue log = ms_instance.log_next_get(state_key);
            if (log == null)
                return 0;

            state_variable.set_int("TempInt_nV", ms_instance.m_log_index_cur);
            return 1;
        }

        public static int VSLogHistoryPush_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue base_p = new StateDStructureValue(_pBase);
            StateDStructureValue state_variable = base_p.state_variable_get();

            StateDStructureValue log_evt = new StateDStructureValue(_pEvent);

            int key_link = 0;
            if (!log_evt.get_int("VSLoggerLinkKey_nV", ref key_link))
                return 0;

            int count = 0;
            int index = ms_instance.log_push(log_evt.copy_get(), ref count);
            VScriptLink link = UxViewStateContent.ms_Instance.linksub_get(key_link);
            if (link == null)
                return 0;

            link.logger_index_set(index);

            int hash = 0;
            if (base_p.param_get("BaseVariableRefer_anV", 0, ref hash))
                state_variable.set_int(hash, count);

            return 1;
        }

        public static int VSLogHistoryReset_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            //StateDStructureValue base_p = new StateDStructureValue(_pBase);
            //StateDStructureValue state_variable = base_p.state_variable_get();

            ms_instance.log_reset();

            return 1;
        }

        public static int VSLogHistoryRecount_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            ms_instance.log_link_recount(UxViewStateContent.ms_Instance.main_key_get());

            return 1;
        }

        public static int VSLogHistoryLinkOldRemove_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue base_p = new StateDStructureValue(_pBase);
            StateDStructureValue state_variable = base_p.state_variable_get();
            int hold_cnt = 0;
            if (!base_p.get_int("VSLogHistoryLinkOldRemove_nF", ref hold_cnt))
                return 0;

            int link = 0;
            if (!base_p.param_get(_pEvent, _pContext, 0, ref link))
                return 0;

            ms_instance.log_link_old_remove(link, hold_cnt);

            return 1;
        }

        public static void StateFuncRegist()
        {
            VLStateManager.ProcessReg("VSLogHistoryLinkOldRemove_nF", VSLogHistoryLinkOldRemove_nF);
            VLStateManager.ProcessReg("VSLogHistoryRecount_nF", VSLogHistoryRecount_nF);
            VLStateManager.ProcessReg("VSLogHistoryFlagOn_nF", VSLogHistoryFlagOn_nF);
            VLStateManager.ProcessReg("VSLogHistoryFlagOff_nF", VSLogHistoryFlagOff_nF);
            VLStateManager.ProcessReg("VSLogHistoryNextGo_nIf", VSLogHistoryNextGo_nIf);
            VLStateManager.ProcessReg("VSLogHistoryNextStateGo_varIf", VSLogHistoryNextStateGo_varIf);
            VLStateManager.ProcessReg("VSLogHistoryPush_nF", VSLogHistoryPush_nF);
            VLStateManager.ProcessReg("VSLogHistoryReset_nF", VSLogHistoryReset_nF);
        }
    }
}