using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using System.IO;
using System.Linq;
using TMPro;
using UnityEngine.UI;

using System.Runtime.InteropServices;

namespace StateSystem
{
    public class VScriptState
    {
        public GameObject go;
        public Vector3 pos; // localPosition;

        public string name;
        public List<GameObject> sub_goa;
        public GameObject select_go;

        public VScriptState()
        {
            sub_goa = new List<GameObject>();
        }
    }

    public class VScriptEventLink
    {
        public int m_evt;
        public int m_link;
        public List<int> m_target_a;
        public List<int> m_linkto_a;
        public string m_param_str3;

        public void target_set(int _target, int _link)
        {
            for (int i = 0; i < m_target_a.Count; i++)
                if (m_target_a[i] == _target
                    && m_linkto_a[i] == _link)
                    return;

            m_target_a.Add(_target);
            m_linkto_a.Add(_link);
        }

        public VScriptEventLink()
        {
            m_evt = 0;
            m_param_str3 = "";
            m_target_a = new List<int>();
            m_linkto_a = new List<int>();
        }
    }

    public class UxViewStateContent : MonoBehaviour
    {
        public GameObject m_state_go;
        public GameObject m_line_go;
        public GameObject m_active_go;
        public GameObject m_leaf_go;
        public GameObject m_leafcopy_go;
        public GameObject m_back_go;
        public float m_line_scale_f = 3.0f;
        GameObject m_board_go, m_linkBoard_go;
        static public string sm_monoBehaviour_str;
        public Camera m_camera;
        string m_strName, m_strPath, m_strPathState, m_user_name;
        private StateDStructure m_dstMain, m_dstLink, m_dstMainPos;

        public StateDStructure m_dstExtVariable = null,
            m_dstExtEvent = null;

        bool m_update_b = false;

        Hashtable m_hashStateButtons;
        List<VScriptState> m_stateButtons_ago;
        Hashtable m_state_active_h;
        List<VScriptState> m_state_active_a;
        HashSet<int> m_state_updated;
        List<GameObject> m_links_ago;

        Hashtable m_evt_source_h;
        List<VScriptEventLink> m_evt_a;

        string m_search_state;
        string m_search_column;
        string m_search_value;

        int m_select_key_n; // selected key
        int m_select_type_n; // 0 is state, 1 is link

        int m_nKeyIndex;
        int m_nIntIndex;
        int m_index_link, m_index_name, m_index_link_type;

        public static UxViewStateContent ms_Instance = null;
        // Start is called before the first frame update

        public VScriptEventLink event_get(int _evt, string _param)
        {
            foreach (VScriptEventLink evt in m_evt_a)
                if (evt.m_evt == _evt && evt.m_param_str3 == _param)
                    return evt;

            return null;
        }

        public bool state_rename(int key, string _name)
        {
            int nKeyTo = VLStateManager.hash(_name);
            /*
            m_dstMain.key
            if (m_pdstBase->change_key(m_nKey, nKeyTo))
            {
                m_pdstBase->set_alloc(nKeyTo, (STATE_KEY), (void*)&nKeyTo);
                m_pdstBase->set_alloc(nKeyTo, (STATE_NAME), (void*)strNameParam);
    
                const int* pnKeys2;
                short nCnt2;
                m_pdstBase->get(nKeyTo, STATE_NEXT_LINK, (const void**)&pnKeys2, &nCnt2);
    
                STLMnstlRecord::iterator it;
                int nKey;
    
                nKey = m_pdstLink->get_first_key(&it);
                bool bUpdate = false;
                while (nKey)
                {
                    m_pdstLink->get(nKey, CON_NAME, (const void**)&strTemp, &nCnt);
                    if (strcmp(strTemp, strBackup) == 0)
                    {
                        m_pdstLink->set_alloc(nKey, (CON_NAME), (void*)strNameParam);
                        bUpdate = true;
                    }
                    nKey = m_pdstLink->get_next_key(&it);
                }
            }
            //*/
            return false;
        }

        public bool event_source_set(int _key, VScriptEventLink _evt)
        {
            List<VScriptEventLink> evt_a = (List<VScriptEventLink>)m_evt_source_h[_key];

            if (evt_a == null)
            {
                evt_a = new List<VScriptEventLink>();
                evt_a.Add(_evt);
                m_evt_source_h[_key] = evt_a;
                return true;
            }

            foreach (VScriptEventLink evt in evt_a)
            {
                if (evt.m_evt == _evt.m_evt
                    && evt.m_param_str3 == _evt.m_param_str3)
                    return false;
            }

            evt_a.Add(_evt);
            return true;
        }

        public VScriptEventLink event_set(int _evt, string _param, int _target, int _link, int _linkto)
        {
            VScriptEventLink evt = event_get(_evt, _param);
            if (evt == null)
            {
                evt = new VScriptEventLink();
                evt.m_evt = _evt;
                evt.m_link = _link;
                evt.m_param_str3 = _param;
                if (_target != 0)
                {
                    evt.m_target_a.Add(_target);
                    evt.m_linkto_a.Add(_linkto);
                }

                m_evt_a.Add(evt);
                return evt;
            }

            if (_target != 0)
                evt.target_set(_target, _linkto);

            if (_link != 0)
                evt.m_link = _link;
            return evt;
        }

        enum EvtOperation
        {
            None,
            Cast,
            Receive,
            Equal,
            Set
        }

        // event를 보내고 받는 Link를 표현하기 위한 준비 데이터 생성
        // event를 보내고 있는지 , 받고 있는지 확인하고 evt_a에는 이벤트를, evt_source_a에는 이벤트를 보내는 State 키를 기록한다.
        public bool event_parse(int _key)
        {
            int str3_key = VLStateManager.hash("TempString3_strV");
            int cast_key = VLStateManager.hash("EventCast");
            int event_key = VLStateManager.hash("Event");

            int index_equal = StateDStructure.get_index(VLStateManager.hash("BaseVariableEqual_anF"));
            int index_refer_str = StateDStructure.get_index(VLStateManager.hash("BaseVariableRefer_strV"));
            int index_refer_n = StateDStructure.get_index(VLStateManager.hash("BaseVariableRefer_anV"));
            int index_set = StateDStructure.get_index(VLStateManager.hash("BaseVariableSet_anF"));
            int index_cast = StateDStructure.get_index(VLStateManager.hash("BaseStateEventGlobalCast_nF"));
            int index_receive = StateDStructure.get_index(VLStateManager.hash("BaseStateEventGlobal"));

            // m_index_link

            int[] link_an = null;
            if (m_dstMain.get_int_index(_key, m_index_link, ref link_an))
            {
                foreach (int link in link_an)
                {
                    EvtOperation type_rec = EvtOperation.None, type_cast = EvtOperation.None;
                    string evt_param = "";
                    int evt_key = 0, evt_cast_key = 0;

                    int count_n = m_dstLink.get_count_column(link);
                    for (int i = 0; i < count_n; i++)
                    {
                        int index = m_dstLink.get_index(link, i);
                        string str = m_dstLink.editor_string_get(link, i);

                        // receiving parse
                        if (index == index_receive)
                        {
                            type_rec = EvtOperation.Receive;
                            evt_key = VLStateManager.hash(str);
                        }

                        if (index == index_equal && type_rec == EvtOperation.Receive)
                        {
                            List<List<string>> str_aa = BaseFile.paser_list(str, ",");
                            if (str_aa.Count > 0 && str_aa[0].Count > 0)
                            {
                                List<string> str_a = str_aa[0];

                                if (VLStateManager.hash(str_a[0]) == event_key)
                                    type_rec = EvtOperation.Equal;
                            }
                        }

                        if (index == index_refer_str && type_rec == EvtOperation.Equal)
                        {
                            List<List<string>> str_aa = BaseFile.paser_list(str, ",");
                            if (str_aa.Count > 0 && str_aa[0].Count > 0)
                            {
                                List<string> str_a = str_aa[0];

                                if (str_a[0] == "TempString3_strV")
                                {
                                    event_set(evt_key, str_a[1], _key, 0, link);
                                    type_rec = EvtOperation.None;
                                }
                            }
                        }

                        // casting parse
                        if (index == index_set)
                        {
                            List<List<string>> str_aa = BaseFile.paser_list(str, ",");
                            if (str_aa.Count > 0 && str_aa[0].Count > 0)
                            {
                                List<string> str_a = str_aa[0];

                                if (VLStateManager.hash(str_a[0]) == cast_key)
                                    type_cast = EvtOperation.Set;
                            }
                        }

                        if (index == index_refer_str && type_cast == EvtOperation.Set)
                        {
                            List<List<string>> str_aa = BaseFile.paser_list(str, ",");
                            if (str_aa.Count > 0 && str_aa[0].Count > 0)
                            {
                                List<string> str_a = str_aa[0];

                                if (str_a[0] == "TempString3_strV")
                                {
                                    evt_param = str_a[1];
                                    type_cast = EvtOperation.Cast;
                                }
                            }
                        }

                        if (index == index_cast)
                        {
                            evt_cast_key = VLStateManager.hash(str);
                            VScriptEventLink evt = event_set(evt_cast_key, evt_param, 0, link, 0);
                            event_source_set(_key, evt);
                        }
                    } // end of for

                    if (evt_key != 0 &&
                        (type_rec == EvtOperation.Receive
                         || type_rec == EvtOperation.Equal
                        )
                       ) // set event what doesn't set receiving event(without param)
                    {
                        event_set(evt_key, "", _key, 0, link);
                    }
                }
            }

            return true;
        }

        void initialize()
        {
            ms_Instance = this;

            m_evt_a = new List<VScriptEventLink>();
            m_evt_source_h = new Hashtable();

            m_nKeyIndex = StateDStructure.get_index(VLStateManager.hash("nKey"));
            m_nIntIndex = StateDStructure.get_index(VLStateManager.hash("FileInt_anV"));
            m_index_link = StateDStructure.get_index(VLStateManager.hash("arrayLink"));
            m_index_name = StateDStructure.get_index(VLStateManager.hash("strName"));
            m_index_link_type = StateDStructure.get_index(VLStateManager.hash("BaseLinkType"));

            m_hashStateButtons = new Hashtable();
            m_stateButtons_ago = new List<VScriptState>();
            m_state_active_h = new Hashtable();
            m_state_active_a = new List<VScriptState>();
            m_links_ago = new List<GameObject>();
            m_state_updated = new HashSet<int>();
            sm_monoBehaviour_str = "VScriptMonoBehaviourUxViewStateContent";

            m_board_go = m_state_go; //(GameObject)GameObject.Find("StateBoard");
            m_linkBoard_go = m_line_go; // (GameObject)GameObject.Find("LinkBoard");
            m_state_go.SetActive(false);
            m_active_go.SetActive(false);
            m_leaf_go.SetActive(false);
            m_leafcopy_go.SetActive(false);

            VLStateManager.monobehaviourPointSet(sm_monoBehaviour_str, this);
        }

        void Start()
        {
            if (ms_Instance == null)
                initialize();
        }

