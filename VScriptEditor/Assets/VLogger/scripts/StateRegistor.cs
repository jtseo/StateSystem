using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace StateSystem
{
    public class StateRegistor : MonoBehaviour
    {
        // Start is called before the first frame update
        public static void StateFuncRegist()
        {
            UxViewMain.StateFuncRegist();
            UxViewStateContent.StateFuncRegist();
            UxGroupID.StateFuncRegist();
            VScriptLink.StateFuncRegist();
            UxViewColumn.StateFuncRegist();
            UxViewColumnEditor.StateFuncRegist();
            //VScriptCosmosMenu.StateFuncRegist();
            VScriptLogHistory.StateFuncRegist();
        }

        void Start()
        {

        }

        // Update is called once per frame
        void Update()
        {

        }
    }
}