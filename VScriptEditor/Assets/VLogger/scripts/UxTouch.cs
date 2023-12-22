using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System;
using System.Collections.Generic;
using TMPro;

// public static class ClipboardExtension
// {
//     /// <summary>
//     /// Puts the string into the Clipboard.
//     /// </summary>
//     /// <param name="str"></param>
//     public static void CopyToClipboard(this string str)
//     {
//         var textEditor = new TextEditor();
//         textEditor.text = str;
//         textEditor.SelectAll();
//         textEditor.Copy();
//     }
// }
namespace StateSystem
{
	public class UxTouch : MonoBehaviour
	{

		int m_cursor_befor = 0;
		int nHashUxPointPush;
		int nHashUxPointRelease;
		int nHashUxPointMove;

		//public GameObject	m_objExitButton;
		static public float ms_fVisualScale = 80;
		static public int ms_nHashFocus;
		static bool ms_bAppend = false;
		static public bool ms_mouse_move_update_enable = true;

		private void Awake()
		{

		}

		// Use this for initialization
		public static UxTouch Instance = null;

		void Start()
		{
			Instance = this;

			nHashUxPointPush = VLStateManager.hash("UxPointPushMiddle");
			nHashUxPointRelease = VLStateManager.hash("UxPointReleaseMiddle");
			nHashUxPointMove = VLStateManager.hash("UxPointMoveMiddle");

			m_stlVTouch = new List<Vector3>();
			m_stlVTouchMiddle = new List<Vector3>();
			m_stlVTouchCur = new List<Vector3>();

		}

		[Tooltip("Mouse cursor 0: push, 1: object over.")]
		public Texture2D[] m_icons_texture_a;

		public CursorMode cursorMode = CursorMode.Auto;
		public Vector2 hotSpot = Vector2.zero;

		public void cursor_set(int type)
		{
			if (m_cursor_befor == type)
				return;
			m_cursor_befor = type;

			if (type != 0)
				Cursor.SetCursor(m_icons_texture_a[type - 1], hotSpot, cursorMode);
			else
				Cursor.SetCursor(null, Vector2.zero, cursorMode);
		}

		public void onSave()
		{
			VLStateManager.event_state_process("UxButtonSaveClick");
		}

		public void onAppendDown()
		{
			ms_nHashFocus = VLStateManager.hash("Append");
			VLStateManager.event_state_process("UxAppendPush");
		}

		public void onAppendClick()
		{
			ms_nHashFocus = VLStateManager.hash("Append");
			VLStateManager.event_state_process("UxAppendClick");
		}

		public void onButtonStateCancelClick()
		{
			VLStateManager.event_state_process("UxButtonStateCancelClick");
		}

		public void onBackward()
		{
			VLStateManager.event_state_process("PlayStepBackward");
		}

		public void OnCastStateEvent(string _strMsg)
		{
			Update();

			GameObject obj = GameObject.Find("Canvas/InputMsg");
			//InputField txt;

			if (_strMsg == "mmm")
			{

				if (Input.GetButtonDown("Submit")
				    || Input.GetKeyDown(KeyCode.Return)
				    || Input.GetKeyDown(KeyCode.KeypadEnter))
					VLStateManager.message(1, "Submit----- Befor\n");
			}

			if (_strMsg == "UxInputTextEnd")
			{
				if (Input.GetButtonDown("Submit")
				    || Input.GetKeyDown(KeyCode.Return)
				    || Input.GetKeyDown(KeyCode.KeypadEnter))
					VLStateManager.message(1, "Submit-----\n");
			}

			VLStateManager.message(1, "OnCastStateEvent----------" + _strMsg);
			VLStateManager.event_state_post(_strMsg);
		}

		public void onForward()
		{
			VLStateManager.event_state_process("PlayStepForward");
		}

		public void onUndo()
		{
			VLStateManager.event_state_process("UxButtonUndo");
		}

		public void onRedo()
		{
			VLStateManager.event_state_process("UxButtonRedo");
		}

		public void onHomeAdd()
		{
			VLStateManager.event_state_process("UxButtonHomeAdd");
		}

		public void onAwayAdd()
		{
			VLStateManager.event_state_process("UxButtonAwayAdd");
		}

		public void onToggelAppend()
		{
			if (ms_bAppend)
				ms_bAppend = false;
			else
				ms_bAppend = true;
		}

		//static bool ms_b3DView = false;
		public void on3DCamera()
		{
			VLStateManager.event_state_process("UxButton3DToggle");
		}

		public void onJog()
		{
			VLStateManager.event_state_process("UxButtonJog");
		}

		public static void frame_trace_up(GameObject _go)
		{
			string name = _go.name;

			Vector3 pos = _go.transform.localPosition;
			Vector3 scale = _go.transform.localScale;

			if (_go.transform.parent == null)
				return;

			frame_trace_up(_go.transform.parent.gameObject);
		}

		public static int ms_nCurIdBefor = 0, ms_nCurIndexStep = 0;
		static bool m_bMouse1 = false, m_bMouse2 = false;
		public static bool ms_bMove = false;
		public static Vector3 m_v3Before; // to support mouse zoom
		public static Vector3 ms_v3Befor, ms_v3BeforMiddle;
		public static List<Vector3> m_stlVTouch, m_stlVTouchCur, m_stlVTouchMiddle;

