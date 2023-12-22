using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using UnityEngine.UI;
using TMPro;
using System.IO;

using System.Runtime.InteropServices;

namespace StateSystem
{
    public class UxViewColumn : MonoBehaviour
    {
        public GameObject m_columnboard_go;
        public GameObject m_scroll_go;
        public GameObject m_viewstate_go;
        UxControlScroll m_scroll;

        UxViewStateContent m_state_dst;

        //public GameObject m_line_go;
        GameObject m_board_go;
        static public string sm_monoBehaviour_str;
        Canvas m_mainCanvas;
        StateDStructure m_main_dst;

        //bool m_update_b = false;

        List<GameObject> m_columns_go;
        List<int> m_column_marker_a;
        List<GameObject> m_event_go;

        // Start is called before the first frame update
        void Start()
        {
            m_column_select_a = new List<int>();
            m_columns_go = new List<GameObject>();
            m_event_go = new List<GameObject>();
            m_column_marker_a = new List<int>();
            //m_links_ago = new List<GameObject>();
            sm_monoBehaviour_str = "VScriptMonoBehaviourUxViewColumn";

            m_board_go = (GameObject)GameObject.Find("ColumnBoard");

            VLStateManager.monobehaviourPointSet(sm_monoBehaviour_str, this);
            m_mainCanvas = GameObject.Find("Canvas").GetComponent<Canvas>();
            m_scroll = m_scroll_go.GetComponent<UxControlScroll>();
            m_state_dst = m_viewstate_go.GetComponent<UxViewStateContent>();
        }

        // Update is called once per frame
        void Update()
        {

        }

        private void Awake()
        {
        }

        int m_column_height = 0;

        bool column_update(int _column, int _row, string _str)
        {
            GameObject go = (GameObject)m_columns_go[_column];
            if (go == null)
                return false;
            TMP_Text[] text = go.GetComponentsInChildren<TMP_Text>();
            if (_row < 0 || _row >= text.Length)
                return false;
            text[_row].text = _str;
            return true;
        }

        List<int> m_column_select_a;

        bool column_select_check(int _column)
        {
            foreach (int i in m_column_select_a)
                if (i == _column)
                    return true;
            return false;
        }

        public void column_select_toggle(int _column)
        {
            if (column_select_check(_column))
            {
                m_column_select_a.Remove(_column);
            }
            else
            {
                m_column_select_a.Add(_column);
            }
        }

        public void column_select_clear()
        {
            m_column_select_a.Clear();
        }

        bool column_paste(List<List<string>> _column_aa, int _index)
            // _count: 1 is only one, 
            // _count: n is n
            // _count: -1 is until before of next function
            // _count: max is until end of column with event variable.
        {
            string[] text_a = new string[4];
            column_select_clear();

            if (_index == 999)
                _index = m_columns_go.Count;

            for (int i = _column_aa.Count - 1; i >= 0; i--)
            {
                List<string> column_a = _column_aa[i];
                for (int j = 0; j < column_a.Count; j++)
                    text_a[j] = column_a[j];

                column_add(_index, text_a);
                m_column_select_a.Add(_index + i);
            }

            return false;
        }

        bool column_copy(int _from, int _to)
        {

            if (_from < 0 || _to < 0
                          || _from >= m_columns_go.Count || _to >= m_columns_go.Count)
                return false;

            int cnt = 1;

            bool move_block = false;
            if (m_column_select_a.Count > 0)
            {
                foreach (int col in m_column_select_a)
                    if (col == _from)
                        move_block = true;
                if (move_block)
                    cnt = 999;
                else
                    m_column_select_a.Clear();
            }

            List<List<string>> list_aa = new List<List<string>>();
            column_copy(ref list_aa, _from, cnt);
            column_paste(list_aa, _to);

            column_pos_refresh();
            return true;
        }

