using UnityEngine;
using UnityEngine.UI;
using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using TMPro;
//using SimpleFileBrowser;
namespace StateSystem
{
	class TextOrg
	{
		public int m_nDepth;
		public int m_nIndex;
		public int m_nLast;
		public bool m_bFull = false;
	}

	public class UxCosmos : MonoBehaviour
	{

		public GameObject m_CosmosBoard;
		public GameObject prefabButton;
		public GameObject preFlower;
		public Transform ParentPanel;
		public float m_radiousMax_f = 300.0f;
		public float m_radiousMin_f = 90.0f;
		static float s_fRadius;

		GameObject m_cosmosboard;

		static public bool ms_bRender = false;

		//static bool ms_bUpdated = false;
		//, ms_bUpdatedPos = false;
		static public List<string> m_stlVMenus;
		static public List<string> m_stlVIconPath;
		static public List<string> m_stlVEvents;
		static public Vector3 m_vPos;
		static List<GameObject> m_stlVButtons;
		static string m_strParent;

		Vector3 m_v3MoveToInScreen;
		// Use this for initialization

		Vector3 m_v3Center;
		List<GameObject> m_agoLeafs;
		List<string> m_Menus_astr, m_menu_original;
		List<TextOrg> m_MenuOrgs_atxt;
		List<List<TextOrg>> m_MenuOrgsStack;

		Hashtable m_hashObjectHide;
		Canvas m_mainCanvas;

		static public int ms_nScreenBoxUp = 0, ms_nScreenBoxDown = 0;
		static public string sm_strMonoBehaviour = "VScriptVariableUxCosmos";

		void Start()
		{
			// FileBrowser.SetFilters( true
			//         , new FileBrowser.Filter( "Datas", ".data" )
			//         , new FileBrowser.Filter( "Text Files", ".txt", ".pdf" ) );
			// FileBrowser.SetDefaultFilter( ".data" );
			// FileBrowser.SetExcludedExtensions( ".lnk", ".tmp", ".zip", ".rar", ".exe" );
			// FileBrowser.AddQuickLink( "State", "../../../lib/State", null );

			m_MenuOrgsStack = new List<List<TextOrg>>();
			m_MenuOrgs_atxt = new List<TextOrg>();
			m_Menus_astr = new List<string>();
			m_menu_original = new List<string>();

			m_stlVIconPath = new List<string>();
			m_stlVMenus = new List<string>();
			m_stlVEvents = new List<string>();
			m_stlVButtons = new List<GameObject>();
			m_hashObjectHide = new Hashtable();

			StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
			dstGlobal.set_variable(sm_strMonoBehaviour, (MonoBehaviour)this);
			m_mainCanvas = GameObject.Find("Canvas").GetComponent<Canvas>();

			GameObject goParent = GameObject.Find("CosmosLeafBoard");

			m_v3Center = goParent.transform.position;
			UpdateCosmos();
		}

		// 20200325 jhm made

		public static int UxDoNotExist_nIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			UndoStack undoStack = GameObject.Find("ScriptStarter").GetComponent<UndoStack>();
			if (undoStack == null)
			{
				Debug.LogError("Can't find UndoStack component");
				return 0;
			}

			int isDoNotExist = undoStack.IsEmptyDo() == true ? 1 : 0;

			return isDoNotExist;
		}

		// 20200324 jhm made

		public static int UxDoExist_nIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			UndoStack undoStack = GameObject.Find("ScriptStarter").GetComponent<UndoStack>();
			if (undoStack == null)
			{
				Debug.LogError("Can't find UndoStack component");
				return 0;
			}

			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			int iInt = 0;
			if (!stBase.get_int("UxDoExist_nIf", ref iInt))
				return 0;

			int isDoExist = undoStack.IsEmptyDo() == true ? 0 : 1;
			if (iInt == 0)
			{
				isDoExist = undoStack.IsEmptyDo() == true ? 1 : 0;
			}