		void StateEventCast(int _nEvent)
		{
			StateDStructureValue stEvent;

			int nCnt = m_stlVTouchMiddle.Count * 2;
			int[] anValues = new int[nCnt];
			for (int i = 0; i < m_stlVTouchMiddle.Count; i++)
			{
				anValues[i * 2] = (int)m_stlVTouchMiddle[i].x;
				anValues[i * 2 + 1] = (int)m_stlVTouchMiddle[i].y;
			}

			if (_nEvent == nHashUxPointRelease)
			{
				nCnt = 2;
				anValues = new int[nCnt];
				anValues[0] = (int)ms_v3BeforMiddle.x;
				anValues[1] = (int)ms_v3BeforMiddle.y;
			}

			stEvent = VLStateManager.create_event("BaseStateEventGlobal");

			stEvent.add_int_arry("BasePos_anV", anValues);
			stEvent.set_int("BaseStateEventGlobal", _nEvent);
			VLStateManager.event_process(stEvent.m_pDStructureValue);

		}

		void UpdateMouseScroll()
		{
			//UxPointScroll (482229611)
			if (Input.mouseScrollDelta.sqrMagnitude > 0)
			{
				StateDStructureValue stEvent;

				int nCnt = 2;
				int[] anValues = new int[nCnt];

				anValues[0] = (int)Input.mouseScrollDelta.x;
				anValues[1] = (int)Input.mouseScrollDelta.y;
				stEvent = VLStateManager.event_state_make("UxPointScroll");
				stEvent.add_int_arry("BasePos_anV", anValues);
				VLStateManager.event_process(stEvent.m_pDStructureValue);
			}
		}

		void UpdateKeyboard()
		{
			if (Input.GetKey("enter"))
			{
				// marker
				System.Diagnostics.Debug.WriteLine("z:\\coint\\trunk\\lib\\test.ini");
				VLStateManager.event_state_post("Submit");
				VLStateManager.message(1, "$$$$$$$$$$$$$ enter\n");
			}

			if (Input.GetKeyDown(KeyCode.KeypadEnter))
			{
				VLStateManager.event_state_post("Submit");
				VLStateManager.message(1, "$$$$$$$$$$$$$ kpad enter\n");
			}

			if (Input.GetKeyDown(KeyCode.Return))
			{
				System.Diagnostics.Debug.WriteLine("Z:\\coint\\trunk\\PowerCad\\Assets\\Scripts\\UxTouch.cs 100");
				Debug.Log("message (at Assets/Scripts/UxTouch.cs:100)");
				VLStateManager.event_state_post("Submit");
				VLStateManager.message(1, "$$$$$$$$$$$$$ key enter\n");
			}

			if (Input.GetKeyDown(KeyCode.Escape))
			{
				VLStateManager.event_state_post("Cancel");
				VLStateManager.message(1, "$$$$$$$$$$$$$ key esc\n");
			}

			if (Input.GetKeyDown(KeyCode.End))
			{
				VLStateManager.message(1, "$$$$$$$$$$$$$ key end\n");
			}

			if (Input.GetKey("backspace"))
			{
				VLStateManager.message(1, "$$$$$$$$$$$$$ backspace\n");
			}

			if (Input.GetKey("q"))
			{
				//VLStateManager.message (1, "$$$$$$$$$$$$$ a\n");
				VLStateManager.event_state_post("UxKeyboardPushQ");
			}

			if (Input.GetKeyDown(KeyCode.Escape))
				VLStateManager.event_state_post("KeyInputBack");
			//Application.Quit(); 

		}

		void UpdateScreen()
		{
			float fSBase;

			if (Screen.height < Screen.width)
				fSBase = Screen.height;
			else
				fSBase = Screen.width;
			//RenderStarter.m_strMessage = fSBase.ToString();

			if (fSBase > 1280)
				fSBase = 1280;

			ms_fVisualScale = 80 * fSBase / 700;
		}

