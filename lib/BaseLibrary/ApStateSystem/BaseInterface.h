//
//  BaseInterface.h
//  BaseLibrary
//
//  Created by Jung Tae Seo on 10/15/15.
//  Copyright © 2015 Jung Tae Seo. All rights reserved.
//

//#ifdef BASELIBRARY_EXPORTS
#ifdef _WIN32
#define DLLEXPORT	__declspec(dllexport)
#else
#define DLLEXPORT
//typedef int (*fnEventProcessor)(const void* _pdstBase, void* _pdstEvent, void* _pdstContext, int _nState);
//typedef signed __int32		INT32;
//typedef signed __int64      INT64;
#endif
float _fmodf(float _fUp, float _fDn);

extern "C" DLLEXPORT void manager_weakup(void *_pManager);
extern "C" DLLEXPORT char *static_translate(const char *_str);
extern "C" DLLEXPORT void *static_variable_make_string(void *_pdsvBase, void *_pdsvEvent, void *_pdsvContext, const char *_strFormat);
extern "C" DLLEXPORT int static_variable_param_int_get(void *_pdsvBase, void *_pdsvEvent, void *_pdsvContext, int _nSeq, void *_pdsvDefault);
extern "C" DLLEXPORT char *static_variable_param_string_get(void *_pdsvBase, void *_pdsvEvent, void *_pdsvContext, int _nSeq, void *_pdsvDefault);

extern "C" DLLEXPORT bool static_variable_param_void_set(void* _pdsvBase, void* _pdsvEvent, void* _pdsvContext, int _nSeq, void* _pdsvDefault, const void* _void_p, int _cnt);
extern "C" DLLEXPORT const void *static_variable_param_void_get(void* _pdsvBase, void* _pdsvEvent, void* _pdsvContext, int _nSeq, int *_cnt);

extern "C" DLLEXPORT int system_memory_alloc_size();

extern "C" DLLEXPORT wchar_t *sys_towide(const char*_in, wchar_t*_out, int _size);
extern "C" DLLEXPORT char* sys_toutf8(const wchar_t* _in, char* _out, int _size);

extern "C" DLLEXPORT void manager_url_set(const char *_str);
extern "C" DLLEXPORT const void *manager_url_get();
extern "C" DLLEXPORT void consol_load_hash(const char *_strFile);
extern "C" DLLEXPORT void consol_hide(const char *_strTitle, const char *_strValue);
extern "C" DLLEXPORT void consol_show(const char *_strTitle, const char *_strValue);
extern "C" DLLEXPORT void consol_set_debugout(fnEventProcessor _fnFunc);
extern "C" DLLEXPORT void* manager_create(const char *_strRootPath); 
extern "C" DLLEXPORT void vscript_init(const char *_strAppName, const char *_strRootPath, const char *_strDataPath, const char *_strConsolIP, int _nLang);// Parameter path should include '/' terminal at the last.
extern "C" DLLEXPORT void* manager_event_make(void *_pManager, const char *_strEvent);
extern "C" DLLEXPORT void manager_event_post(void *_pManager, void *_pdstEvent, int _space, int _priority);
extern "C" DLLEXPORT void manager_event_process(void *_pManager, void *_pdstEvent);
extern "C" DLLEXPORT void manager_update(void *_pManager);
extern "C" DLLEXPORT void manager_close(void *_pManager);
extern "C" DLLEXPORT void manager_reg_func(const char *_strName, fnEventProcessor _fnFunc, const char *_file, int _line);
extern "C" DLLEXPORT int manager_hash(const char *_strHash);
extern "C" DLLEXPORT void manager_message(int _nFilter, const char *_strMsg);
extern "C" DLLEXPORT bool manager_enum_get_int(void *_pManager, const char *_strEnum, int _nKey, int _nHashCol, INT32 *_pnValue);
extern "C" DLLEXPORT void* manager_enum_get(void *_pManager, const char *_strEnum);
extern "C" DLLEXPORT void *manager_enum_get_string(void *_pManager, const char *_strEnum, int _nKey, int _nHashCol);
extern "C" DLLEXPORT void* manager_variable_global_get(void *_pManager);
extern "C" DLLEXPORT void manager_keypush(void *_manager_p, int _scan1, bool _push, int _scan2);
extern "C" DLLEXPORT void manager_variable_define(void *_manager, const char *_name, int _type);
extern "C" DLLEXPORT void manager_structure_define(void* _manager_p, const char* _table, const char* _columns);
extern "C" DLLEXPORT void* manager_event_group_make(void* _manager_p, int _evt, int _groupId, int _id);
extern "C" DLLEXPORT bool manager_state_active(void *_manager_p, int _nKeyName, int _group, int _id, int _nSerial);

