using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using System;
using System.Runtime.InteropServices;

namespace StateSystem
{
    public class StateInterface
    {
        public delegate int fnFuncProcessor(IntPtr _pBase, IntPtr _pEvent, IntPtr _pContext, int _nState);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern int dst_get_type_count(IntPtr _pdst);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void thread_end();
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern int dst_get_type_flag(IntPtr _pdst, int _nIndex);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dst_set_type_flag(IntPtr _pdst, int _nIndex, int _nFlag);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr dst_get_type_name(IntPtr _pdst, int _nIndex);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern int dst_get_type(int _nIndex);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern int dst_get_type_size(IntPtr _pdst, int _nIndex);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr dst_get_type_enum_state(IntPtr _pdst, int _nIndex);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr dst_get_type_comment(IntPtr _pdst, int _nIndex);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dst_set_type_enum(IntPtr _pdst, int _nIndex, byte[] _strEnum);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dst_set_type_comment(IntPtr _pdst, int _nIndex, byte[] _strComment);

        // here from 2019/09/18

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr base_file_create();

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool base_file_asc_open(IntPtr _bfile, byte[] _strStream, int _size_n);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool base_file_open_u8(IntPtr _bfile, byte[] _filename_str);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool base_file_open(IntPtr _bfile, byte[] _filename_str, int _type);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool base_file_asc_write_line(IntPtr _bfile);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool base_file_asc_write_string(IntPtr _bfile, byte[] _str);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void base_file_seperator_set(IntPtr _bfile, byte[] _strSeperator);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void base_file_delector_set(IntPtr _bfile, byte[] _strDelector);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool base_file_asc_read_line(IntPtr _bfile);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr base_file_asc_read_string(IntPtr _bfile);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool base_file_asc_read_int(IntPtr _bfile, ref int _nValue);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool base_file_asc_read_float(IntPtr _bfile, ref float _nValue);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool base_file_delete(IntPtr _bfile);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr vscript_column_name_get_(int _nIndex);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr static_variable_make_string(IntPtr _pdsvBase, IntPtr _pdsvEvent,
            IntPtr _pdsvContext, byte[] _strFormat);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool load_asc_for_etagen(IntPtr _pdstVoid, byte[] _strFilename);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool save_asc_for_etagen(IntPtr _pdstVoid, byte[] _strFilename);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern int static_variable_param_int_get(IntPtr _pdsvBase, IntPtr _pdsvEvent, IntPtr _pdsvContext,
            int _nSeq, IntPtr _pdsvDefault);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr static_variable_param_string_get(IntPtr _pdsvBase, IntPtr _pdsvEvent,
            IntPtr _pdsvContext, int _nSeq, IntPtr _pdsvDefault);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool static_variable_param_void_set(IntPtr _pdsvBase, IntPtr _pdsvEvent,
            IntPtr _pdsvContext, int _nSeq, IntPtr _pdsvDefault, IntPtr _value_p, int _cnt);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr static_variable_param_void_get(IntPtr _pdsvBase, IntPtr _pdsvEvent,
            IntPtr _pdsvContext, int _nSeq, ref int _size);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr static_translate(byte[] _str);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr filedialog_open(byte[] _str);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void manager_update(IntPtr _manager);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr filedialog_save(byte[] _str);


#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void manager_url_set(byte[] _str);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr manager_url_get();
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr dst_create();
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void dst_delete(IntPtr _pdst);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dst_save(IntPtr _pdst, byte[] _strFileName, int _type);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dst_load(IntPtr _pdst, byte[] _strFileName, int _type);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern int dst_get_index(int _nHash);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void dst_clear(IntPtr _pdst);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dst_add_string(IntPtr _pdst, int _nKey, int _nIndex, byte[] _strString);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dst_set_string(IntPtr _pdst, int _nKey, int _nIndex, byte[] _strString);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dst_add_float(IntPtr _pdst, int _nKey, int _nIndex, float[] _paData, int _nCnt);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dst_set_int(IntPtr _pdst, int _nKey, int _nIndex, int[] _paData, int _nCnt);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dst_add_int(IntPtr _pdst, int _nKey, int _nIndex, int[] _paData, int _nCnt);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dst_get_int(IntPtr _pdst, int _nKey, int _nSequence, ref IntPtr _ppaData,
            ref int _pnCnt);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dst_get_int_index(IntPtr _pdst, int _nKey, int _nIndex, ref IntPtr _ppaData,
            ref int _pnCnt);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr dst_get_string(IntPtr _pdst, int _nKey, int _nSequence);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dst_get_float(IntPtr _pdst, int _nKey, int _nSequence, ref IntPtr _ppaData,
            ref int _pnCnt);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr dsv_create(byte[] _strMainName, byte[] _strStateName);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr dsv_copy_get(IntPtr _pdst);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void dsv_delete(IntPtr _pdst);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr
            dsv_event_group_make(IntPtr _pdst, int _evt, int _groupId, int _id); // _pdst should be dsvBase.


#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr manager_event_group_make(IntPtr _manager_p, int _evt, int _groupId, int _id);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool manager_state_active(IntPtr _manager_p, int _key, int _group, int _id, int _serial);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void dsv_group_id_set(IntPtr _dsv_p, int _groupId, int _id);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_get_float(IntPtr _pdst, byte[] _strColName, ref float _fValue);
        
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_get_hash_float(IntPtr _pdst, int _hash, ref float _fValue);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_get_int(IntPtr _pdst, byte[] _strColName, ref int _pnLong);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_get_int2(IntPtr _pdst, int _nHash, ref int _pnLong);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_set_ptr(IntPtr _pdst, int _hash, IntPtr _pPtr, int _cnt);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_get_ptr(IntPtr _pdst, byte[] _strColName, ref IntPtr _pPtr);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_get_ptr2(IntPtr _pdst, int _hash, ref IntPtr _pPtr, ref int _cnt, int _seq);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_set_float(IntPtr _pdst, byte[] _strColName, float _fValue);
        
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_set_hash_float(IntPtr _pdst, int _hash, float _fValue);
        
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_set_hash_int(IntPtr _pdst, int _col_hash, int _nValue);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_set_int(IntPtr _pdst, byte[] _strColName, int _nValue);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_add_int(IntPtr _pdst, byte[] _strColName, int _nValue);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr dsv_get_string(IntPtr _pdst, byte[] _strColName);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr dsv_get_ldata(IntPtr _pdst, byte[] _strColName, ref int _pnCnt);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr dsv_get_string2(IntPtr _pdst, int _nHash);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_set_string(IntPtr _pdst, byte[] _strColName, byte[] _strValue);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_set_string2(IntPtr _pdst, int _nHash, byte[] _strValue);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif

        public static extern bool dsv_add_ldata(IntPtr _pdst, byte[] _strColName, byte[] _strValue, int _size);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_add_string(IntPtr _pdst, byte[] _strColName, byte[] _strValue);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_set_variable(IntPtr _pManager, IntPtr _pdst, byte[] _strColName, int _nValue);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_set_variable2(IntPtr _pManager, IntPtr _pdst, int _nHash, int _nValue);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool
            dsv_set_variable_ptr(IntPtr _pManager, IntPtr _pdst, byte[] _strColName, IntPtr _pPtr);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_set_variable_string(IntPtr _pManager, IntPtr _pdst, byte[] _strColName,
            byte[] _strValue);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool
            dsv_set_variable_string2(IntPtr _pManager, IntPtr _pdst, int _nHash, byte[] _strValue);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_add_variable(IntPtr _pManager, IntPtr _pdst, byte[] _strColName, int _nValue);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_add_variable_string(IntPtr _pManager, IntPtr _pdst, byte[] _strColName,
            byte[] _string);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_get_float_array(IntPtr _pdst, byte[] _strColName, ref IntPtr _ppnValue,
            ref int _pnCnt);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_get_int_array(IntPtr _pdst, byte[] _strColName, ref IntPtr _ppnValue,
            ref int _pnCnt);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_add_int_array(IntPtr _pdst, byte[] _strColName, int[] _anValue, int _pnCnt);
        
#if UNITY_IOS
        [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_set_void(IntPtr _pdst, int _hash, IntPtr _aValue, int _pnCnt);
        
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_set_float_array(IntPtr _pdst, byte[] _strColName, float[] _afValue, int _pnCnt);
#if UNITY_IOS
        [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_add_float_array(IntPtr _pdst, byte[] _strColName, float[] _afValue, int _pnCnt);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dsv_param_get(IntPtr _pdst, byte[] _strHash, ref int _pnSeq, ref int _pnHashVariable);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern int dsv_serial_get(IntPtr _pdst);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr dsv_state_variable(IntPtr _pdsv);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern int dsv_state_timelocal_get(IntPtr _pdsv);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern int dsv_state_function_hash(IntPtr _pdsv);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void dsv_event_cast_reset(IntPtr _pdsv);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr dsv_event_cast_get(IntPtr _pdsv, int _event, bool _new);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr dsv_column_get(IntPtr _pdsv, int _column_n, ref int _hash_n, ref int _count_pn);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void consol_load_hash(byte[] _strFile);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void consol_hide(byte[] _strTitle, byte[] _strValue);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void consol_show(byte[] _strTitle, byte[] _strValue);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr manager_enum_get(IntPtr _pManager, byte[] _strEnum);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr
            manager_create(byte[] _strRootPath); // Parameter path should include '/' terminal at the last.
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr vscript_init(byte[] _strAppName, byte[] _strRootPath, byte[] _strDataPath,
            byte[] _strConsolIP, int _nLang); // Parameter path should include '/' terminal at the last.
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void
            manager_language_change(int _nCountryCode); // Parameter path should include '/' terminal at the last.
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr manager_event_make(IntPtr _pManager, byte[] _strEvent);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void manager_weakup(IntPtr _pManager);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void manager_variable_define(IntPtr _manager, byte[] _name, int _type);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void manager_structure_define(IntPtr _manager, byte[] _table, byte[] _columns);


#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void db_column_set(byte[] _column, int _type, byte[] _table);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void db_columntable_set(int _table_hash, byte[] _columns);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void manager_event_post(IntPtr _pManager, IntPtr _pdstEvent, int _space, int _priority);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void manager_event_process(IntPtr _pManager, IntPtr _pdstEvent);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void manager_close(IntPtr _pManager);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern int manager_hash(byte[] _strHash);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void manager_message(int _nFilter, byte[] _strMsg);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool manager_enum_get_int(IntPtr _pManager, byte[] _strEnum, int _nKey, int _nHashCol,
            ref int _pnValue);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr
            manager_enum_get_string(IntPtr _pManager, byte[] _strEnum, int _nKey, int _nHashCol);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void manager_reg_func(byte[] _strColum, fnFuncProcessor _stFunc, byte[] _file, int _line);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern int dst_key_first_get(IntPtr _pdst);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern int dst_key_next_get(IntPtr _pdst);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr manager_variable_global_get(IntPtr _pManager);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void manager_keypush(IntPtr _manager_p, int _scan1, bool _push, int _scan2);
        // 2019/09/19 appended from here
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dst_type_original_check(IntPtr _pdst, int _nHash);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern int dst_get_hash(int _nIndex);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void dst_add_row_alloc(IntPtr _pdst, int _nKey, int _nIndex, IntPtr _pVoid, short _nCount);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern int dst_get_index_seq(IntPtr _pdst, int _nKey, int _nSequence);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dst_get(IntPtr _pdst, int _nKey, ref IntPtr _ppVoid, ref short _pnCount,
            int _nIndexSequence);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dst_get_param(IntPtr _pdst, int _nKey, int _nIndex, ref IntPtr _ppVoid,
            ref short _pnCount, int _nIndexStartSequence);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void
            dst_set_alloc(IntPtr _pdst, int _nKey, int _nIndex, IntPtr _pVoid,
                short _nCount); // create memory, copy and record the position
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void
            dst_set(IntPtr _pdst, int _nKey, int _nIndex,
                IntPtr _pVoid); // just record the point, it can make memory error.
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dst_get_by_hash(IntPtr _pdst, int _nKey, int _nHash, ref IntPtr _ppVoid);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dst_release(IntPtr _pdst, int _nKey);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dst_release_rows(IntPtr _pdst, int _nKey);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dst_is_exist(IntPtr _pdst, int _nKey); // check exist value
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dst_change_key(IntPtr _pdst, int _nKey, int _nKeyto);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern int dst_get_count_colum(IntPtr _pdst, int _nKey);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern int dst_getn_flag(IntPtr _pdst, byte[] _strFlag);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr dst_getstr_flag(IntPtr _pdst, int _nFlag);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern int dst_create_key_link(IntPtr _pdst, byte[] _str_key);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr dst_editor_string_get(IntPtr _pdst, int _key_n, int _seq_n);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern bool dst_editor_string_add(IntPtr _pdst, int _key_n, int _index_n, byte[] _str_value);

#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern IntPtr base_malloc(int _size_n);
#if UNITY_IOS
    [DllImport ("__Internal")]
#else
        [DllImport("ApStateSystem")]
#endif
        public static extern void base_free(IntPtr _point);

    }
}