        bool column_move(int _from, int _to)
        {

            if (_from < 0 || _to < 0
                          || _from >= m_columns_go.Count || _to >= m_columns_go.Count)
                return false;

            List<List<string>> list_aa = new List<List<string>>();

            int cnt = 1;

            bool move_block = false;
            if (m_column_select_a.Count > 0)
            {
                foreach (int col in m_column_select_a)
                    if (col == _from)
                        move_block = true;
                if (move_block)
                    cnt = 999;
                else
                    m_column_select_a.Clear();
            }

            column_copy(ref list_aa, _from, cnt);
            column_remove(_from, cnt);

            if (_to >= _from + m_column_select_a.Count)
                _to -= m_column_select_a.Count;
            else if (_to > _from)
                _to = _from;

            column_paste(list_aa, _to);
            if (!move_block)
                column_select_clear();

            column_pos_refresh();
            return true;
        }

        bool column_copy(ref List<List<string>> _column_aa, int _index, int _count)
            // _count: 1 is only one, 
            // _count: n is n
            // _count: -1 is until before of next function
            // _count: max is until end of column with event variable.
        {
            List<int> copy_a = new List<int>();
            m_column_select_a.Sort();

            if (_count == 999 && m_column_select_a.Count > 0)
            {
                foreach (int index in m_column_select_a)
                {
                    copy_a.Add(index);
                }
            }
            else
            {
                for (int i = 0; i < _count; i++)
                {
                    if (m_columns_go.Count <= _index + i)
                        break;
                    copy_a.Add(_index + i);
                }
            }

            for (int i = 0; i < copy_a.Count; i++)
            {
                GameObject go = (GameObject)m_columns_go[copy_a[i]];

                TMP_Text[] text = go.GetComponentsInChildren<TMP_Text>();
                List<string> column_a = new List<string>();

                column_a.Add(text[0].text);
                column_a.Add(text[1].text);
                column_a.Add(text[2].text);
                column_a.Add(text[3].text);

                _column_aa.Add(column_a);
            }

            return true;
        }

        bool column_remove(int _index, int _count)
        {
            // _count: 1 is only one, 
            // _count: n is n
            // _count: -1 is until before of next function
            // _count: max is until end of column with event variable.
            if (_count == 999 && m_column_select_a.Count > 0)
            {
                m_column_select_a.Sort();
                for (int i = m_column_select_a.Count - 1; i >= 0; i--)
                {
                    int index = m_column_select_a[i];
                    GameObject go = (GameObject)m_columns_go[index];
                    m_columns_go.Remove(go);
                    m_column_marker_a.RemoveAt(index);
                    Destroy(go);
                }

                return true;
            }
            else
            {
                for (int i = 0; i < _count; i++)
                {
                    GameObject go = (GameObject)m_columns_go[_index];
                    m_columns_go.Remove(go);
                    m_column_marker_a.RemoveAt(_index);
                    Destroy(go);
                }

                return true;
            }
        }

        bool column_add(int _index, string[] text_a)
        {
            if (!list_add(m_columns_go, _index, text_a))
            {
                return false;
            }

            if (_index >= m_columns_go.Count)
                _index = m_columns_go.Count - 1;
            VScriptLink link = m_columns_go[_index].GetComponent<VScriptLink>();
            if (text_a[0].Length > 0 && text_a[0] != "L")
            {
                link.marker_toggle(2, true);
                m_column_marker_a.Insert(_index, 2);
            }
            else
            {
                m_column_marker_a.Insert(_index, -1);
            }

            link.m_key = _index;

            return column_pos_refresh();
        }

        bool event_add(string[] text_a)
        {
            if (!list_add(m_event_go, m_event_go.Count, text_a))
                return false;
            return event_pos_refresh();
        }

        bool event_pos_refresh()
        {
            int start = m_columns_go.Count + 2;
            for (int i = 0; i < m_event_go.Count; i++)
            {
                GameObject go = m_event_go[i];
                go.transform.SetSiblingIndex(i + m_columns_go.Count + 2);

                VScriptLink link = go.GetComponent<VScriptLink>();
                link.m_key = i + m_columns_go.Count + 2;
            }

            return true;
        }