extern "C" DLLEXPORT void db_column_set(const char* _column, int _type, const char* _table);
extern "C" DLLEXPORT void db_columntable_set(int _table_hash, const char* _columns);
extern "C" DLLEXPORT void thread_end();

extern "C" DLLEXPORT void manager_language_change(int _nCountryCode);
extern "C" DLLEXPORT char *filedialog_open(const char *_ext);
extern "C" DLLEXPORT char *filedialog_save(const char *_ext);

extern "C" DLLEXPORT const char*vscript_column_name_get_(int _nIndex);
extern "C" DLLEXPORT bool dsv_get_ptr2(void *_pdst, int _hash, void **_pPtr, int *_cnt, int _seq);
extern "C" DLLEXPORT bool dsv_get_ptr(void *_pdst, const char *_strColName, void **_pPtr);
extern "C" DLLEXPORT bool dsv_set_variable_ptr(void *_pManager, void *_pdst, const char *_strColName, void *_pPtr);

extern "C" DLLEXPORT int dsv_state_function_hash(void* _pdsv);
extern "C" DLLEXPORT void* dsv_state_variable(void *_pdsv);
extern "C" DLLEXPORT int dsv_state_timelocal_get(void *_pdsv);
extern "C" DLLEXPORT void *dsv_event_cast_get(void* _pdsv, int _event, bool _new);
extern "C" DLLEXPORT void dsv_event_cast_reset(void* _pdsv);
extern "C" DLLEXPORT void *dsv_create(const char *_strMainName, const char *_strStateName);
extern "C" DLLEXPORT bool dsv_get_float(void *_pdst, const char *_strColName, float *_pfValue);
extern "C" DLLEXPORT bool dsv_get_hash_float(void* _pdst,int _hash, float* _pfValue);
extern "C" DLLEXPORT bool dsv_get_float_array(void *_pdst, const char *_strColName, const float **_ppfValue, INT32 *_pnCount);
extern "C" DLLEXPORT bool dsv_set_float_array(void* _pdst, const char* _strColName, const float* _pfValue, INT32 _nCount);
extern "C" DLLEXPORT bool dsv_set_void(void* _pdst, int _hash, const void *_pValue, INT32 _nCount);
extern "C" DLLEXPORT bool dsv_get_int2(void *_pdst, int _nHash, INT32 *_pnLong);
extern "C" DLLEXPORT bool dsv_get_int(void *_pdst, const char *_strColName, INT32 *_pnInt);
extern "C" DLLEXPORT bool dsv_get_int_array(void *_pdst, const char *_strColName, const INT32 **_ppnIntArray, INT32 *_pnCount);
extern "C" DLLEXPORT bool dsv_set_ptr(void * _pdst, int _hash, void *_pPtr, int _cnt);
extern "C" DLLEXPORT bool dsv_set_int(void *_pdst, const char *_strColName, INT32 _nValue);
extern "C" DLLEXPORT bool dsv_set_hash_int(void* _pdst, int _col_hash, INT32 _nValue);
extern "C" DLLEXPORT bool dsv_set_float(void* _pdst, const char* _strColName, float _fValue);
extern "C" DLLEXPORT bool dsv_set_hash_float(void* _pdst, int _hash, float _fValue);
extern "C" DLLEXPORT bool dsv_add_int(void *_pdst, const char *_strColName, INT32 _nValue);
extern "C" DLLEXPORT bool dsv_add_int64(void* _pdst, const char* _strColName, INT64 _nValue);
extern "C" DLLEXPORT bool dsv_add_int_array(void *_pdst, const char *_strColName, INT32 *_anValue, INT32 _nCnt);
extern "C" DLLEXPORT bool dsv_add_float_array(void* _pdst, const char* _strColName, float* _afValue, INT32 _nCnt);
extern "C" DLLEXPORT void *dsv_get_string(void *_pdst, const char *_strColName);
extern "C" DLLEXPORT void *dsv_get_string2(void *_pdst, INT32 _nHash);
extern "C" DLLEXPORT bool dsv_set_string(void *_pdst, const char *_strColName, const char *_strValue);
extern "C" DLLEXPORT bool dsv_set_string2(void *_pdst, INT32 _nHash, const char *_strValue);
extern "C" DLLEXPORT bool dsv_add_string(void *_pdst, const char *_strColName, const char *_strValue);
extern "C" DLLEXPORT bool dsv_add_ldata(void* _pdst, const char* _strColName, const char* _strValue, int _size);
extern "C" DLLEXPORT void* dsv_get_ldata(void* _pdst, INT32 _nKey, int *_pnCnt);
extern "C" DLLEXPORT bool dsv_set_variable(void *_pManager, void *_pdst, const char *_strColName, INT32 _nValue);
extern "C" DLLEXPORT bool dsv_set_variable2(void *_pManager, void *_pdst, int _nHash, INT32 _nValue);
extern "C" DLLEXPORT bool dsv_set_variable_string(void *_pManager, void *_pdst, const char *_strColName, const char *_strValue);
extern "C" DLLEXPORT bool dsv_set_variable_string2(void *_pManager, void *_pdst, INT32 _nHash, const char *_strValue);
extern "C" DLLEXPORT bool dsv_add_variable_string(void* _pManager, void* _pdst, const char* _strColName, const char* _str);
extern "C" DLLEXPORT bool dsv_add_variable(void *_pManager, void *_pdst, const char *_strColName, INT32 _nValue);
extern "C" DLLEXPORT bool dsv_param_get(void *_pdst, const char *_strHash, INT32 *_pnSeq, INT32 *_pnHashVariable);
extern "C" DLLEXPORT int dsv_serial_get(void *_pdsv);
extern "C" DLLEXPORT const void* dsv_column_get(void* _dsv_p, int _column_n, int* _hash_pn, int *_count_pn);
extern "C" DLLEXPORT void* dsv_copy_get(void* _dsv_p);
extern "C" DLLEXPORT void dsv_delete(void *_dsv_p);
extern "C" DLLEXPORT void * dsv_event_group_make(void *_dsv_p, int _evt, int _groupId, int _id);
extern "C" DLLEXPORT void dsv_group_id_set(void* _dsv_p, int _groupId, int _id);