		bool UpdateTouch()
		{
			if (m_stlVTouchMiddle == null)
				return true;
			//StateDStructureValue stEvent;
			int nPreTouchCnt = m_stlVTouchMiddle.Count;
			int nCurTouchCnt = Input.touchCount;

			if (nPreTouchCnt > 0)
				ms_v3BeforMiddle = m_stlVTouchMiddle[0];

			if ((nPreTouchCnt == 0 && nCurTouchCnt > 0)
			    || (nCurTouchCnt == 0 && !m_bMouse1 && !m_bMouse2
			        && (Input.GetButton("Fire1") || Input.GetButton("Fire2") || Input.GetButton("Fire3")))
			   )
			{
				//ActorCutter cut = new ActorCutter();
				//cut.init();
				//Scene.m_ActorStack.DoCutter(cut);

				int nFire1 = 0, nFire2 = 0;
				if (Input.GetButton("Fire1"))
					nFire1 = 1;
				if (Input.GetButton("Fire2"))
					nFire2 = 1;
				VLStateManager.message(1,
					"Push Cutter, Pre: " + nPreTouchCnt + " Cur: " + nCurTouchCnt + " Fire1: " + nFire1 + " Fire2: " +
					nFire2);
			}

			//Text uniText = m_objExitButton.GetComponentInChildren<Text>();
			//uniText.text = ms_strExitButtonName;

			if (!m_bMouse1 && !m_bMouse2)
			{
				m_stlVTouchCur.Clear();
				for (int i = 0; i < nCurTouchCnt; i++)
					m_stlVTouchCur.Add(Input.GetTouch(i).position);

				nCurTouchCnt = m_stlVTouchCur.Count;

				if (nCurTouchCnt != nPreTouchCnt)
				{
					m_stlVTouchMiddle.Clear();
					for (int i = 0; i < nCurTouchCnt; i++)
						m_stlVTouchMiddle.Add(m_stlVTouchCur[i]);
				}

				//if(!UxCosmos.InButton ()){
				// If there are two touches on the device...
				if (nCurTouchCnt > nPreTouchCnt)
				{
					m_v3Before = m_stlVTouchMiddle[0];
					StateEventCast(nHashUxPointPush);
				}
				else if (nCurTouchCnt < nPreTouchCnt)
				{
					StateEventCast(nHashUxPointRelease);
				} // end of if(nCurTouchCnt)
				//}

				bool bPostMove = false;
				for (int i = 0; i < nCurTouchCnt; i++)
				{
					Vector3 touch = m_stlVTouchCur[i];
					if (touch != m_stlVTouchMiddle[i])
					{
						bPostMove = true;
						m_stlVTouchMiddle[i] = touch;
					}
				}

				if (bPostMove)
				{
					m_v3Before = m_stlVTouchMiddle[0];
					ms_bMove = true;
					StateEventCast(nHashUxPointMove);
				}
				else
				{
					ms_bMove = false;
				}
			}

			if (nCurTouchCnt != 0) // If touch device is working, Fire event shouldn't be processed.
				return true;
			return false;
		}

		void UpdateMouse()
		{

			// support device what does now support touch input.
			if (Input.GetButton("Fire1"))
			{
				Vector3 pos;
				pos = Input.mousePosition;
				m_v3Before = pos;

				if (m_bMouse1 == false)
				{
					m_bMouse1 = true;
					m_stlVTouchMiddle.Clear();
					m_stlVTouchMiddle.Add(pos);
					//m_stlVTouchMiddle.Add (pos);
					//if(!UxCosmos.InButton ()){
					StateEventCast(nHashUxPointPush);
					//}
				}
				else if (m_stlVTouchMiddle.Count > 0)
				{
					if (pos != m_stlVTouchMiddle[0])
					{
						m_stlVTouchMiddle[0] = pos;
						//m_stlVTouchMiddle[1] = pos;
						ms_bMove = true;
						StateEventCast(nHashUxPointMove);
					}
					else
					{
						ms_bMove = false;
					}
				}
			}
			else
			{
				if (m_bMouse1 == true)
				{
					if (m_stlVTouchMiddle.Count > 0)
					{
						m_stlVTouchMiddle.Clear();
						StateEventCast(nHashUxPointRelease);
					}

					m_bMouse1 = false;
				}

				if (Input.GetButton("Fire2") || Input.GetButton("Fire3"))
				{
					Vector3 pos, v3Dif;
					v3Dif.x = 0;
					v3Dif.y = 1000;
					v3Dif.z = 0;
					pos = Input.mousePosition;

					if (m_bMouse2 == false)
					{
						m_bMouse2 = true;
						m_v3Before = pos;
						m_stlVTouchMiddle.Clear();
						m_stlVTouchMiddle.Add(pos + v3Dif);
						m_stlVTouchMiddle.Add(pos - v3Dif);
						StateEventCast(nHashUxPointPush);
					}
					else if (m_stlVTouchMiddle.Count > 0)
					{
						if (m_v3Before.y != pos.y
						    || m_v3Before.x != pos.x
						   )
						{
							v3Dif = pos - m_v3Before;
							m_stlVTouchMiddle[0] += v3Dif;
							if (Input.GetButton("Fire3"))
								m_stlVTouchMiddle[1] += v3Dif;
							else
								m_stlVTouchMiddle[1] -= v3Dif;
							ms_bMove = true;
							StateEventCast(nHashUxPointMove);
						}
						else
						{
							ms_bMove = false;
						}

						m_v3Before = pos;
					}
				}
				else
				{
					if (m_bMouse2 == true)
					{
						if (m_stlVTouchMiddle.Count > 0)
						{
							m_stlVTouchMiddle.Clear();
							StateEventCast(nHashUxPointRelease);
						}

						m_bMouse2 = false;
					}
					else
					{
						Vector3 pos = Input.mousePosition;
						if (pos != m_v3Before && m_stlVTouchMiddle.Count == 0)
						{
							if (ms_mouse_move_update_enable)
							{
								m_stlVTouchMiddle.Add(pos);
								StateEventCast(nHashUxPointMove);
								m_stlVTouchMiddle.Clear();
							}

							m_v3Before = pos;
						}
					}
				} // end of else for Fire2
			} // end of elase for Fire1

		}

