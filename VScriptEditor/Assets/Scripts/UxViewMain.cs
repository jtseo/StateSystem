using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using TMPro;

namespace StateSystem
{
    public class UxViewMain : MonoBehaviour
    {
        public GameObject m_MainBtn;

        static public string sm_strMonoBehaviour;
        Canvas m_mainCanvas;

        string m_strStatePath;

        // Start is called before the first frame update
        void Start()
        {
            sm_strMonoBehaviour = "VScriptMonoBehaviourUxViewMain";

            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
            dstGlobal.set_variable(sm_strMonoBehaviour, (MonoBehaviour)this);
            m_mainCanvas = GameObject.Find("Canvas").GetComponent<Canvas>();

            m_list_mains = StateDStructure.create();
            //rt.localScale = new Vector3(1.5f, 1.5f);
        }

        // Update is called once per frame
        void Update()
        {

        }

        public string PathGet()
        {
            return m_strStatePath;
        }

        StateDStructure m_list_mains;

        public StateDStructure state_list_get()
        {
            return m_list_mains;
        }

        public bool Load(string _strPath)
        {

            GameObject goAdd, goBoard;

            goBoard = (GameObject)GameObject.Find("StateMainBoard");

            //int nLastPath = _strPath.LastIndexOf('/');

            //if (nLastPath <= 0)
            //    nLastPath = _strPath.LastIndexOf('\\');

            //if (nLastPath <= 0)
            //    return false;

            //nLastPath++;
            m_strStatePath = ""; //_strPath.Substring(0,nLastPath);

            VLStateManager.variable_global_get().set_variable_string("SystemPathRoot_strV", m_strStatePath);

            _strPath = m_strStatePath + "StateList";
            if (!m_list_mains.load(_strPath, 3))
                return false;

            List<string> state_astr = new List<string>();

            int nStep = 30, nCnt = 0;
            int nKey = m_list_mains.key_first_get();

            string strName = "";
            do
            {
                if (!m_list_mains.get_string(nKey, 1, ref strName))
                    break;

                state_astr.Add(strName);
                nKey = m_list_mains.key_next_get();
            } while (nKey != 0);

            state_astr.Sort();

            StateDStructure loader_scan = StateDStructure.create();
            for (int i = 0; i < state_astr.Count; i++)
            {
                strName = state_astr[i];

                loader_scan.load(m_strStatePath + strName, 3);
                loader_scan.load(m_strStatePath + strName + "Link", 3);
                nKey = VLStateManager.hash(strName);

                goAdd = (GameObject)Instantiate(m_MainBtn);
                goAdd.transform.SetParent(goBoard.transform.parent, false);
                goAdd.transform.position = goBoard.transform.position;
                goAdd.transform.position -= new Vector3(0, nStep * nCnt * m_mainCanvas.scaleFactor, 0);

                TMP_Text txtMep;

                txtMep = goAdd.GetComponentInChildren<TMP_Text>();
                txtMep.text = strName;
                nCnt++;
            }

            loader_scan.delete();

            GameObject ViewBox;
            ViewBox = GameObject.Find("ContentMain");
            RectTransform rt = ViewBox.GetComponent<RectTransform>();
            rt.sizeDelta = new Vector2(100, (nStep * nCnt + nStep) * m_mainCanvas.scaleFactor);

            return true;
        }

        public static void StateFuncRegist()
        {
            VLStateManager.ProcessReg("VScriptLoad_nF", VScriptLoad_nF);
        }


        public static int VScriptLoad_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
            MonoBehaviour uxViewMain = null;
            if (dstGlobal.get_ptr(sm_strMonoBehaviour, ref uxViewMain))
            {
                StateDStructureValue stVariable = stBase.state_variable_get();
                string strPath = "";
                stVariable.get_string("TempString_strV", ref strPath);

                if (!((UxViewMain)uxViewMain).Load(strPath))
                    return 0;
                return 1;
            }

            return 0;
        }

    }
}