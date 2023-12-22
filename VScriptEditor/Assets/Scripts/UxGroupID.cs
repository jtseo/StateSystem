using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using UnityEngine.UI;
using System;
using System.IO;
using TMPro;

namespace StateSystem
{
    public class UxGroupID : MonoBehaviour
    {
        // Start is called before the first frame update
        public GameObject m_group_type_go;
        public GameObject m_group_id_go;

        List<int> m_group_type_a;
        List<List<int>> m_group_id_aa;
        List<int> m_group_check_a;
        List<List<int>> m_group_check_aa;

        public static UxGroupID ms_instance;

        void Start()
        {
            ms_instance = this;
            m_group_type_a = new List<int>();
            m_group_id_aa = new List<List<int>>();
            m_group_check_a = new List<int>();
            m_group_check_aa = new List<List<int>>();

            int key_all = VLStateManager.hash("All");
            group_add(key_all, 0);
            group_type_check(key_all);
            group_id_check(0);
        }

        bool group_find(int _group_type, int _group_id)
        {
            int type = -1;
            for (int i = 0; i < m_group_type_a.Count; i++)
            {
                if (m_group_type_a[i] == _group_type)
                {
                    type = i;
                    break;
                }
            }

            if (type == -1)
                return false;

            for (int i = 0; i < m_group_id_aa[type].Count; i++)
            {
                if (m_group_id_aa[type][i] == _group_id)
                {
                    if (m_group_check_aa[type][i] != 0)
                        return true;
                    else
                        return false;
                }
            }

            return false;
        }

        public bool group_filter_check(int[] _group_id_a)
        {
            if (m_group_check_a[0] == 1)
                return true;

            for (int i = 0; i < _group_id_a.Length; i += 2)
            {
                if (group_find(_group_id_a[i], _group_id_a[i + 1]))
                    return true; // it will return by only a group id. mean OR filter operation. 
            }

            return false;
        }

        public void group_add(int _group, int _id)
        {
            bool find = false;
            for (int i = 0; i < m_group_type_a.Count; i++)
            {
                if (m_group_type_a[i] == _group)
                {
                    find = true;
                    for (int j = 0; j < m_group_id_aa[i].Count; j++)
                    {
                        if (m_group_id_aa[i][j] == _id)
                            return;
                    }

                    m_group_id_aa[i].Add(_id);
                    m_group_check_aa[i].Add(0);
                    return;
                }
            }

            if (!find)
            {
                m_group_type_a.Add(_group);
                List<int> id_a = new List<int>();
                id_a.Add(_id);
                m_group_id_aa.Add(id_a);

                m_group_check_a.Add(0);
                List<int> check_a = new List<int>();
                check_a.Add(0);
                m_group_check_aa.Add(check_a);
            }
        }

        void group_type_check(int _key)
        {
            UxComponentDropdown drop = UxComponentDropdown.get(VLStateManager.hash("Group_Type"));

            int key_all = VLStateManager.hash("All");
            drop.list_check(_key);
            for (int i = 0; i < m_group_type_a.Count; i++)
            {
                if (m_group_type_a[i] == _key)
                {
                    if (m_group_check_a[i] == 0)
                    {
                        m_group_check_a[i] = 1;
                        if (key_all == _key
                            && VScriptLogHistory.ms_instance != null)
                            VScriptLogHistory.ms_instance.m_filter = false;
                    }
                    else
                    {
                        if (m_group_type_a[i] == key_all
                            && VScriptLogHistory.ms_instance != null)
                            VScriptLogHistory.ms_instance.m_filter = true;
                        m_group_check_a[i] = 0;
                    }
                }
            }

            if (VScriptLogHistory.ms_instance != null)
                VScriptLogHistory.ms_instance.log_link_recount(0);
        }

        void group_id_check(int _key)
        {
            int active_group = group_type_get();
            UxComponentDropdown drop = UxComponentDropdown.get(VLStateManager.hash("Group_ID"));

            for (int i = 0; i < m_group_check_aa[active_group].Count; i++)
            {
                if (m_group_id_aa[active_group][i] == _key)
                {
                    if (m_group_check_aa[active_group][i] == 0)
                        m_group_check_aa[active_group][i] = 1;
                    else
                        m_group_check_aa[active_group][i] = 0;
                    drop.list_check(_key);
                }
            }

            if (VScriptLogHistory.ms_instance != null)
                VScriptLogHistory.ms_instance.log_link_recount(0);
        }

        public void group_type_popup()
        {
            int hash_str = VLStateManager.hash("strName");
            UxComponentDropdown drop = UxComponentDropdown.get(VLStateManager.hash("Group_Type"));

            int key_all = VLStateManager.hash("All");

            List<string> list_a = new List<string>();
            for (int i = 0; i < m_group_type_a.Count; i++)
            {
                string str = "All";
                if (m_group_type_a[i] != key_all)
                    VLStateManager.enum_get("EnumGroupID", m_group_type_a[i], hash_str, ref str);

                list_a.Add(str);
            }

            drop.list_make(list_a, m_group_check_a);

            // for(int i=0; i<m_group_check_a.Count; i++)
            // {
            //     if(m_group_check_a[i] != 0)
            //         drop.list_check(m_group_type_a[i]);
            // }
        }