        bool column_pos_refresh()
        {
            for (int i = 0; i < m_columns_go.Count; i++)
            {
                GameObject go = m_columns_go[i];
                go.transform.SetSiblingIndex(i);

                VScriptLink link = go.GetComponent<VScriptLink>();
                link.m_key = i;

                int marker = m_column_marker_a[i];
                if (column_select_check(i))
                    marker = 3;

                if (marker != -1)
                    link.marker_toggle(marker, true);
                else
                    link.marker_clear();
            }

            event_pos_refresh();
            return true;
        }

        bool list_add(List<GameObject> _list, int _index, string[] text_a)
        {
            GameObject column_go = (GameObject)Instantiate(m_columnboard_go);
            column_go.transform.SetParent(m_board_go.transform, false);

            if (_index > _list.Count)
                _index = _list.Count;
            _list.Insert(_index, column_go);

            TMP_Text[] text = column_go.GetComponentsInChildren<TMP_Text>();
            //Button[] button = column_go.GetComponentsInChildren<Button>();
            //TMP_InputField[] input = column_go.GetComponentsInChildren<TMP_InputField>();

            if (text != null && text.Length > 3)
            {
                for (int i = 0; i < text.Length && i < text_a.Length; i++)
                    text[i].text = text_a[i];
            }

            return true;
        }

        bool column_event_add(StateDStructureValue _evt)
        {
            int hash = 0, seq = 0, cnt = 0;
            IntPtr ret;
            seq = -1;

            int hash_name = VLStateManager.hash("strName");
            int hash_start = VLStateManager.hash("RevGroupStart");
            int hash_end = VLStateManager.hash("RevGroupEnd");
            int[] hash_skip = new int[8];
            int hash_cnt = 0;
            hash_skip[hash_cnt++] = VLStateManager.hash("VSLoggerStateKey_nV"); // 0
            hash_skip[hash_cnt++] = VLStateManager.hash("VSLoggerLinkKey_nV"); // 1
            hash_skip[hash_cnt++] = VLStateManager.hash("VSLoggerGroup_anV"); // 2
            hash_skip[hash_cnt++] = VLStateManager.hash("VSLoggerColumnResult_anV"); // 3
            hash_skip[hash_cnt++] = VLStateManager.hash("VSLoggerEventCast_anV"); // 4
            hash_skip[hash_cnt++] = VLStateManager.hash("VSLoggerEventReceive_anV"); // 5
            hash_skip[hash_cnt++] = VLStateManager.hash("VSLoggerSerial_nV"); // 6
            hash_skip[hash_cnt++] = VLStateManager.hash("VSLoggerStateKey_nV"); // 7
            bool start = false;
            List<string> column_a = new List<string>();
            do
            {
                seq++;
                ret = _evt.column_get_(seq, ref hash, ref cnt);
                if (ret == IntPtr.Zero)
                    break;

                if (hash == hash_start)
                {
                    start = true;
                    continue;
                }

                if (hash == hash_end)
                    break;

                if (!start)
                {
                    continue;
                }

                bool skip = false;
                int skip_index = 0;
                for (; skip_index < hash_skip.Length; skip_index++)
                {
                    if (hash_skip[skip_index] != hash)
                        continue;

                    skip = true;
                    if (skip_index == 1 || skip_index == 6)
                    {
                        skip = false;

                        if (skip_index == 6)
                        {
                            int[] int3_a = new int[cnt];
                            Marshal.Copy(ret, int3_a, 0, cnt);

                            GameObject go = GameObject.Find("LogSerialTMP");
                            TMP_Text txt;
                            txt = go.GetComponentInChildren<TMP_Text>();

                            txt.text = int3_a[0].ToString();
                        }

                        break;
                    }

                    if (skip_index != 3) // VSLoggerColumnResult_anV
                        continue;
                    int[] int2_a = new int[cnt];
                    Marshal.Copy(ret, int2_a, 0, cnt);

                    for (int i = 0; i < cnt; i += 2)
                    {
                        GameObject go = (GameObject)m_columns_go[int2_a[i]];
                        if (go == null)
                            continue;

                        VScriptLink column = go.GetComponent<VScriptLink>();
                        if (column == null)
                            continue;

                        if (int2_a[i + 1] == 0)
                        {
                            column.marker_toggle(0, true);
                            m_column_marker_a[int2_a[i]] = 0;
                        }
                        else
                        {
                            m_column_marker_a[int2_a[i]] = 1;
                            column.marker_toggle(1, true);
                        }
                    }

                    break;
                }

                if (skip)
                    continue;

                string[] text_a = new string[3];
                text_a[0] = "";

                int type_index = StateDStructure.get_index(hash);
                int type = StateDStructure.get_type(type_index);

                string str = "";
                if (type == 4 || type == 11) // int
                {
                    int[] int_a = new int[cnt];
                    Marshal.Copy(ret, int_a, 0, cnt);
                    foreach (int value in int_a)
                    {
                        str += value.ToString() + ",";
                    }
                }
                else if (type == 12 || type == 5) // long
                {
                    long[] int_a = new long[cnt];
                    Marshal.Copy(ret, int_a, 0, cnt);
                    foreach (long value in int_a)
                    {
                        str += value.ToString() + ",";
                    }
                }
                else if (type == 6 || type == 13) // float
                {
                    float[] f_a = new float[cnt];
                    Marshal.Copy(ret, f_a, 0, cnt);
                    foreach (float value in f_a)
                    {
                        str += value.ToString() + ",";
                    }
                }
                else if (type == 8) // string
                {
                    str = VLStateManager.MarshalToString(ret);
                }

                string name = "";
                if (VLStateManager.enum_get("EnumVariableDefine", hash, hash_name, ref name))
                    text_a[1] = name;
                else
                    text_a[1] = "Unknow";

                text_a[2] = str;

                //column_add(_index, text_a);
                event_add(text_a);
            } while (true);

            return true;
        }