        public StateDStructure StateMainGet()
        {
            return m_dstMain;
        }

        public StateDStructure StateLinkGet()
        {
            return m_dstLink;
        }

        public int select_key_get(ref int _type_n)
        {
            _type_n = m_select_type_n;
            return m_select_key_n;
        }

        Vector3 s_pos_ani, s_pos_org;

        float theta_f = 0;

        // Update is called once per frame
        void Update()
        {
            if (ms_Instance == null)
                initialize();
            UpdateActive();

            if (m_update_b)
            {
                ;
                m_update_b = false;
                scene_link_remake();
            }
            else
            {
                if (m_stateButtons_ago.Count > 0)
                {
                    s_pos_ani = new Vector3(0, Mathf.Sin(theta_f) * 500.0f, 0);
                    s_pos_ani += s_pos_org;
                    //m_stateButtons_ago[0].transform.localPosition = s_pos_ani;

                    theta_f += 0.1f;
                    ;
                }
            }
        }

        Hashtable m_state_reserved;

        bool state_system_is(int _key_n)
        {
            if (m_state_reserved == null)
            {
                m_state_reserved = new Hashtable();
                m_state_reserved.Add(VLStateManager.hash("state_dummy"), "state_dummy");
                m_state_reserved.Add(VLStateManager.hash("state_context"), "state_context");
                m_state_reserved.Add(VLStateManager.hash("state_return"), "state_return");
                m_state_reserved.Add(VLStateManager.hash("state_release"), "state_release");
            }

            if (m_state_reserved[_key_n] == null)
                return false;
            return true;
        }

        public void scene_update()
        {
            m_update_b = true;
        }

        public bool StateMove(int key_n, Vector3 _pos_move, Hashtable _moved, int _typeMove)
        {
            if (_moved.Contains(key_n))
                return true;

            VScriptState goBtn = (VScriptState)m_hashStateButtons[key_n];

            //goBtn.pos += _pos_move;
            if (goBtn == null)
                return false;

            Vector3 pos_org = goBtn.go.transform.position;
            goBtn.go.transform.position += _pos_move;

            int[] anPos, anKey;
            anPos = new int[2];
            anKey = new int[1];

            Vector3 pos_v3 = goBtn.go.transform.localPosition;
            anKey[0] = key_n;
            anPos[0] = (int)pos_v3.x;
            anPos[1] = (int)pos_v3.y;

            if (!m_dstMainPos.get_int(key_n, 0, ref anKey))
            {
                anKey[0] = key_n;
                m_dstMainPos.add_int(key_n, m_nKeyIndex, anKey);
            }

            if (anPos[1] > 2000 || anPos[1] < -2000)
            {
                int x = 0;
                x++;
            }

            m_dstMainPos.set_int(key_n, m_nIntIndex, anPos);

            if (_typeMove == 2)
                return true;

            _moved.Add(key_n, goBtn);

            string name_str = "";
            int[] Link_an = null;
            int LinkIndex_n = StateDStructure.get_index(VLStateManager.hash("arrayLink"));
            int index_link_type = StateDStructure.get_index(VLStateManager.hash("BaseLinkType"));
            int type_chagne = VLStateManager.hash("state_change");
            int[] link_type_a = null;

            if (m_dstMain.get_int_index(key_n, LinkIndex_n, ref Link_an))
            {
                for (int i = 0; i < Link_an.Length; i++)
                {
                    bool change = false;
                    if (m_dstLink.get_int_index(Link_an[i], index_link_type, ref link_type_a))
                    {
                        if (link_type_a[0] == type_chagne)
                            change = true;
                    }
                    else
                    {
                        change = true;
                    }

                    if (m_dstLink.get_string(Link_an[i], 1, ref name_str))
                    {
                        int key_n2 = VLStateManager.hash(name_str);
                        if (state_system_is(key_n2))
                            continue;

                        if (change)
                        {
                            VScriptState state_link = (VScriptState)m_hashStateButtons[key_n2];
                            if (state_link.go.transform.position.x < pos_org.x)
                                continue;
                        }


                        if (!StateMove(key_n2, _pos_move, _moved, _typeMove))
                        {
                            StateDStructureValue evt = VLStateManager.event_state_make("UxNotifierAuto");
                            evt.set_variable_string("TempString_strV", "Fail to find state '" + name_str + "'");
                            VLStateManager.event_post(evt);
                        }
                    }
                }
            }

            return true;
        }

        public int main_key_get()
        {
            return VLStateManager.hash(m_strName);
        }

        public bool state_add(string _name, Vector3 _pos)
        {
            int nKey = VLStateManager.hash(_name);
            VScriptState goBtn = (VScriptState)m_hashStateButtons[nKey];

            _pos.z = 0;
            if (goBtn != null)
                return false;

            int[] key_a = new int[1];
            key_a[0] = nKey;
            m_dstMain.add_int(nKey, 0, key_a);
            m_dstMain.add_string(nKey, 1, _name);
            int[] pos_a = new int[2];
            pos_a[0] = (int)_pos.x;
            pos_a[1] = (int)_pos.y;

            if (!m_dstMainPos.get_int(nKey, 0, ref key_a))
            {
                key_a[0] = nKey;
                m_dstMainPos.add_int(nKey, m_nKeyIndex, key_a);
            }

            m_dstMainPos.set_int(nKey, m_nIntIndex, pos_a);
            m_dstMainPos.get_int(nKey, 1, ref pos_a);

            state_add_(_name, _pos);

            return true;
        }

        void state_add_(string _name, Vector3 _pos)
        {
            GameObject goAdd;
            goAdd = (GameObject)Instantiate(m_state_go);
            goAdd.transform.SetParent(m_board_go.transform.parent, false);
            goAdd.SetActive(true);

            goAdd.transform.localPosition = _pos;

            TMP_Text txtMep;
            int nStateKey = VLStateManager.hash(_name);

            VScriptState vscript_state = new VScriptState();
            vscript_state.go = goAdd;
            vscript_state.pos = goAdd.transform.position;
            vscript_state.name = _name;

            if (m_hashStateButtons.ContainsKey(nStateKey))
            {
                VScriptState dual_staet = (VScriptState)m_hashStateButtons[nStateKey];
            }

            m_hashStateButtons.Add(nStateKey, vscript_state);
            m_stateButtons_ago.Add(vscript_state);
            txtMep = goAdd.GetComponentInChildren<TMP_Text>();
            txtMep.text = _name;
        }


        Camera m_cam_viewer;

        public Vector3 ScreenToWorld(string _camera, Vector3 _pos)
        {
            if (_camera.Length > 0)
            {
                GameObject cam_go = GameObject.Find(_camera);
                m_cam_viewer = cam_go.GetComponent<Camera>();
            }

            if (m_cam_viewer == null)
                return Vector3.zero;

            Vector3 pos = m_cam_viewer.ScreenToWorldPoint(_pos);
            pos.z = 0;
            return pos;
        }

        public bool StateMove(string _strName, Vector3 _v3Pos, int _typeMove)
        {
            int nKey = VLStateManager.hash(_strName);
            VScriptState goBtn = (VScriptState)m_hashStateButtons[nKey];

            if (goBtn == null)
                return false;

            Vector3 pos_mov = _v3Pos - goBtn.go.transform.position;

            StateMove(nKey, pos_mov, new Hashtable(), _typeMove);

            return true;
        }

        public bool UpdateActive()
        {
            foreach (int key in m_state_updated)
            {
                VScriptState btn = (VScriptState)m_state_active_h[key];
                if (btn == null)
                    continue;

                foreach (GameObject leaf in btn.sub_goa)
                    GameObject.Destroy(leaf);
                btn.sub_goa.Clear();

                //StateLeafShow(key);
                LinkShow(key);
            }

            m_state_updated.Clear();
            return true;
        }

        public bool StateLeafHide(string _name)
        {
            int key = VLStateManager.hash(_name);
            VScriptState btn = (VScriptState)m_state_active_h[key];

            if (btn == null)
                return false;

            GameObject.Destroy(btn.select_go);
            foreach (GameObject leaf in btn.sub_goa)
            {
                GameObject.Destroy(leaf);
            }

            btn.sub_goa.Clear();
            m_state_active_h.Remove(key);
            m_state_active_a.Remove(btn);

            scene_link_remake();
            return true;
        }

        int state_get_by_link(int _key_link)
        {
            foreach (VScriptState btn in m_state_active_a)
            {
                foreach (GameObject leaf in btn.sub_goa)
                {
                    VScriptLink link = leaf.GetComponent<VScriptLink>();
                    if (link.m_key == _key_link)
                        return VLStateManager.hash(btn.name);
                }
            }

            return 0;
        }

        public VScriptLink linksub_get(int _key)
        {
            foreach (VScriptState btn in m_state_active_a)
            {
                foreach (GameObject leaf in btn.sub_goa)
                {
                    VScriptLink link = leaf.GetComponent<VScriptLink>();
                    if (link.m_key == _key)
                        return link;
                }
            }

            return null;
        }

        public bool StateLeafHide()
        {
            foreach (VScriptState btn in m_state_active_a)
            {
                GameObject.Destroy(btn.select_go);
                foreach (GameObject leaf in btn.sub_goa)
                {
                    GameObject.Destroy(leaf);
                }

                btn.sub_goa.Clear();
            }

            m_state_active_h.Clear();
            m_state_active_a.Clear();

            foreach (VScriptState btn in m_stateButtons_ago)
            {
                VScriptLink link = btn.go.GetComponent<VScriptLink>();
                if (link == null)
                    continue;
                link.marker_toggle(-1, false);
            }

            // marker
            return true;
        }

        static int access_cnt = 1;

        public bool StateLeafShow(string _strName)
        {
            int nKey = VLStateManager.hash(_strName);

            return StateLeafShow(nKey);
        }

        public bool state_delete(string _name)
        {
            StateLeafHide(_name);
            int key = VLStateManager.hash(_name);

            m_dstMain.release(key);

            int nKey, key_del = 0;
            nKey = m_dstLink.key_first_get();

            do
            {
                key_del = 0;
                do
                {
                    string name = "";
                    if (m_dstLink.get_string(nKey, 1, ref name))
                    {
                        if (name == _name)
                        {
                            key_del = nKey;
                            break;
                        }
                    }

                    nKey = m_dstLink.key_next_get();
                } while (nKey != 0);

                if (key_del != 0)
                {
                    delete_link(key_del);
                    nKey = m_dstLink.key_first_get();
                }
            } while (key_del != 0);

            VScriptState btn = (VScriptState)m_hashStateButtons[key];

            GameObject.Destroy(btn.go);
            m_hashStateButtons.Remove(key);
            m_stateButtons_ago.Remove(btn);

            return true;
        }

        public void state_save()
        {
            m_dstMain.save(null, 3);
            m_dstLink.save(null, 3);

            VLStateManager.event_state_post("VScriptFilesaved");
        }