		// Update is called once per frame
		void Update()
		{

			UpdateMouseScroll();
			UpdateKeyboard();
			UpdateScreen();

			//		Animator ani = m_objExitButton.GetComponent<Animator>();
			//		if (ms_nExitButtonState == 1) {
			//			ani.SetBool("bVisible", true);
			//		} else {
			//			ani.SetBool("bVisible", false);
			//		}

			if (UpdateTouch())
				return;

			UpdateMouse();
		}

		static public void _transit_mouse_record_before(IntPtr _pEvent, IntPtr _pContext)
		{
			StateDStructureValue stEvent = new StateDStructureValue(_pEvent);
			StateDStructureValue stContext = new StateDStructureValue(_pContext);
			int[] anValues;

			if (_pEvent == IntPtr.Zero)
				anValues = stContext.get_int_arry("BasePos_anV");
			else
			{
				anValues = stEvent.get_int_arry("BasePos_anV");
				if (anValues == null)
					anValues = stContext.get_int_arry("BasePos_anV");
			}

			if (anValues == null)
				return;

			if (UxTouch.m_stlVTouch.Count > 0)
				UxTouch.ms_v3Befor = UxTouch.m_stlVTouch[0];
		}

		static public void _transit_mouse_pos(IntPtr _pEvent, StateDStructureValue _stContext)
		{
			StateDStructureValue stEvent = new StateDStructureValue(_pEvent);
			int[] anValues;

			if (_pEvent == IntPtr.Zero)
				anValues = _stContext.get_int_arry("BasePos_anV");
			else
			{
				anValues = stEvent.get_int_arry("BasePos_anV");
				if (anValues == null)
					anValues = _stContext.get_int_arry("BasePos_anV");
			}

			if (anValues == null)
				return;

			UxTouch.m_stlVTouch.Clear();
			for (int i = 0; i < anValues.Length / 2; i++)
			{
				Vector3 v3 = new Vector3((float)anValues[i * 2], (float)anValues[i * 2 + 1], 0);
				UxTouch.m_stlVTouch.Add(v3);
			}
		}


		public static int UxMouseMoveDistRateLE_fIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			float fDistRate = 0;
			if (!stBase.get_float("UxMouseMoveDistRateLE_fIf", ref fDistRate))
				return 0;

			Vector3 v3ScreenLength = new Vector3(Screen.width, Screen.height);

			if (UxTouch.m_stlVTouchMiddle.Count == 0)
				return 0;

			Vector3 v3Speed = UxTouch.ms_v3BeforMiddle - UxTouch.m_stlVTouchMiddle[0];

			if (fDistRate >= (v3Speed.sqrMagnitude / v3ScreenLength.sqrMagnitude * 10000.0f))
				return 1;

			return 0;
		}


		public static int UxMouseMoveDistanceGE_nIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			int nDist = 0;
			if (!stBase.get_int("UxMouseMoveDistanceGE_nIf", ref nDist))
				return 0;

			if (UxTouch.m_stlVTouchMiddle.Count == 0)
				return 0;

			Vector3 v3Speed = UxTouch.ms_v3BeforMiddle - UxTouch.m_stlVTouchMiddle[0];

			if (nDist <= (int)v3Speed.magnitude)
				return 1;

