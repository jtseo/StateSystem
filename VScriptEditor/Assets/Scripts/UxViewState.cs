using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

namespace StateSystem
{
    public class UxViewState : ScrollRect
    {
        public override void OnBeginDrag(PointerEventData eventData)
        {
            base.OnBeginDrag(eventData);
        }

        public void dragManual(Vector3 _mov_v3)
        {
            Canvas.ForceUpdateCanvases();

            verticalScrollbar.value = 300f;
            Canvas.ForceUpdateCanvases();
        }

        public override void OnDrag(PointerEventData eventData)
        {
            bool bBlock = false;
            StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
            MonoBehaviour uxViewState = null;
            if (dstGlobal.get_ptr(UxViewStateContent.sm_monoBehaviour_str, ref uxViewState))
            {
                //bBlock = ((UxViewStateContent)uxViewState).DragBlock();
            }

            if (!bBlock)
                base.OnDrag(eventData);
        }

        public override void OnEndDrag(PointerEventData eventData)
        {
            base.OnEndDrag(eventData);
        }
    }
}