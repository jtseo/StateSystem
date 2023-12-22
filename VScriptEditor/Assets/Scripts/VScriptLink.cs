using System.Collections;
using System.Collections.Generic;
using UnityEngine.UI;
using System;
using UnityEngine;
using TMPro;

namespace StateSystem
{
    public class VScriptLink : MonoBehaviour
    {
        // Start is called before the first frame update
        public int m_key;
        public float m_alpha = 0.5f;
        public string m_name;
        public GameObject m_origin_go;
        List<GameObject> m_list_a;
        public GameObject[] m_marker;
        public TMP_Text m_count;
        public TMP_Text m_serial;
        public GameObject m_marker_select;

        public List<string> m_column_a;

        bool m_select = false;
        static bool ms_first = true;
        int last_index = -1;

        public void Awake()
        {

        }

        void Start()
        {
            if (ms_first)
            {
                ms_first = false;
            }

            m_list_a = new List<GameObject>();
            for (int i = 0; i < m_marker.Length; i++)
            {
                if (i != last_index)
                    m_marker[i].SetActive(false);
            }

            if (m_marker_select != null)
                m_marker_select.SetActive(m_select);
        }


        void OnDestroy()
        {
            list_clear();
        }

        public void select(bool _select)
        {
            m_select = _select;
            if (m_marker_select != null)
                m_marker_select.SetActive(_select);
        }

        public bool select_get()
        {
            return m_select;
        }

        public bool marker_toggle(int _index, bool _on, List<string> _column_a)
        {
            m_column_a = _column_a;
            list_update();
            return marker_toggle(_index, _on);
        }

        public void marker_clear()
        {
            foreach (GameObject go in m_marker)
                go.SetActive(false);
        }

        public bool marker_toggle(int _index, bool _on)
        {
            for (int i = 0; i < m_marker.Length; i++)
            {
                Image img = m_marker[i].GetComponent<Image>();
                Color col = img.color;
                col.a = m_alpha;
                if (i == _index)
                {
                    m_marker[i].SetActive(_on);
                    if (_on)
                    {
                        col.a = 1.0f;
                        last_index = _index;
                    }
                }
                else
                {
                    m_marker[i].SetActive(false);
                }

                if (!_on && last_index >= 0 && last_index < m_marker.Length
                    && _index >= 0)
                    m_marker[last_index].SetActive(true);
                img.color = col;
            }

            return true;
        }

        // Update is called once per frame
        void Update()
        {

        }

        int m_logger_index = -1;

        public void logger_index_set(int _index)
        {
            m_logger_index = _index;
        }

        public int logger_index_get()
        {
            return m_logger_index;
        }

        public bool list_update()
        {
            if (m_list_a != null)
            {
                for (int i = 0; i < m_list_a.Count; i++)
                {
                    TMP_Text txt = m_list_a[i].GetComponentInChildren<TMP_Text>();
                    if (i < m_column_a.Count)
                        txt.text = m_column_a[i];
                }
            }

            return true;
        }

        public bool list_make(Vector3 _pos_v3)
        {
            list_clear();
            _pos_v3.x += 100;
            for (int i = 0; i < m_column_a.Count; i++)
            {
                GameObject go = GameObject.Instantiate(m_origin_go);
                go.transform.SetParent(m_origin_go.transform.parent, false);
                _pos_v3.y = -32 * i;
                go.transform.localPosition = _pos_v3;
                TMP_Text txt = go.GetComponentInChildren<TMP_Text>();
                txt.text = m_column_a[i];

                VScriptLink link = go.GetComponent<VScriptLink>();
                if (link != null)
                    link.m_key = m_key;

                m_list_a.Add(go);
            }

            return true;
        }

        public bool list_clear()
        {
            if (m_list_a == null)
                return true;

            for (int i = 0; i < m_list_a.Count; i++)
            {
                GameObject.Destroy(m_list_a[i]);
            }

            m_list_a.Clear();
            return true;
        }

        public bool move_a(Vector3 _move)
        {
            for (int i = 0; i < m_list_a.Count; i++)
            {
                _move.y += 32;
                m_list_a[i].transform.localPosition = _move;
            }

            return true;
        }

        public void name_set(string _name)
        {
            m_name = _name;
            int key;
            if (int.TryParse(m_name, out key))
                m_key = key;
            else
                m_key = VLStateManager.hash(_name);
        }

        public static int UxComponentMarkerSet_astrF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue base_p = new StateDStructureValue(_pBase);

            List<string> param_stra = null;
            if (!base_p.get_strings("UxComponentMarkerSet_astrF", ref param_stra))
                return 0;

            if (param_stra.Count < 3)
                return 0;

            GameObject go = GameObject.Find(param_stra[0]);
            if (go == null)
                return 0;

            VScriptLink link = go.GetComponent<VScriptLink>();

            int index = int.Parse(param_stra[1]);
            if (link == null)
                return 0;

            if (param_stra[2].ToLower() == "on")
                link.marker_toggle(index, true);
            else
                link.marker_toggle(index, false);
            return 1;
        }

        public static void StateFuncRegist()
        {
            VLStateManager.ProcessReg("UxComponentMarkerSet_astrF", UxComponentMarkerSet_astrF);
        }
    }
}