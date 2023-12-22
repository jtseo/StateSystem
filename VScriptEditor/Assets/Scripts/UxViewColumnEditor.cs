using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using UnityEngine.UI;
using TMPro;

namespace StateSystem
{
    public class UxViewColumnEditor : MonoBehaviour
    {
        public GameObject m_columnboard_go;
        public GameObject m_scroll_go;
        public GameObject m_viewstate_go;
        UxControlScroll m_scroll;

        UxViewMain m_state_dst;

        //public GameObject m_line_go;
        GameObject m_board_go;
        static public string sm_monoBehaviour_str;
        Canvas m_mainCanvas;
        StateDStructure m_main_dst;

        //bool m_update_b = false;

        Hashtable m_columns_hash;
        List<GameObject> m_columns_go;

        // Start is called before the first frame update
        void Start()
        {
            m_columns_hash = new Hashtable();
            m_columns_go = new List<GameObject>();
            //m_links_ago = new List<GameObject>();
            sm_monoBehaviour_str = "VScriptMonoBehaviourUxVewColumnEditor";

            m_board_go = (GameObject)GameObject.Find("ColumnEditorBoard");

            VLStateManager.monobehaviourPointSet(sm_monoBehaviour_str, this);
            m_mainCanvas = GameObject.Find("Canvas").GetComponent<Canvas>();
            m_scroll = m_scroll_go.GetComponent<UxControlScroll>();
            m_state_dst = m_viewstate_go.GetComponent<UxViewMain>();
        }

        // Update is called once per frame
        void Update()
        {

        }

        private void Awake()
        {
        }

        public static void StateFuncRegist()
        {
            VLStateManager.ProcessReg("VScriptColumnEditorShow_nF", VScriptColumnEditorShow_nF);
        }

        public bool show_columns()
        {
            m_main_dst = m_state_dst.state_list_get();

            if (m_main_dst == null)
                return false;

            for (int i = 0; i < m_columns_go.Count; i++)
            {
                Destroy(m_columns_go[i]);
            }

            m_columns_hash.Clear();
            m_columns_go.Clear();

            int count_n = m_main_dst.get_type_count();
            RectTransform rt = (RectTransform)m_board_go.transform;
            float height_f = (rt.rect.height + 2) * m_mainCanvas.scaleFactor;

            int count2_n = 0;
            for (int i = 0; i < count_n; i++)
            {
                int flag_n = m_main_dst.get_type_flag(i);
                string name_str = m_main_dst.get_type_name(i);
                Debug.Log("process:" + name_str);
                string comment_str = m_main_dst.get_type_comment(i);
                int type_n = StateDStructure.get_type(i);

                count2_n++;
                if (flag_n == 0)
                    break;

                GameObject column_go = (GameObject)Instantiate(m_columnboard_go);
                column_go.transform.SetParent(m_board_go.transform.parent, false);
                Vector3 pos_v3 = column_go.transform.position;
                pos_v3.y -= i * height_f;
                column_go.transform.position = pos_v3;

                m_columns_go.Add(column_go);
                m_columns_hash.Add(i, column_go);

                VScriptLink link = column_go.GetComponent<VScriptLink>();
                if (link != null)
                    link.m_key = i;

                TMP_Text[] text = column_go.GetComponentsInChildren<TMP_Text>();
                //Button[] button = column_go.GetComponentsInChildren<Button>();
                //TMP_InputField[] input = column_go.GetComponentsInChildren<TMP_InputField>();

                if (text != null && text.Length > 3)
                {
                    text[0].text = flag_n.ToString();
                    text[1].text = name_str;
                    text[2].text = type_n.ToString();
                    text[3].text = comment_str;
                }
            }

            GameObject ViewBox;
            ViewBox = GameObject.Find("ColumnEditorContent");
            rt = ViewBox.GetComponent<RectTransform>();
            height_f = m_columns_go[0].transform.position.y - m_columns_go[m_columns_go.Count - 1].transform.position.y;
            height_f /= m_mainCanvas.scaleFactor;
            rt.sizeDelta = new Vector2(rt.sizeDelta.x, height_f + 200);

            return true;
        }


        public static int VScriptColumnEditorShow_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            UxViewColumnEditor columnEditor_ux = null;

            columnEditor_ux = (UxViewColumnEditor)VLStateManager.monobehaviourPointGet(sm_monoBehaviour_str);

            if (columnEditor_ux == null)
                return 0;

            if (columnEditor_ux.show_columns())
                return 1;

            return 0;
        }
    }
}