        class SubStringCompare : IComparer<GameObject>
        {
            public int Compare(GameObject x, GameObject y)
            {
                string xs, ys;
                TMP_Text txt;
                txt = x.GetComponentInChildren<TMP_Text>();
                xs = txt.name.Substring(txt.name.IndexOf("("));

                txt = y.GetComponentInChildren<TMP_Text>();
                ys = txt.name.Substring(txt.name.IndexOf("("));

                return xs.CompareTo(ys);

            }
        }

        public bool LinkShow(int _key)
        {
            VScriptState goBtn = (VScriptState)m_hashStateButtons[_key];
            // add leaf node
            string strComment = "";
            int[] Link_an = null;
            int LinkIndex_n = StateDStructure.get_index(VLStateManager.hash("arrayLink"));
            int index_link_type = StateDStructure.get_index(VLStateManager.hash("BaseLinkType"));
            int index_priority = StateDStructure.get_index(VLStateManager.hash("BaseLinkPriority_n"));
            int index_LEx = StateDStructure.get_index(VLStateManager.hash("BaseExclusiveLink"));
            int index_SEx = StateDStructure.get_index(VLStateManager.hash("BaseExclusiveState"));
            int index_EEx = StateDStructure.get_index(VLStateManager.hash("BaseExclusiveEvent"));
            int index_onetime = StateDStructure.get_index(VLStateManager.hash("BaseLinkOneTime_b"));

            int hash_comment = VLStateManager.hash("Comment");
            int[] type = new int[3];
            string[] type_name_a = new string[3];
            type_name_a[0] = "state_dummy";
            type_name_a[1] = "state_return";
            type_name_a[2] = "state_release";
            for (int i = 0; i < type.Length; i++)
                type[i] = VLStateManager.hash(type_name_a[i]);
            int[] link_type_a = null;
            int hash_link_copy = VLStateManager.hash("state_linkcopy");

            if (m_dstMain.get_int_index(_key, LinkIndex_n, ref Link_an))
            {
                List<int> keys_an = new List<int>();
                List<int> keyLinks_an = new List<int>();
                List<int> type_an = new List<int>();
                for (int i = 0; i < Link_an.Length; i++)
                {
                    string name = "";
                    m_dstLink.get_string(Link_an[i], 1, ref name);
                    if (name.Length == 0)
                    {
                        delete_link(Link_an[i]);
                        continue;
                    }

                    if (!m_dstLink.get_int_index(Link_an[i], index_link_type, ref link_type_a))
                    {
                        link_type_a = new int[1];
                        link_type_a[0] = VLStateManager.hash("state_change");
                    }

                    GameObject leaf;

                    leaf = (GameObject)Instantiate(m_leaf_go);
                    if (link_type_a[0] == hash_link_copy)
                    {
                        StateLeafShow(name);
                    }

                    leaf.transform.SetParent(goBtn.go.transform, false);
                    leaf.SetActive(true);

                    VScriptLink vs_link = leaf.GetComponent<VScriptLink>();
                    vs_link.m_key = Link_an[i];
                    int serial = 0;
                    int cnt = VScriptLogHistory.ms_instance.log_link_cnt(vs_link.m_key, ref serial);
                    if (cnt > 0)
                    {
                        vs_link.m_count.text = cnt.ToString();
                        vs_link.m_serial.text = serial.ToString();
                    }

                    TMP_Text txt;
                    txt = leaf.GetComponentInChildren<TMP_Text>();
                    txt.text = name;
                    if (m_dstLink.get_hash_string(Link_an[i], hash_comment, ref strComment))
                        txt.text += "(" + strComment + ")";

                    int[] priority_a = null;
                    if (m_dstLink.get_int_index(Link_an[i], index_priority, ref priority_a))
                        txt.text += "P:" + priority_a[0].ToString();

                    if (m_dstLink.get_int_index(Link_an[i], index_LEx, ref priority_a))
                        txt.text += ":xL";

                    if (m_dstLink.get_int_index(Link_an[i], index_SEx, ref priority_a))
                        txt.text += ":xS";

                    if (m_dstLink.get_int_index(Link_an[i], index_EEx, ref priority_a))
                        txt.text += ":xE";

                    if (m_dstLink.get_int_index(Link_an[i], index_onetime, ref priority_a))
                    {
                        if (priority_a[0] != 0)
                            txt.text += ":One";
                    }

                    vs_link.m_name = txt.text;
                    goBtn.sub_goa.Add(leaf);
                }

                if (goBtn.select_go != null)
                {
                    goBtn.select_go.transform.localPosition = Vector3.zero; //goBtn.go.transform.localPosition;

                    float delta = 0;
                    foreach (GameObject leaf in goBtn.sub_goa)
                    {
                        Vector3 pos = Vector3.zero; //goBtn.go.transform.localPosition;
                        pos.x += 50;
                        pos.y -= delta;
                        delta += 15;
                        leaf.transform.localPosition = pos;
                    }
                }
            }

            state_column_search(_key, m_search_column, m_search_value);
            return true;
        }

        public bool StateLeafShow(int _key)
        {
            VScriptState goBtn = (VScriptState)m_hashStateButtons[_key];

            if (goBtn == null)
                return false;

            if (goBtn.select_go != null)
                return false;

            if (goBtn.select_go == null)
            {
                goBtn.select_go = (GameObject)Instantiate(m_active_go);
                goBtn.select_go.transform.SetParent(goBtn.go.transform, false);
                goBtn.select_go.SetActive(true);

                m_state_active_h[_key] = goBtn;
                m_state_active_a.Add(goBtn);

                LinkShow(_key);

                scene_link_remake();
            }

            return true;
        }

        public void StateSavePos()
        {
            m_dstMainPos.save(m_strPath + "Pos", 3);
        }

        bool link_type_set(int key, string _type)
        {
            int type = VLStateManager.hash(_type);

            int[] key_an = new int[1];
            key_an[0] = type;
            m_dstLink.set_int(key, m_index_link_type, key_an);
            ;
            scene_link_remake();

            return true;
        }

        public bool delete_link(int _link)
        {
            m_dstLink.release(_link);

            int nKey;
            nKey = m_dstMain.key_first_get();
            int[] link_an = null;
            do
            {
                if (m_dstMain.get_int_index(nKey, m_index_link, ref link_an))
                {
                    int[] link_update_an = null;
                    if (link_an.Length > 1)
                        link_update_an = new int[link_an.Length - 1];
                    bool find = false;
                    int k = 0;
                    for (int i = 0; i < link_an.Length; i++)
                    {
                        if (link_an[i] != _link)
                        {
                            if (k == link_an.Length - 1)
                                break;
                            link_update_an[k] = link_an[i];
                            k++;
                        }
                        else
                        {
                            find = true;
                        }
                    }

                    if (find)
                    {
                        m_dstMain.set_int(nKey, m_index_link, link_update_an);
                        m_state_updated.Add(nKey);

                        scene_link_remake();
                        return true;
                    }
                }

                nKey = m_dstMain.key_next_get();
            } while (nKey != 0);

            return false;
        }

        public int select_link(Vector3 _pos_v3)
        {
            //Curve cuv2 = m_linkBoard_go.GetComponent<Curve>();
            //cuv2.line_set(100, Vector3.zero, _pos_v3, 1);

            for (int i = 0; i < m_links_ago.Count; i++)
            {
                Curve cuv = m_links_ago[i].GetComponent<Curve>();

                if (cuv == null)
                    return 0;

                if (cuv.select(_pos_v3))
                {
                    m_select_key_n = cuv.key_get();
                    m_select_type_n = 1;
                    return m_select_key_n;
                }
            }

            return 0;
        }

        public bool LinkAdd(int _key_n, int _from_n, int _to_n, int _type, int _link_end)
        {
            VScriptState from_go, to_go; // mark_link
            from_go = (VScriptState)m_hashStateButtons[_from_n];
            to_go = (VScriptState)m_hashStateButtons[_to_n];

            if (from_go == null || to_go == null)
                return false;

            GameObject link_go = (GameObject)Instantiate(m_line_go);
            link_go.transform.SetParent(m_linkBoard_go.transform.parent, false);
            link_go.transform.position = new Vector3(0, 0, 0);
            //link_go.transform.localScale = new Vector3(scale_f, scale_f, scale_f);
            Curve cuv = link_go.GetComponent<Curve>();

            Vector3 from_v3 = Vector3.zero, to_v3;
            from_v3 = from_go.go.transform.position;
            foreach (GameObject leaf in from_go.sub_goa)
            {
                VScriptLink link = leaf.GetComponent<VScriptLink>();
                if (link.m_key == _key_n)
                {
                    from_v3 = link.m_marker[0].transform.position;
                    cuv.m_wing_start = 5;
                    break;
                }
            }

            to_v3 = to_go.go.transform.position;
            foreach (GameObject leaf in to_go.sub_goa)
            {
                VScriptLink link = leaf.GetComponent<VScriptLink>();
                if (link.m_key == _link_end)
                {
                    to_v3 = link.m_marker[0].transform.position;
                    cuv.m_wing_end = 5;
                    break;
                }
            }

            cuv.line_set(_key_n, from_v3, to_v3, _type);

            m_links_ago.Add(link_go);

            return true;
        }

        public bool LinksAdd(int _KeyFrom, int[] _Key_an, int[] _To_an, int[] _type_a, int[] _keyto)
        {
            bool bRet = false;
            for (int i = 0; i < _Key_an.Length; i++)
            {
                bRet = LinkAdd(_Key_an[i], _KeyFrom, _To_an[i], _type_a[i], _keyto[i]);
                if (!bRet)
                    return bRet;
            }

            return true;
        }

        public void scene_release()
        {
            scene_state_release();
            scene_link_release();
        }

        public void scroll_size_update()
        {
            Vector3 min_v3, max_v3, pos_v3;

            min_v3 = new Vector3(0, 0);
            max_v3 = new Vector3(0, 0);
            for (int i = 0; i < m_stateButtons_ago.Count; i++)
            {
                pos_v3 = m_stateButtons_ago[i].go.transform.localPosition;

                if (pos_v3.x < min_v3.x)
                    min_v3.x = pos_v3.x;
                if (pos_v3.y < min_v3.y)
                    min_v3.y = pos_v3.y;

                if (pos_v3.x > max_v3.x)
                    max_v3.x = pos_v3.x;
                if (pos_v3.y > max_v3.y)
                    max_v3.y = pos_v3.y;
            }

            min_v3 = max_v3 - min_v3;
            min_v3 += new Vector3(200, 200, 0);

        }

        public void scene_link_release()
        {
            for (int i = 0; i < m_links_ago.Count; i++)
            {
                GameObject.Destroy(m_links_ago[i]);
            }

            m_links_ago.Clear();
        }

