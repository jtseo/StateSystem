using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using UnityEngine.UI;
using TMPro;
using System.IO;
using UnityEngine.EventSystems;

namespace StateSystem
{
    public class UxControlScroll : ScrollRect
    {
        public bool m_scrollBlock_b = false;

        public bool dragBlockSet(bool _block)
        {
            m_scrollBlock_b = _block;
            return _block;
        }

        public override void OnBeginDrag(PointerEventData eventData)
        {
            base.OnBeginDrag(eventData);
        }

        public override void OnDrag(PointerEventData eventData)
        {
            if (!m_scrollBlock_b)
                base.OnDrag(eventData);
        }

        public override void OnEndDrag(PointerEventData eventData)
        {
            base.OnEndDrag(eventData);
        }

        public static int UxComponentScrollBlock_astrF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
        {
            StateDStructureValue stBase = new StateDStructureValue(_pBase);
            StateDStructureValue variable_state = stBase.state_variable_get();

            List<string> str_a = null;
            if (!stBase.get_strings("UxComponentScrollBlock_astrF", ref str_a))
                return 0;

            if (str_a.Count < 2)
                return 0;

            GameObject go = (GameObject)GameObject.Find(str_a[0]);
            if (go == null)
                return 0;
            UxControlScroll scroll = go.GetComponent<UxControlScroll>();
            if (scroll == null)
                return 0;

            if (str_a[1].ToLower() == "enable")
                scroll.dragBlockSet(true);
            else
                scroll.dragBlockSet(false);
            return 1;
        }

        public static void StateFuncRegist()
        {
            VLStateManager.ProcessReg("UxComponentScrollBlock_astrF", UxComponentScrollBlock_astrF,
                "/VScriptEditor/Assets/Scripts/UxControlScroll.cs", 0);
        }
    }
}