        int m_key = 0;

        public bool log_next(int _inc)
        {
            VScriptLink link = UxViewStateContent.ms_Instance.linksub_get(m_key);
            if (link == null)
                return false;
            int log_index = link.logger_index_get();

            VScriptLogHistory.ms_instance.log_direction_set(_inc);
            VScriptLogHistory.ms_instance.log_index_cur_set(log_index);
            log_index = VScriptLogHistory.ms_instance.log_next_link_get(m_key);

            if (log_index < 0)
                return false;

            link.logger_index_set(log_index);
            if (!show_columns(m_key))
                return false;

            return true;
        }

        public bool log_backward()
        {
            return log_next(-1);
        }

        public bool log_forward()
        {
            return log_next(1);
        }

        public bool column_apply()
        {
            UxViewStateContent.ms_Instance.link_update(m_key);
            m_main_dst.release_rows(m_key);

            foreach (GameObject go in m_columns_go)
            {
                TMP_Text[] text = go.GetComponentsInChildren<TMP_Text>();
                if (text[1].text.Length == 0)
                    return false;
                int index = StateDStructure.get_index(VLStateManager.hash(text[1].text));
                if (index >= 0)
                    m_main_dst.editor_string_add(m_key, index, text[2].text);
            }

            return true;
        }

        public bool column_add(string _func_str)
        {
            string[] text_a = new string[4];
            int index_n = StateDStructure.get_index(VLStateManager.hash(_func_str));
            if (index_n < 0)
                return false;

            int flag_n = m_main_dst.get_type_flag(index_n);
            text_a[3] = m_main_dst.get_type_enum_state(index_n);
            text_a[1] = _func_str;
            text_a[0] = m_main_dst.flag_string_get(flag_n);

            column_add(m_columns_go.Count, text_a);
            return true;
        }