        public void scene_link_remake()
        {
            scene_link_release();

            string strName = "";
            int[] Link_an = null;
            int LinkIndex_n = StateDStructure.get_index(VLStateManager.hash("arrayLink"));
            int index_link_type = StateDStructure.get_index(VLStateManager.hash("BaseLinkType"));
            int[] type = new int[4];
            type[0] = VLStateManager.hash("state_change");
            type[1] = VLStateManager.hash("state_add");
            type[2] = VLStateManager.hash("state_linkcopy");
            type[3] = VLStateManager.hash("state_call");
            int[] link_type_a = null;

            if (m_dstMain == null)
                return;

            int nKey;
            nKey = m_dstMain.key_first_get();
            do
            {
                if (state_system_is(nKey))
                {
                    nKey = m_dstMain.key_next_get();
                    continue;
                }

                if (m_dstMain.get_int_index(nKey, LinkIndex_n, ref Link_an))
                {
                    List<int> keys_an = new List<int>();
                    List<int> keyLinks_an = new List<int>();
                    List<int> type_an = new List<int>();
                    for (int i = 0; i < Link_an.Length; i++)
                    {
                        int link_type = 0;
                        if (m_dstLink.get_string(Link_an[i], 1, ref strName))
                        {
                            int space_n = strName.IndexOf(' ');
                            if (space_n > 0)
                            {
                                strName = strName.Substring(0, space_n);
                            }

                            int key_n = VLStateManager.hash(strName);
                            if (state_system_is(key_n))
                                continue;
                            if (m_dstLink.get_int_index(Link_an[i], index_link_type, ref link_type_a))
                            {
                                for (int k = 0; k < type.Length; k++)
                                    if (type[k] == link_type_a[0])
                                        link_type = k;
                            }

                            keys_an.Add(key_n);
                            keyLinks_an.Add(Link_an[i]);
                            type_an.Add(link_type);
                        }
                    }

                    int[] To_an = new int[keys_an.Count];
                    int[] links_an = new int[keyLinks_an.Count];
                    int[] type_a = new int[type_an.Count];
                    int[] linkto_a = new int[type_an.Count];
                    for (int i = 0; i < keys_an.Count; i++)
                    {
                        links_an[i] = keyLinks_an[i];
                        To_an[i] = keys_an[i];
                        type_a[i] = type_an[i];
                        linkto_a[i] = 0;
                    }

                    LinksAdd(nKey, links_an, To_an, type_a, linkto_a);
                }

                if (m_state_active_h[nKey] != null)
                {
                    List<VScriptEventLink> evt_a = (List<VScriptEventLink>)m_evt_source_h[nKey];
                    if (evt_a != null)
                    {
                        foreach (VScriptEventLink evt in evt_a)
                        {
                            int[] links_an = new int[evt.m_target_a.Count];
                            int[] To_an = new int[evt.m_target_a.Count];
                            int[] type_a = new int[evt.m_target_a.Count];
                            int[] linkto_a = new int[evt.m_target_a.Count];

                            for (int i = 0; i < evt.m_target_a.Count; i++)
                            {
                                links_an[i] = evt.m_link;
                                To_an[i] = evt.m_target_a[i];
                                type_a[i] = 4;
                                linkto_a[i] = evt.m_linkto_a[i];
                            }

                            LinksAdd(nKey, links_an, To_an, type_a, linkto_a);
                        }
                    }
                }

                nKey = m_dstMain.key_next_get();
            } while (nKey != 0);

        }

        public void scene_state_release()
        {
            StateLeafHide();
            m_hashStateButtons.Clear();

            m_evt_a.Clear();
            m_evt_source_h.Clear();

            for (int i = 0; i < m_stateButtons_ago.Count; i++)
            {
                m_stateButtons_ago[i].go.SetActive(false);
                GameObject.Destroy(m_stateButtons_ago[i].go);
            }

            m_stateButtons_ago.Clear();
        }


        class GFG : IComparer<string>
        {
            public int Compare(string x, string y)
            {
                // CompareTo() method 
                return x.CompareTo(y);

            }
        }

        public void scene_make()
        {
            scene_state_release();

            int nCnt = 0;

            string strName = "";
            int nKey = 0;

            List<string> state_astr = new List<string>();

            nKey = m_dstMain.key_first_get();
            do
            {
                if (!m_dstMain.get_string(nKey, 1, ref strName))
                    break;

                if (state_system_is(nKey))
                {
                    nKey = m_dstMain.key_next_get();
                    continue;
                }

                event_parse(nKey);

                state_astr.Add(strName);
                nKey = m_dstMain.key_next_get();
            } while (nKey != 0);

            GFG cc = new GFG();
            state_astr.Sort(cc);

            int[] key_a = new int[1];
            int[] anPos = new int[2];
            Vector3 pos_init = Vector3.zero;
            Vector3 pos = Vector3.zero;
            for (int i = 0; i < state_astr.Count; i++)
            {
                strName = state_astr[i];
                nKey = VLStateManager.hash(strName);

                if (m_dstMainPos.get_int(nKey, 1, ref anPos))
                {
                    pos = new Vector3(anPos[0], anPos[1], 0);
                }
                else
                {
                    pos_init.y += 1;
                    pos = pos_init;
                    key_a[0] = nKey;
                    anPos[0] = 0;
                    anPos[1] = (int)pos.y;
                    m_dstMainPos.add_int(nKey, m_nKeyIndex, key_a);
                    m_dstMainPos.set_int(nKey, m_nIntIndex, anPos);
                }


                state_add_(strName, pos);
                nCnt++;
            }

            scene_link_remake();
            scroll_size_update();

            if (m_stateButtons_ago.Count > 0)
                s_pos_org = m_stateButtons_ago[0].pos;
        }

        public bool reload()
        {
            if (m_dstMain == null)
                return false;

            scene_release();

            m_dstMain.delete();
            m_dstLink.delete();

            m_dstMain = StateDStructure.create();
            m_dstLink = StateDStructure.create();

            if (!m_dstMain.load(m_strPath, 3))
                return false;
            if (!m_dstLink.load(m_strPath + "Link", 3))
                m_dstLink.save(m_strPath + "Link", 3);
            scene_make();
            VScriptLogHistory.ms_instance.key_main_set(main_key_get());
            VLStateManager.event_state_post("VScriptFileloaded");

            return true;
        }

        public bool Load(string _strStateMain)
        {

            m_strName = _strStateMain;

            if (m_dstMain != null)
            {
                m_dstMain.delete();
                m_dstMainPos.delete();
                m_dstLink.delete();
            }

            m_dstMain = StateDStructure.create();
            m_dstMainPos = StateDStructure.create();
            m_dstLink = StateDStructure.create();

            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
            MonoBehaviour uxViewMain = null;
            if (dstGlobal.get_ptr(UxViewMain.sm_strMonoBehaviour, ref uxViewMain))
            {
                m_strPathState = ((UxViewMain)uxViewMain).PathGet();
                m_strPath = ((UxViewMain)uxViewMain).PathGet() + _strStateMain;

            }
            else
            {
                return false;
            }

            if (!m_dstMain.load(m_strPath, 3))
                return false;
            if (!m_dstLink.load(m_strPath + "Link", 3))
                m_dstLink.save(m_strPath + "Link", 3);
            if (!m_dstMainPos.load(m_strPath + "Pos", 4))
                m_dstMainPos.save(m_strPath + "Pos", 3);
            state_define();
            VLStateManager.event_state_post("VScriptFileloaded");
            scene_make();

            // ===================== read configEdit to load user name, it will used to crate link hash to make sure difference key.
            int index_path = m_strPath.LastIndexOf('/');
            if (index_path < 0)
                index_path = m_strPath.LastIndexOf('\\');
            string path = m_strPath.Substring(0, index_path + 1);
            path += "configEdit.ini";

            BaseFile file = BaseFile.create();
            if (file.open_file(path))
            {
                file.asc_read_line();
                file.asc_read_line();
                file.asc_read_string(ref m_user_name);
            }

            file.delete();

            VScriptLogHistory.ms_instance.key_main_set(main_key_get());
            return true;
        }

        bool state_define()
        {
            int[] link_an = null;
            int cnt = 0;
            int define = VLStateManager.hash("state_define");
            if (!m_dstMain.get_int_index(define, m_index_link, ref link_an))
                return false;
            cnt = link_an.Length;

            if (m_dstExtVariable != null)
                m_dstExtVariable.delete();
            m_dstExtVariable = StateDStructure.create();

            if (m_dstExtEvent != null)
                m_dstExtEvent.delete();
            m_dstExtEvent = StateDStructure.create();

            foreach (int key in link_an)
            {
                List<List<string>> defines = null;
                m_dstLink.string_get(key, ref defines);
                if (defines.Count < 5)
                    continue;

                int type = 0;
                if (defines[3][1] == "VariableInt")
                    type = 1;
                else if (defines[3][1] == "VariableString")
                    type = 2;
                else if (defines[3][1] == "StateEvent")
                    type = 3;
                else if (defines[3][1] == "VariableArrayFloat")
                    type = 4;
                else if (defines[3][1] == "VariableFloat")
                    type = 5;
                else if (defines[3][1] == "VariableArrayInt")
                    type = 6;
                else if (defines[3][1] == "VariableInt64")
                    type = 7;
                else if (defines[3][1] == "VariableArrayInt64")
                    type = 8;
                else if (defines[3][1] == "VariableArrayByte")
                    type = 9;

                int[] anKey;
                anKey = new int[1];
                for (int i = 4; i < defines.Count; i++)
                {
                    string name = defines[i][1];
                    int nameKey = VLStateManager.hash(name);
                    anKey[0] = nameKey;

                    switch (type)
                    {
                        case 1:
                        case 2:
                        case 4:
                        case 5:
                        case 6:
                        case 7:
                        case 8:
                        case 9:
                            m_dstExtVariable.add_int(nameKey, 0, anKey);
                            m_dstExtVariable.add_string(nameKey, 1, name);
                            break;
                        case 3:
                            m_dstExtEvent.add_int(nameKey, 0, anKey);
                            m_dstExtEvent.add_string(nameKey, 1, name);
                            break;
                    }
                }
            }

            return true;
        }

        public bool link_active_set(int _key)
        {
            BaseFile file1 = BaseFile.create();
            file1.seperator_set("\t");

            if (file1.open_file_write(m_strPathState + "active.ini"))
            {
                file1.asc_write_string(access_cnt.ToString());
                file1.asc_write_string(m_strName);
                file1.asc_write_string(_key.ToString());
                file1.asc_write_line();
                access_cnt++;
            }

            file1.delete();
            return true;
        }


        public static int VScriptStateSelect_nIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue stEvent = new StateDStructureValue(_pEvent);
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
            MonoBehaviour uxViewState = null;

            int[] anPos;
            anPos = stEvent.get_int_arry("BasePos_anV");

            if (dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                int key = ((UxViewStateContent)uxViewState).select_link(new Vector3(anPos[0], anPos[1], 0));
                if (key != 0)
                    return 1;
            }

            return 0;
        }

        public static int VScriptStateDelete_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
            MonoBehaviour uxViewState = null;

            if (dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                StateDStructureValue stVariable = stBase.state_variable_get();
                string strPath = "";
                stVariable.get_string("TempString_strV", ref strPath);

                if (((UxViewStateContent)uxViewState).state_delete(strPath))
                    return 1;
            }