extern "C" DLLEXPORT void *dst_create();
extern "C" DLLEXPORT void dst_delete(void *_pdst);
extern "C" DLLEXPORT bool load_asc_for_etagen(void *_pdstVoid, const char *_strFilename);
extern "C" DLLEXPORT bool save_asc_for_etagen(void *_pdstVoid, const char *_strFilename);
extern "C" DLLEXPORT bool dst_save(void *_pdst, const char *_strFileName, int _type);
extern "C" DLLEXPORT bool dst_load(void *_pdst, const char *_strFileName, int _type);
extern "C" DLLEXPORT int dst_get_index(INT32 _nHash);
extern "C" DLLEXPORT void dst_clear(void *_pdst);
extern "C" DLLEXPORT bool dst_add_string(void *_pdst, INT32 _nKey, INT32 _nIndex, const char *_strString);
extern "C" DLLEXPORT bool dst_set_string(void *_pdst, INT32 _nKey, INT32 _nIndex, const char *_strString);
extern "C" DLLEXPORT bool dst_add_float(void *_pdst, INT32 _nKey, INT32 _nIndex, float *_paData, INT32 _nCnt);
extern "C" DLLEXPORT bool dst_add_int(void *_pdst, INT32 _nKey, INT32 _nIndex, INT32 *_paData, INT32 _nCnt);
extern "C" DLLEXPORT bool dst_set_int(void* _pdst, INT32 _nKey, INT32 _nIndex, INT32* _paData, INT32 _nCnt);
extern "C" DLLEXPORT bool dst_get_int(void *_pdst, INT32 _nKey, INT32 _nSequence, const INT32 **_paData, INT32 *_pnCnt);
extern "C" DLLEXPORT bool dst_get_int_index(void* _pdst, INT32 _nKey, INT32 _nIndex, const INT32** _paData, INT32* _pnCnt);
extern "C" DLLEXPORT void *dst_get_string(void *_pdst, INT32 _nKey, INT32 _nSequence);
extern "C" DLLEXPORT bool dst_get_float(void *_pdst, INT32 _nKey, INT32 _nSequence, const float **_paData, INT32 *_pnCnt);
// update by 2019/09/05
extern "C" DLLEXPORT int dst_key_first_get(void* _pdst);
extern "C" DLLEXPORT int dst_key_next_get(void* _pdst);
// append from 2019/09/18
extern "C" DLLEXPORT int dst_get_type_count(void *_pdst);
extern "C" DLLEXPORT int dst_get_type_flag(void* _pdst, int _nIndex);
extern "C" DLLEXPORT bool dst_set_type_flag(void* _pdst, int _nIndex, int _nFlag);
extern "C" DLLEXPORT const char* dst_get_type_name(void* _pdst, int _nIndex);
extern "C" DLLEXPORT int dst_get_type(int _nIndex);
extern "C" DLLEXPORT int dst_get_type_size(void* _pdst, int _nIndex);
extern "C" DLLEXPORT const char* dst_get_type_enum_state(void* _pdst, int _nIndex);
extern "C" DLLEXPORT const char* dst_get_type_comment(void* _pdst, int _nIndex);
extern "C" DLLEXPORT bool dst_set_type_enum(void* _pdst, int _nIndex, const char* _strEnum);
extern "C" DLLEXPORT bool dst_set_type_comment(void* _pdst, int _nIndex, const char* _strComment);
//append from 2019/09/19
extern "C" DLLEXPORT bool dst_type_original_check(void* _pdst, int _nHash);
extern "C" DLLEXPORT int dst_get_hash(int _nIndex);
extern "C" DLLEXPORT void dst_add_row_alloc(void* _pdst, int _nKey, int _nIndex, const void* _pVoid, short _nCount);
extern "C" DLLEXPORT int dst_get_index_seq(void* _pdst, int _nKey, int _nSequence);
extern "C" DLLEXPORT bool dst_get(void* _pdst, int _nKey, const void** _ppVoid, short* _pnCount, int _nIndexSequence);
extern "C" DLLEXPORT bool dst_get_param(void* _pdst, int _nKey, int _nIndex, const void** _ppVoid, short* _pnCount, int _nIndexStartSequence);
//extern "C" DLLEXPORT bool dst_get(void* _pdst, int _nKey, int _nIndex, const void** _ppVoid, short* _pnCount, int _nIndexSequence);
extern "C" DLLEXPORT void dst_set_alloc(void* _pdst, int _nKey, int _nIndex, const void* _pVoid, short _nCount);	// create memory, copy and record the position
extern "C" DLLEXPORT void dst_set(void* _pdst, int _nKey, int _nIndex, void* _pVoid);		// just record the point, it can make memory error.
extern "C" DLLEXPORT bool dst_get_by_hash(void* _pdst, int _nKey, int _nHash, const void** _ppVoid);
extern "C" DLLEXPORT bool dst_release(void* _pdst, int _nKey);
extern "C" DLLEXPORT bool dst_release_rows(void* _pdst, int _nKey);
extern "C" DLLEXPORT bool dst_is_exist(void* _pdst, int _nKey);	// check exist value
extern "C" DLLEXPORT bool dst_change_key(void* _pdst, int _nKey, int _nKeyto);
extern "C" DLLEXPORT int dst_get_count_colum(void* _pdst, int _nKey);
extern "C" DLLEXPORT int dst_getn_flag(void* _pdst, const char* _strFlag);
extern "C" DLLEXPORT char *dst_getstr_flag(void* _pdst, int _nFlag);
extern "C" DLLEXPORT int dst_create_key_link(void* _pdst, const char *_str_key);
extern "C" DLLEXPORT char *dst_editor_string_get(void* _pdst, int _key_n, int _seq_n);
extern "C" DLLEXPORT bool dst_editor_string_add(void* _pdst, int _key_n, int _seq_n, char *_str_value);