        public bool ext_name_get(string _enum, int[] _keys, ref string _ret)
        {
            if (_keys.Length == 0)
                return false;

            StateDStructure dst = null, dstOrg = null;
            string enumVariable = "EnumVariableDefine";
            string enumEvent = "EnumStateEvent";
            if (_enum == enumVariable)
            {
                dst = m_state_dst.m_dstExtVariable;
                dstOrg = VLStateManager.enum_get(enumVariable);
            }
            else if (_enum == enumEvent)
            {
                dst = m_state_dst.m_dstExtEvent;
                dstOrg = VLStateManager.enum_get(enumEvent);
            }

            if (dst == null)
                return false;

            bool string_add(int _key, ref string _ret)
            {
                string str = "";
                if (dstOrg.get_string(_key, 1, ref str))
                {
                    _ret = str;
                    return true;
                }

                if (!dst.get_string(_key, 1, ref str))
                    return false;

                _ret = str;
                return true;
            }

            if (!string_add(_keys[0], ref _ret))
                return false;
            for (int i = 1; i < _keys.Length; i++)
            {
                string str = "";
                if (!string_add(_keys[i], ref str))
                    str = _keys[i].ToString();
                _ret += ", ";
                _ret += str;
            }

            return true;
        }

        public bool show_columns(int _key)
        {
            m_key = _key;

            m_main_dst = m_state_dst.StateLinkGet(); //m_state_dst.StateMainGet();

            if (m_main_dst == null)
                return false;

            int i = 0;
            for (; i < m_columns_go.Count; i++)
            {
                Destroy(m_columns_go[i]);
            }

            m_columns_go.Clear();

            for (i = 0; i < m_event_go.Count; i++)
            {
                Destroy(m_event_go[i]);
            }

            m_event_go.Clear();

            int count_n = m_main_dst.get_count_column(_key);
            RectTransform rt = (RectTransform)m_board_go.transform;
            //float width_f = rt.rect.width * m_mainCanvas.scaleFactor;
            m_column_height = (int)((rt.rect.height + 2) * m_mainCanvas.scaleFactor);

            i = 0;
            int[] values = null;
            for (; i < count_n; i++)
            {
                string[] text_a = new string[4];
                int index_n = m_main_dst.get_index(_key, i);
                int flag_n = m_main_dst.get_type_flag(index_n);
                text_a[1] = m_main_dst.get_type_name(index_n);
                string comment_str = m_main_dst.get_type_comment(index_n);
                text_a[3] = m_main_dst.get_type_enum_state(index_n);
                int type_n = StateDStructure.get_type(index_n);
                if (!m_main_dst.get_int(_key, i, ref values)
                    || !ext_name_get(text_a[3], values, ref text_a[2]))
                    text_a[2] = m_main_dst.editor_string_get(_key, i);
                text_a[0] = m_main_dst.flag_string_get(flag_n);
                column_add(i, text_a);
            }

            i++;
            VScriptLink link = UxViewStateContent.ms_Instance.linksub_get(_key);
            if (link == null)
                return true;

            int log_index = link.logger_index_get();
            if (log_index >= 0)
            {
                StateDStructureValue evt = VScriptLogHistory.ms_instance.log_get(log_index);
                if (evt != null)
                    column_event_add(evt);
            }

            return true;
        }

        public static int VScriptColumnShow_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            UxViewColumn column_ux = null;

            int hash = 0;
            if (!stBase.get_int("VScriptColumnShow_nF", ref hash))
                return 0;

            column_ux = (UxViewColumn)VLStateManager.monobehaviourPointGet(sm_monoBehaviour_str);

            if (column_ux == null)
                return 0;

            StateDStructureValue variable_state = stBase.state_variable_get();

            float[] pos = new float[3];
            pos[0] = 3;
            pos[1] = 2;
            pos[2] = 91;
            variable_state.set_variable("PnPosition_v3", pos);
            int key = 0;
            if (!variable_state.get_int(hash, ref key))
                return 0;

            if (column_ux.show_columns(key))
                return 1;

            return 0;
        }

        public static int VScriptColumnChange_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            UxViewColumn column_ux = null;