            return 0;
        }

        public static int VScriptLinkDelete_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
            MonoBehaviour uxViewState = null;

            int hash = 0;
            if (!stBase.get_int("VScriptLinkDelete_nF", ref hash))
                return 0;

            if (dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                StateDStructureValue stVariable = stBase.state_variable_get();
                int key = 0;
                stVariable.get_int(hash, ref key);

                if (((UxViewStateContent)uxViewState).delete_link(key))
                    return 1;
            }

            return 0;
        }

        public static int VScriptStateNamePaste_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);

            int hash = 0;
            if (!stBase.get_int("VScriptStateNamePaste_varF", ref hash))
                return 0;

            string name = GUIUtility.systemCopyBuffer;
            StateDStructureValue stVariable = stBase.state_variable_get();
            if (!stVariable.set_string(hash, name))
                return 0;

            return 1;
        }

        public static int VScriptStateTreeCopy_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);

            int hash = 0;
            if (!stBase.get_int("VScriptStateNameCopy_F", ref hash))
                return 0;
            
            string name = "";
            StateDStructureValue stVariable = stBase.state_variable_get();
            if (!stVariable.get_string(hash, ref name))
                return 0;

            
            return 1;
        }

        public static int VScriptStateNameCopy_F(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);

            int hash = 0;
            if (!stBase.get_int("VScriptStateNameCopy_F", ref hash))
                return 0;

            string name = "";
            StateDStructureValue stVariable = stBase.state_variable_get();
            if (!stVariable.get_string(hash, ref name))
                return 0;

            GUIUtility.systemCopyBuffer = name;
            //name.CopyToClipboard();
            return 1;
        }

        public static int VScriptLinkSelectOnEditor_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
            MonoBehaviour uxViewState = null;

            int hash = 0;
            if (!stBase.get_int("VScriptLinkSelectOnEditor_nF", ref hash))
                return 0;

            if (dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                int key = 0;
                StateDStructureValue stVariable = stBase.state_variable_get();
                if (!stVariable.get_int(hash, ref key))
                    return 0;

                if (((UxViewStateContent)uxViewState).link_active_set(key))
                    return 1;

            }

            return 0;
        }

        public static int VScriptLinkSelect_nIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue stEvent = new StateDStructureValue(_pEvent);
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
            MonoBehaviour uxViewState = null;

            int hash = 0;
            if (!stBase.get_int("VScriptLinkSelect_nIf", ref hash))
                return 0;

            int[] anPos;
            anPos = stEvent.get_int_arry("BasePos_anV");

            Camera cam = Camera.main;
            string camera_str = "";
            stBase.get_string("VScriptStateMoveCamera_strV", ref camera_str); // 1: move tree, 2: move singe

            if (dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                Vector3 pos =
                    ((UxViewStateContent)uxViewState).ScreenToWorld(camera_str, new Vector3(anPos[0], anPos[1], 0));
                pos.z = 0;
                int key = ((UxViewStateContent)uxViewState).select_link(pos);
                if (key != 0)
                {
                    StateDStructureValue stVariable = stBase.state_variable_get();
                    if (!stVariable.set_variable(hash, key))
                        return 0;

                    return 1;
                }
            }

            return 0;
        }


        public static int VScriptStateMoveLock_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue stEvent = new StateDStructureValue(_pEvent);
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
            MonoBehaviour uxViewState = null;

            int typeMove_n = 0;
            if (!stBase.get_int("VScriptStateMoveLock_nF", ref typeMove_n)) // 1: move tree, 2: move singe
                return 0;

            Camera cam = Camera.main;
            string camera_str = "";
            if (stBase.get_string("VScriptStateMoveCamera_strV", ref camera_str)) // 1: move tree, 2: move singe
            {
                GameObject cam_go = GameObject.Find(camera_str);
                cam = cam_go.GetComponent<Camera>();
            }

            StateDStructureValue stVariable = stBase.state_variable_get();
            string strState = "";
            if (!stVariable.get_string("TempString_strV", ref strState))
                return 0;

            int[] anPos;
            anPos = stEvent.get_int_arry("BasePos_anV");

            if (anPos.Length < 2)
                return 0;

            if (dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                Vector3 pos_mouse_v3 = cam.ScreenToWorldPoint(new Vector3(anPos[0], anPos[1], 0));
                pos_mouse_v3.z = 0;
                ((UxViewStateContent)uxViewState).StateMove(strState, pos_mouse_v3, 3);
                return 1;
            }

            return 0;
        }


        public static int VScriptStateMove_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue stEvent = new StateDStructureValue(_pEvent);
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
            MonoBehaviour uxViewState = null;

            int typeMove_n = 0;
            if (!stBase.get_int("VScriptStateMove_nF", ref typeMove_n)) // 1: move tree, 2: move singe
                return 0;

            Camera cam = Camera.main;
            string camera_str = "";
            stBase.get_string("VScriptStateMoveCamera_strV", ref camera_str); // 1: move tree, 2: move singe

            StateDStructureValue stVariable = stBase.state_variable_get();
            string strState = "";
            if (!stVariable.get_string("TempString_strV", ref strState))
                return 0;

            int[] anPos;
            anPos = stEvent.get_int_arry("BasePos_anV");

            if (anPos.Length < 2)
                return 0;

            if (dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                Vector3 pos_mouse_v3 =
                    ((UxViewStateContent)uxViewState).ScreenToWorld(camera_str, new Vector3(anPos[0], anPos[1], 0));
                pos_mouse_v3.z = 0;
                ((UxViewStateContent)uxViewState).StateMove(strState, pos_mouse_v3, typeMove_n);
                return 1;
            }

            return 0;
        }


        public static int VScriptStateUpdate_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
            MonoBehaviour uxViewState = null;

            if (dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                ((UxViewStateContent)uxViewState).scene_update();
                return 1;
            }

            return 0;
        }

        public static int VScriptStateAppend_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue stEvent = new StateDStructureValue(_pEvent);
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
            StateDStructureValue variable_state = stBase.state_variable_get();
            MonoBehaviour uxViewState = null;

            int hash = 0;
            if (!stBase.get_int("VScriptStateAppend_varF", ref hash))
                return 0;

            int[] anPos;
            anPos = stEvent.get_int_arry("BasePos_anV");

            string state_name = "";
            if (!variable_state.get_string(hash, ref state_name))
                return 0;

            Camera cam = Camera.main;
            string camera_str = "";
            if (stBase.get_string("VScriptStateMoveCamera_strV", ref camera_str)) // 1: move tree, 2: move singe
            {
                GameObject cam_go = GameObject.Find(camera_str);
                cam = cam_go.GetComponent<Camera>();
            }

            if (dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                Vector3 pos_mouse_v3 = cam.ScreenToWorldPoint(new Vector3(anPos[0], anPos[1], 0));
                if (((UxViewStateContent)uxViewState).state_add(state_name, pos_mouse_v3))
                    return 1;
            }

            return 0;
        }


        public static int VScriptStateSizeUpdate_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
            MonoBehaviour uxViewState = null;

            if (dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                ((UxViewStateContent)uxViewState).scroll_size_update();
                return 1;
            }

            return 0;
        }

        public static int VScriptStateActiveRelease_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
            MonoBehaviour uxViewState = null;

            if (dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                StateDStructureValue stVariable = stBase.state_variable_get();
                string strPath = "";
                stVariable.get_string("TempString_strV", ref strPath);

                if (((UxViewStateContent)uxViewState).StateLeafHide(strPath))
                    return 1;
            }

            return 0;
        }

        public static int VScriptStateActiveAllRelease_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
            MonoBehaviour uxViewState = null;

            if (dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                if (((UxViewStateContent)uxViewState).StateLeafHide())
                    return 1;
            }

            return 0;
        }

        public static int VScriptStateActiveSet_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
            MonoBehaviour uxViewState = null;

            if (dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                StateDStructureValue stVariable = stBase.state_variable_get();
                string strPath = "";
                stVariable.get_string("TempString_strV", ref strPath);

                if (((UxViewStateContent)uxViewState).StateLeafShow(strPath))
                    return 1;
                else
                    ((UxViewStateContent)uxViewState).StateLeafHide(strPath);
            }

            return 0;
        }


        public static int VScriptStateSavePos_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
            MonoBehaviour uxViewState = null;

            if (dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                ((UxViewStateContent)uxViewState).StateSavePos();
                return 1;
            }

            return 0;
        }


        public static int VScriptMainLoad_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
            MonoBehaviour uxViewState = null;
            if (dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                StateDStructureValue stVariable = stBase.state_variable_get();
                string strPath = "";
                stVariable.get_string("TempString_strV", ref strPath);

                if (!((UxViewStateContent)uxViewState).Load(strPath))
                    return 0;
                return 1;
            }

            return 0;
        }

        public static int VScriptMainReload_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
            MonoBehaviour uxViewState = null;
            if (dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                if (!((UxViewStateContent)uxViewState).reload())
                    return 0;
                return 1;
            }

            return 0;
        }

        public void link_update(int _key)
        {
            int key_state = state_get_by_link(_key);
            m_state_updated.Add(key_state);
            scene_link_remake();
        }

        bool state_link_make(string _from, string _to, string _differname)
        {
            int from, to;
            from = VLStateManager.hash(_from);
            to = VLStateManager.hash(_to);

            int[] link_an = null;
            int[] linkNew_an = null;
            int cnt = 0;
            if (m_dstMain.get_int_index(from, m_index_link, ref link_an))
                cnt = link_an.Length;
            linkNew_an = new int[cnt + 1];
            for (int i = 0; i < cnt; i++)
                linkNew_an[i] = link_an[i];
            string keyMade = _to + _differname;
            int linknew = m_dstLink.create_key_link(keyMade);
            linkNew_an[cnt] = linknew;

            int[] key_an = new int[1];
            m_dstMain.set_int(from, m_index_link, linkNew_an);
            m_state_updated.Add(from);

            key_an[0] = linknew;
            m_dstLink.set_int(linknew, m_nKeyIndex, key_an);
            m_dstLink.add_string(linknew, m_index_name, _to);

            if (state_system_is(to))
                key_an[0] = to;
            else
                key_an[0] = VLStateManager.hash("state_change");
            m_dstLink.set_int(linknew, m_index_link_type, key_an);
            ;
            scene_link_remake();

            return true;
        }

        public static int VScriptStateLinkMake_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
            int hash = 0;
            if (!stBase.get_int("VScriptStateLinkMake_nF", ref hash))
                return 0;

            StateDStructureValue stVariable = stBase.state_variable_get();
            string state_link = "", state_from = "";
            stVariable.get_string("TempString_strV", ref state_link);
            stVariable.get_string(hash, ref state_from);

            MonoBehaviour uxViewState = null;
            if (dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                string username = "";
                dstGlobal.get_string("UserName_strV", ref username);
                if (!((UxViewStateContent)uxViewState).state_link_make(state_from, state_link, username))
                    return 0;
                return 1;
            }

            return 0;
        }

        public static int VScriptStateSystemAdd_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
            string state_link = "";
            if (!stBase.get_string("VScriptStateSystemAdd_strF", ref state_link))
                return 0;

            StateDStructureValue stVariable = stBase.state_variable_get();
            string state_from = "";
            stVariable.get_string("TempString_strV", ref state_from);

            MonoBehaviour uxViewState = null;
            if (dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                string username = "";
                dstGlobal.get_string("UserName_strV", ref username);
                if (!((UxViewStateContent)uxViewState).state_link_make(state_from, state_link, username))
                    return 0;
                return 1;
            }

            return 0;
        }

        public static int VScriptLinkTypeSet_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
            string state_link = "";
            if (!stBase.get_string("VScriptLinkTypeSet_strF", ref state_link))
                return 0;

            StateDStructureValue stVariable = stBase.state_variable_get();
            int key = 0;
            stVariable.get_int("TempInt_nV", ref key);

            MonoBehaviour uxViewState = null;
            if (dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                if (!((UxViewStateContent)uxViewState).link_type_set(key, state_link))
                    return 0;
                return 1;
            }

            return 0;
        }

        public bool state_count_set(int _key, int _count, int _serial)
        {
            VScriptState btn = (VScriptState)m_hashStateButtons[_key];

            if (btn == null)
                return false;

            TMP_Text[] cnt_ago = btn.go.GetComponentsInChildren<TMP_Text>();

            foreach (TMP_Text txt in cnt_ago)
            {
                if (txt.name == "Count")
                {
                    txt.text = _count.ToString();
                    return true;
                }
            }

            return false;
        }

        public bool link_count_set(int _key, int _count, int _serial)
        {
            GameObject go = null;
            foreach (VScriptState btn in m_state_active_a)
            {
                foreach (GameObject link_go in btn.sub_goa)
                {
                    VScriptLink link2 = link_go.GetComponent<VScriptLink>();

                    if (link2.m_key == _key)
                    {
                        go = link2.gameObject;
                        break;
                    }
                }

                if (go != null)
                    break;
            }

            if (go == null)
                return false;

            TMP_Text[] cnt_ago = go.GetComponentsInChildren<TMP_Text>();

            foreach (TMP_Text txt in cnt_ago)
            {
                if (txt.name == "Count")
                {
                    txt.text = _count.ToString();
                }
                else if (txt.name == "Serial")
                {
                    txt.text = _serial.ToString();
                }
            }

            return false;
        }

        public bool link2state_change(int _from, string _to)
        {
            int key_from = 0;
            foreach (VScriptState btn in m_state_active_a)
            {
                foreach (GameObject link_go in btn.sub_goa)
                {
                    VScriptLink link2 = link_go.GetComponent<VScriptLink>();

                    if (link2.m_key == _from)
                        key_from = VLStateManager.hash(btn.name);
                }
            }

            if (key_from == 0)
                return false;
            
            m_dstLink.set_string(_from, m_index_name, _to);
            m_state_updated.Add(key_from);

            return true;
        }
        public bool link2state_move(int _from, int _to_key)
        {
            int key_from = 0;
            foreach (VScriptState btn in m_state_active_a)
            {
                foreach (GameObject link_go in btn.sub_goa)
                {
                    VScriptLink link2 = link_go.GetComponent<VScriptLink>();

                    if (link2.m_key == _from)
                        key_from = VLStateManager.hash(btn.name);
                }
            }

            if (key_from == 0)
                return false;

            int LinkIndex_n = StateDStructure.get_index(VLStateManager.hash("arrayLink"));
            int[] link_from_a = null;
            int[] link_to_a = null;
            //m_dstLink
            if (!m_dstMain.get_int_index(key_from, LinkIndex_n, ref link_from_a))
                return false;

            int[] link2_from_a = new int[link_from_a.Length - 1];
            int k = 0;
            for (int i = 0; i < link_from_a.Length; i++)
                if (link_from_a[i] != _from)
                    link2_from_a[k++] = link_from_a[i];
            m_dstMain.set_int(key_from, LinkIndex_n, link2_from_a);
            m_state_updated.Add(key_from);

            int[] link2_to_a = null; // new int[link_to_a.Length + 1];
            if (!m_dstMain.get_int_index(_to_key, LinkIndex_n, ref link_to_a))
                link2_to_a = new int[1];
            else
                link2_to_a = new int[link_to_a.Length + 1];

            k = 0;
            for (int i = 0; i < link2_to_a.Length - 1; i++)
                link2_to_a[k++] = link_to_a[i];
            link2_to_a[k] = _from;

            m_dstMain.set_int(_to_key, LinkIndex_n, link2_to_a);
            m_state_updated.Add(_to_key);
            return true;
        }

        public bool link_copy(ref List<List<string>> _column_astr, int _key_link)
        {
            return m_dstLink.string_get(_key_link, ref _column_astr);
        }

        public bool link_state_paste(List<List<string>> _column_astr, int _key_state)
        {
            int key_to = _key_state;

            string name_link = _column_astr[1][1]; // link name get
            name_link += m_user_name;

            int key_new = m_dstLink.create_key_link(name_link);
            _column_astr[0][1] = key_new.ToString();
            if (!m_dstLink.string_add(key_new, _column_astr)) // regist new link key.
                return false;

            // now insert new link to a state.
            int LinkIndex_n = StateDStructure.get_index(VLStateManager.hash("arrayLink"));
            int[] link_to_a = null;
            if (!m_dstMain.get_int_index(key_to, LinkIndex_n, ref link_to_a))
                return false;

            int[] link2_to_a = new int[link_to_a.Length + 1];
            int k = 0;
            link2_to_a[k++] = key_new;
            for (int i = 0; i < link_to_a.Length; i++)
            {
                link2_to_a[k++] = link_to_a[i];
            }

            m_dstMain.set_int(key_to, LinkIndex_n, link2_to_a);
            m_state_updated.Add(key_to);

            return true;
        }

        public bool link_paste(List<List<string>> _column_astr, int _key_link)
        {
            int key_to = 0;
            foreach (VScriptState btn in m_state_active_a)
            {
                foreach (GameObject link_go in btn.sub_goa)
                {
                    VScriptLink link2 = link_go.GetComponent<VScriptLink>();

                    if (link2.m_key == _key_link)
                        key_to = VLStateManager.hash(btn.name);
                }
            }

            if (key_to == 0)
                return false;

            string name_link = _column_astr[1][1]; // link name get
            name_link += m_user_name;

            int key_new = m_dstLink.create_key_link(name_link);
            _column_astr[0][1] = key_new.ToString();
            if (!m_dstLink.string_add(key_new, _column_astr)) // regist new link key.
                return false;

            // now insert new link to a state.
            int LinkIndex_n = StateDStructure.get_index(VLStateManager.hash("arrayLink"));
            int[] link_to_a = null;
            if (!m_dstMain.get_int_index(key_to, LinkIndex_n, ref link_to_a))
                return false;

            int[] link2_to_a = new int[link_to_a.Length + 1];
            int k = 0;
            for (int i = 0; i < link_to_a.Length; i++)
            {
                if (link_to_a[i] == _key_link)
                    link2_to_a[k++] = key_new;
                link2_to_a[k++] = link_to_a[i];
            }

            m_dstMain.set_int(key_to, LinkIndex_n, link2_to_a);
            m_state_updated.Add(key_to);

            return true;
        }

        public bool link_move(int _from, int _to)
        {
            int key_from = 0, key_to = 0;
            foreach (VScriptState btn in m_state_active_a)
            {
                foreach (GameObject link_go in btn.sub_goa)
                {
                    VScriptLink link2 = link_go.GetComponent<VScriptLink>();

                    if (link2.m_key == _from)
                        key_from = VLStateManager.hash(btn.name);

                    if (link2.m_key == _to)
                        key_to = VLStateManager.hash(btn.name);
                }
            }

            if (key_from == 0 || key_to == 0)
                return false;

            int LinkIndex_n = StateDStructure.get_index(VLStateManager.hash("arrayLink"));
            int[] link_from_a = null;
            int[] link_to_a = null;
            //m_dstLink
            if (!m_dstMain.get_int_index(key_from, LinkIndex_n, ref link_from_a))
                return false;

            int[] link2_from_a = new int[link_from_a.Length - 1];
            int k = 0;
            for (int i = 0; i < link_from_a.Length; i++)
                if (link_from_a[i] != _from)
                    link2_from_a[k++] = link_from_a[i];
            m_dstMain.set_int(key_from, LinkIndex_n, link2_from_a);
            m_state_updated.Add(key_from);

            if (!m_dstMain.get_int_index(key_to, LinkIndex_n, ref link_to_a))
                return false;

            int[] link2_to_a = new int[link_to_a.Length + 1];
            k = 0;
            for (int i = 0; i < link_to_a.Length; i++)
            {
                if (link_to_a[i] == _to)
                    link2_to_a[k++] = _from;
                link2_to_a[k++] = link_to_a[i];
            }

            m_dstMain.set_int(key_to, LinkIndex_n, link2_to_a);
            m_state_updated.Add(key_to);
            return true;
        }

        public bool marker_link_set(int _key_link, int _index, bool _on)
        {
            foreach (VScriptState btn in m_state_active_a)
            {
                foreach (GameObject link_go in btn.sub_goa)
                {
                    VScriptLink link2 = link_go.GetComponent<VScriptLink>();

                    if (link2.m_key == _key_link)
                    {
                        int serial = 0;
                        int cnt = VScriptLogHistory.ms_instance.log_link_cnt(_key_link, ref serial);
                        link2.marker_toggle(_index, _on);
                        link2.m_count.text = cnt.ToString();
                        link2.m_serial.text = serial.ToString();
                        return true;
                    }
                }
            }

            return false;
        }

        public bool marker_set(int _key, int _index, bool _on, List<string> _column_a)
        {
            VScriptState goBtn = (VScriptState)m_hashStateButtons[_key];

            if (goBtn == null)
                return false;

            if (!_on)
            {
                foreach (GameObject link_go in goBtn.sub_goa)
                {
                    VScriptLink link2 = link_go.GetComponent<VScriptLink>();
                    link2.marker_toggle(_index, _on);
                }
            }

            VScriptLink link = goBtn.go.GetComponent<VScriptLink>();

            if (link == null)
                return false;

            return link.marker_toggle(_index, _on);
        }

        public static int VScriptStateSave_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();

            MonoBehaviour uxViewState = null;
            if (dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                ((UxViewStateContent)uxViewState).state_save();
                return 1;
            }

            return 0;
        }

        static bool marker_link_set(IntPtr _pBase, IntPtr _pEvent, int _index, bool _on)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue stEvent = new StateDStructureValue(_pEvent);
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();

            int key = 0;
            if (!stBase.param_get(_pEvent, IntPtr.Zero, 0, ref key))
                return false;
            int link = 0;
            stBase.param_get(_pEvent, IntPtr.Zero, 1, ref link);

            if (link == 0)
                return false;

            MonoBehaviour uxViewState = null;
            if (dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                if (!((UxViewStateContent)uxViewState).marker_link_set(link, _index, _on))
                    return false;
                return true;
            }

            return false;
        }

        static bool marker_set(IntPtr _pBase, IntPtr _pEvent, int _index, bool _on)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue stEvent = new StateDStructureValue(_pEvent);
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();

            int key = 0;
            if (!stBase.param_get(_pEvent, IntPtr.Zero, 0, ref key))
                return false;
            int link = 0;
            stBase.param_get(_pEvent, IntPtr.Zero, 1, ref link);

            MonoBehaviour uxViewState = null;
            if (dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                if (!((UxViewStateContent)uxViewState).marker_set(key, _index, _on, null))
                    return false;
                return true;
            }

            return false;
        }

        public static int VScriptStateMarkOff_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);

            int index = 0;
            if (!stBase.get_int("VScriptStateMarkOff_nF", ref index))
                return 0;

            if (marker_set(_pBase, _pEvent, index, false))
            {
                //marker_link_set(_pBase, _pEvent, 1, true);
                return 1;
            }

            return 0;
        }

        public static int VScriptLinkMarkOn_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);

            int index = 0;
            if (!stBase.get_int("VScriptLinkMarkOn_nF", ref index))
                return 0;

            if (marker_link_set(_pBase, _pEvent, index, true))
                return 1;
            return 0;
        }

        public static int VScriptStateMarkOn_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);

            int index = 0;
            if (!stBase.get_int("VScriptStateMarkOn_nF", ref index))
                return 0;

            if (marker_set(_pBase, _pEvent, index, true))
            {
                marker_link_set(_pBase, _pEvent, 1, true);
                return 1;
            }

            return 0;
        }

        public static int UxObjectListPopup_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue stEvt = new StateDStructureValue(_pEvent);
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();

            StateDStructureValue variable_state = stBase.state_variable_get();

            int hash = 0;
            if (!stBase.get_int("UxObjectListPopup_varF", ref hash))
                return 0;

            int key_link = 0;
            if (!variable_state.get_int(hash, ref key_link))
                return 0;

            MonoBehaviour uxViewState = null;
            if (!dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
                return 0;

            VScriptLink link_leaf = ((UxViewStateContent)uxViewState).linksub_get(key_link);
            if (link_leaf == null)
                return 0;

            string camera_str = "";
            stBase.get_string("VScriptStateMoveCamera_strV", ref camera_str); // 1: move tree, 2: move singe

            Vector3 pos = ((UxViewStateContent)uxViewState).ScreenToWorld(camera_str, stBase.mouse_pos_get(stEvt));
            if (link_leaf.list_make(pos))
                return 1;

            return 0;
        }

        public static int UxObjectListClear_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();

            StateDStructureValue variable_state = stBase.state_variable_get();

            int hash = 0;
            if (!stBase.get_int("UxObjectListClear_varF", ref hash))
                return 0;

            int key_link = 0;
            if (!variable_state.get_int(hash, ref key_link))
                return 0;

            MonoBehaviour uxViewState = null;
            if (!dstGlobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
                return 0;

            VScriptLink link_leaf = ((UxViewStateContent)uxViewState).linksub_get(key_link);
            if (link_leaf == null)
                return 0;
            if (link_leaf.list_clear())
                return 1;
            return 0;
        }


        public static int UxObjectMove_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue dsvbase = new StateDStructureValue(_pBase);
            StateDStructureValue stEvt = new StateDStructureValue(_pEvent);
            StateDStructureValue dsvglobal = VLStateManager.variable_global_get();

            int hash = 0;
            if (!dsvbase.get_int("UxObjectMove_nF", ref hash))
                return 0;

            StateDStructureValue dsvstate = dsvbase.state_variable_get();
            int link = 0;
            if (!dsvstate.get_int(hash, ref link))
                return 0;

            MonoBehaviour uxViewState = null;
            if (!dsvglobal.get_ptr(sm_monoBehaviour_str, ref uxViewState))
                return 0;

            string camera_str = "";
            dsvbase.get_string("VScriptStateMoveCamera_strV", ref camera_str); // 1: move tree, 2: move singe

            Vector3 pos = ((UxViewStateContent)uxViewState).ScreenToWorld(camera_str, dsvbase.mouse_pos_get(stEvt));
            VScriptLink vsLink = ((UxViewStateContent)uxViewState).linksub_get(link);
            vsLink.move_a(pos);

            return 1;
        }

        public static int VScriptStateCountSet_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue base_p = new StateDStructureValue(_pBase);
            StateDStructureValue variable_state = base_p.state_variable_get();
            StateDStructureValue global = VLStateManager.variable_global_get();

            int hash = 0;
            if (!base_p.get_int("VScriptStateCountSet_varF", ref hash))
                return 0;

            int key = 0;
            if (!variable_state.get_int(hash, ref key))
                return 0;

            int count = 0, serial = 0;
            if (!base_p.param_get(_pEvent, _pContext, 0, ref count))
                return 0;
            if (!base_p.param_get(_pEvent, _pContext, 1, ref serial))
                return 0;

            MonoBehaviour uxViewState = null;
            if (global.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                if (!((UxViewStateContent)uxViewState).state_count_set(key, count, serial))
                    return 0;
                return 1;
            }

            return 1;
        }


        public static int VScriptLinkCountSet_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue base_p = new StateDStructureValue(_pBase);
            StateDStructureValue variable_state = base_p.state_variable_get();
            StateDStructureValue global = VLStateManager.variable_global_get();

            int hash = 0;
            if (!base_p.get_int("VScriptLinkCountSet_varF", ref hash))
                return 0;

            int key = 0;
            if (!variable_state.get_int(hash, ref key))
                return 0;

            int count = 0, serial = 0;
            if (!base_p.param_get(_pEvent, _pContext, 0, ref count))
                return 0;
            if (!base_p.param_get(_pEvent, _pContext, 0, ref serial))
                return 0;

            MonoBehaviour uxViewState = null;
            if (global.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                if (!((UxViewStateContent)uxViewState).link_count_set(key, count, serial))
                    return 0;
                return 1;
            }

            return 1;
        }

        public static int VScriptLink2StateMove_avarF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue global = VLStateManager.variable_global_get();
            StateDStructureValue variable_state = stBase.state_variable_get();
            int[] hash_a;
            hash_a = stBase.get_int_arry("VScriptLink2StateMove_avarF");
            if (hash_a == null
                || hash_a.Length < 2)
                return 0;

            int from = 0, to = 0;
            if (!variable_state.get_int(hash_a[0], ref from))
                return 0;
            if (!variable_state.get_int(hash_a[1], ref to))
                return 0;

            MonoBehaviour uxViewState = null;
            if (global.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                if (!((UxViewStateContent)uxViewState).link2state_move(from, to))
                    return 0;
                return 1;
            }

            return 1;
        }

        public static int VScriptLink2StateChange_avarF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue global = VLStateManager.variable_global_get();
            StateDStructureValue variable_state = stBase.state_variable_get();
            int[] hash_a;
            hash_a = stBase.get_int_arry("VScriptLink2StateMove_avarF");
            if (hash_a == null
                || hash_a.Length < 2)
                return 0;

            int from = 0;
            string to = "";
            if (!variable_state.get_int(hash_a[0], ref from))
                return 0;
            if (!variable_state.get_string(hash_a[1], ref to))
                return 0;

            MonoBehaviour uxViewState = null;
            if (global.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                if (!((UxViewStateContent)uxViewState).link2state_change(from, to))
                    return 0;
                return 1;
            }

            return 1;
        }

        public static int VScriptLink2StateCopy_avarF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue global = VLStateManager.variable_global_get();
            StateDStructureValue variable_state = stBase.state_variable_get();
            int[] hash_a;
            hash_a = stBase.get_int_arry("VScriptLink2StateCopy_avarF");
            if (hash_a == null
                || hash_a.Length < 2)
                return 0;

            int from = 0, to = 0;
            if (!variable_state.get_int(hash_a[0], ref from))
                return 0;
            if (!variable_state.get_int(hash_a[1], ref to))
                return 0;

            MonoBehaviour uxViewState = null;
            if (global.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                UxViewStateContent view = (UxViewStateContent)uxViewState;
                List<List<string>> column_astr = null;
                if (!view.link_copy(ref column_astr, from))
                    return 0;
                if (!view.link_state_paste(column_astr, to))
                    return 0;

                return 1;
            }

            return 1;
        }

        public static int VScriptLinkMove_avarF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue global = VLStateManager.variable_global_get();
            StateDStructureValue variable_state = stBase.state_variable_get();
            int[] hash_a;
            hash_a = stBase.get_int_arry("VScriptLinkMove_avarF");
            if (hash_a == null
                || hash_a.Length < 2)
                return 0;

            int from = 0, to = 0;
            if (!variable_state.get_int(hash_a[0], ref from))
                return 0;
            if (!variable_state.get_int(hash_a[1], ref to))
                return 0;

            MonoBehaviour uxViewState = null;
            if (global.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                if (!((UxViewStateContent)uxViewState).link_move(from, to))
                    return 0;
                return 1;
            }

            return 1;
        }

        public static int VScriptLinkCopy_avarF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue global = VLStateManager.variable_global_get();
            StateDStructureValue variable_state = stBase.state_variable_get();
            int[] hash_a;
            hash_a = stBase.get_int_arry("VScriptLinkCopy_avarF");
            if (hash_a == null
                || hash_a.Length < 2)
                return 0;

            int from = 0, to = 0;
            if (!variable_state.get_int(hash_a[0], ref from))
                return 0;
            if (!variable_state.get_int(hash_a[1], ref to))
                return 0;

            MonoBehaviour uxViewState = null;
            if (global.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                UxViewStateContent view = (UxViewStateContent)uxViewState;
                List<List<string>> column_astr = null;
                if (!view.link_copy(ref column_astr, from))
                    return 0;
                if (!view.link_paste(column_astr, to))
                    return 0;
                return 1;
            }

            return 1;
        }

        public void screen_scale(float _scale)
        {
            Camera cam = m_camera;
            cam.orthographicSize += _scale * cam.orthographicSize;
            m_update_b = true;
            return;
        }

        public static int VScriptCameraScaleInc_fF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue global = VLStateManager.variable_global_get();
            StateDStructureValue event_dsv = new StateDStructureValue(_pEvent);

            float scale_f = 0;
            int[] mouse_pos_an = event_dsv.get_int_arry("BasePos_anV");

            scale_f = mouse_pos_an[1] / 10.0f;

            MonoBehaviour uxViewState = null;
            if (global.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                UxViewStateContent view = (UxViewStateContent)uxViewState;
                view.screen_scale(scale_f);
            }

            return 1;
        }

        public int state_column_search(int _key, string _str, string _str_value)
        {
            int cnt = 0;
            int[] Link_an = null;
            int LinkIndex_n = StateDStructure.get_index(VLStateManager.hash("arrayLink"));

            if ((_str == null || _str.Length == 0) && (_str_value == null || _str_value.Length == 0))
                return 0;


            bool search_linkadd = false;
            if (_str_value == "linkadd")
                search_linkadd = true;

            List<int> extValues = new List<int>();

            if(_str_value != null && _str_value.Length > 1) {
                int exKey = m_dstExtVariable.key_first_get();
                string value = "";
                while (exKey != 0)
                {
                    if (m_dstExtVariable.get_string(exKey, 1, ref value))
                    {
                        value = value.ToLower();
                        if (value.IndexOf(_str_value) >= 0)
                            extValues.Add(exKey);
                    }
                    exKey = m_dstExtVariable.key_next_get();
                }
                exKey = m_dstExtEvent.key_first_get();
                while (exKey != 0)
                {
                    if (m_dstExtEvent.get_string(exKey, 1, ref value))
                    {
                        value = value.ToLower();
                        if (value.IndexOf(_str_value) >= 0)
                            extValues.Add(exKey);
                    }
                    exKey = m_dstExtEvent.key_next_get();
                }
            }

            VScriptState state_active = (VScriptState)m_state_active_h[_key];
            if ( //_str != null && _str.Length > 0 && 
                m_dstMain.get_int_index(_key, LinkIndex_n, ref Link_an))
            {
                for (int j = 0; j < Link_an.Length; j++)
                {
                    VScriptLink link2 = null;
                    if (state_active != null)
                        link2 = state_active.sub_goa[j].GetComponent<VScriptLink>();

                    bool find_op = false;
                    bool find = false;
                    int count_n = m_dstLink.get_count_column(Link_an[j]);
                    int i = 0;
                    for (; i < count_n; i++)
                    {
                        string column = "";
                        int index_n = m_dstLink.get_index(Link_an[j], i);
                        if (search_linkadd)
                        {
                            int flag = m_dstLink.get_type_flag(index_n);
                            string flag_str = m_dstLink.flag_string_get(flag);

                            if (flag_str == "L_LA")
                            {
                                if (find_op)
                                {
                                    cnt++;
                                    find = true;
                                }
                            }

                            if (flag_str == "L_SS_LC")
                                find_op = true;
                        }
                        else
                        {
                            column = m_dstLink.get_type_name(index_n).ToLower();
                            int type = StateDStructure.get_type(index_n);
                            
                            if (_str == null || _str.Length == 0 || column.IndexOf(_str) >= 0)
                            {
                                if (_str_value != null && _str_value.Length > 0)
                                {
                                    string value = m_dstLink.editor_string_get(Link_an[j], i).ToLower();

                                    if (value.IndexOf(_str_value) >= 0) // only finding value is true
                                    {
                                        cnt++;
                                        find = true;
                                        break;
                                    }else if (value.Length > 0 && value.Length < 20 && type == 4 || type == 11) // type is int
                                    {
                                        int nValue = 0;
                                        List<List<string>> list_aa = BaseFile.paser_list(value, ",");
                                        if (list_aa.Count > 0)
                                        {
                                            List<string> list_a = list_aa[0];
                                            foreach (string str in list_a)
                                            {
                                                if (int.TryParse(str, out nValue))
                                                {
                                                    if (extValues.FindIndex(ex => ex == nValue) >= 0)
                                                    {
                                                        cnt++;
                                                        find = true;
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    cnt++;
                                    find = true;
                                    break;
                                }
                            }
                        } // end of else
                    } // end of for

                    if (link2 != null)
                        link2.select(find);
                }
            }

            return cnt;
        }

        public int column_search()
        {
            int cnt = 0;

            foreach (VScriptState state in m_stateButtons_ago)
            {
                VScriptLink link = state.go.GetComponent<VScriptLink>();
                if (link == null)
                    continue;

                int key = VLStateManager.hash(state.name);

                int cnt2 = state_column_search(key, m_search_column, m_search_value);
                link.select(cnt2 > 0);
                cnt += cnt2;
            }

            return cnt;
        }

        public int value_search(string _str)
        {
            m_search_value = _str.ToLower();

            return column_search();
        }

        public int column_search(string _str)
        {
            m_search_column = _str.ToLower();

            return column_search();
        }

        public int state_search(string _str)
        {
            m_search_state = _str.ToLower();
            int cnt = 0;
            foreach (VScriptState state in m_stateButtons_ago)
            {
                VScriptLink link = state.go.GetComponent<VScriptLink>();
                if (link == null)
                    continue;

                string name = state.name.ToLower();
                if (name.IndexOf(m_search_state) >= 0 && m_search_state.Length > 0)
                {
                    cnt++;
                    link.select(true);
                }
                else
                {
                    link.select(false);
                }
            }

            return cnt;
        }

        public bool link_refresh_count()
        {
            foreach (VScriptState btn in m_state_active_a)
            {
                foreach (GameObject leaf in btn.sub_goa)
                {
                    VScriptLink link = leaf.GetComponent<VScriptLink>();
                    int serial = 0;
                    link.m_count.text = VScriptLogHistory.ms_instance.log_link_cnt(link.m_key, ref serial).ToString();
                    link.m_serial.text = serial.ToString();
                }
            }

            return true;
        }

        public static int VScriptLinkRefreshCount_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue global = VLStateManager.variable_global_get();

            MonoBehaviour uxViewState = null;
            if (global.get_ptr(sm_monoBehaviour_str, ref uxViewState))
            {
                UxViewStateContent view = (UxViewStateContent)uxViewState;
                if (view.link_refresh_count())
                    return 1;
            }

            return 1;
        }

        public static int VScriptStateSearch_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue var = stBase.state_variable_get();

            int hash = 0;
            if (!stBase.get_int("VScriptStateSearch_varF", ref hash))
                return 0;

            string str = "";
            if (!var.get_string(hash, ref str))
                return 0;

            return UxViewStateContent.ms_Instance.state_search(str);
        }

        public static int VScriptColumnValueSearch_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue var = stBase.state_variable_get();

            int hash = 0;
            if (!stBase.get_int("VScriptColumnValueSearch_varF", ref hash))
                return 0;

            string str = "";
            if (!var.get_string(hash, ref str))
                return 0;

            return UxViewStateContent.ms_Instance.value_search(str);
        }

        public static int VScriptColumnSearch_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue var = stBase.state_variable_get();

            int hash = 0;
            if (!stBase.get_int("VScriptColumnSearch_varF", ref hash))
                return 0;

            string str = "";
            if (!var.get_string(hash, ref str))
                return 0;

            return UxViewStateContent.ms_Instance.column_search(str);
        }

        public static void StateFuncRegist()
        {
            VLStateManager.ProcessReg("VScriptStateSearch_varF", VScriptStateSearch_varF);
            VLStateManager.ProcessReg("VScriptColumnSearch_varF", VScriptColumnSearch_varF);
            VLStateManager.ProcessReg("VScriptColumnValueSearch_varF", VScriptColumnValueSearch_varF);

            VLStateManager.ProcessReg("VScriptLinkRefreshCount_nF", VScriptLinkRefreshCount_nF);
            VLStateManager.ProcessReg("VScriptCameraScaleInc_fF", VScriptCameraScaleInc_fF);
            VLStateManager.ProcessReg("VScriptLink2StateCopy_avarF", VScriptLink2StateCopy_avarF);
            VLStateManager.ProcessReg("VScriptLink2StateMove_avarF", VScriptLink2StateMove_avarF);
            VLStateManager.ProcessReg("VScriptLink2StateChange_avarF", VScriptLink2StateChange_avarF);
            VLStateManager.ProcessReg("VScriptLinkMove_avarF", VScriptLinkMove_avarF);
            VLStateManager.ProcessReg("VScriptLinkCopy_avarF", VScriptLinkCopy_avarF);
            VLStateManager.ProcessReg("VScriptStateCountSet_varF", VScriptStateCountSet_varF);
            VLStateManager.ProcessReg("VScriptMainReload_nF", VScriptMainReload_nF);
            VLStateManager.ProcessReg("VScriptMainLoad_nF", VScriptMainLoad_nF);
            VLStateManager.ProcessReg("VScriptStateMove_nF", VScriptStateMove_nF);
            VLStateManager.ProcessReg("VScriptStateUpdate_nF", VScriptStateUpdate_nF);
            VLStateManager.ProcessReg("VScriptStateSizeUpdate_nF", VScriptStateSizeUpdate_nF);
            VLStateManager.ProcessReg("VScriptStateSavePos_nF", VScriptStateSavePos_nF);
            VLStateManager.ProcessReg("VScriptStateSelect_nIf", VScriptStateSelect_nIf);
            VLStateManager.ProcessReg("VScriptLinkSelect_nIf", VScriptLinkSelect_nIf);
            VLStateManager.ProcessReg("VScriptStateMoveLock_nF", VScriptStateMoveLock_nF);
            VLStateManager.ProcessReg("VScriptStateActiveSet_nF", VScriptStateActiveSet_nF);
            VLStateManager.ProcessReg("VScriptStateActiveRelease_nF", VScriptStateActiveRelease_nF);
            VLStateManager.ProcessReg("VScriptStateActiveAllRelease_nF", VScriptStateActiveAllRelease_nF);
            VLStateManager.ProcessReg("VScriptStateAppend_varF", VScriptStateAppend_varF);
            VLStateManager.ProcessReg("VScriptStateDelete_nF", VScriptStateDelete_nF);
            VLStateManager.ProcessReg("VScriptStateLinkMake_nF", VScriptStateLinkMake_nF);
            VLStateManager.ProcessReg("VScriptLinkDelete_nF", VScriptLinkDelete_nF);
            VLStateManager.ProcessReg("VScriptStateSystemAdd_strF", VScriptStateSystemAdd_strF);
            VLStateManager.ProcessReg("VScriptLinkTypeSet_strF", VScriptLinkTypeSet_strF);
            VLStateManager.ProcessReg("VScriptStateSave_nF", VScriptStateSave_nF);
            VLStateManager.ProcessReg("VScriptStateMarkOn_nF", VScriptStateMarkOn_nF);
            VLStateManager.ProcessReg("VScriptLinkMarkOn_nF", VScriptLinkMarkOn_nF);
            VLStateManager.ProcessReg("VScriptStateMarkOff_nF", VScriptStateMarkOff_nF);
            VLStateManager.ProcessReg("VScriptLinkSelectOnEditor_nF", VScriptLinkSelectOnEditor_nF);
            VLStateManager.ProcessReg("VScriptStateNameCopy_F", VScriptStateNameCopy_F);
            VLStateManager.ProcessReg("VScriptStateNamePaste_varF", VScriptStateNamePaste_varF);

            VLStateManager.ProcessReg("UxObjectListPopup_varF", UxObjectListPopup_varF);
            VLStateManager.ProcessReg("UxObjectListClear_varF", UxObjectListClear_varF);
            VLStateManager.ProcessReg("UxObjectMove_nF", UxObjectMove_nF);

        }
    }
}