extern "C" DLLEXPORT void *base_file_create();
extern "C" DLLEXPORT bool base_file_open_u8(void* _bfile, const char* _filename_str);
extern "C" DLLEXPORT bool base_file_open(void* _bfile, const char* _filename_str, int _type);
extern "C" DLLEXPORT bool base_file_asc_open(void* _bfile, const char* _strStream, int _size_n);
extern "C" DLLEXPORT void base_file_seperator_set(void* _bfile, const char* _strSeperator);
extern "C" DLLEXPORT void base_file_delector_set(void* _bfile, const char* _strDelector);
extern "C" DLLEXPORT bool base_file_asc_read_line(void* _bfile);
extern "C" DLLEXPORT bool base_file_asc_write_line(void* _bfile);
extern "C" DLLEXPORT bool base_file_asc_write_string(void* _bfile, const char* _str);
extern "C" DLLEXPORT void *base_file_asc_read_string(void* _bfile);
extern "C" DLLEXPORT void* base_file_read(void* _bfile);
extern "C" DLLEXPORT bool base_file_asc_read_int(void* _bfile, int *_nValue);
extern "C" DLLEXPORT bool base_file_asc_read_float(void* _bfile, float* _nValue);
extern "C" DLLEXPORT bool base_file_delete(void* _bfile);

extern "C" DLLEXPORT bool base_file_read_ushort(void* _bfile, unsigned short *_value);
extern "C" DLLEXPORT bool base_file_read_ulong(void* _bfile, unsigned long *_value);
//extern "C" DLLEXPORT char *base_file_read_str(void* _bfile);
extern "C" DLLEXPORT bool base_file_read_uchar(void* _bfile, unsigned char *_value);
extern "C" DLLEXPORT bool base_file_read_double(void* _bfile, double *_value);
extern "C" DLLEXPORT bool base_file_read_ullong(void* _bfile, unsigned long long *_value);
extern "C" DLLEXPORT bool base_file_seek(void* _bfile, unsigned long _offset);

extern "C" DLLEXPORT void* base_malloc(int _size_n);
extern "C" DLLEXPORT void base_free(void* _point);