            int change = 0; // 1 is backword, 2 is forward.
            if (!stBase.get_int("VScriptColumnChange_nF", ref change))
                return 0;
            column_ux = (UxViewColumn)VLStateManager.monobehaviourPointGet(sm_monoBehaviour_str);

            bool ret = false;
            if (change == VLStateManager.hash("Log_backward"))
                ret = column_ux.log_backward();
            if (change == VLStateManager.hash("Log_forward"))
                ret = column_ux.log_forward();
            if (!ret)
                return 0;
            return 1;
        }

        public static int VScriptColumnAdd_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            UxViewColumn column_ux = null;
            int hash = 0;
            if (!stBase.get_int("VScriptColumnAdd_varF", ref hash))
                return 0;

            string col_func = "";
            StateDStructureValue variable_state = stBase.state_variable_get();
            if (!variable_state.get_string(hash, ref col_func))
                return 0;
            column_ux = (UxViewColumn)VLStateManager.monobehaviourPointGet(sm_monoBehaviour_str);

            if (!column_ux.column_add(col_func))
                return 0;
            return 1;
        }

        public static int VScriptColumnValueGet_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue global = VLStateManager.variable_global_get();

            int hash = 0;
            if (!stBase.get_int("VScriptColumnValueGet_varF", ref hash))
                return 0;

            int column = 0;
            if (!stBase.param_get(_pEvent, _pContext, 0, ref column))
                return 0;

            int row = 0;
            if (!stBase.param_get(_pEvent, _pContext, 1, ref row))
                return 0;

            UxViewColumn column_ux = null;
            column_ux = (UxViewColumn)VLStateManager.monobehaviourPointGet(sm_monoBehaviour_str);
            string str = column_ux.column_get(column, row);
            if (str == null)
                return 0;

            StateDStructureValue variable_state = stBase.state_variable_get();
            if (!variable_state.set_variable_string(hash, str))
                return 0;
            return 1;
        }

        public static int VScriptColumnValueSet_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue global = VLStateManager.variable_global_get();

            int hash = 0;
            if (!stBase.get_int("VScriptColumnValueSet_varF", ref hash))
                return 0;

            StateDStructureValue variable_state = stBase.state_variable_get();
            string value_str = "";
            if (!variable_state.get_string(hash, ref value_str))
                return 0;

            int column = 0;
            if (!stBase.param_get(_pEvent, _pContext, 0, ref column))
                return 0;

            UxViewColumn column_ux = null;
            column_ux = (UxViewColumn)VLStateManager.monobehaviourPointGet(sm_monoBehaviour_str);
            if (!column_ux.column_update(column, 2, value_str))
                return 0;
            return 1;
        }

        public string column_get(int _column, int _row)
        {
            if (m_columns_go.Count <= _column)
                return null;
            GameObject go = (GameObject)m_columns_go[_column];
            if (go == null)
                return null;
            TMP_Text[] text = go.GetComponentsInChildren<TMP_Text>();
            if (_row < 0 || _row >= text.Length)
                return null;
            return text[_row].text;
        }

        static void list_append(StateDStructure _dst, ref List<string> _strs)
        {
            if (_dst == null)
                return;

            string name = "";

            int key = _dst.key_first_get();

            if (key != 0)
            {
                while (key != 0)
                {
                    _dst.get_string(key, 1, ref name);
                    _strs.Add(name);
                    key = _dst.key_next_get();
                }
            }
            else
            {
                _strs.Add("Null");
            }
        }

        public static int VScriptCosmosMenuEnum_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue global = VLStateManager.variable_global_get();

            int hash = 0;
            if (!stBase.get_int("VScriptCosmosMenuEnum_varF", ref hash))
                return 0;

            StateDStructureValue variable_state = stBase.state_variable_get();
            int column = 0;
            if (!variable_state.get_int(hash, ref column))
                return 0;

            UxViewColumn column_ux = null;
            column_ux = (UxViewColumn)VLStateManager.monobehaviourPointGet(sm_monoBehaviour_str);

            string enum_str = "";
            enum_str = column_ux.column_get(column, 3);
            if (enum_str == null)
                return 0;

            MonoBehaviour uxCosmos = null;
            if (!global.get_ptr(UxCosmos.sm_strMonoBehaviour, ref uxCosmos))
                return 0;

            StateDStructure enum_dst = VLStateManager.enum_get(enum_str);

            if (enum_dst == null)
                return 0;

            //MonoBehaviour uxViewMain = null;

            List<string> enum_a = new List<string>();

            list_append(enum_dst, ref enum_a);
            if (enum_str == "EnumVariableDefine")
                list_append(column_ux.m_state_dst.m_dstExtVariable, ref enum_a);
            if (enum_str == "EnumStateEvent")
                list_append(column_ux.m_state_dst.m_dstExtEvent, ref enum_a);

            ((UxCosmos)uxCosmos).MenuListMake(enum_a);
            return 1;
        }

        public static int VScriptColumnApply_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue global = VLStateManager.variable_global_get();

            UxViewColumn column_ux = null;
            column_ux = (UxViewColumn)VLStateManager.monobehaviourPointGet(sm_monoBehaviour_str);

            if (!column_ux.column_apply())
                return 0;

            return 1;
        }

        public static int VScriptColumnRemove_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue global = VLStateManager.variable_global_get();
            StateDStructureValue variable_state = stBase.state_variable_get();
            UxViewColumn column_ux = null;
            column_ux = (UxViewColumn)VLStateManager.monobehaviourPointGet(sm_monoBehaviour_str);

            int hash = 0;
            if (!stBase.get_int("VScriptColumnRemove_varF", ref hash))
                return 0;

            int column = 0;
            if (!variable_state.get_int(hash, ref column))
                return 0;

            if (!column_ux.column_remove(column, 1))
                return 0;

            return 1;
        }

        public static int VScriptColumnRefresh_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue global = VLStateManager.variable_global_get();
            StateDStructureValue variable_state = stBase.state_variable_get();
            UxViewColumn column_ux = null;
            column_ux = (UxViewColumn)VLStateManager.monobehaviourPointGet(sm_monoBehaviour_str);

            if (!column_ux.column_pos_refresh())
                return 0;
            //if(!column_ux.event_pos_refresh())
            //    return 0;

            return 1;
        }

        public static int VScriptColumnCopy_avarF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue global = VLStateManager.variable_global_get();
            StateDStructureValue variable_state = stBase.state_variable_get();
            int[] hash_a;
            hash_a = stBase.get_int_arry("VScriptColumnCopy_avarF");
            if (hash_a == null
                || hash_a.Length < 2)
                return 0;
            UxViewColumn column_ux = null;
            column_ux = (UxViewColumn)VLStateManager.monobehaviourPointGet(sm_monoBehaviour_str);

            int from = 0, to = 0;
            if (!variable_state.get_int(hash_a[0], ref from))
                return 0;
            if (!variable_state.get_int(hash_a[1], ref to))
                return 0;

            if (!column_ux.column_copy(from, to))
                return 0;

            return 1;
        }

        public static int VScriptColumnMove_avarF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue global = VLStateManager.variable_global_get();
            StateDStructureValue variable_state = stBase.state_variable_get();
            int[] hash_a;
            hash_a = stBase.get_int_arry("VScriptColumnMove_avarF");
            if (hash_a == null
                || hash_a.Length < 2)
                return 0;
            UxViewColumn column_ux = null;
            column_ux = (UxViewColumn)VLStateManager.monobehaviourPointGet(sm_monoBehaviour_str);

            int from = 0, to = 0;
            if (!variable_state.get_int(hash_a[0], ref from))
                return 0;
            if (!variable_state.get_int(hash_a[1], ref to))
                return 0;

            if (!column_ux.column_move(from, to))
                return 0;

            return 1;
        }

        public static int VScriptColumnCopy_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue variable_state = stBase.state_variable_get();
            UxViewColumn column_ux = null;
            column_ux = (UxViewColumn)VLStateManager.monobehaviourPointGet(sm_monoBehaviour_str);

            int hash = 0;
            if (!stBase.get_int("VScriptColumnCopy_varF", ref hash))
                return 0;

            int column = 0;
            if (!variable_state.get_int(hash, ref column))
                return 0;

            List<List<string>> column_aa = new List<List<string>>();
            if (!column_ux.column_copy(ref column_aa, column, 999))
                return 0;
            // marker
            GUIUtility.systemCopyBuffer = BaseFile.make_string(column_aa, '\t');

            return 1;
        }

        public static int VScriptColumnPaste_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue variable_state = stBase.state_variable_get();
            UxViewColumn column_ux = null;
            column_ux = (UxViewColumn)VLStateManager.monobehaviourPointGet(sm_monoBehaviour_str);

            int hash = 0;
            if (!stBase.get_int("VScriptColumnPaste_varF", ref hash))
                return 0;

            int column = 0;
            if (!variable_state.get_int(hash, ref column))
                return 0;

            List<List<string>> column_aa = BaseFile.paser_list(GUIUtility.systemCopyBuffer, "\t");
            if (!column_ux.column_paste(column_aa, column))
                return 0;

            return 1;
        }

        public static int VScriptColumnSelectToggle_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue variable_state = stBase.state_variable_get();
            UxViewColumn column_ux = null;
            column_ux = (UxViewColumn)VLStateManager.monobehaviourPointGet(sm_monoBehaviour_str);

            int hash = 0;
            if (!stBase.get_int("VScriptColumnSelectToggle_varF", ref hash))
                return 0;

            int column = 0;
            if (!variable_state.get_int(hash, ref column))
                return 0;

            //List<List<string>> column_aa = BaseFile.paser_list(GUIUtility.systemCopyBuffer, "\t");
            column_ux.column_select_toggle(column);

            return 1;
        }

        public static int VScriptColumnSelectClear_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue variable_state = stBase.state_variable_get();
            UxViewColumn column_ux = null;
            column_ux = (UxViewColumn)VLStateManager.monobehaviourPointGet(sm_monoBehaviour_str);

            //List<List<string>> column_aa = BaseFile.paser_list(GUIUtility.systemCopyBuffer, "\t");
            column_ux.column_select_clear();

            return 1;
        }


        public static void StateFuncRegist()
        {
            VLStateManager.ProcessReg("VScriptColumnSelectToggle_varF", VScriptColumnSelectToggle_varF);
            VLStateManager.ProcessReg("VScriptColumnSelectClear_nF", VScriptColumnSelectClear_nF);
            VLStateManager.ProcessReg("VScriptColumnCopy_avarF", VScriptColumnCopy_avarF);
            VLStateManager.ProcessReg("VScriptColumnCopy_varF", VScriptColumnCopy_varF);
            VLStateManager.ProcessReg("VScriptColumnPaste_varF", VScriptColumnPaste_varF);
            VLStateManager.ProcessReg("VScriptColumnMove_avarF", VScriptColumnMove_avarF);
            VLStateManager.ProcessReg("VScriptColumnRefresh_nF", VScriptColumnRefresh_nF);
            VLStateManager.ProcessReg("VScriptColumnRemove_varF", VScriptColumnRemove_varF);
            VLStateManager.ProcessReg("VScriptColumnApply_varF", VScriptColumnApply_varF);
            VLStateManager.ProcessReg("VScriptColumnValueSet_varF", VScriptColumnValueSet_varF);
            VLStateManager.ProcessReg("VScriptColumnValueGet_varF", VScriptColumnValueGet_varF);
            VLStateManager.ProcessReg("VScriptCosmosMenuEnum_varF", VScriptCosmosMenuEnum_varF);
            VLStateManager.ProcessReg("VScriptColumnAdd_varF", VScriptColumnAdd_varF);
            VLStateManager.ProcessReg("VScriptColumnChange_nF", VScriptColumnChange_nF);
            VLStateManager.ProcessReg("VScriptColumnShow_nF", VScriptColumnShow_nF);
        }
    }
}