			return isDoExist;
		}


		public static int UxComponentCurToVariable_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			string strCom = "";
			if (!stBase.get_string("UxComponentCurToVariable_strF", ref strCom))
				return 0;

			StateDStructureValue stVariable = stBase.state_variable_get();

			GameObject component_go = null;
			StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
			MonoBehaviour uxCosmos = null;
			if (dstGlobal.get_ptr(sm_strMonoBehaviour, ref uxCosmos))
				component_go = ((UxCosmos)uxCosmos).ComponentGet(strCom);

			if (component_go == null)
				return 0;
			stVariable.set_variable("UxComponentCurPtr", component_go);

			return 1;
		}

		int MenuOrganizeRoot2(List<TextOrg> _MenuOrg, int _nEnd)
		{
			int nCnt = 1, nCntBefor;

			do
			{
				nCntBefor = nCnt;
				nCnt = MenuOrganizeChild(_MenuOrg, _nEnd, nCntBefor);

				if (_MenuOrg[0].m_nDepth > 20)
					break;
			} while (_MenuOrg.Count < 8);

			return nCnt;
		}

		int MenuOrganizeChild(List<TextOrg> _MenuOrg, int _nEnd, int _nOrgCnt)
		{
			string strPre = "", strNext = "";
			TextOrg txtOrg = _MenuOrg[0];
			int nLen = txtOrg.m_nDepth + 1;
			txtOrg.m_nDepth = nLen;
			int nStart = txtOrg.m_nIndex;
			int nOrgIndex = 0;
			int nLenLoc = 0;

			if (_nEnd <= nStart + 1)
				_MenuOrg[0].m_bFull = true;

			for (int i = nStart + 1; i < _nEnd; i++)
			{
				if (nOrgIndex < _MenuOrg.Count && i >= _MenuOrg[nOrgIndex].m_nIndex)
				{
					txtOrg = _MenuOrg[nOrgIndex];
					txtOrg.m_nDepth = nLen;

					if (nOrgIndex > 0 && _MenuOrg[nOrgIndex - 1].m_nIndex == i - 1)
						_MenuOrg[nOrgIndex - 1].m_bFull = true;
					nOrgIndex++;
				}

				if (m_Menus_astr[i].Length <= nLen
				    && m_Menus_astr[txtOrg.m_nIndex].Length <= nLen)
					return _nOrgCnt;
				nLenLoc = nLen;
				if (nLenLoc > m_Menus_astr[txtOrg.m_nIndex].Length)
					nLenLoc = m_Menus_astr[txtOrg.m_nIndex].Length;
				strPre = m_Menus_astr[txtOrg.m_nIndex].Substring(0, nLenLoc);
				nLenLoc = nLen;
				if (nLenLoc > m_Menus_astr[i].Length)
					nLenLoc = m_Menus_astr[i].Length;
				strNext = m_Menus_astr[i].Substring(0, nLenLoc);
				if (strPre != strNext)
				{
					if (nOrgIndex < _MenuOrg.Count && i == _MenuOrg[nOrgIndex].m_nIndex)
						continue;

					txtOrg = new TextOrg();
					txtOrg.m_nDepth = nLen;
					txtOrg.m_nIndex = i;
					txtOrg.m_nLast = _nEnd;
					if (nOrgIndex + 1 < _MenuOrg.Count)
						txtOrg.m_nLast = _MenuOrg[nOrgIndex + 1].m_nIndex;
					if (nOrgIndex > 0)
						_MenuOrg[nOrgIndex - 1].m_nLast = i;
					if (i == _nEnd - 1)
						txtOrg.m_bFull = true;
					_MenuOrg.Insert(nOrgIndex, txtOrg);

					if (nOrgIndex > 0 && _MenuOrg[nOrgIndex - 1].m_nIndex == i - 1)
						_MenuOrg[nOrgIndex - 1].m_bFull = true;

					nOrgIndex++;
					_nOrgCnt++;
				}
			}

			/*
			if (_nOrgCnt < 8 && (_nEnd - nStart) > 1)
			{
			    for (int i = 0; i < _MenuOrg.Count; i++)
			    {
			        int nEnd = _nEnd;
			        if (i + 1 < _MenuOrg.Count)
			            nEnd = _MenuOrg[i + 1].m_nIndex;
	
			        List<TextOrg> MenuOrg = new List<TextOrg>();
			        MenuOrg.Add(_MenuOrg[i]);
			        _nOrgCnt = MenuOrganizeChild(MenuOrg, nEnd, _nOrgCnt);
	
			        for(int j=1; j<MenuOrg.Count; j++)
			            _MenuOrg.Insert(i+j, MenuOrg[j]);
			        i += (MenuOrg.Count - 1);
			    }
			}
			//*/
			return _nOrgCnt;
		}

		void Click()
		{
			float fDist = 0;
			int nCur = CurPoint(m_radiousMax_f, ref fDist, m_radiousMin_f);

			if (nCur == 0)
			{
				if (m_MenuOrgsStack.Count > 0)
				{
					m_MenuOrgs_atxt = m_MenuOrgsStack[m_MenuOrgsStack.Count - 1];
					m_MenuOrgsStack.RemoveAt(m_MenuOrgsStack.Count - 1);
					MenuListSet(0, m_MenuOrgs_atxt.Count);
				}
				else
				{
					VLStateManager.event_state_post("VScriptCosmosCancel");
				}

				return;
			}

			nCur--;

			m_MenuOrgsStack.Add(m_MenuOrgs_atxt);

			int nP = MenuPosGet(nCur, 0, m_MenuOrgs_atxt.Count);
			if (nP >= m_MenuOrgs_atxt.Count)
				nP = m_MenuOrgs_atxt.Count - 1;
			int nStart = m_MenuOrgs_atxt[nP].m_nIndex;
			int nEnd = m_MenuOrgs_atxt[nP].m_nLast;

			int nLen = m_MenuOrgs_atxt[nP].m_nDepth;

			if (m_MenuOrgs_atxt[nP].m_bFull)
			{
				StateDStructureValue dstEvent = VLStateManager.event_state_make("VScriptCosmosSelect");
				dstEvent.set_variable_string("TempString_strV", m_Menus_astr[nStart]);
				Debug.Log("CosmosSelect=" + m_Menus_astr[nStart]);
				VLStateManager.event_post(dstEvent);
				return;
			}

			m_MenuOrgs_atxt = new List<TextOrg>();
			TextOrg txtOrg = new TextOrg();
			txtOrg.m_nDepth = nLen;
			txtOrg.m_nIndex = nStart;
			txtOrg.m_nLast = nEnd;
			m_MenuOrgs_atxt.Add(txtOrg);

			MenuOrganizeRoot2(m_MenuOrgs_atxt, nEnd);
			MenuListSet(0, m_MenuOrgs_atxt.Count);
		}

		void MenuOrganizeRoot()
		{
			m_MenuOrgs_atxt.Clear();
			m_MenuOrgsStack.Clear();
			TextOrg txtOrg = new TextOrg();
			txtOrg.m_nDepth = 0;
			txtOrg.m_nIndex = 0;
			txtOrg.m_nLast = m_Menus_astr.Count;
			m_MenuOrgs_atxt.Add(txtOrg);
			m_Menus_astr.Sort();
			m_menu_original.Clear();
			foreach (string str in m_Menus_astr)
				m_menu_original.Add(str);

			MenuOrganizeRoot2(m_MenuOrgs_atxt, m_Menus_astr.Count);
		}

		void MenuSearch(string _str)
		{
			m_MenuOrgs_atxt.Clear();
			m_MenuOrgsStack.Clear();
			string str_low = _str.ToLower();
			m_Menus_astr.Clear();

			foreach (string str in m_menu_original)
			{
				string low = str.ToLower();
				if (low.IndexOf(str_low) >= 0)
					m_Menus_astr.Add(str);
			}

			if (m_Menus_astr.Count == 0)
				m_Menus_astr.Add("- Nothing -");

			TextOrg txtOrg = new TextOrg();
			txtOrg.m_nDepth = 0;
			txtOrg.m_nIndex = 0;
			txtOrg.m_nLast = m_Menus_astr.Count;
			m_MenuOrgs_atxt.Add(txtOrg);
			m_Menus_astr.Sort();

			MenuOrganizeRoot2(m_MenuOrgs_atxt, m_Menus_astr.Count);
			MenuListSet(0, m_MenuOrgs_atxt.Count);
		}

		int MenuPosGet(int _i, int _nFrom, int _nTo)
		{
			int nLen = (_nTo - 1 - _nFrom);
			int nLow = 7;
			if (nLen < nLow)
				nLow = nLen;
			if (nLow == 0)
				return 0;
			return _i * nLen / nLow;
		}

		void MenuListSet(int _nFrom, int _nTo)
		{
			for (int i = 0; i < m_agoLeafs.Count; i++)
			{
				TMP_Text txtMep;

				txtMep = m_agoLeafs[i].GetComponentInChildren<TMP_Text>();

				txtMep.text = "";
			}

			for (int i = 0; i < 8 && i < m_MenuOrgs_atxt.Count; i++)
			{
				TMP_Text txtMep;
				int nP = MenuPosGet(i, _nFrom, _nTo);
				TextOrg org = m_MenuOrgs_atxt[nP + _nFrom];

				txtMep = m_agoLeafs[i].GetComponentInChildren<TMP_Text>();

				string menu = m_Menus_astr[org.m_nIndex];
				if (!org.m_bFull)
				{
					int nLen = 10;
					if (nLen < org.m_nDepth)
						nLen = org.m_nDepth;
					if (nLen > menu.Length)
						nLen = menu.Length;
					menu = menu.Substring(0, nLen) + " ...";
				}

				txtMep.text = menu;
			}
		}

		public void MenuListMake(List<string> _astrList)
		{
			m_Menus_astr = _astrList;
			MenuOrganizeRoot();
			MenuListSet(0, m_MenuOrgs_atxt.Count);
		}

		void UpdateCosmos()
		{
			bool bFirst = false;
			if (m_agoLeafs == null)
			{
				m_agoLeafs = new List<GameObject>();
				bFirst = true;
			}

			if (!bFirst && m_v3Center != m_v3MoveToInScreen)
			{
				Vector3 dir = m_v3MoveToInScreen - m_v3Center;

				dir /= 2.0f;
				m_v3Center += dir;

				if (m_cosmosboard != null)
					m_cosmosboard.transform.position = m_v3Center;
			}

			for (int i = 0; i < 8; i++)
			{
				bool bOpposite = false;
				if (i > 2 && i < 7)
					bOpposite = true;

				GameObject goLeaf;

				if (bFirst)
				{
					if (!bOpposite)
						goLeaf = (GameObject)Instantiate(m_CosmosBoard);
					else
						goLeaf = (GameObject)Instantiate(prefabButton);

					m_agoLeafs.Add(goLeaf);
				}
				else
				{
					goLeaf = m_agoLeafs[i];
				}

				goLeaf.transform.SetParent(ParentPanel.parent);
				goLeaf.transform.localScale = new Vector3(1, 1, 1);
				goLeaf.transform.position = m_v3Center;

				float fRadian = 45 * i;
				if (bOpposite)
					fRadian += 180;

				goLeaf.transform.localEulerAngles = new Vector3(0, 0, fRadian);

				float fDist = m_radiousMin_f * 2.0f * m_mainCanvas.scaleFactor;
				float fCosDist = 0, scale_f = 1;
				if (!bFirst && CurPoint(m_radiousMax_f, ref fCosDist, m_radiousMin_f) == i + 1)
				{
					//if (fCosDist > m_radiousMin_f)
					{
						fDist += m_radiousMin_f * 1.2f * m_mainCanvas.scaleFactor;
						scale_f = 1.5f;
					}
				}

				float x, y;
				float fAngle = (float)i * Mathf.PI / 4.0f;
				y = Mathf.Sin(fAngle) * fDist;
				x = Mathf.Cos(fAngle) * fDist;
				goLeaf.transform.position += new Vector3(x, y, 0);
				goLeaf.transform.localScale = new Vector3(scale_f, scale_f, scale_f);
			}
		}

		GameObject ComponentGet(string _strName)
		{
			bool bHashHas = true;
			GameObject goObj = (GameObject)m_hashObjectHide[_strName];
			if (goObj == null)
			{
				goObj = GameObject.Find(_strName);
				bHashHas = false;
			}

			if (goObj == null)
				return null;

			if (!bHashHas)
				m_hashObjectHide.Add(_strName, goObj);

			return goObj;
		}

		bool ComponentSizeAdapt(string _adapt, string _base, int _rate)
		{
			GameObject adapt_go = ComponentGet(_adapt);
			GameObject target_go = ComponentGet(_base);

			if (adapt_go == null
			    || target_go == null)
				return false;
			RectTransform adapt_rt = (RectTransform)adapt_go.transform;
			RectTransform target_rt = (RectTransform)target_go.transform;

			float rate = (float)_rate / 100.0f;
			Image loadingBar = adapt_go.GetComponent<Image>();
			loadingBar.fillAmount = rate;

			//Debug.LogWarning(string.Format("KKIComponentSizeAdapt {0}  {1}  {2}", loadingBar.gameObject.name, adapt_go.gameObject.name, rate));


			//adapt_go.transform.position = target_go.transform.position;
			//adapt_rt.sizeDelta = new Vector2(rate * target_rt.rect.width, target_rt.rect.height);
			return true;
		}

		bool ComponentPosSet(string _strName, Vector3 _v3Pos)
		{
			GameObject goObj = ComponentGet(_strName);

			if (goObj == null)
				return false;

			goObj.transform.position = _v3Pos;
			return true;
		}

		bool ComponentHide(string _strName)
		{
			GameObject goObj = ComponentGet(_strName);

			if (goObj == null)
				return false;

			goObj.SetActive(false);
			return true;
		}

		void ComponentShow(string _strName)
		{
			GameObject goHide = (GameObject)m_hashObjectHide[_strName];
			if (goHide == null)
				return;

			m_cosmosboard = goHide;
			m_v3Center = goHide.transform.position;

			m_v3MoveToInScreen = m_v3Center;

			if (m_v3MoveToInScreen.x + m_radiousMax_f > Screen.width)
				m_v3MoveToInScreen.x = Screen.width - m_radiousMax_f;
			if (m_v3MoveToInScreen.y + m_radiousMax_f > Screen.height)
				m_v3MoveToInScreen.y = Screen.height - m_radiousMax_f;

			if (m_v3MoveToInScreen.x < m_radiousMax_f)
				m_v3MoveToInScreen.x = m_radiousMax_f;
			if (m_v3MoveToInScreen.y < m_radiousMax_f)
				m_v3MoveToInScreen.y = m_radiousMax_f;

			goHide.SetActive(true);
		}

		bool CameraEnable(string _name_str)
		{
			GameObject go = ComponentGet(_name_str);

			Camera cam = go.GetComponent<Camera>();

			if (cam == null)
				return false;

			cam.enabled = true;
			//UxCamera.m_cam3D = cam;

			return true;
		}

		bool CameraDisable(string _name_str)
		{
			GameObject go = ComponentGet(_name_str);

			Camera cam = go.GetComponent<Camera>();

			if (cam == null)
				return false;

			cam.enabled = false;

			return true;
		}

		void ButtonClicked(int buttonNo)
		{
			if (m_stlVEvents.Count > buttonNo && buttonNo >= 0)
				UxTouch.ms_nHashFocus = VLStateManager.hash(m_stlVEvents[buttonNo]);
			VLStateManager.event_state_post("UxCosmosSelect");
		}

		public void onClick()
		{
		}

		int CurPoint(float fMax, ref float fDist, float _Minf)
		{
			Vector3 vposms = UxTouch.ms_v3BeforMiddle;
			if (UxTouch.m_stlVTouchMiddle.Count > 0)
			{
				vposms = UxTouch.m_stlVTouchMiddle[0];
			}

			fMax *= m_mainCanvas.scaleFactor;
			_Minf *= m_mainCanvas.scaleFactor;
			//vposms *= m_mainCanvas.scaleFactor;

			Vector3 v3Zero = new Vector3(1, 0, 0), v3Up = new Vector3(0, 1, 0);
			Vector3 v3Dir = vposms - m_v3Center;

			fDist = v3Dir.magnitude;

			if (v3Dir.magnitude < _Minf)
				return 0;

			if (v3Dir.magnitude > fMax)
				return 0;

			v3Dir.Normalize();

			float fAngle = Mathf.Acos(Vector3.Dot(v3Zero, v3Dir));

			if (Vector3.Dot(v3Dir, v3Up) < 0)
				fAngle *= -1;

			fAngle += Mathf.PI / 8.0f;
			if (fAngle < 0)
				fAngle += Mathf.PI * 2.0f;

			int nMenu = (int)(fAngle * 4.0f / Mathf.PI);
			nMenu++;

			return nMenu;
		}

		bool CheckInPetal(string _strEvent)
		{
			float fT = 0;
			int nMenu = CurPoint(s_fRadius, ref fT, m_radiousMin_f);

			if (nMenu >= 0 && nMenu < m_stlVEvents.Count)
			{
				if (m_stlVEvents[nMenu] == _strEvent)
					return true;
			}

			return false;
		}

		static public bool InButton()
		{
			for (int i = 0; i < m_stlVButtons.Count; i++)
			{
				//UxCosmosButton scriptBtn = m_stlVButtons[i].GetComponent<UxCosmosButton> ();
				//if(scriptBtn == null)
				//	return false;
				//if(scriptBtn.InButton())
				//	return true;
			}

			GameObject btn;
			/*
			btn = GameObject.Find ("Canvas/NotificationMsg/ButtonStateCancel");
			if (btn) {
				UxCosmosButton scriptBtn = btn.GetComponent<UxCosmosButton> ();
				if(scriptBtn == null)
					return false;
				if(scriptBtn.InButton())
					return true;
			}
	//*/
			btn = GameObject.Find("Canvas/ButtonAdd");
			if (btn)
			{
				//UxCosmosButton scriptBtn = btn.GetComponent<UxCosmosButton> ();
				//if(scriptBtn == null)
				//	return false;
				//if(scriptBtn.InButton())
				//	return true;
			}

			return false;
		}

		static public void Updated()
		{
			//ms_bUpdated = true;
		}

		void MenuCreate()
		{
			for (int i = 0; i < m_stlVButtons.Count; i++)
				Destroy(m_stlVButtons[i]);
			m_stlVButtons.Clear();

			bool bParent = false;
			if (m_strParent != "")
			{
				GameObject goParent = GameObject.Find(m_strParent);
				if (goParent)
					ParentPanel = goParent.transform;
				bParent = true;
			}
			else
			{
				GameObject goParent = GameObject.Find("Canvas");
				if (goParent)
					ParentPanel = goParent.transform;
			}

			if (bParent)
			{
				m_vPos = ParentPanel.position;
			}

			GameObject goButton = (GameObject)Instantiate(prefabButton);
			goButton.transform.localScale = new Vector3(1, 1, 1);
			goButton.transform.SetParent(ParentPanel, false);
			goButton.transform.position = m_vPos;
			Text uniText = goButton.GetComponentInChildren<Text>();
			uniText.text = m_stlVMenus[0];

			//Button tempButton = goButton.GetComponent<Button>();
			//int tempCancel = 0;
			//tempButton.onClick.AddListener(() => ButtonClicked(tempCancel));
			m_stlVButtons.Add(goButton);

			Transform m4Parent = goButton.transform;
			Vector3 v3PosMin, v3PosMax;
			v3PosMax = v3PosMin = goButton.transform.position;

			//CircleMenuController.Instance.Show(m4Parent);
			for (int i = 0; i < 8; i++)
			{
				s_fRadius = m_radiousMax_f * 2.3f;
				//float fActive = 1;
				float fDistRet = m_radiousMax_f, fDist = 0;
				if (i == CurPoint(s_fRadius, ref fDist, m_radiousMin_f) - 1
				    && UxTouch.m_stlVTouchMiddle.Count > 0)
					fDistRet = fDist - 80;

				float fAngle = i / 4.0f * Mathf.PI;
				float x, y;
				y = Mathf.Sin(fAngle) * fDistRet;
				x = Mathf.Cos(fAngle) * fDistRet;
				if (m_stlVIconPath.Count > i + 1 && m_stlVIconPath[i + 1].Length > 0)
				{
					GameObject _gobHierarchy = GameObject.Find(m_stlVIconPath[i + 1]);
					if (null == _gobHierarchy)
					{
						Debug.LogError(string.Format("MenuCreate : Not Exist - {0}", m_stlVIconPath[i + 1]));
					}
					else
					{
						goButton = (GameObject)Instantiate(_gobHierarchy);
					}
				}
				else
				{
					goButton = (GameObject)Instantiate(preFlower);
				}

				//             goButton.transform.SetParent(m4Parent, false);
				//             goButton.transform.localScale = new Vector3(1, 1, 1);
				//             goButton.transform.position = new Vector3(m_vPos.x + x, m_vPos.y + y, 0);
				//             fAngle = i * 45;
				//             if (fAngle > 45 && fAngle < 270)
				//                 fAngle += 180;
				uniText = goButton.GetComponentInChildren<Text>();
				//if (!bIcon)
				//    goButton.transform.rotation = Quaternion.Euler(0, 0, fAngle);
				//else
				//    goButton.transform.rotation = Quaternion.Euler(0, 0, 0);

				//CircleMenuController.Instance.AddMenuEntity(goButton, i);
				if (uniText != null)
				{
					if (i < m_stlVMenus.Count - 1)
					{
						uniText.text = m_stlVMenus[i + 1];
						uniText.color = new Color(1.0f, 1.0f, 1.0f);
					}
					else
						uniText.text = "";
				}

				RectTransform rt = (RectTransform)goButton.transform;
				if (v3PosMin.x > rt.rect.xMin + rt.position.x)
					v3PosMin.x = rt.rect.xMin + rt.position.x;
				if (v3PosMin.y > rt.rect.yMin + rt.position.y)
					v3PosMin.y = rt.rect.yMin + rt.position.y;

				if (v3PosMax.x < rt.rect.xMin + rt.rect.width + rt.position.x)
					v3PosMax.x = rt.rect.xMin + rt.rect.width + rt.position.x;
				if (v3PosMax.y < rt.rect.yMin + rt.rect.height + rt.position.y)
					v3PosMax.y = rt.rect.yMin + rt.rect.height + rt.position.y;

				m_stlVButtons.Add(goButton);
			}
			//ms_bUpdated = false;

			m_v3MoveToInScreen = new Vector3(0, 0, 0);
			if (v3PosMax.x > Screen.width)
				m_v3MoveToInScreen.x = Screen.width - v3PosMax.x;
			if (v3PosMax.y > Screen.height - ms_nScreenBoxUp)
				m_v3MoveToInScreen.y = Screen.height - v3PosMax.y - ms_nScreenBoxUp;

			if (v3PosMin.x < 0)
				m_v3MoveToInScreen.x = -v3PosMin.x;
			if (v3PosMin.y < ms_nScreenBoxDown)
				m_v3MoveToInScreen.y = -v3PosMin.y + ms_nScreenBoxDown;
		}

		void MenuUpdate()
		{
			GameObject goButton = m_stlVButtons[0];
			goButton.transform.position = m_vPos;

			Transform m4Parent = goButton.transform;
			Vector3 v3PosMin, v3PosMax;
			v3PosMax = v3PosMin = goButton.transform.position;

			for (int i = 0; i < 8; i++)
			{
				//bool isTouching = false;

				s_fRadius = m_radiousMax_f * 2.3f;
				//float fActive = 1, 
				float fDistRet = m_radiousMax_f, fDist = 0;
				if (i == CurPoint(s_fRadius, ref fDist, m_radiousMin_f) - 1
				    && UxTouch.m_stlVTouchMiddle.Count > 0)
				{
					fDistRet = fDist - 80;
					//isTouching = true;
				}



				float fAngle = i / 4.0f * Mathf.PI;
				float x, y;
				//bool bIcon = false;
				y = Mathf.Sin(fAngle) * fDistRet;
				x = Mathf.Cos(fAngle) * fDistRet;
				goButton = m_stlVButtons[i + 1];
				//if (m_stlVIconPath.Count > i + 1 && m_stlVIconPath[i + 1].Length > 0)
				//{
				//    bIcon = true;
				//}

				//CircleMenuController.Instance.UpdatePosition(i, fDistRet, isTouching);
				//goButton.transform.position = new Vector3(m_vPos.x + x, m_vPos.y + y, 0);
				fAngle = i * 45;
				if (fAngle > 45 && fAngle < 270)
					fAngle += 180;

				RectTransform rt = (RectTransform)goButton.transform;
				if (v3PosMin.x > rt.rect.xMin + rt.position.x)
					v3PosMin.x = rt.rect.xMin + rt.position.x;
				if (v3PosMin.y > rt.rect.yMin + rt.position.y)
					v3PosMin.y = rt.rect.yMin + rt.position.y;

				if (v3PosMax.x < rt.rect.xMin + rt.rect.width + rt.position.x)
					v3PosMax.x = rt.rect.xMin + rt.rect.width + rt.position.x;
				if (v3PosMax.y < rt.rect.yMin + rt.rect.height + rt.position.y)
					v3PosMax.y = rt.rect.yMin + rt.rect.height + rt.position.y;
			}
			//ms_bUpdated = false;

			m_v3MoveToInScreen = new Vector3(0, 0, 0);
			if (v3PosMax.x > Screen.width)
				m_v3MoveToInScreen.x = Screen.width - v3PosMax.x;
			if (v3PosMax.y > Screen.height - ms_nScreenBoxUp)
				m_v3MoveToInScreen.y = Screen.height - v3PosMax.y - ms_nScreenBoxUp;

			if (v3PosMin.x < 0)
				m_v3MoveToInScreen.x = -v3PosMin.x;
			if (v3PosMin.y < ms_nScreenBoxDown)
				m_v3MoveToInScreen.y = -v3PosMin.y + ms_nScreenBoxDown;
		}

		void Update()
		{

		}

		public static int UxLanguageChange_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			int nCountryCode = 0;
			if (!stBase.get_int("UxLanguageChange_nF", ref nCountryCode))
				return 0;

			VLStateManager.language_change(nCountryCode);
			return 1;
		}


		public static int UxRecordScreen_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			int nRecordOn = 0;
			if (!stBase.get_int("UxRecordScreen_nF", ref nRecordOn))
				return 0;


			return 1;
		}



		public static int UxOpenURL_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			string strURL = "";
			if (!stBase.get_string("UxOpenURL_strF", ref strURL))
				return 0;

			string strURLOpen = VLStateManager.variable_make_string(_pBase, _pEvent, _pContext, strURL);

			Application.OpenURL(strURLOpen);

			return 1;
		}


		public static int UxComponentCreate_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			GameObject obj;
			GameObject goButton;

			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			string strCom = "";
			if (!stBase.get_string("UxComponentCreate_strF", ref strCom))
				return 0;

			//GameObject 
			obj = GameObject.Find(strCom);
			if (obj == null)
				return 0;

			Transform ParentPanel = GameObject.Find("Canvas/NotificationMsg").transform;

			//GameObject 
			goButton = (GameObject)Instantiate(obj);
			goButton.transform.localScale = new Vector3(1, 1, 1);
			goButton.transform.SetParent(ParentPanel, false);
			goButton.transform.position = new Vector3(0, 100, 0);
			//Text uniText = goButton.GetComponentInChildren<Text>();
			//uniText.text = m_stlVMenus [0];
			StateDStructureValue stContext = new StateDStructureValue(_pContext);
			stContext.set_variable("UxComponent", goButton);
			return 1;
		}


		public static int UxComponentDestory_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			GameObject obj = null;
			StateDStructureValue stContext = new StateDStructureValue(_pContext);
			stContext.get_ptr("UxComponent", ref obj);

			if (obj == null)
				return 0;

			Destroy(obj);
			return 1;
		}


		public static int UxComponentTextSet_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			int nHash = 0;
			if (!stBase.get_int("UxComponentTextSet_nF", ref nHash))
				return 0;

			string component_str = "";
			if (!stBase.get_string("UxComponentEnable_strF", ref component_str))
				return 0;

			GameObject obj = GameObject.Find(component_str);
			if (obj == null)
				return 0;

			string strText = "";
			if (!stBase.state_variable_get().get_string(nHash, ref strText))
				return 0;

			Text txt = obj.GetComponentInChildren<Text>();
			if (txt != null)
			{
				txt.text = strText;
			}

			TMP_Text txtp = obj.GetComponentInChildren<TMP_Text>();
			if (txtp == null)
				return 0;
			txtp.text = strText;
			// marker

			return 1;
		}


		public static int UxComponentAnimatorAttach_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			string strCom = "";
			if (!stBase.get_string("UxComponentAnimatorAttach_strF", ref strCom))
				return 0;

			GameObject obj = null;
			StateDStructureValue stContext = new StateDStructureValue(_pContext);
			stContext.get_ptr("UxComponent", ref obj);

			if (obj == null)
				return 0;

			return 1;
		}


		public static int UxComponentAnimatorUpdate_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			return 0;
		}


		public static int UxScreenSizeSmaller_nIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			int nSize = 0;
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			if (!stBase.get_int("UxScreenSizeSmaller_nIf", ref nSize))
				return 0;

			if (Screen.width < nSize)
				return 1;
			if (Screen.height < nSize)
				return 1;

			return 0;
		}


		public static int UxDeviceInforDisplay_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			int nSize = 0;

			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			if (!stBase.get_int("UxDeviceInforDisplay_nF", ref nSize))
				return 0;

			VLStateManager.message(1, "DeviceInfor Screen-" + Screen.width.ToString() + "x"
			                          + Screen.height.ToString() + " Device-" + SystemInfo.deviceName + ","
			                          + SystemInfo.deviceModel);

			return 1;
		}



		public static int UxSystemVariableGet_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			int nType = 0;
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			if (!stBase.get_int("UxSystemVariableGet_nF", ref nType))
				return 0;

			int nSeq = 0, nHashVariable = 0;
			if (!stBase.param_get("BaseVariableRefer_anV", nSeq, ref nHashVariable))
				return 0;

			StateDStructureValue stVariable = VLStateManager.variable_global_get();

			int nValue = 0;

			switch (nType)
			{
				case 0:
					nValue = Screen.width;
					break;
				case 1:
					nValue = Screen.height;
					break;
				case 2:
					nValue = (int)(Screen.dpi * 1000000.0f);
					break;
			}

			if (!stVariable.set_variable(nHashVariable, nValue))
				return 0;

			return 1;
		}


		public static int UxSystemVariableGetStr_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			int nType = 0;
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			if (!stBase.get_int("UxSystemVariableGetStr_nF", ref nType))
				return 0;

			int nSeq = 0, nHashVariable = 0;
			if (!stBase.param_get("BaseVariableRefer_anV", nSeq, ref nHashVariable))
				return 0;

			StateDStructureValue stVariable = VLStateManager.variable_global_get();

			string strValue = "";

			switch (nType)
			{
				case 0:
					strValue = "DeviceInfor Screen-" + Screen.width.ToString() + "x"
					           + Screen.height.ToString() + " Device-" + SystemInfo.deviceName + ","
					           + SystemInfo.deviceModel;
					break;
			}

			if (!stVariable.set_variable_string(nHashVariable, strValue))
				return 0;

			return 1;
		}



		public static int UxScreenSizeBigger_nIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			int nSize = 0;
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			if (!stBase.get_int("UxScreenSizeBigger_nIf", ref nSize))
				return 0;

			if (Screen.width > nSize)
				return 1;
			if (Screen.height > nSize)
				return 1;

			return 0;
		}


		public static int UxScreenRateBigger_nIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			int nRate = 0;
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			if (!stBase.get_int("UxScreenRateBigger_nIf", ref nRate))
				return 0;

			float up = 0.0f;
			float dn = 0.0f;
			float com = (float)nRate;
			com = com / 1000.0f;
			if (Screen.width > Screen.height)
			{
				up = Screen.width;
				dn = Screen.height;
			}
			else
			{
				up = Screen.height;
				dn = Screen.width;
			}

			if (up / dn > com)
				return 1;

			return 0;
		}


		public static int UxComponentTransformUpdate_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			string strParams = "";
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			if (!stBase.get_string("UxComponentTransformUpdate_strF", ref strParams))
				return 0;

			string strSource, strFormat;
			int nSep = strParams.IndexOf(", ");
			if (nSep < 1)
				return 0;

			strSource = strParams.Substring(0, nSep);
			strFormat = strParams.Substring(nSep + 2);

			GameObject objSrc, objFmt;

			objSrc = GameObject.Find(strSource);
			objFmt = GameObject.Find(strFormat);

			if (objSrc == null || objFmt == null)
				return 0;

			RectTransform rectSrc, rectFmt;

			rectSrc = objSrc.GetComponent<RectTransform>();
			rectFmt = objFmt.GetComponent<RectTransform>();

			if (rectSrc != null && rectFmt != null)
			{
				rectSrc.sizeDelta = rectFmt.sizeDelta;
			}

			objSrc.transform.localPosition = objFmt.transform.localPosition;

			Image img;
			img = objSrc.GetComponent<Image>();

			if (img)
			{
				Color col = img.color;
				col.a = 1;
				img.color = col;
			}


			return 1;
		}


		public static int UxComponentAnimatorUpdate_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			int nHash = 0;
			if (!stBase.get_int("UxComponentAnimatorUpdate_nF", ref nHash))
				return 0;

			int nValue = 0;
			StateDStructureValue stContext = new StateDStructureValue(_pContext);

			int nHashName = VLStateManager.hash("strName");
			string name = "";
			if (!VLStateManager.enum_get("EnumVariableDefine", nHash, nHashName, ref name))
				return 0;

			if (!stContext.get_int(name, ref nValue))
				return 0;

			GameObject obj = null;
			stContext.get_ptr("UxComponent", ref obj);

			if (obj == null)
				return 0;

			Animator ani = obj.GetComponentInChildren<Animator>();

			if (ani == null)
				return 0;

			//ani.SetInteger (name, nValue);
			if (nValue == 0)
				ani.SetBool(name, false);
			else
				ani.SetBool(name, true);

			return 1;
		}


		public static int UxComponentRectAdapt_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			string strCom = "";
			if (!stBase.get_string("UxComponentRectAdapt_strF", ref strCom))
				return 0;

			string strComForm = VLStateManager.param_variable_get_string(_pBase, _pEvent, _pContext, 0, IntPtr.Zero);

			GameObject obj = GameObject.Find(strCom);
			if (obj == null)
				return 0;
			RectTransform transObj = obj.GetComponent<RectTransform>();

			obj = GameObject.Find(strComForm);
			if (obj == null)
				return 0;

			RectTransform transObjForm = obj.GetComponent<RectTransform>();

			transObj.position = transObjForm.position;
			transObj.sizeDelta = transObjForm.sizeDelta;
			return 1;
		}

		public static int UxComponentSizeAdapt_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			string strCom = "";
			if (!stBase.get_string("UxComponentSizeAdapt_strF", ref strCom))
				return 0;

			StateDStructureValue stVariable = stBase.state_variable_get();
			int rate = 0;
			if (!stBase.param_get(_pEvent, _pContext, 1, ref rate))
				return 0;

			string target_com = "";
			if (!stBase.param_get(_pEvent, _pContext, 0, ref target_com))
				return 0;

			{
				StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
				MonoBehaviour uxCosmos = null;
				if (dstGlobal.get_ptr(sm_strMonoBehaviour, ref uxCosmos))
				{
					if (((UxCosmos)uxCosmos).ComponentSizeAdapt(strCom, target_com, rate))
						return 1;
				}
			}
			return 0;
		}


		public static int UxComponentPosSet_afF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			Vector3 pos = Vector3.zero;
			if (!stBase.get_vector("UxComponentPosSet_afF", ref pos))
				return 0;

			string component_str = "";
			if (!stBase.get_string("UxComponentEnable_strF", ref component_str))
				return 0;

			GameObject obj = GameObject.Find(component_str);
			if (obj == null)
				return 0;

			obj.transform.localPosition = pos;

			// marker
			return 1;
		}

		public static int UxComponentPositionSet_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			StateDStructureValue event_dsv = new StateDStructureValue(_pEvent);
			string strCom = "";
			if (!stBase.get_string("UxComponentPositionSet_strF", ref strCom))
				return 0;

			StateDStructureValue stVariable = stBase.state_variable_get();

			Vector3 pos = stBase.mouse_pos_get(event_dsv);

			if (pos != Vector3.zero)
			{
				StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
				MonoBehaviour uxCosmos = null;
				if (dstGlobal.get_ptr(sm_strMonoBehaviour, ref uxCosmos))
				{
					if (((UxCosmos)uxCosmos).ComponentPosSet(strCom, pos))
						return 1;
				}
			}

			return 0;
		}


		public static int UxComponentPositionYSet_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			string strCom = "";
			if (!stBase.get_string("UxComponentPositionYSet_strF", ref strCom))
				return 0;

			int nOffetY = VLStateManager.param_variable_get_int(_pBase, _pEvent, _pContext, 0, _pContext);

			GameObject obj = GameObject.Find(strCom);
			if (obj == null)
				return 0;

			Vector3 v3Pos = obj.transform.position;
			v3Pos.y += nOffetY;
			obj.transform.position = v3Pos;

			return 1;
		}


		public static int UxComponentTextSet_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			string strCom = "";
			if (!stBase.get_string("UxComponentTextSet_strF", ref strCom))
				return 0;

			GameObject obj = GameObject.Find(strCom);
			if (obj == null)
				return 0;

			string strMsg = null;
			strMsg = VLStateManager.param_variable_get_string(_pBase, _pEvent, _pContext, 0, IntPtr.Zero);
			strMsg = VLStateManager.translate(strMsg);

			Text txt = (Text)obj.GetComponent(typeof(Text));
			if (txt == null)
				return 0;

			txt.text = strMsg;
			return 1;
		}


		public static int UxFileSave_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			string strExt = "";
			if (!stBase.get_string("UxFileSave_strF", ref strExt))
				return 0;

			StateDStructureValue stVariable = VLStateManager.variable_global_get();
			int nSeq = 0, nHashVariable = 0;
			if (!stBase.param_get("BaseVariableRefer_anV", nSeq, ref nHashVariable))
				return 0;

			string strValue = "";