			return 0;
		}


		public static int UxMouseCountButton_nIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			int nBool = 0;
			if (!stBase.get_int("UxMouseCountButton_nIf", ref nBool))
				return 0;

			if (nBool > 0)
			{
				if (UxTouch.m_stlVTouchMiddle.Count > 0 && nBool == 1)
					return 1;
				if (UxTouch.m_stlVTouchMiddle.Count == nBool && nBool > 1)
					return 1;
			}
			else
			{
				if (UxTouch.m_stlVTouchMiddle.Count == 0)
					return 1;
			}

			return 0;
		}


		public static int UxMouseCastPointMove_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			//_transit_mouse_pos (_pEvent, _pContext);
			//VLStateManager.event_state_post ("UxPointMove");

			//if (UxJogVertical.ms_stlVJogs != null){
			//	for (int i=0; i<UxJogVertical.ms_stlVJogs.Count; i++)
			//		UxJogVertical.ms_stlVJogs[i].CheckJog();
			//}
			return 1;
		}

		public static int UxMouseCastPointPush_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			//_transit_mouse_pos (_pEvent, _pContext);
			//VLStateManager.event_state_post ("UxPointPush");
			return 1;
		}

		public static int UxMouseCastPointRelease_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			//_transit_mouse_pos (_pEvent, _pContext);
			//VLStateManager.event_state_post ("UxPointRelease");
			return 1;
		}


		public static int UxAppendMode_nIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			if (ms_bAppend == true)
				return 1;
			return 0;
		}


		public static int UxPlatformCheck_anIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			int[] anValue;
			anValue = stBase.get_int_arry("UxPlatformCheck_anIf");

			for (int i = 0; i < anValue.Length; i++)
			{
				switch (anValue[i])
				{
					case 1: // ios;
						if (Application.platform == RuntimePlatform.IPhonePlayer)
							return 1;
						break;
					case 2: // android;
						if (Application.platform == RuntimePlatform.Android)
							return 1;
						break;
					case 3: // mac;
						if (Application.platform == RuntimePlatform.OSXPlayer
						    || Application.platform == RuntimePlatform.OSXEditor)
							return 1;
						break;
					case 4: // windows;
						if (Application.platform == RuntimePlatform.WindowsPlayer
						    || Application.platform == RuntimePlatform.WindowsEditor)
							return 1;
						break;
				}
			}

			return 0;
		}


		public static int UxSystemCheckSystem_anIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);

			int[] anSystem = stBase.get_int_arry("UxSystemCheckSystem_anIf");

			for (int i = 0; i < anSystem.Length; i++)
			{
				switch (anSystem[i])
				{
					case 1:
						if (Application.platform == RuntimePlatform.IPhonePlayer)
							return 1;
						break;
					case 2:
						if (Application.platform == RuntimePlatform.Android)
							return 1;
						break;
					case 3:
						if (Application.platform == RuntimePlatform.OSXPlayer
						    || Application.platform == RuntimePlatform.OSXEditor)
							return 1;
						break;
					case 4:
						if (Application.platform == RuntimePlatform.WindowsPlayer
						    || Application.platform == RuntimePlatform.WindowsEditor)
							return 1;
						break;
				}
			}

			return 0;
		}


		public static int UxSystemCheckIsMobile_nIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			if (SystemInfo.deviceType == DeviceType.Handheld)
				return 1;
			return 0;
		}


		public static int UxCommandPushCutter_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			//ActorCutter cut = new ActorCutter();
			//cut.init();
			//Scene.m_ActorStack.DoCutter(cut);
			return 1;
		}


		public static int UxCommandCancel_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			//Scene.m_ActorStack.Cancel ();
			return 1;
		}


		public static int UxCommandCuttorLastRemove_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			//Scene.m_ActorStack.CutterLastRemove ();
			return 1;
		}


		public static int UxSystemTerminate_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			Application.Quit();
			return 1;
		}

		public bool device_id_check(string _strID)
		{
			if (_strID == null)
				return false;
			if (_strID.Length == 0)
				return false;

			int nCnt = 0;
			for (int i = 0; i < _strID.Length; i++)
			{
				if (_strID[i] > 32)
					nCnt++;
			}

			if (nCnt == 0)
				return false;

			return true;
		}


		public static int UxDeviceGet_anF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			int nHash = 0;
			if (!stBase.get_int("UxDeviceGet_anF", ref nHash))
			{
				return 0;
			}

			String strDeviceId;
#if UNITY_IPHONE
		strDeviceId = UnityEngine.iOS.Device.advertisingIdentifier;
		if(device_id_check(strDeviceId))
            strDeviceId = "ApA:" + strDeviceId;
        else
		    strDeviceId = "ApD:" + SystemInfo.deviceUniqueIdentifier;
#else
			if (SystemInfo.deviceType == DeviceType.Desktop)
			{
				// PC System
				strDeviceId = "DEK:" + SystemInfo.deviceUniqueIdentifier;
			}
			else
			{
				// Android
				strDeviceId = SystemInfo.deviceUniqueIdentifier;
			}
