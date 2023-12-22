#if UNITY_EDITOR
 using UnityEditor;
 using UnityEditorInternal;
#endif
 using AOT;
 using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System;
using System.Runtime.InteropServices;
using System.IO;

using System.Text;

using System.Collections.Generic;

namespace StateSystem
{
	public struct StateClass
	{
		public string name;
		public List<string> functions;
	}
	public class VLStateManager : MonoBehaviour
	{
		public bool DebugMode = true;
		public string VScriptIP = "localhost";
		public string ApplicationName = "VScript";

		static IntPtr m_pStateManager;
		static List<KeyCode> m_keypushed_a = new List<KeyCode>();

#if UNITY_EDITOR
		public List<StateClass> classes;
#endif
		public static IntPtr get_manager()
		{
			return m_pStateManager;
		}

		public static string variable_make_string(IntPtr _pdsvBase, IntPtr _pdsvEvent, IntPtr _pdsvContext,
			string _strFormat)
		{
			byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strFormat + "\0");
			IntPtr pRet = StateInterface.static_variable_make_string(_pdsvBase, _pdsvEvent, _pdsvContext, temp);

			string strRet = VLStateManager.MarshalToString(pRet);
			return strRet;
		}

		public static bool ms_bDebugMode = true;
		//public static bool ms_bDebugMode = false;


		public static string MarshalToString(IntPtr _ptrString)
		{
			string strString = "err";
			if (_ptrString == IntPtr.Zero)
				return strString;

			try
			{
#if UNITY_STANDALONE_WIN || UNITY_EDITOR_WIN
				strString = Marshal.PtrToStringAnsi(_ptrString);
#else
                strString = Marshal.PtrToStringAuto(_ptrString);
#endif
			}
			catch (ArgumentException e)
			{
				Debug.Log(e.GetType().Name + ":=-=-=-" + e.Message);
			}
			catch (Exception e)
			{
				Debug.Log(e.GetType().Name + ":=-=-=-" + e.Message);
			}

			if (strString == "dbcolumn_pnid_state")
			{
				int x = 0;
				x++;
			}

			return strString;
		}

		static public string sActive = "";
        [MonoPInvokeCallback(typeof(StateInterface.fnFuncProcessor))]
		public static int BaseBreakStart_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			string strComment = "";
			if (!stBase.get_string("BaseBreakStart_strF", ref strComment))
			{
				return 0;
			}