# if UNITY_STANDALONE
			//strValue = UnityEditor.EditorUtility.SaveFilePanel("Save Scene", "", "default", strExt);
			strValue = VLStateManager.filedialog_save(strExt);
#else
        return 0;
#endif

			stVariable.set_string(nHashVariable, strValue);

			if (strValue == "")
			{
				VLStateManager.event_state_post("Cancel");
			}
			else
			{
				VLStateManager.event_state_post("Submit");
			}

			return 1;
		}


		public static int UxFileOpen_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			string strExt = "";
			if (!stBase.get_string("UxFileOpen_strF", ref strExt))
				return 0;

			StateDStructureValue stVariable = VLStateManager.variable_global_get();
			int nSeq = 0, nHashVariable = 0;
			if (!stBase.param_get("BaseVariableRefer_anV", nSeq, ref nHashVariable))
				return 0;

			string strValue = "";

#if UNITY_STANDALONE
			//strValue = UnityEditor.EditorUtility.OpenFilePanel("Load Scene", "", strExt);
			strValue = VLStateManager.filedialog_open(strExt);
#else
        return 0;
#endif

			stVariable.set_string(nHashVariable, strValue);

			if (strValue == "")
			{
				VLStateManager.event_state_post("Cancel");
			}
			else
			{
				VLStateManager.event_state_post("Submit");
			}

			return 1;
		}



		public static int UxComponentInputSetFromVariable_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext,
			int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			string strCom = "";
			if (!stBase.get_string("UxComponentInputSetFromVariable_strF", ref strCom))
				return 0;

			StateDStructureValue stVariable = stBase.state_variable_get();
			int nSeq = 0, nHashVariable = 0;
			if (!stBase.param_get("BaseVariableRefer_anV", nSeq, ref nHashVariable))
				return 0;

			string strValue = "";
			if (!stVariable.get_string(nHashVariable, ref strValue))
				return 0;

			GameObject obj = GameObject.Find(strCom);
			if (obj == null)
				return 0;
			InputField input = obj.GetComponent<InputField>();
			if (input == null)
			{
				TMPro.TMP_InputField tInput = obj.GetComponent<TMPro.TMP_InputField>();
				if (tInput == null)
				{
					Text txt_go = obj.GetComponent<Text>();
					if (txt_go == null)
						return 0;
					txt_go.text = strValue;
					return 1;
				}

				tInput.text = strValue;
				return 1;
			}

			input.text = strValue;
			return 1;
		}


		public static int UxComponentToVariable_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			string strCom = "";
			if (!stBase.get_string("UxComponentToVariable_strF", ref strCom))
				return 0;

			StateDStructureValue stVariable = stBase.state_variable_get();
			int nSeq = 0, nHashVariable = 0;
			if (!stBase.param_get("BaseVariableRefer_anV", nSeq, ref nHashVariable))
				return 0;

			GameObject obj = GameObject.Find(strCom);
			if (obj == null)
				return 0;

			InputField input = obj.GetComponent<InputField>();
			if (input == null)
			{
				TMPro.TMP_InputField tInput = obj.GetComponent<TMPro.TMP_InputField>();
				if (tInput == null)
				{
					Text txt_go = obj.GetComponent<Text>();
					if (txt_go == null)
						return 0;
					stVariable.set_string(nHashVariable, txt_go.text);
					return 1;
				}

				stVariable.set_string(nHashVariable, tInput.text);
				return 1;
			}

			if (input == null)
				return 0;
			stVariable.set_string(nHashVariable, input.text);
			return 1;
		}


		public static int UxCosmosMakeDir_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			string strPath = "";
			stBase.get_string("UxCosmosMakeDir_strF", ref strPath);

			StateDStructureValue stVariable = VLStateManager.variable_global_get();
			int nHashVariable = VLStateManager.hash(strPath);

			if (!stVariable.get_string(nHashVariable, ref strPath))
				return 0;

			//CircleMenuController.Instance.Close();
			UxCosmos.m_stlVMenus.Clear();
			UxCosmos.m_stlVIconPath.Clear();
			UxCosmos.m_stlVEvents.Clear();

			UxCosmos.m_stlVMenus.Add("Cancel");
			UxCosmos.m_stlVIconPath.Add("Cancel");
			UxCosmos.m_stlVEvents.Add("Cancel");

			//UxCosmos.m_stlVMenus.Add ("..");
			//UxCosmos.m_stlVEvents.Add("..");

			DirectoryInfo dirInf = new DirectoryInfo(strPath);
			if (!dirInf.Exists)
			{
				Debug.Log("Creating subdirectory");
				dirInf.Create();
			}
			else
			{
				FileInfo[] info = dirInf.GetFiles("*.scene");
				foreach (FileInfo f in info)
				{
					string strName = f.Name.Substring(0, f.Name.LastIndexOf('.'));

					byte[] bytes = System.Text.Encoding.UTF8.GetBytes(strName);
					strName = System.Text.Encoding.UTF8.GetString(bytes);

					UxCosmos.m_stlVMenus.Add(strName);
					UxCosmos.m_stlVEvents.Add(f.Name);
				}
			}

			string strComponent = "";
			stBase.get_string("UxCosmosMakeParentComponent_strV", ref strComponent);

			UxCosmos.m_strParent = strComponent;
			UxCosmos.ms_bRender = true;

			return 1;
		}


		public static int UxComponentDisable_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			string strCom = "";
			if (!stBase.get_string("UxComponentDisable_strF", ref strCom))
				return 0;

			StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
			MonoBehaviour uxCosmos = null;
			if (dstGlobal.get_ptr(sm_strMonoBehaviour, ref uxCosmos))
			{
				((UxCosmos)uxCosmos).ComponentHide(strCom);
				return 1;
			}

			return 0;
		}


		public static int UxComponentSelect_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			string strCom = "";
			if (!stBase.get_string("UxComponentSelect_strF", ref strCom))
				return 0;

			GameObject go = GameObject.Find(strCom);
			if (go == null)
				return 0;

			InputField input = go.GetComponent<InputField>();
			if (input == null)
			{
				TMPro.TMP_InputField tInput = go.GetComponent<TMPro.TMP_InputField>();
				if (tInput != null)
				{
					tInput.Select();
					//tInput.ActiveInputField();
				}
			}
			else
			{
				input.Select();
				//input.ActiveInputField();
			}

			return 1;
		}


		public static int UxComponentEnable_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			string strCom = "";
			if (!stBase.get_string("UxComponentEnable_strF", ref strCom))
				return 0;

			StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
			MonoBehaviour uxCosmos = null;
			if (dstGlobal.get_ptr(sm_strMonoBehaviour, ref uxCosmos))
			{
				((UxCosmos)uxCosmos).ComponentShow(strCom);
				return 1;
			}

			//if (!Scene.SceneObjectShow (strCom))
			//	return 0;

			string strMsg = "";
			if (!stBase.get_string("UxComponentEnableMessage_strV", ref strMsg))
			{
				strMsg = VLStateManager.param_variable_get_string(_pBase, _pEvent, _pContext, 0, IntPtr.Zero);
			}

			if (strMsg != null && strMsg.Length > 0)
			{
				strMsg = VLStateManager.translate(strMsg);
				//Scene.SceneObjectTextSet(strCom, strMsg);
			}

			return 1;
		}


		public static int UxCosmosMakeFolder_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			return 1;
		}


		public static int UxCosmosMakeDirMore_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			return 1;
		}

		//
		//public static int UxCosmosInFile_nIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		//{
		//	float fT = 0;
		//       int i = UxCosmos.CurPoint (s_fRadius, ref fT);

		//       if (i == 0 || i >= UxCosmos.m_stlVMenus.Count)
		//           return 0;

		//	StateDStructureValue stBase = new StateDStructureValue (_pBase);
		//	int nHash = 0;
		//	if (!stBase.get_int ("UxCosmosInFile_nIf", ref nHash))
		//		return 0;

		//	StateDStructureValue stValue = VLStateManager.variable_global_get ();

		//	if (!stValue.set_string(nHash, UxCosmos.m_stlVEvents [i]))
		//		return 0;

		//	return 1;
		//}


		public static int UxCosmosInFolder_nIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			return 1;
		}


		public static int UxCosmosInMore_nIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			return 1;
		}


		public static int UxInputFilename_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			return 1;
		}


		public static int UxCosmosScreenAdBound_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			int nValue = 0;
			if (!stBase.get_int("UxCosmosScreenAdBound_nF", ref nValue))
				return 0;

			ms_nScreenBoxDown = 0;
			ms_nScreenBoxUp = 0;

			ms_nScreenBoxDown = 100;
			/*
			if (nValue == 1) {
				if (Application.platform == RuntimePlatform.Android) {
					ms_nScreenBoxUp = 100;
				} else if (Application.platform == RuntimePlatform.IPhonePlayer){
					ms_nScreenBoxDown = 100;
				}
			} else if (nValue == 2) {
				
			} else {
				//#if UNITY_IPHONE
				//bool deviceIsIphoneX = UnityEngine.iOS.Device.generation == UnityEngine.iOS.DeviceGeneration.iPhoneX;
				//if (deviceIsIphoneX) {
					ms_nScreenBoxDown = 100;
				//}
				//#endif
			}
			//*/
			return 1;
		}


		public static int UxCosmosAppend_anF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			int[] anValue;
			anValue = stBase.get_int_arry("UxCosmosAppend_anF");
			if (anValue == null)
				return 0;

			int nHashEvent = VLStateManager.hash("strName");
			int nHashMenu = VLStateManager.hash("Comment");
			string strEvent = "", strMenu = "";

			//UxCosmos.m_stlVMenus.Clear ();
			//UxCosmos.m_stlVEvents.Clear ();
			for (int i = 0; i < anValue.Length; i++)
			{
				VLStateManager.enum_get("EnumUXID", anValue[i], nHashEvent, ref strEvent);
				if (VLStateManager.enum_get("EnumUXID", anValue[i], nHashMenu, ref strMenu))
					UxCosmos.m_stlVMenus.Add(strMenu);
				else
					UxCosmos.m_stlVMenus.Add(strEvent);
				UxCosmos.m_stlVEvents.Add(strEvent);
				UxCosmos.Updated();
			}

			UxCosmos.ms_bRender = true;
			return 1;
		}


		public static int UxCosmosCheckIn_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
			MonoBehaviour uxCosmos = null;
			if (dstGlobal.get_ptr(sm_strMonoBehaviour, ref uxCosmos))
			{
				((UxCosmos)uxCosmos).Click();
			}

			return 0;
		}


		public static int UxCosmosMake_anF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			int[] anValue;
			anValue = stBase.get_int_arry("UxCosmosMake_anF");
			if (anValue == null)
				return 0;

			int nHashEvent = VLStateManager.hash("strName");
			int nHashMenu = VLStateManager.hash("Comment");
			string strEvent = "", strMenu = "";

			//CircleMenuController.Instance.Close();
			UxCosmos.m_stlVMenus.Clear();
			UxCosmos.m_stlVIconPath.Clear();
			UxCosmos.m_stlVEvents.Clear();
			for (int i = 0; i < anValue.Length; i++)
			{
				VLStateManager.enum_get("EnumUXID", anValue[i], nHashEvent, ref strEvent);
				if (VLStateManager.enum_get("EnumUXID", anValue[i], nHashMenu, ref strMenu))
					UxCosmos.m_stlVMenus.Add(VLStateManager.translate(strMenu));
				else
					UxCosmos.m_stlVMenus.Add(VLStateManager.translate(strEvent));
				UxCosmos.m_stlVEvents.Add(strEvent);
			}

			string strComponent = "";
			stBase.get_string("UxCosmosMakeParentComponent_strV", ref strComponent);

			UxCosmos.m_strParent = strComponent;
			UxCosmos.ms_bRender = true;
			UxCosmos.Updated();
			return 1;
		}


		public static int UxCosmosClose_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			//StateDStructureValue stBase = new StateDStructureValue (_pBase);
			//CircleMenuController.Instance.Close();
			m_stlVMenus.Clear();
			UxCosmos.ms_bRender = true;
			return 1;
		}

		public static int VScriptCosmosMenuStates_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			//StateDStructureValue stBase = new StateDStructureValue (_pBase);

			StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
			MonoBehaviour uxCosmos = null;
			if (dstGlobal.get_ptr(sm_strMonoBehaviour, ref uxCosmos))
			{
				List<string> astrNames = new List<string>();

				string strName = "";
				int nIndex = 3;
				do
				{
					if (!VLStateManager.vscript_column_name_get(nIndex, ref strName))
						break;

					if ("var:" == strName.Substring(0, 4))
						break;
					nIndex++;
					astrNames.Add(strName);
				} while (true);

				((UxCosmos)uxCosmos).MenuListMake(astrNames);
				return 1;
			}

			return 0;
		}

		public static int VScriptCosmosMenu_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);

			int hash = 0;
			if (!stBase.get_int("VScriptCosmosMenu_varF", ref hash))
				return 0;

			string menus_str = "";
			StateDStructureValue variable_state = stBase.state_variable_get();
			if (!variable_state.get_string(hash, ref menus_str))
				return 0;

			StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
			MonoBehaviour uxCosmos = null;
			if (dstGlobal.get_ptr(sm_strMonoBehaviour, ref uxCosmos))
			{
				BaseFile paser = BaseFile.create();
				paser.asc_open(menus_str);
				paser.seperator_set(",");
				paser.delector_set(" ");
				paser.asc_read_line();

				List<string> cosmos_menus_str = new List<string>();
				string menu_str = "";
				while (paser.asc_read_string(ref menu_str))
				{
					menu_str = VLStateManager.translate(menu_str);
					cosmos_menus_str.Add(menu_str);
				}

				paser.delete();

				if (cosmos_menus_str.Count > 0)
				{
					((UxCosmos)uxCosmos).MenuListMake(cosmos_menus_str);
					return 1;
				}
			}

			return 0;
		}

		public static int VScriptCosmosMenu_astrF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			string menus_str = "";
			if (!stBase.get_string("VScriptCosmosMenu_astrF", ref menus_str))
				return 0;

			StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
			MonoBehaviour uxCosmos = null;
			if (dstGlobal.get_ptr(sm_strMonoBehaviour, ref uxCosmos))
			{
				BaseFile paser = BaseFile.create();
				paser.asc_open(menus_str);
				paser.seperator_set(",");
				paser.delector_set(" ");
				paser.asc_read_line();

				List<string> cosmos_menus_str = new List<string>();
				string menu_str = "";
				while (paser.asc_read_string(ref menu_str))
				{
					menu_str = VLStateManager.translate(menu_str);
					cosmos_menus_str.Add(menu_str);
				}

				paser.delete();

				if (cosmos_menus_str.Count > 0)
				{
					((UxCosmos)uxCosmos).MenuListMake(cosmos_menus_str);
					return 1;
				}
			}

			return 0;
		}


		public static int VScriptCosmosMenuUpdate_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
			MonoBehaviour uxCosmos = null;
			if (dstGlobal.get_ptr(sm_strMonoBehaviour, ref uxCosmos))
			{
				((UxCosmos)uxCosmos).UpdateCosmos();
				return 1;
			}

			return 0;
		}


		public static int VScriptFileBrowser_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			int nType = 2; //1:save 2:load
			if (!stBase.get_int("VScriptFileBrowser_nF", ref nType))
				return 0;

			string ext_str = VLStateManager.param_variable_get_string(_pBase, _pEvent, _pContext, 0, IntPtr.Zero);
			string default_str = VLStateManager.param_variable_get_string(_pBase, _pEvent, _pContext, 1, IntPtr.Zero);
			if (default_str != null)
			{
				int dot = default_str.LastIndexOf('.');
				if (dot != -1)
					default_str = default_str.Substring(0, dot);
			}

			string[] paths = new string[1];
			//int pathLength;

			// if (nType == 2)
			// {
			//     FileBrowser.ShowLoadDialog( ( paths ) 
			//                     => { 
			//                         string path2 = "";
			//                         Debug.Log( "Selected: " + paths[0] ); 
			//                         path2 = paths[0] + "/temp.data";

			//                         StateDStructureValue dstEvent = VLStateManager.event_state_make("Submit");
			//                         dstEvent.set_variable_string("TempString_strV", path2);
			//                         VLStateManager.event_post(dstEvent);
			//                     },
			//                     () => { 
			//                         Debug.Log( "Canceled" ); 
			//                         VLStateManager.event_state_post("Cancel");
			//                     },
			//                     FileBrowser.PickMode.Folders, false, null, null, "Select Folder", "Select" );

			//     //paths = StandaloneFileBrowser.OpenFilePanel("Open File", "", ext_str, false);

			//     pathLength = paths.Length;
			// }
			// else
			// {
			//     //paths[0] =  StandaloneFileBrowser.SaveFilePanel("Save File", "", default_str, ext_str);
			//     pathLength = paths[0].Length;
			// }

			// if (pathLength== 0)
			// {
			//     return 1;
			// }

			return 1;
		}


		public static int UxCamera2DNavigationStart_fF(IntPtr _base_pdsv, IntPtr _event_pdsv, IntPtr _group_pdsv,
			int _process_type_n)
		{
			GameObject camera_go = null;
			Camera camera_state_view_cam = null;

			StateDStructureValue base_dsv = new StateDStructureValue(_base_pdsv);
			float sensitive_f = 0;
			if (!base_dsv.get_float("UxCamera2DNavigationStart_fF", ref sensitive_f))
				return 0;

			StateDStructureValue variable_dsv = base_dsv.state_variable_get();
			if (!variable_dsv.get_ptr("UxComponentCurPtr", ref camera_go))
				return 0;
			camera_state_view_cam = camera_go.GetComponent<Camera>();
			StateDStructureValue event_dsv = new StateDStructureValue(_event_pdsv);

			int[] mouse_pos_an = null;
			mouse_pos_an = event_dsv.get_int_arry("BasePos_anV");
			int cnt_n = mouse_pos_an.Length;

			float zoom_f = camera_state_view_cam.orthographicSize;
			float[] navi_start_af = new float[cnt_n + 1 + 2];

			navi_start_af[0] = zoom_f;
			for (int i = 0; i < cnt_n; i++)
				navi_start_af[i + 1] = mouse_pos_an[i];

			Vector3 pos = camera_state_view_cam.transform.localPosition;
			navi_start_af[cnt_n + 1] = pos.x;
			navi_start_af[cnt_n + 2] = pos.y;
			variable_dsv.set_variable("CameraNaviStart_af", navi_start_af);
			return 0;
		}


		public static int UxCamera2DNavigationUpdate_nF(IntPtr _base_pdsv, IntPtr _event_pdsv, IntPtr _group_pdsv,
			int _process_type_n)
		{
			GameObject camera_go = null;
			Camera camera_state_view_cam = null;

			StateDStructureValue base_dsv = new StateDStructureValue(_base_pdsv);
			float sensitive_f = 0;
			if (!base_dsv.get_float("UxCamera2DNavigationUpdate_nF", ref sensitive_f))
				return 0;

			StateDStructureValue variable_dsv = base_dsv.state_variable_get();
			if (!variable_dsv.get_ptr("UxComponentCurPtr", ref camera_go))
				return 0;
			camera_state_view_cam = camera_go.GetComponent<Camera>();
			StateDStructureValue event_dsv = new StateDStructureValue(_event_pdsv);
			int count_n = 0;
			float[] navi_start_af = variable_dsv.get_float_arry("CameraNaviStart_af", ref count_n);
			int[] mouse_pos_an = event_dsv.get_int_arry("BasePos_anV");

			float zoom_f; // = camera_state_view_cam.orthographicSize;

			int[] mouse_pos_org_an = new int[navi_start_af.Length - 1 - 2];

			zoom_f = navi_start_af[0];
			for (int i = 1; i < navi_start_af.Length - 2; i++)
				mouse_pos_org_an[i - 1] = (int)navi_start_af[i];
			Vector3 pos = camera_state_view_cam.transform.localPosition;
			pos.x = navi_start_af[navi_start_af.Length - 2];
			pos.y = navi_start_af[navi_start_af.Length - 1];

			if (mouse_pos_an.Length >= 4)
			{
				float zoom_delta_f, zoom_org_f;
				Vector3 pos1, pos2;
				pos1 = new Vector3(mouse_pos_org_an[0], mouse_pos_org_an[1], 0);
				pos2 = new Vector3(mouse_pos_org_an[2], mouse_pos_org_an[3], 0);
				zoom_org_f = (pos1 - pos2).magnitude;
				pos1 = new Vector3(mouse_pos_an[0], mouse_pos_an[1], 0);
				pos2 = new Vector3(mouse_pos_an[2], mouse_pos_an[3], 0);
				zoom_delta_f = (pos1 - pos2).magnitude - zoom_org_f;

				camera_state_view_cam.orthographicSize = zoom_f + zoom_delta_f;
			}
			else
			{
				zoom_f = camera_state_view_cam.orthographicSize / 200.0f;

				pos.x -= (mouse_pos_an[0] - mouse_pos_org_an[0]) * zoom_f;
				pos.y -= (mouse_pos_an[1] - mouse_pos_org_an[1]) * zoom_f;

				int x, y;
				x = (int)pos.x;
				y = (int)pos.y;
				pos.x = x;
				pos.y = y;
				camera_state_view_cam.transform.localPosition = pos;
			}

			return 0;
		}


		public static int UxCameraEnable_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			string name_str = "";
			if (!stBase.get_string("UxCameraEnable_strF", ref name_str))
				return 0;

			StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
			MonoBehaviour uxCosmos = null;
			if (dstGlobal.get_ptr(sm_strMonoBehaviour, ref uxCosmos))
			{
				((UxCosmos)uxCosmos).CameraEnable(name_str);
				return 1;
			}

			return 0;
		}


		public static int UxCameraDisable_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			string name_str = "";
			if (!stBase.get_string("UxCameraDisable_strF", ref name_str))
				return 0;

			StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
			MonoBehaviour uxCosmos = null;
			if (dstGlobal.get_ptr(sm_strMonoBehaviour, ref uxCosmos))
			{
				((UxCosmos)uxCosmos).CameraDisable(name_str);
				return 1;
			}

			return 0;
		}

		public static int UxCosmosMenuSearch_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			int hash = 0;
			if (!stBase.get_int("UxCosmosMenuSearch_varF", ref hash))
				return 0;

			string search = "";
			StateDStructureValue variable_state = stBase.state_variable_get();
			List<string> str_a = null;
			if (!variable_state.get_strings(hash, ref str_a))
				return 0;

			if (str_a.Count > 0)
				search = str_a[str_a.Count - 1];
			StateDStructureValue dstGlobal = VLStateManager.variable_global_get();
			MonoBehaviour uxCosmos = null;
			if (!dstGlobal.get_ptr(sm_strMonoBehaviour, ref uxCosmos))
				return 0;
			((UxCosmos)uxCosmos).MenuSearch(search);

			if (str_a.Count > 1)
			{
				string str_cur = "";
				//str_cur = str_a[0];
				for (int i = 0; i < str_a.Count - 1; i++)
					str_cur += str_a[i] + ", ";

				variable_state.set_variable_string("TempString2_strV", str_cur);
			}
			else
			{
				variable_state.set_ptr(VLStateManager.hash("TempString2_strV"), IntPtr.Zero, 0);
			}

			return 0;
		}

		public static void StateFuncRegist()
		{
			VLStateManager.ProcessReg("UxCosmosMenuSearch_varF", UxCosmosMenuSearch_varF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("VScriptFileBrowser_nF", VScriptFileBrowser_nF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("VScriptCosmosMenuUpdate_nF", VScriptCosmosMenuUpdate_nF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("VScriptCosmosMenuStates_nF", VScriptCosmosMenuStates_nF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("VScriptCosmosMenu_astrF", VScriptCosmosMenu_astrF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("VScriptCosmosMenu_varF", VScriptCosmosMenu_varF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);

			VLStateManager.ProcessReg("UxCameraEnable_strF", UxCameraEnable_strF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxCameraDisable_strF", UxCameraDisable_strF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);

			VLStateManager.ProcessReg("UxCosmosScreenAdBound_nF", UxCosmosScreenAdBound_nF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxCosmosAppend_anF", UxCosmosAppend_anF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxCosmosMake_anF", UxCosmosMake_anF, "VScriptEditor/Assets/Scripts/UxCosmos.cs",
				0);
			//VLStateManager.ProcessReg("UxCosmosIconMake_anF", UxCosmosIconMake_anF, "VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxCosmosClose_nF", UxCosmosClose_nF, "VScriptEditor/Assets/Scripts/UxCosmos.cs",
				0);
			VLStateManager.ProcessReg("UxCosmosMakeDir_strF", UxCosmosMakeDir_strF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxCosmosMakeFolder_nF", UxCosmosMakeFolder_nF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxCosmosMakeDirMore_strF", UxCosmosMakeDirMore_strF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			//VLStateManager.ProcessReg ("UxCosmosInFile_nIf", UxCosmosInFile_nIf, "VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxCosmosInFolder_nIf", UxCosmosInFolder_nIf,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxCosmosInMore_nIf", UxCosmosInMore_nIf,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxCosmosCheckIn_nF", UxCosmosCheckIn_nF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxInputFilename_strF", UxInputFilename_strF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			//VLStateManager.ProcessReg ("F3DBoardFileLoad_nF", F3DBoardFileLoad_nF, "VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			//VLStateManager.ProcessReg ("F3DBoardFileSave_nF", F3DBoardFileSave_nF, "VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			//VLStateManager.ProcessReg ("F3DBoardFileDelete_nF", F3DBoardFileDelete_nF, "VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxComponentEnable_strF", UxComponentEnable_strF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxComponentSelect_strF", UxComponentSelect_strF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxComponentDisable_strF", UxComponentDisable_strF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxComponentToVariable_strF", UxComponentToVariable_strF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxComponentInputSetFromVariable_strF", UxComponentInputSetFromVariable_strF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxComponentTextSet_strF", UxComponentTextSet_strF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxComponentSizeAdapt_strF", UxComponentSizeAdapt_strF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxComponentPositionSet_strF", UxComponentPositionSet_strF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxComponentPositionYSet_strF", UxComponentPositionYSet_strF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxComponentCreate_strF", UxComponentCreate_strF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0); // copy component on variable UxComponet of State.
			VLStateManager.ProcessReg("UxComponentAnimatorAttach_strF", UxComponentAnimatorAttach_strF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0); // attach specified animator
			VLStateManager.ProcessReg("UxComponentAnimatorUpdate_strF", UxComponentAnimatorUpdate_strF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0); // update variable on animator
			VLStateManager.ProcessReg("UxComponentAnimatorUpdate_nF", UxComponentAnimatorUpdate_nF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0); // update variable on animator
			VLStateManager.ProcessReg("UxComponentRectAdapt_strF", UxComponentRectAdapt_strF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0); // update variable on animator
			VLStateManager.ProcessReg("UxComponentDestory_nF", UxComponentDestory_nF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxComponentTextSet_nF", UxComponentTextSet_nF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxComponentTransformUpdate_strF", UxComponentTransformUpdate_strF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxComponentCurToVariable_strF", UxComponentCurToVariable_strF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxComponentPosSet_afF", UxComponentPosSet_afF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);

			VLStateManager.ProcessReg("UxCamera2DNavigationStart_fF", UxCamera2DNavigationStart_fF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0); //
			VLStateManager.ProcessReg("UxCamera2DNavigationUpdate_nF", UxCamera2DNavigationUpdate_nF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0); //

			VLStateManager.ProcessReg("UxScreenSizeSmaller_nIf", UxScreenSizeSmaller_nIf,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			//VLStateManager.ProcessReg ("F3DBoardPrimitiveTextSet_nF", F3DBoardPrimitiveTextSet_nF, "VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			//VLStateManager.ProcessReg ("F3DBoardPlayerSetBacknumber_nF", F3DBoardPlayerSetBacknumber_nF, "VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			//VLStateManager.ProcessReg ("F3DBoardPlayerSetName_nF", F3DBoardPlayerSetName_nF, "VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxOpenURL_strF", UxOpenURL_strF, "VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxLanguageChange_nF", UxLanguageChange_nF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxDeviceInforDisplay_nF", UxDeviceInforDisplay_nF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxScreenSizeBigger_nIf", UxScreenSizeBigger_nIf,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxScreenRateBigger_nIf", UxScreenRateBigger_nIf,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxRecordScreen_nF", UxRecordScreen_nF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxSystemVariableGetStr_nF", UxSystemVariableGetStr_nF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxSystemVariableGet_nF", UxSystemVariableGet_nF,
				"VScriptEditor/Assets/Scripts/UxCosmos.cs", 0);
			VLStateManager.ProcessReg("UxFileOpen_strF", UxFileOpen_strF, "VScriptEditor/Assets/Scripts/UxCosmos.cs",
				0);
			VLStateManager.ProcessReg("UxFileSave_strF", UxFileSave_strF, "VScriptEditor/Assets/Scripts/UxCosmos.cs",
				0);

			VLStateManager.ProcessReg("UxDoExist_nIf", UxDoExist_nIf, "VScriptEditor/Assets/Scripts/UxCosmos.cs",
				0); // 20200324 jhm made
			VLStateManager.ProcessReg("UxDoNotExist_nIf", UxDoNotExist_nIf, "VScriptEditor/Assets/Scripts/UxCosmos.cs",
				0); // 20200325 jhm made

			/*
			 * 
			UxComponentDestory_nF
			UxComponentTextSet_nF
			UxComponentCreate_strF
			UxComponentAnimatorAttach_strF
			UxComponentAnimatorUpdate_strF
			UxComponentAnimatorUpdate_nF
			 */
		}

	}
}