#endif

			StateDStructureValue pdstVariable = VLStateManager.variable_global_get();
			pdstVariable.set_variable_string(nHash, strDeviceId);

			return 1;
		}


		public static int UxCommandUndo_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			//Scene.m_ActorStack.Undo ();
			return 1;
		}


		public static int UxCommandRedo_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			//Scene.m_ActorStack.Redo ();
			return 1;
		}


		public static int UxPointInCheckPos_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			_transit_mouse_pos(_pEvent, stBase.state_variable_get());
			//CollisionCheck ();
			return 1;
		}

		static public bool IsInValue(float _start, float _end, float _value)
		{
			if (_end > _start)
			{
				if (_start <= _value && _value <= _end)
					return true;
			}
			else
			{
				if (_end <= _value && _value <= _start)
					return true;
			}

			return false;
		}

		static public bool IsInRect(Transform _transform, Vector3 _v3Pos, GameObject _canvas_go)
		{
			RectTransform rtTr = _transform.GetComponent<RectTransform>();

			float scale_f = 1;

			if (_canvas_go != null)
			{
				Canvas mainCanvas = _canvas_go.GetComponent<Canvas>();
				scale_f = mainCanvas.scaleFactor;
			}

			scale_f *= rtTr.localScale.x;
//         if (IsInValue(rt.position.x + rt.rect.xMin, rt.position.x + rt.rect.xMin + rt.rect.width, _v3Pos.x)
//             && IsInValue(rt.position.y + rt.rect.yMin, rt.position.y + rt.rect.yMin + rt.rect.height, _v3Pos.y))
			if (IsInValue(rtTr.position.x + (rtTr.rect.xMin * scale_f),
				    rtTr.position.x + ((rtTr.rect.xMin + rtTr.rect.width) * scale_f), _v3Pos.x)
			    && IsInValue(rtTr.position.y + (rtTr.rect.yMin * scale_f),
				    rtTr.position.y + ((rtTr.rect.yMin + rtTr.rect.height) * scale_f), _v3Pos.y))
			{
				//Rect rectBound = RectTransformUtility.PixelAdjustRect(rtTr, mainCanvas);

				//RenderStarter.ms_v3DbgPoint1 = new Vector3(rtTr.position.x + (rtTr.rect.xMin * mainCanvas.scaleFactor), rtTr.position.y + (rtTr.rect.yMin * mainCanvas.scaleFactor), 0);
				//RenderStarter.ms_v3DbgPoint2 = new Vector3(rtTr.position.x + ((rtTr.rect.xMin + rtTr.rect.width) * mainCanvas.scaleFactor), rtTr.position.y + ((rtTr.rect.yMin + rtTr.rect.height) * mainCanvas.scaleFactor), 0);
				return true;
			}

			return false;
		}


		public static int UxButtonBeforRecord_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			_transit_mouse_record_before(_pEvent, _pContext);
			return 1;
		}


		public static int UxButtonsCollision_strIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			StateDStructureValue stEvent = new StateDStructureValue(_pEvent);

			string strName = "";
			if (stBase.get_string("UxButtonsCollision_strIf", ref strName))
			{
				string camera_str = "Main Camera";
				GameObject camera_go;
				stBase.get_string("UxButtonsCollision_Camera_strV", ref camera_str);
				camera_go = GameObject.Find(camera_str);
				Camera cam = camera_go.GetComponent<Camera>();

				if (cam == null)
					return 0;

				Vector3 mouse_pos_v3 = stBase.mouse_pos_get(stEvent);
				mouse_pos_v3 = cam.ScreenToWorldPoint(mouse_pos_v3);

				Button[] btns = GameObject.FindObjectsOfType<Button>();
				for (int i = 0; i < btns.Length; i++)
				{
					if (btns[i].name == strName)
					{

						if (IsInRect(btns[i].transform, mouse_pos_v3, null))
						{
							TMP_Text txtMep;
							txtMep = btns[i].GetComponentInChildren<TMP_Text>();
							if (txtMep != null)
								stBase.state_variable_get().set_variable_string("TempString_strV", txtMep.text);

							VScriptLink link = btns[i].GetComponent<VScriptLink>();
							if (link != null)
								stBase.state_variable_get().set_variable("TempInt_nV", link.m_key);
							return 1;
						}
					}
				}
			}

			return 0;
		}

		public static int UxButtonsInPos_astrIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			StateDStructureValue stEvent = new StateDStructureValue(_pEvent);

			string canvas_str = "Canvas";
			GameObject canvas_go = GameObject.Find(canvas_str);

			List<string> name_a = null;
			if (stBase.get_strings("UxButtonsInPos_astrIf", ref name_a))
			{
				if (name_a.Count < 2)
					return 0;

				Vector3 pos = stBase.mouse_pos_get(stEvent);

				Button[] btns = GameObject.FindObjectsOfType<Button>();
				for (int i = 0; i < btns.Length; i++)
				{
					if (btns[i].name == name_a[0])
					{
						Vector3 vposms = pos;

						if (IsInRect(btns[i].transform, vposms, canvas_go))
						{
							Button[] btns2 = btns[i].gameObject.GetComponentsInChildren<Button>();
							for (int j = 0; j < btns2.Length; j++)
							{
								if (btns2[j].name == name_a[1])
								{
									if (IsInRect(btns2[j].transform, vposms, canvas_go))
									{
										VScriptLink link = btns[i].GetComponent<VScriptLink>();
										if (link != null)
											stBase.state_variable_get().set_variable("TempInt_nV", link.m_key);

										TMP_Text txtMep;
										txtMep = btns[i].GetComponentInChildren<TMP_Text>();
										StateDStructureValue dstEvent =
											VLStateManager.event_state_make("UxButtonSelect");
										dstEvent.set_variable_string("TempString_strV", txtMep.text);
										VLStateManager.event_post(dstEvent);

										return 1;
									}
								}
							}
						}
					}
				}
			}

			return 0;
		}

		public static int UxButtonsInPos_strIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			StateDStructureValue stEvent = new StateDStructureValue(_pEvent);

			string canvas_str = "Canvas";
			GameObject canvas_go = GameObject.Find(canvas_str);

			string strName = "";
			if (stBase.get_string("UxButtonsInPos_strIf", ref strName))
			{
				Vector3 pos = stBase.mouse_pos_get(stEvent);

				Button[] btns = GameObject.FindObjectsOfType<Button>();
				for (int i = 0; i < btns.Length; i++)
				{
					if (btns[i].name == strName)
					{
						Vector3 vposms = pos;

						if (IsInRect(btns[i].transform, vposms, canvas_go))
						{
							VScriptLink link = btns[i].GetComponent<VScriptLink>();
							if (link != null)
								stBase.state_variable_get().set_variable("TempInt_nV", link.m_key);

							TMP_Text txtMep;
							txtMep = btns[i].GetComponentInChildren<TMP_Text>();
							if (txtMep != null)
							{
								StateDStructureValue dstEvent = VLStateManager.event_state_make("UxButtonSelect");
								dstEvent.set_variable_string("TempString_strV", txtMep.text);
								VLStateManager.event_post(dstEvent);
							}

							return 1;
						}
					}
				}
			}

			return 0;
		}


		public static int UxButtonInPos_strIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			StateDStructureValue stEvent = new StateDStructureValue(_pEvent);

			string canvas_str = "Canvas";
			GameObject canvas_go = GameObject.Find(canvas_str);

			string strName = "";
			if (stBase.get_string("UxButtonInPos_strIf", ref strName))
			{
				GameObject btn = GameObject.Find(strName);
				if (btn)
				{
					Vector3 pos = stBase.mouse_pos_get(stEvent);

					if (IsInRect(btn.transform, pos, canvas_go))
					{
						return 1;
					}
				}
			}

			return 0;
		}


		public static int UxRaycastInLayer_strIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			StateDStructureValue stEvent = new StateDStructureValue(_pEvent);

			string strName = "";
			if (stBase.get_string("UxRaycastInLayer_strIf", ref strName))
			{
				Vector3 vposms = stBase.mouse_pos_get(stEvent);

				StateDStructureValue variable_dsv = stBase.state_variable_get();
				GameObject camera_go = null;

				if (!variable_dsv.get_ptr("UxComponentCurPtr", ref camera_go))
					return 0;
				Camera camera_state_view_cam = null;
				camera_state_view_cam = camera_go.GetComponent<Camera>();
				Debug.LogWarning(string.Format("KKIUxRaycastInLayer_strIf {0}", camera_state_view_cam));

				var ray = camera_state_view_cam.ScreenPointToRay(vposms);
				var hit = default(RaycastHit);

				//int grnd = 1 << LayerMask.NameToLayer("GroundEnemy");
				//int fly = 1 << LayerMask.NameToLayer("FlyingEnemy");
				//int mask = grnd | fly;
				LayerMask _lM = 1 << LayerMask.NameToLayer(strName);
				if (Physics.Raycast(ray, out hit, float.PositiveInfinity, _lM) == true)
				{
					Debug.LogWarning(string.Format("KKIUxRaycastInLayer_strIf {0}", hit.collider.gameObject));
					return 1;
				}
			}

			return 0;
		}


		public static int UxPointInCheckCollide_nIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			_transit_mouse_pos(_pEvent, stBase.state_variable_get());
			int nHash = 0;
			if (stBase.get_int("UxPointInCheckCollide_nIf", ref nHash))
			{
				//CollisionDetect ();
				if (nHash == ms_nHashFocus)
					return 1;
			}

			return 0;
		}


		public static int UxPointInCheckCollideRecordBefor_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext,
			int _nState)
		{
			//ms_nCurIdBefor = Scene.ms_nCurID;
			//ms_nCurIndexStep = Scene.ms_nSelTimestep;
			return 1;
		}


		public static int UxPointInCheckCollideDouble_nIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			//if (Scene.ms_nCurID == ms_nCurIdBefor
			//	&& Scene.ms_nSelTimestep == ms_nCurIndexStep)
			//	return 1;
			return 0;
		}


		public static int UxPointInCheckConst_nIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			_transit_mouse_pos(_pEvent, stBase.state_variable_get());

			int nHash = 0;
			if (stBase.get_int("UxPointInCheckConst_nIf", ref nHash))
			{
				//if(nHash == CollisionGet())
				//	return 1;
			}

			return 0;
		}


		public static int UxPointInCheck_nIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			//StateDStructureValue stEvent = new StateDStructureValue (_pEvent);
			//StateDStructureValue stContext = new StateDStructureValue (_pContext);
			int nHash = 0;
			if (stBase.get_int("UxPointInCheck_nIf", ref nHash))
			{
				if (nHash == ms_nHashFocus)
					return 1;
			}

			return 0;
		}

		public static int UxPointInCheck_strIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			//StateDStructureValue stEvent = new StateDStructureValue (_pEvent);
			//StateDStructureValue stContext = new StateDStructureValue (_pContext);
			string hash_str = "";
			if (stBase.get_string("UxPointInCheck_strIf", ref hash_str))
			{
				int hash = VLStateManager.hash(hash_str);
				if (hash == ms_nHashFocus)
					return 1;
			}

			return 0;
		}


		public static int UxPointCountPush_nIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			_transit_mouse_pos(_pEvent, stBase.state_variable_get());
			//StateDStructureValue stEvent = new StateDStructureValue (_pEvent);
			//StateDStructureValue stContext = new StateDStructureValue (_pContext);
			int nCnt = 0;
			if (stBase.get_int("UxPointCountPush_nIf", ref nCnt))
			{
				if (nCnt == m_stlVTouch.Count)
					return 1;
			}

			return 0;
		}


		public static int UxPointCountPushNot_nIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			_transit_mouse_pos(_pEvent, stBase.state_variable_get());
			//StateDStructureValue stEvent = new StateDStructureValue (_pEvent);
			//StateDStructureValue stContext = new StateDStructureValue (_pContext);
			int nCnt = 0;
			if (stBase.get_int("UxPointCountPushNot_nIf", ref nCnt))
			{
				if (nCnt != m_stlVTouch.Count)
					return 1;
			}

			return 0;
		}

		public static int UxTouchCursorSet_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			StateDStructureValue event_dsv = new StateDStructureValue(_pEvent);

			int type = 0;
			if (!stBase.get_int("UxTouchCursorSet_nF", ref type))
				return 0;

			UxTouch.Instance.cursor_set(type);
			return 1;
		}

		public static void StateFuncRegist()
		{

			VLStateManager.ProcessReg("UxTouchCursorSet_nF", UxTouchCursorSet_nF,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);
			//UxMouseMoveDistanceGE_nIf
			VLStateManager.ProcessReg("UxMouseMoveDistRateLE_fIf", UxMouseMoveDistRateLE_fIf,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);
			VLStateManager.ProcessReg("UxMouseCountButton_nIf", UxMouseCountButton_nIf,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);
			VLStateManager.ProcessReg("UxMouseCastPointMove_nF", UxMouseCastPointMove_nF,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0); //	INT32		Cast UxPointMove Event
			VLStateManager.ProcessReg("UxMouseCastPointPush_nF", UxMouseCastPointPush_nF,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0); //	INT32		Cast UxPointPush Event
			VLStateManager.ProcessReg("UxMouseCastPointRelease_nF", UxMouseCastPointRelease_nF,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0); //	INT32		Cast UxPointRelease Event
			VLStateManager.ProcessReg("UxAppendMode_nIf", UxAppendMode_nIf, "VScriptEditor/Assets/Scripts/UxTouch.cs",
				0); //	INT32		Check is append mode 1:true 0:false

			VLStateManager.ProcessReg("UxPlatformCheck_anIf", UxPlatformCheck_anIf,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);
			VLStateManager.ProcessReg("UxSystemCheckIsMobile_nIf", UxSystemCheckIsMobile_nIf,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);
			VLStateManager.ProcessReg("UxCommandUndo_nF", UxCommandUndo_nF, "VScriptEditor/Assets/Scripts/UxTouch.cs",
				0);
			VLStateManager.ProcessReg("UxCommandRedo_nF", UxCommandRedo_nF, "VScriptEditor/Assets/Scripts/UxTouch.cs",
				0);
			VLStateManager.ProcessReg("UxCommandCancel_nF", UxCommandCancel_nF,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);
			VLStateManager.ProcessReg("UxCommandPushCutter_nF", UxCommandPushCutter_nF,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);
			VLStateManager.ProcessReg("UxPointCountPush_nIf", UxPointCountPush_nIf,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);
			VLStateManager.ProcessReg("UxPointInCheck_strIf", UxPointInCheck_strIf,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);

			VLStateManager.ProcessReg("UxPointCountPushNot_nIf", UxPointCountPushNot_nIf,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);
			VLStateManager.ProcessReg("UxPointInCheck_nIf", UxPointInCheck_nIf,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);
			VLStateManager.ProcessReg("UxPointInCheckCollide_nIf", UxPointInCheckCollide_nIf,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);
			VLStateManager.ProcessReg("UxPointInCheckConst_nIf", UxPointInCheckConst_nIf,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);
			VLStateManager.ProcessReg("UxPointInCheckCollideDouble_nIf", UxPointInCheckCollideDouble_nIf,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);
			VLStateManager.ProcessReg("UxPointInCheckCollideRecordBefor_nF", UxPointInCheckCollideRecordBefor_nF,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);
			VLStateManager.ProcessReg("UxPointInCheckPos_nF", UxPointInCheckPos_nF,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);
			VLStateManager.ProcessReg("UxButtonInPos_strIf", UxButtonInPos_strIf,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);
			VLStateManager.ProcessReg("UxButtonsInPos_strIf", UxButtonsInPos_strIf,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);
			VLStateManager.ProcessReg("UxButtonsInPos_astrIf", UxButtonsInPos_astrIf,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);

			VLStateManager.ProcessReg("UxButtonsCollision_strIf", UxButtonsCollision_strIf,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);

			VLStateManager.ProcessReg("UxButtonBeforRecord_nF", UxButtonBeforRecord_nF,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);
			VLStateManager.ProcessReg("UxCommandCuttorLastRemove_nF", UxCommandCuttorLastRemove_nF,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);
			VLStateManager.ProcessReg("UxDeviceGet_anF", UxDeviceGet_anF, "VScriptEditor/Assets/Scripts/UxTouch.cs", 0);
			VLStateManager.ProcessReg("UxSystemTerminate_nF", UxSystemTerminate_nF,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);
			VLStateManager.ProcessReg("UxSystemCheckSystem_anIf", UxSystemCheckSystem_anIf,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);

			VLStateManager.ProcessReg("UxRaycastInLayer_strIf", UxRaycastInLayer_strIf,
				"VScriptEditor/Assets/Scripts/UxTouch.cs", 0);
		}
	}
}