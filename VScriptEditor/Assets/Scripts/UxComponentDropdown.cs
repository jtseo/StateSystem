using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using TMPro;

namespace StateSystem
{
    public class UxComponentDropdown : MonoBehaviour
    {
        // Start is called before the first frame update
        public GameObject m_target_go;
        public Canvas m_canvas;
        string m_name;
        int m_key;
        static List<UxComponentDropdown> ms_dropdown_a = null;
        static Hashtable ms_dropdown_h = null;

        List<GameObject> m_list_ago;
        List<string> m_list_a;
        List<int> m_checked_a;

        void Start()
        {
            m_list_ago = new List<GameObject>();
            m_list_a = new List<string>();
            m_checked_a = new List<int>();

            if (ms_dropdown_a == null)
                ms_dropdown_a = new List<UxComponentDropdown>();

            ms_dropdown_a.Add(this);

            m_name = m_target_go.name;
            m_key = VLStateManager.hash(m_name);

            if (ms_dropdown_h == null)
                ms_dropdown_h = new Hashtable();
            ms_dropdown_h.Add(m_key, this);
        }

        public void list_remove()
        {
            foreach (GameObject go in m_list_ago)
                GameObject.Destroy(go);
            m_list_ago.Clear();
            m_list_a.Clear();
            m_checked_a.Clear();
        }

        public void list_make(List<string> _list_a, List<int> _check_a)
        {
            list_remove();
            m_list_a = _list_a;
            m_checked_a.Clear();
            Vector3 pos = m_target_go.transform.position;

            int i = 0;
            foreach (string str in m_list_a)
            {
                // marker
                GameObject go = (GameObject)Instantiate(m_target_go);
                go.transform.SetParent(m_target_go.transform.parent);
                m_target_go.transform.localScale = new Vector3(1, 1, 1);
                go.transform.localScale = new Vector3(1, 1, 1);
                RectTransform rt = m_target_go.transform.GetComponent<RectTransform>();
                pos.y -= rt.rect.height * m_canvas.scaleFactor;
                go.transform.position = pos;

                TMP_Text txt = go.GetComponentInChildren<TMP_Text>();
                txt.text = str;
                go.SetActive(true);
                m_list_ago.Add(go);

                VScriptLink link = go.GetComponent<VScriptLink>();
                link.name_set(m_list_a[i]);
                if (_check_a != null)
                {
                    m_checked_a.Add(_check_a[i]);
                    if (_check_a[i] != 0)
                        link.marker_toggle(0, true);
                }
                else
                    m_checked_a.Add(0);

                i++;
            }
        }

        public void list_popup()
        {
            foreach (GameObject go in m_list_ago)
                go.SetActive(true);
        }

        public void list_close()
        {
            foreach (GameObject go in m_list_ago)
                go.SetActive(false);
        }

        public void list_check(int _key)
        {
            for (int i = 0; i < m_list_ago.Count; i++)
            {
                VScriptLink link = m_list_ago[i].GetComponent<VScriptLink>();
                link.m_alpha = 0;
                if (link.m_key == _key)
                {
                    if (m_checked_a[i] != 0)
                    {
                        m_checked_a[i] = 0;
                        link.marker_toggle(0, false);
                    }
                    else
                    {
                        m_checked_a[i] = 1;
                        link.marker_toggle(0, true);
                    }
                }
            }
        }

        static public UxComponentDropdown get(int _key)
        {
            return (UxComponentDropdown)ms_dropdown_h[_key];
        }

        static public UxComponentDropdown get(string _str)
        {
            int key = VLStateManager.hash(_str);
            return get(key);
        }

        static bool remove(int _key)
        {
            bool find = false;
            foreach (UxComponentDropdown drop in ms_dropdown_a)
            {
                ms_dropdown_a.Remove(drop);
                find = true;
            }

            ms_dropdown_h.Remove(_key);
            return find;
        }

        // Update is called once per frame
        void Update()
        {

        }
    }
}