        public void group_type_close()
        {
            UxComponentDropdown drop = UxComponentDropdown.get(VLStateManager.hash("Group_Type"));
            drop.list_close();
        }

        public int group_type_get()
        {
            for (int i = 0; i < m_group_type_a.Count; i++)
            {
                if (m_group_check_a[i] != 0)
                    return i;
            }

            return 0;
        }

        public void group_id_popup()
        {
            int active_group = group_type_get();
            UxComponentDropdown drop = UxComponentDropdown.get(VLStateManager.hash("Group_ID"));

            if (m_group_id_aa.Count <= active_group)
                return;

            List<string> list_a = new List<string>();
            for (int i = 0; i < m_group_id_aa[active_group].Count; i++)
            {
                list_a.Add(m_group_id_aa[active_group][i].ToString());
            }

            drop.list_make(list_a, m_group_check_aa[active_group]);
        }

        public void group_id_close()
        {
            UxComponentDropdown drop = UxComponentDropdown.get(VLStateManager.hash("Group_ID"));
            drop.list_close();
        }

        // Update is called once per frame
        void Update()
        {

        }

        public static int VScriptGroupIDAdd_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue base_p = new StateDStructureValue(_pBase);
            StateDStructureValue event_p = new StateDStructureValue(_pEvent);

            int hash = 0;
            if (!base_p.get_int("VScriptGroupIDAdd_varF", ref hash))
                return 0;

            StateDStructureValue variable_state = base_p.state_variable_get();

            int[] key_a = event_p.get_int_arry(hash, 0);
            if (key_a == null)
            {
                key_a = variable_state.get_int_arry(hash, 0);
                if (key_a == null)
                    return 0;
            }

            for (int i = 0; i < key_a.Length; i += 2)
            {
                if (key_a.Length > i + 1)
                    UxGroupID.ms_instance.group_add(key_a[i], key_a[i + 1]);
                else
                    UxGroupID.ms_instance.group_add(key_a[i], 0);
            }

            return 1;
        }

        public static int VScriptGroupTypePopup_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            UxGroupID.ms_instance.group_type_popup();
            return 1;
        }

        public static int VScriptGroupIDPopup_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            UxGroupID.ms_instance.group_id_popup();
            return 1;
        }

        public static int VScriptGroupIDCheck_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue base_p = new StateDStructureValue(_pBase);

            int hash = 0;
            if (!base_p.get_int("VScriptGroupIDCheck_varF", ref hash))
                return 0;

            StateDStructureValue variable_state = base_p.state_variable_get();
            int key = 0;
            if (!variable_state.get_int(hash, ref key))
                return 0;

            UxGroupID.ms_instance.group_id_check(key);
            return 1;
        }

        public static int VScriptGroupTypeCheck_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue base_p = new StateDStructureValue(_pBase);

            int hash = 0;
            if (!base_p.get_int("VScriptGroupTypeCheck_varF", ref hash))
                return 0;

            StateDStructureValue variable_state = base_p.state_variable_get();
            int key = 0;
            if (!variable_state.get_int(hash, ref key))
                return 0;

            UxGroupID.ms_instance.group_type_check(key);
            return 1;
        }

        public static int VScriptGroupTypeClose_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            UxGroupID.ms_instance.group_type_close();
            return 1;
        }

        public static int VScriptGroupIDClose_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            UxGroupID.ms_instance.group_id_close();
            return 1;
        }

        public static void StateFuncRegist()
        {
            VLStateManager.ProcessReg("VScriptGroupIDAdd_varF", VScriptGroupIDAdd_varF,
                "VScriptEditor/Assets/Scripts/UxGroupID.cs", 0);
            VLStateManager.ProcessReg("VScriptGroupTypePopup_nF", VScriptGroupTypePopup_nF,
                "VScriptEditor/Assets/Scripts/UxGroupID.cs", 0);
            VLStateManager.ProcessReg("VScriptGroupIDPopup_nF", VScriptGroupIDPopup_nF,
                "VScriptEditor/Assets/Scripts/UxGroupID.cs", 0);
            VLStateManager.ProcessReg("VScriptGroupTypeCheck_varF", VScriptGroupTypeCheck_varF,
                "VScriptEditor/Assets/Scripts/UxGroupID.cs", 0);
            VLStateManager.ProcessReg("VScriptGroupTypeClose_nF", VScriptGroupTypeClose_nF,
                "VScriptEditor/Assets/Scripts/UxGroupID.cs", 0);
            VLStateManager.ProcessReg("VScriptGroupIDCheck_varF", VScriptGroupIDCheck_varF,
                "VScriptEditor/Assets/Scripts/UxGroupID.cs", 0);
            VLStateManager.ProcessReg("VScriptGroupIDClose_nF", VScriptGroupIDClose_nF,
                "VScriptEditor/Assets/Scripts/UxGroupID.cs", 0);
        }
    }
}