			return 1;
		}
        [MonoPInvokeCallback(typeof(StateInterface.fnFuncProcessor))]
		public static int BaseBreakAppend_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			string strComment = "";
			if (!stBase.get_string("BaseBreakAppend_strF", ref strComment))
			{
				return 0;
			}

			return 1;
		}
        [MonoPInvokeCallback(typeof(StateInterface.fnFuncProcessor))]
		public static int BaseBreakComplete_strF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue stBase = new StateDStructureValue(_pBase);
			string strComment = "";
			if (!stBase.get_string("BaseBreakComplete_strF", ref strComment))
			{
				return 0;
			}

			return 1;
		}

		static public bool vscript_column_name_get(int _nIndex, ref string _strName)
		{
			IntPtr pStr = StateInterface.vscript_column_name_get_(_nIndex);

			if (pStr != IntPtr.Zero)
			{
				_strName = VLStateManager.MarshalToString(pStr);
				return true;
			}

			return false;
		}

		static public StateDStructure enum_get(string _strEnum)
		{
			byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strEnum + "\0");
			StateDStructure enum_dst = new StateDStructure(StateInterface.manager_enum_get(m_pStateManager, temp));

			return enum_dst;
		}

		static public bool enum_get(string _strEnum, int _nKey, int _nHash, ref int _pnValue)
		{
			byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strEnum + "\0");
			if (m_pStateManager == IntPtr.Zero)
				return false;
			return StateInterface.manager_enum_get_int(m_pStateManager, temp, _nKey, _nHash, ref _pnValue);
		}

		static public string url_get()
		{
			string strRet;
			IntPtr pRet = StateInterface.manager_url_get();
			strRet = VLStateManager.MarshalToString(pRet);
			return strRet;
		}

		static public void url_set(string _str)
		{
			byte[] temp = System.Text.Encoding.UTF8.GetBytes(_str + "\0");
			StateInterface.manager_url_set(temp);
		}

		static public bool enum_get(string _strEnum, int _nKey, int _nHash, ref string _strValue)
		{
			byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strEnum + "\0");
			if (m_pStateManager == IntPtr.Zero)
				return false;
			IntPtr pRet = StateInterface.manager_enum_get_string(m_pStateManager, temp, _nKey, _nHash);
			_strValue = "";
			if (pRet == IntPtr.Zero)
				return false;
			_strValue = VLStateManager.MarshalToString(pRet);
			return true;
		}

		static public void ProcessReg(string _strName, StateInterface.fnFuncProcessor _fnFunc, string _file = "Anknow",
			int _line = -1)
		{
			byte[] strColumName = System.Text.Encoding.UTF8.GetBytes(_strName + "\0");
			byte[] file_str = System.Text.Encoding.UTF8.GetBytes(_file + "\0");

			StateInterface.manager_reg_func(strColumName, _fnFunc, file_str, _line);
		}

		static public StateDStructureValue variable_global_get()
		{
			if (m_pStateManager == IntPtr.Zero)
				return null;
			IntPtr pstGlobalVariable = StateInterface.manager_variable_global_get(m_pStateManager);
			StateDStructureValue stVariable = new StateDStructureValue(pstGlobalVariable);
			return stVariable;
		}

		static public StateDStructureValue create_event(string _strName)
		{
			if (m_pStateManager == IntPtr.Zero)
				return null;
			byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strName + "\0");
			IntPtr pstEvent = StateInterface.manager_event_make(m_pStateManager, temp);
			StateDStructureValue stEvent = new StateDStructureValue(pstEvent);
			return stEvent;
		}

		static public StateDStructureValue create_event_group(int _event, int _group, int _id)
		{
			if (m_pStateManager == IntPtr.Zero)
				return null;
			IntPtr pstEvent = StateInterface.manager_event_group_make(m_pStateManager, _event, _group, _id);
			StateDStructureValue stEvent = new StateDStructureValue(pstEvent);
			return stEvent;
		}

		static public bool state_active(int _key, int _group, int _id, int _serial = 0)
		{
			if (m_pStateManager == IntPtr.Zero)
				return false;
			return StateInterface.manager_state_active(m_pStateManager, _key, _group, _id, _serial);
		}

		static public void hide_log(string _strTitle, string _strValue)
		{
			byte[] temp1 = System.Text.Encoding.UTF8.GetBytes(_strTitle + "\0");
			byte[] temp2 = System.Text.Encoding.UTF8.GetBytes(_strValue + "\0");

			StateInterface.consol_hide(temp1, temp2);
		}

		static public void show_log(string _strTitle, string _strValue)
		{
			byte[] temp1 = System.Text.Encoding.UTF8.GetBytes(_strTitle + "\0");
			byte[] temp2 = System.Text.Encoding.UTF8.GetBytes(_strValue + "\0");

			StateInterface.consol_show(temp1, temp2);
		}

		static public void hash_load(string _strFile)
		{
			byte[] temp1 = System.Text.Encoding.UTF8.GetBytes(_strFile + "\0");

			StateInterface.consol_load_hash(temp1);
		}

		static public void language_change(int _nCountryCode)
		{
			StateInterface.manager_language_change(_nCountryCode);
		}

		static public int hash(string _strHash)
		{
			byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strHash + "\0");

			return StateInterface.manager_hash(temp);
		}

		void LaunchApp(string via)
		{

			via = "receive";
		}

		void OnApplicationPause(bool pauseStatus)
		{
			//VLStateManager.message (1, AndroidDeepLink.GetURL ());
			//VLStateManager.message (1, AndroidDeepLink.GetValueInString ("scene"));

			if (!pauseStatus)
			{
				StateInterface.manager_weakup(m_pStateManager);
				VLStateManager.event_state_post("UxApplicationResume");
			}
			else
			{
				VLStateManager.event_state_post("UxApplicationPause");
			}
		}

		static public void event_post(IntPtr _pEvent, int _space = 0, int _priority = 0)
		{
			if (m_pStateManager == IntPtr.Zero)
				return;
			StateInterface.manager_event_post(m_pStateManager, _pEvent, _space, _priority);
		}

		static public void event_post(StateDStructureValue _dsvEvent, int _space = 0, int _priority = 0)
		{
			if (m_pStateManager == IntPtr.Zero)
				return;
			StateInterface.manager_event_post(m_pStateManager, _dsvEvent.m_pDStructureValue, _space, _priority);
		}

		static public StateDStructureValue event_state_make(string _strEvent)
		{
			if (m_pStateManager == IntPtr.Zero)
				return null;

			int nHash = VLStateManager.hash(_strEvent);
			StateDStructureValue stEvent;
			stEvent = VLStateManager.create_event("BaseStateEventGlobal");
			stEvent.set_int("BaseStateEventGlobal", nHash);

			return stEvent;
		}

		static public void variable_define(string _name, int _type)
		{
			if (m_pStateManager == IntPtr.Zero)
				return;

			byte[] temp = System.Text.Encoding.UTF8.GetBytes(_name + "\0");
			StateInterface.manager_variable_define(m_pStateManager, temp, _type);
		}

		static public void structure_define(string _table, List<string> _column_a)
		{
			if (m_pStateManager == IntPtr.Zero)
				return;

			byte[] temp = System.Text.Encoding.UTF8.GetBytes(_table + "\0");
			string hand = "";
			foreach (string str in _column_a)
			{
				hand += str;
				hand += ",";
			}

			byte[] temp2 = System.Text.Encoding.UTF8.GetBytes(hand + "\0");
			StateInterface.manager_structure_define(m_pStateManager, temp, temp2);
		}

		static public void db_column_set(string _column, int _type, string _table)
		{
			byte[] temp = System.Text.Encoding.UTF8.GetBytes(_column + "\0");
			byte[] temp2 = System.Text.Encoding.UTF8.GetBytes(_table + "\0");
			StateInterface.db_column_set(temp, _type, temp2);
		}

		static public void db_columntable_set(int _table_hash, List<string> _column_a)
		{
			string hand = "";
			foreach (string str in _column_a)
			{
				hand += str;
				hand += ",";
			}

			byte[] temp = System.Text.Encoding.UTF8.GetBytes(hand + "\0");
			StateInterface.db_columntable_set(_table_hash, temp);
		}

		static public StateDStructureValue event_state_make(string _strEvent, string _strParam, int _nValue)
		{
			StateDStructureValue pEvent;

			pEvent = VLStateManager.event_state_make(_strEvent);
			pEvent.add_variable(_strParam, _nValue);

			return pEvent;
		}

		public void ui_event_state_post(string _strEvent)
		{
			StateDStructureValue pEvent;
			pEvent = VLStateManager.event_state_make(_strEvent);
			event_post(pEvent);
		}

		static public void event_state_post(string _strEvent, string _strParam, int _nValue)
		{
			event_post(event_state_make(_strEvent, _strParam, _nValue));
		}

		static public StateDStructureValue event_state_make(string _strEvent, string _strParam, string _strValue)
		{
			StateDStructureValue pEvent;

			pEvent = VLStateManager.event_state_make(_strEvent);
			pEvent.set_variable_string(_strParam, _strValue);

			return pEvent;
		}

		static public void event_state_post(string _strEvent, string _strParam, string _strValue)
		{
			event_post(event_state_make(_strEvent, _strParam, _strValue));
		}

		static public void event_state_post(string _strEvent, int _space = 0, int _priority = 0)
		{
			if (m_pStateManager == IntPtr.Zero)
				return;

			int nHash = VLStateManager.hash(_strEvent);
			StateDStructureValue stEvent;
			stEvent = VLStateManager.create_event("BaseStateEventGlobal");

			stEvent.set_int("BaseStateEventGlobal", nHash);

			StateInterface.manager_event_post(m_pStateManager, stEvent.m_pDStructureValue, _space, _priority);

			return;
		}

		static public void event_process(IntPtr _pEvent)
		{
			if (m_pStateManager == IntPtr.Zero)
				return;

			StateInterface.manager_event_process(m_pStateManager, _pEvent);
		}

		static public void event_state_process(string _strEvent)
		{
			if (m_pStateManager == IntPtr.Zero)
				return;

			int nHash = VLStateManager.hash(_strEvent);
			StateDStructureValue stEvent;
			stEvent = VLStateManager.create_event("BaseStateEventGlobal");
			stEvent.set_int("BaseStateEventGlobal", nHash);

			StateInterface.manager_event_process(m_pStateManager, stEvent.m_pDStructureValue);
		}

		static public void message(int _nFilter, string _strMsg)
		{
			byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strMsg + "\0");
			StateInterface.manager_message(_nFilter, temp);
		}

		static public string filedialog_open(string _strExt)
		{
			byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strExt + "\0");
			string strRet;
			IntPtr pRet = StateInterface.filedialog_open(temp);
			if (pRet == IntPtr.Zero)
				return null;
			strRet = VLStateManager.MarshalToString(pRet);

			return strRet;
		}

		static public string filedialog_save(string _strExt)
		{
			byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strExt + "\0");
			string strRet;
			IntPtr pRet = StateInterface.filedialog_save(temp);
			if (pRet == IntPtr.Zero)
				return null;
			strRet = VLStateManager.MarshalToString(pRet);

			return strRet;
		}

		static public string translate(string _str)
		{
			byte[] temp = System.Text.Encoding.UTF8.GetBytes(_str + "\0");
			string strRet;
			IntPtr pRet = StateInterface.static_translate(temp);
			if (pRet == IntPtr.Zero)
				return null;
			strRet = VLStateManager.MarshalToString(pRet);

			strRet = strRet.Replace("\\n", "\n");
			return strRet;
		}

		static public string param_variable_get_string(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nSeq,
			IntPtr _pDefault)
		{
			if (m_pStateManager == IntPtr.Zero)
				return null;

			string strRet;
			IntPtr pRet = StateInterface.static_variable_param_string_get(_pBase, _pEvent, _pContext, _nSeq, _pDefault);
			if (pRet == IntPtr.Zero)
				return null;
			strRet = VLStateManager.MarshalToString(pRet);
			return strRet;
		}

		static public int param_variable_get_int(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nSeq,
			IntPtr _pDefault)
		{
			if (m_pStateManager == IntPtr.Zero)
				return 0;

			return StateInterface.static_variable_param_int_get(_pBase, _pEvent, _pContext, _nSeq, _pDefault);
		}

        static public bool param_variable_set_int(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nSeq,
            IntPtr _pDefault, int _value)
        {
            if (m_pStateManager == IntPtr.Zero)
                return false;

            GCHandle handle = GCHandle.Alloc(_value, GCHandleType.Pinned);
            IntPtr valuePtr = handle.AddrOfPinnedObject();

            bool ret = StateInterface.static_variable_param_void_set(_pBase, _pEvent, _pContext, _nSeq, _pDefault, valuePtr, 0);

            handle.Free();
            return ret;
        }
        
        static public bool param_variable_set_floata(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nSeq,
            IntPtr _pDefault, float[] _value, int _cnt)
        {
            if (m_pStateManager == IntPtr.Zero)
                return false;

            GCHandle handle = GCHandle.Alloc(_value, GCHandleType.Pinned);
            IntPtr valuePtr = handle.AddrOfPinnedObject();

            bool ret = StateInterface.static_variable_param_void_set(_pBase, _pEvent, _pContext, _nSeq, _pDefault, valuePtr, _cnt);

            handle.Free();
            return ret;
        }
        
        static public bool param_variable_get_floata(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nSeq,
            IntPtr _pDefault, ref float[] _value, ref int _cnt)
        {
            if (m_pStateManager == IntPtr.Zero)
                return false;

            IntPtr valuePtr = StateInterface.static_variable_param_void_get(_pBase, _pEvent, _pContext, _nSeq, ref _cnt);
            if (valuePtr == IntPtr.Zero)
                return false;

            if (_cnt != 3)
                return false;
            Marshal.Copy(valuePtr, _value, 0, _cnt);
            return true;
        }
        
        static public bool param_variable_set_vector(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nSeq,
            IntPtr _pDefault, Vector3 _vec)
        {
            if (m_pStateManager == IntPtr.Zero)
                return false;

            float[] vec = new float[3];
            for (int i = 0; i < 3; i++)
                vec[i] = _vec[i];

            return param_variable_set_floata(_pBase, _pEvent, _pContext, _nSeq, _pDefault, vec, 3);
        }
        
        static public bool param_variable_get_vector(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nSeq,
            IntPtr _pDefault, ref Vector3 _vec)
        {
            if (m_pStateManager == IntPtr.Zero)
                return false;

            float[] vec = new float[3];
            int cnt = 3;
            bool ret = param_variable_get_floata(_pBase, _pEvent, _pContext, _nSeq, _pDefault,ref vec, ref cnt);
            if (!ret)
                return ret;

            for(int i=0; i<3; i++)
                _vec[i] = vec[i];
            
            return ret;
        }
        
        static public bool param_variable_set_string(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nSeq,
            IntPtr _pDefault, string _value)
        {
            if (m_pStateManager == IntPtr.Zero)
                return false;

            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_value + "\0");
            GCHandle handle = GCHandle.Alloc(temp, GCHandleType.Pinned);
            IntPtr valuePtr = handle.AddrOfPinnedObject();

            bool ret = StateInterface.static_variable_param_void_set(_pBase, _pEvent, _pContext, _nSeq, _pDefault, valuePtr, 0);

            handle.Free();
            return ret;
        }

        [MonoPInvokeCallback(typeof(StateInterface.fnFuncProcessor))]
		public static int DebugOutput(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			string strLog;
			strLog = VLStateManager.MarshalToString(_pBase);
			if (strLog.Contains("OnStart"))
				UnityEngine.Debug.Log(strLog);
			if (strLog.Contains("OnResume") && !strLog.Contains("MainTabView") && !strLog.Contains("MainTabFile")
			    && !strLog.Contains("MainTabEdit"))
			{
				if (true == strLog.Contains("SActive:"))
				{
					int startIndex = strLog.IndexOf("SActive:");
					int endIndex = strLog.IndexOf(" ", startIndex);
					if (-1 != startIndex)
					{
						if (-1 == endIndex)
						{
							endIndex = strLog.Length;
						}

						string NextActive = strLog.Substring(startIndex, endIndex - startIndex);
						if (NextActive != sActive)
						{
                            UnityEngine.Debug.Log(strLog);
							//Debug.LogWarning(string.Format("KKIDebugOutput {0}", sActive));
						}

						sActive = NextActive;
					}
				}
			}

			if (_nState == 0)
			{
				//RenderStarter.m_strMessage = strLog;
			}

			return 1;
		}
        [MonoPInvokeCallback(typeof(StateInterface.fnFuncProcessor))]
		public static int UxSystemGarbageCollection_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			Resources.UnloadUnusedAssets();
			return 1;
		}

		void load_save(string _strFolder, string _strFile)
		{
			// in case of android, all of resource is compressed, so files should be released and save in normal data to read in plugin.
			TextAsset ta = Resources.Load(_strFile) as TextAsset;
			if (ta == null)
			{
				UnityEngine.Debug.Log($"[StateSys]: Fail to load android resource {_strFile}");
			}
			else
			{
				UnityEngine.Debug.Log($"[StateSys]: success to load android resource {_strFile}");
			}

			byte[] bytes = ta.bytes;
			if (bytes == null)
			{
				UnityEngine.Debug.Log($"[StateSys]: Fail to load android byte {_strFile}");
			}
			else
			{
				UnityEngine.Debug.Log($"[StateSys]: success to load android byte {_strFile}");
			}

			DirectoryInfo dirInf = new DirectoryInfo(Application.persistentDataPath + _strFolder);
			if (!dirInf.Exists)
			{
                UnityEngine.Debug.Log("[StateSys]:  Creating subdirectory");
				dirInf.Create();
			}

			FileStream fs = File.OpenWrite(Application.persistentDataPath + _strFolder + "/" + _strFile);
			fs.Write(bytes, 0, bytes.Length);
			fs.Close();
		}

		// Use this for initialization
		void Awake()
		{
			ms_bDebugMode = DebugMode;

			QualitySettings.vSyncCount = 0; // VSync must be disabled
			Application.targetFrameRate = 30;

			string dbgPath = Application.persistentDataPath;
			byte[] dataPath = System.Text.Encoding.UTF8.GetBytes(Application.persistentDataPath + "\0");
			byte[] rootPath;

			if (Application.platform == RuntimePlatform.Android)
			{
				//if (true) {
				load_save("/state", "Basedefine.define");
				load_save("/state", "EnumDBColumn.data");
				load_save("/state", "EnumDBColumnLink.data");
				load_save("/state", "EnumEvent.data");
				load_save("/state", "EnumEventKey.data");
				load_save("/state", "EnumEventKeyLink.data");
				load_save("/state", "EnumEventLink.data");
				load_save("/state", "EnumGroupID.data");
				load_save("/state", "EnumGroupIDLink.data");
				load_save("/state", "EnumReturnValue.data");
				load_save("/state", "EnumReturnValueLink.data");
				load_save("/state", "EnumStateEvent.data");
				load_save("/state", "EnumStateEventLink.data");
				load_save("/state", "EnumParams.data");
				load_save("/state", "EnumParamsLink.data");
				load_save("/state", "EnumUXID.data");
				load_save("/state", "EnumUXIDLink.data");
				load_save("/state", "EnumVariableDefine.data");
				load_save("/state", "EnumVariableDefineLink.data");
				load_save("/state", "EnumVariableType.data");
				load_save("/state", "EnumVariableTypeLink.data");
				load_save("/state", "Football3DBoard.data");
				load_save("/state", "Football3DBoardLink.data");
				load_save("/state", "Football3DBoardArchive.data");
				load_save("/state", "Football3DBoardArchiveLink.data");
				load_save("/state", "MouseEventControl.data");
				load_save("/state", "MouseEventControlLink.data");
				load_save("/state", "ResourceHeader.data");
				load_save("/state", "SceneDisplayControl.data");
				load_save("/state", "SceneDisplayControlLink.data");
				load_save("/state", "StateList.data");
				load_save("/state", "hash_table.ini");
				load_save("/state", "FileTransferLink.data");
				load_save("/state", "FileTransfer.data");
				load_save("/state", "ASystemSettingLink.data");
				load_save("/state", "ASystemSetting.data");
				load_save("/state", "languagepack.bin");
				load_save("/state", "encoder.cod");

				rootPath = dataPath;
			}
			else
			{
				rootPath = System.Text.Encoding.UTF8.GetBytes(Application.streamingAssetsPath + "\0");
			}
			//load_save ("", "new.scene");

			byte[] strApp = System.Text.Encoding.UTF8.GetBytes(ApplicationName + "\0");
			//byte[] strIP = System.Text.Encoding.UTF8.GetBytes ("192.168.1.11" + "\0");

			byte[] strIP;

			if (ms_bDebugMode)
				strIP = System.Text.Encoding.UTF8.GetBytes(VScriptIP + "\0");
			//strIP = System.Text.Encoding.UTF8.GetBytes ("192.168.1.50" + "\0");
			//strIP = System.Text.Encoding.UTF8.GetBytes("10.138.71.224" + "\0"); // for songpa lib
			//strIP = System.Text.Encoding.UTF8.GetBytes ("192.168.2.7" + "\0");
			else
				strIP = System.Text.Encoding.UTF8.GetBytes("forrelease" + "\0");

			//172.30.6.26

			VLStateManager.ProcessReg("DebugOutput", DebugOutput, "StateManager.cs", 0);

			//192.168.1.11
			int nLang = 1;
			SystemLanguage sLang = Application.systemLanguage;
			if (ms_bDebugMode)
				sLang = SystemLanguage.Korean;

			switch (sLang)
			{
				case SystemLanguage.English:
					nLang = 1;
					break;
				case SystemLanguage.Korean:
					nLang = 82;
					break;
				case SystemLanguage.German:
					nLang = 49;
					break;
				case SystemLanguage.Spanish:
					nLang = 34;
					break;
				case SystemLanguage.Japanese:
					nLang = 81;
					break;
				case SystemLanguage.Chinese:
					nLang = 86;
					break;
				case SystemLanguage.ChineseTraditional:
					nLang = 886;
					break;
				case SystemLanguage.Portuguese:
					nLang = 351;
					break;
				case SystemLanguage.Arabic:
					nLang = 20;
					break;
			}

			StateInterface.vscript_init(strApp, rootPath, dataPath, strIP, nLang);
			m_pStateManager = StateInterface.manager_create(rootPath);

			StateFuncRegist(); // all of function registration should be called after manager_create(), 
			UxTouch.StateFuncRegist();
			UxControlScroll.StateFuncRegist();
			UndoStack.StateFuncRegist();
			UxCosmos.StateFuncRegist();
			UndoStack.StateFuncRegist();
			StateRegistor.StateFuncRegist();

			//#if UNITY_ANDROID || UNITY_IOS
			//AdManager.StateFuncRegist ();
			//#endif
			//UxScrollView.StateFuncRegist ();
#if UNITY_STANDALONE_WIN || UNITY_EDITOR_WIN
#else
		//InAppPurchasing.StateFuncRegist ();
#endif
			//ArchiveListController.StateFuncRegist();

			//m_pStateManager = IntPtr.Zero;

			//hash_load (Application.streamingAssetsPath + "/state/hash_table.ini");

			int nHash = VLStateManager.hash("ClientMobile");
			//int nHash = StateManager.hash("ClientTestModule");
			StateDStructureValue stEvent;
			stEvent = VLStateManager.create_event("BaseStateEventGlobal");

			stEvent.set_int("BaseStateEventGlobal", nHash);
			stEvent.set_variable_string("FileWritablePath", Application.persistentDataPath);

			StateInterface.manager_event_post(m_pStateManager, stEvent.m_pDStructureValue, 0, 0);
			//event_state_post ("ClientTestModule");
		}

		public static void monobehaviourPointSet(string _variable_name, MonoBehaviour _mono)
		{
			StateDStructureValue dstVariable = variable_global_get();
			dstVariable.set_variable(_variable_name, (MonoBehaviour)_mono);
		}

		public static MonoBehaviour monobehaviourPointGet(string _variable_name)
		{
			StateDStructureValue dstVariable = variable_global_get();
			MonoBehaviour mono = null;
			dstVariable.get_ptr(_variable_name, ref mono);
			return mono;
		}

		public static bool IsActivated()
		{
#if UNITY_EDITOR
			return InternalEditorUtility.isApplicationActive;
#else
		return Application.isFocused;
#endif
		}

		void Start()
		{
			StateDStructureValue global = variable_global_get();
			global.add_variable("BaseProductName_strV", ApplicationName);

			if (Application.platform == RuntimePlatform.Android
			    || Application.platform == RuntimePlatform.IPhonePlayer)
			{

				event_state_post("PlatformMobile");
			}

			String strDeviceId = SystemInfo.deviceUniqueIdentifier;
			//RenderStarter.m_deviceID = strDeviceId;

			if (!ms_bDebugMode)
				hide_log("all", "all");

			hide_log("SActive", "PlayModeStop");
			//hide_log ("SEvent", "-1313658956"); // BaseStateEventGlobal
			//hide_log ("SAction", "PopEvent");
			//hide_log("SAction", "OnResume");
			hide_log("SMain", "MouseEventControl");
			//hide_log ("SAction", "PushEvent");
			//hide_log ("SAction", "PostEvent");
			//hide_log ("SMain", "Football3DBoard");
			hide_log("SMain", "SceneDisplayControl");
			hide_log("SMain", "ASystemSetting");
			hide_log("SMain", "VScriptEditor");
			hide_log("SMain", "ClientTestModule");
			hide_log("EVal", "-912894067"); // ReqPlayDelta
			hide_log("EValue", "-912894067"); // ReqPlayDelta
			hide_log("SActive", "PlayModeStop2D"); // ReqPlayDelta
			hide_log("SEvent", "-1347460426"); //BaseStateEventGlobalUnprocess=-1347460426

			// for Logger
			hide_log("EValue", "-260146324"); // LoggerStateOn
			hide_log("EValue", "5105738"); // LoggerStateOff
			hide_log("EValue", "-1081868672"); // LoggerStateResume

#if UNITY_STANDALONE_WIN || UNITY_EDITOR_WIN
#else
		//InAppPurchasing.init ();
#endif
			event_state_post("UxOriantationChangePortrait");
		}

		void OnApplicationQuit()
		{
			VLStateManager.event_state_process("UxApplicationTerminate");
			StateInterface.manager_close(m_pStateManager);
		}

		ScreenOrientation ms_screenOrt = ScreenOrientation.Portrait;
		// Update is called once per frame

		int __delay_state_machine = 15;
		bool m_focus = true;

		void Update()
		{
			if (m_pStateManager == IntPtr.Zero)
				return;

			if (IsActivated() != m_focus)
			{
				m_focus = !m_focus;

				if (m_focus)
					event_state_post("ApplicationFocusOn");
				else
					event_state_post("ApplicationFocusOff");
			}

			if (Input.anyKeyDown)
			{
				for (int i = 27; (KeyCode)i <= KeyCode.Joystick8Button19; i++)
				{
					if (Input.GetKeyDown((KeyCode)i))
					{
						if (i == 100)
						{
							int x = 0;
							x++;
						}

						StateInterface.manager_keypush(m_pStateManager, i, true, 0);
						m_keypushed_a.Add((KeyCode)i);

					}
				}
			}

			for (int i = 0; i < m_keypushed_a.Count; i++)
			{
				if (!Input.GetKey(m_keypushed_a[i]))
				{
					StateInterface.manager_keypush(m_pStateManager, (int)m_keypushed_a[i], false, 0);
					m_keypushed_a.RemoveAt(i);
					i--;
				}
			}

			if (__delay_state_machine < 1)
				StateInterface.manager_update(m_pStateManager);
			else
				__delay_state_machine--;

			if (Screen.orientation != ms_screenOrt)
			{
				ms_screenOrt = Screen.orientation;

				if (ms_screenOrt == ScreenOrientation.Portrait
				    || ms_screenOrt == ScreenOrientation.PortraitUpsideDown)
				{
					event_state_post("UxOriantationChangePortrait");
					//UxCamera.ms_screenOrt = ScreenOrientation.Portrait;
				}
				else
				{
					event_state_post("UxOriantationChangeLandscape");
					//UxCamera.ms_screenOrt = ScreenOrientation.Landscape;
				}
			}


		}

		public static int UxSystemPerformanceUpdateSet_nF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue base_dsv = new StateDStructureValue(_pBase);
			int framerate = 30;

			if (!base_dsv.get_int("UxSystemPerformanceUpdateSet_nF", ref framerate))
				return 0;

			QualitySettings.vSyncCount = 0; // VSync must be disabled
			Application.targetFrameRate = framerate;

			if (framerate < 30)
			{
				UxTouch.ms_mouse_move_update_enable = false;
			}
			else
			{
				UxTouch.ms_mouse_move_update_enable = true;
			}

			return 1;
		}

		public static int UxSystemFileExist_strIf(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue base_dsv = new StateDStructureValue(_pBase);
			string filename = "";
			if (!base_dsv.get_string("UxSystemFileExist_strIf", ref filename))
				return 0;

			string fullpath = Application.streamingAssetsPath + filename;

			if (System.IO.File.Exists(fullpath))
			{
				return 1;
			}

			return 0;
		}

		public static int UxSystemPathRootGet_varF(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState)
		{
			StateDStructureValue base_dsv = new StateDStructureValue(_pBase);
			StateDStructureValue var_state = base_dsv.state_variable_get();

			int hash = 0;
			if (!base_dsv.get_int("UxSystemPathRootGet_varF", ref hash))
				return 0;

			string fullpath = Application.streamingAssetsPath;
			int index_slash = fullpath.LastIndexOf('/');
			fullpath = fullpath.Substring(0, index_slash);
			index_slash = fullpath.LastIndexOf('/');
			fullpath = fullpath.Substring(0, index_slash + 1);

			if (!var_state.set_variable_string(hash, fullpath))
				return 0;

			return 1;
		}

		public static void StateFuncRegist()
		{
			//manager_set_debugout (DebugOutput);
			VLStateManager.ProcessReg("UxSystemPathRootGet_varF", UxSystemPathRootGet_varF,
				"VScriptEditor/Assets/Scripts/StateManager.cs", 0);
			VLStateManager.ProcessReg("UxSystemFileExist_strIf", UxSystemFileExist_strIf,
				"VScriptEditor/Assets/Scripts/StateManager.cs", 0);
			VLStateManager.ProcessReg("UxSystemPerformanceUpdateSet_nF", UxSystemPerformanceUpdateSet_nF,
				"VScriptEditor/Assets/Scripts/StateManager.cs", 0);
			VLStateManager.ProcessReg("BaseBreakStart_strF", BaseBreakStart_strF,
				"VScriptEditor/Assets/Scripts/StateManager.cs", 0);
			VLStateManager.ProcessReg("BaseBreakAppend_strF", BaseBreakAppend_strF,
				"VScriptEditor/Assets/Scripts/StateManager.cs", 0);
			VLStateManager.ProcessReg("BaseBreakComplete_strF", BaseBreakComplete_strF,
				"VScriptEditor/Assets/Scripts/StateManager.cs", 0);
			VLStateManager.ProcessReg("UxSystemGarbageCollection_nF", UxSystemGarbageCollection_nF,
				"VScriptEditor/Assets/Scripts/StateManager.cs", 0);
		}
	}
}