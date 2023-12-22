using UnityEngine;
using System.Collections;
using System;
using System.Runtime.InteropServices;

using System.IO;
using UnityEngine.UI;
using System.Text;
using System.Collections.Generic;

namespace StateSystem
{
    public class BaseFile
    {
        IntPtr m_dst_p;

        public BaseFile(IntPtr _dst_p)
        {
            m_dst_p = _dst_p;
        }

        public static BaseFile create()
        {
            IntPtr dst_p = StateInterface.base_file_create();
            BaseFile fileBase = new BaseFile(dst_p);

            return fileBase;
        }

        public bool asc_open(string _stream_str)
        {
            if (m_dst_p == IntPtr.Zero)
                return false;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_stream_str + "\0");
            return StateInterface.base_file_asc_open(m_dst_p, temp, temp.Length);
        }

        public bool open_file_u8(string _filename_str)
        {
            if (m_dst_p == IntPtr.Zero)
                return false;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_filename_str + "\0");
            return StateInterface.base_file_open_u8(m_dst_p, temp);
        }

        public bool open_file(string _filename_str)
        {
            if (m_dst_p == IntPtr.Zero)
                return false;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_filename_str + "\0");
            return StateInterface.base_file_open(m_dst_p, temp, 0);
        }

        public bool open_file_write(string _filename_str)
        {
            if (m_dst_p == IntPtr.Zero)
                return false;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_filename_str + "\0");
            return StateInterface.base_file_open(m_dst_p, temp, 1);
        }

        public void seperator_set(string _seperator_str)
        {
            if (m_dst_p == IntPtr.Zero)
                return;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_seperator_str + "\0");
            StateInterface.base_file_seperator_set(m_dst_p, temp);
        }

        public void delector_set(string _delector_str)
        {
            if (m_dst_p == IntPtr.Zero)
                return;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_delector_str + "\0");
            StateInterface.base_file_delector_set(m_dst_p, temp);
        }

        public bool asc_write_line()
        {
            if (m_dst_p == IntPtr.Zero)
                return false;
            return StateInterface.base_file_asc_write_line(m_dst_p);
        }

        public bool asc_write_string(string _str)
        {
            if (m_dst_p == IntPtr.Zero)
                return false;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_str + "\0");
            return StateInterface.base_file_asc_write_string(m_dst_p, temp);
        }

        public bool asc_read_line()
        {
            if (m_dst_p == IntPtr.Zero)
                return false;
            return StateInterface.base_file_asc_read_line(m_dst_p);
        }

        public bool asc_read_string(ref string _str)
        {
            if (m_dst_p == IntPtr.Zero)
                return false;
            IntPtr pRet = StateInterface.base_file_asc_read_string(m_dst_p);

            if (pRet == IntPtr.Zero)
                return false;

            _str = VLStateManager.MarshalToString(pRet);
            return true;
        }

        public bool asc_read_int(ref int _n)
        {
            if (m_dst_p == IntPtr.Zero)
                return false;
            return StateInterface.base_file_asc_read_int(m_dst_p, ref _n);
        }

        public bool asc_read_float(ref float _f)
        {
            if (m_dst_p == IntPtr.Zero)
                return false;
            return StateInterface.base_file_asc_read_float(m_dst_p, ref _f);
        }

        public void delete()
        {
            if (m_dst_p == IntPtr.Zero)
                return;
            StateInterface.base_file_delete(m_dst_p);
        }

        public static string make_string(List<List<string>> _column_aa, char _seperator)
        {
            string stream = "";
            for (int i = 0; i < _column_aa.Count; i++)
            {
                List<string> str_a = _column_aa[i];
                for (int j = 0; j < str_a.Count; j++)
                {
                    stream += str_a[j];
                    if (j != str_a.Count - 1)
                        stream += _seperator;
                }

                if (i != _column_aa.Count - 1)
                    stream += '\r';
            }

            return stream;
        }

        public static List<List<string>> paser_list(string _str, string _seperator)
        {
            List<List<string>> list_aa = new List<List<string>>();
            // marker
            BaseFile paser = BaseFile.create();
            paser.asc_open(_str);

            paser.seperator_set(_seperator);
            //paser.delector_set(" ");

            while (paser.asc_read_line())
            {
                List<string> list_a = new List<string>();
                string tag = "";
                while (paser.asc_read_string(ref tag))
                {
                    //menu_str = VLStateManager.translate(menu_str);
                    list_a.Add(tag);
                }

                list_aa.Add(list_a);
            }

            paser.delete();
            return list_aa;
        }
    }

    public class StateDStructure
    {
        //BaseLibrary
        //__Internal
        public IntPtr m_pDst;
        public IntPtr m_pManager;

        public enum type
        {
            TYPE_INT32 = 4,
            TYPE_FLOAT = 6,
            TYPE_STRING = 8,
            TYPE_ARRAY_INT32 = 11,
            TYPE_ARRAY_FLOAT = 13
        };

        static public StateDStructure create()
        {
            StateDStructure pdst;
            pdst = new StateDStructure(StateInterface.dst_create());

            return pdst;
        }

        public void delete()
        {
            StateInterface.dst_delete(m_pDst);
        }

        public StateDStructure(IntPtr _pDst)
        {
            m_pDst = _pDst;
            m_pManager = VLStateManager.get_manager();
        }

        public bool save(string _strFilename, int _type)
        {
            if (m_pDst == IntPtr.Zero)
                return false;

            if (_strFilename == null)
                return StateInterface.dst_save(m_pDst, null, _type);
            //System.Text.Encoding. .Default.GetBytes
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strFilename + "\0");
            return StateInterface.dst_save(m_pDst, temp, _type);
            //return VLStateManager.save_asc_for_etagen(m_pDst, temp);
            //return false;
        }

        public bool load(string strFilename, int _type)
        {
            // _type:0 normal, _type:1 ascii, _type:2 reload define with ascii
            if (m_pDst == IntPtr.Zero)
                return false;

            byte[] temp = System.Text.Encoding.UTF8.GetBytes(strFilename + "\0");
            return StateInterface.dst_load(m_pDst, temp, _type);
            //return VLStateManager.load_asc_for_etagen(m_pDst, temp);
        }

        static public int get_index(int _nHash)
        {
            return StateInterface.dst_get_index(_nHash);
        }


        public bool clear(int _nHash)
        {
            if (m_pDst == IntPtr.Zero)
                return false;
            StateInterface.dst_clear(m_pDst);
            return true;
        }

        public bool add_string(int _nKey, int _nIndex, string _str)
        {
            if (m_pDst == IntPtr.Zero)
                return false;

            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_str + "\0");
            return StateInterface.dst_add_string(m_pDst, _nKey, _nIndex, temp);
        }

        public bool set_string(int _nKey, int _nIndex, string _str)
        {
            if (m_pDst == IntPtr.Zero)
                return false;

            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_str + "\0");
            return StateInterface.dst_set_string(m_pDst, _nKey, _nIndex, temp);
        }
        
        public bool add_float(int _nKey, int _nIndex, float[] _value)
        {
            if (m_pDst == IntPtr.Zero)
                return false;

            return StateInterface.dst_add_float(m_pDst, _nKey, _nIndex, _value, _value.Length);
        }

        public bool add_vector3(int _nKey, int _nIndex, Vector3 _v3V)
        {
            if (m_pDst == IntPtr.Zero)
                return false;

            float[] afVector = new float[3];
            afVector[0] = _v3V.x;
            afVector[1] = _v3V.y;
            afVector[2] = _v3V.z;

            return StateInterface.dst_add_float(m_pDst, _nKey, _nIndex, afVector, 3);
        }

        // _nKey is the key for access row, the value of key is same with 'nKey' value.
        // If 'nKey' value is not exist in the DST(Dynamic Structure), the row can't access.
        // I mean that you should add the 'nKey' value for a none exist row.
        public bool add_int(int _nKey, int _nIndex, int[] _value)
        {
            if (m_pDst == IntPtr.Zero)
                return false;

            return StateInterface.dst_add_int(m_pDst, _nKey, _nIndex, _value, _value.Length);
        }

        public bool set_int(int _nKey, int _nIndex, int[] _value)
        {
            if (m_pDst == IntPtr.Zero)
                return false;
            int len = 0;
            if (_value != null)
                len = _value.Length;
            return StateInterface.dst_set_int(m_pDst, _nKey, _nIndex, _value, len);
        }

        public bool get_int(int _nKey, int _nSequence, ref int[] _value)
        {
            if (m_pDst == IntPtr.Zero)
                return false;

            bool bRet = false;
            int nCnt = 0;
            IntPtr pnValue = IntPtr.Zero;
            bRet = StateInterface.dst_get_int(m_pDst, _nKey, _nSequence, ref pnValue, ref nCnt);

            if (bRet)
            {
                _value = new int[nCnt];
                Marshal.Copy(pnValue, _value, 0, nCnt);
            }

            return bRet;
        }

        public bool get_int_index(int _nKey, int _nIndex, ref int[] _value)
        {
            if (m_pDst == IntPtr.Zero)
                return false;

            bool bRet = false;
            int nCnt = 0;
            IntPtr pnValue = IntPtr.Zero;
            bRet = StateInterface.dst_get_int_index(m_pDst, _nKey, _nIndex, ref pnValue, ref nCnt);

            if (bRet)
            {
                _value = new int[nCnt];
                Marshal.Copy(pnValue, _value, 0, nCnt);
            }

            return bRet;
        }

        public bool get_hash_string(int _key, int _hash, ref string _str)
        {
            if (m_pDst == IntPtr.Zero)
                return false;

            IntPtr pRet = IntPtr.Zero;
            if (!StateInterface.dst_get_by_hash(m_pDst, _key, _hash, ref pRet))
                return false;

            _str = VLStateManager.MarshalToString(pRet);
            _str = _str.Trim();
            return true;
        }

        public bool get_string(int _nKey, int _nSequence, ref string _str)
        {
            if (m_pDst == IntPtr.Zero)
                return false;

            IntPtr pRet = StateInterface.dst_get_string(m_pDst, _nKey, _nSequence);
            if (pRet == IntPtr.Zero)
                return false;

            _str = VLStateManager.MarshalToString(pRet);
            _str = _str.Trim();
            return true;
        }

        public bool get_float(int _nKey, int _nSequence, ref float[] _value)
        {
            if (m_pDst == IntPtr.Zero)
                return false;

            bool bRet = false;
            int nCnt = 0;
            IntPtr pnValue = IntPtr.Zero;
            bRet = StateInterface.dst_get_float(m_pDst, _nKey, _nSequence, ref pnValue, ref nCnt);

            if (bRet)
            {
                _value = new float[nCnt];
                Marshal.Copy(pnValue, _value, 0, nCnt);
            }

            return bRet;
        }

        public bool get_vector3(int _nKey, int _nSequence, ref Vector3 _v3)
        {
            bool bRet;
            float[] afV3 = new float[3];
            bRet = get_float(_nKey, _nSequence, ref afV3);
            if (afV3.Length < 3)
                bRet = false;
            if (bRet)
                _v3.Set(afV3[0], afV3[1], afV3[2]);
            return bRet;
        }

        public int key_first_get()
        {
            if (m_pDst == IntPtr.Zero)
                return 0;
            return StateInterface.dst_key_first_get(m_pDst);
        }

        public int key_next_get()
        {
            return StateInterface.dst_key_next_get(m_pDst);
        }

        public int get_type_count()
        {
            if (m_pDst == IntPtr.Zero)
                return 0;
            return StateInterface.dst_get_type_count(m_pDst);
        }

        public int get_type_flag(int _nIndex)
        {
            if (m_pDst == IntPtr.Zero)
                return -1;
            return StateInterface.dst_get_type_flag(m_pDst, _nIndex);
        }

        public bool set_type_flag(int _nIndex, int _nFlag)
        {
            if (m_pDst == IntPtr.Zero)
                return false;
            return StateInterface.dst_set_type_flag(m_pDst, _nIndex, _nFlag);
        }

        public string get_type_name(int _nIndex)
        {
            if (m_pDst == IntPtr.Zero)
                return null;
            IntPtr pRet = StateInterface.dst_get_type_name(m_pDst, _nIndex);
            if (pRet == IntPtr.Zero)
                return null;

            return VLStateManager.MarshalToString(pRet);
        }

        static public int get_type(int _nIndex)
        {
            return StateInterface.dst_get_type(_nIndex);
        }

        public int get_type_size(int _nIndex)
        {
            if (m_pDst == IntPtr.Zero)
                return 0;
            return StateInterface.dst_get_type_size(m_pDst, _nIndex);
        }

        public string get_type_enum_state(int _nIndex)
        {
            if (m_pDst == IntPtr.Zero)
                return null;
            IntPtr pRet = StateInterface.dst_get_type_enum_state(m_pDst, _nIndex);
            if (pRet == IntPtr.Zero)
                return null;

            return VLStateManager.MarshalToString(pRet);
        }

        public string get_type_comment(int _nIndex)
        {
            if (m_pDst == IntPtr.Zero)
                return null;
            IntPtr pRet = StateInterface.dst_get_type_comment(m_pDst, _nIndex);
            if (pRet == IntPtr.Zero)
                return null;

            return VLStateManager.MarshalToString(pRet);
        }

        public bool set_type_enum(int _nIndex, string _strEnum)
        {
            if (m_pDst == IntPtr.Zero)
                return false;

            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strEnum + "\0");
            return StateInterface.dst_set_type_enum(m_pDst, _nIndex, temp);
        }

        public bool set_type_comment(int _nIndex, string _strComment)
        {
            if (m_pDst == IntPtr.Zero)
                return false;

            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strComment + "\0");
            return StateInterface.dst_set_type_comment(m_pDst, _nIndex, temp);
        }

        public int get_count_column(int _key_n)
        {
            if (m_pDst == IntPtr.Zero)
                return 0;

            return StateInterface.dst_get_count_colum(m_pDst, _key_n);
        }

        public int create_key_link(string _key_str)
        {
            if (m_pDst == IntPtr.Zero)
                return -1;

            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_key_str + "\0");
            return StateInterface.dst_create_key_link(m_pDst, temp);
        }

        public int get_index(int _key_n, int _seq_n)
        {
            if (m_pDst == IntPtr.Zero)
                return -1;

            return StateInterface.dst_get_index_seq(m_pDst, _key_n, _seq_n);
        }

        public bool string_add(int _key, List<List<string>> _column_aa)
        {
            for (int c = 0; c < _column_aa.Count; c++)
            {
                List<string> column_a = _column_aa[c];

                int hash = VLStateManager.hash(column_a[0]);
                int index = get_index(hash);

                if (index == -1)
                    continue;

                editor_string_add(_key, index, column_a[1]);
            }

            return true;
        }

        public bool string_get(int _key, ref List<List<string>> _column_aa)
        {
            int count_n = get_count_column(_key);

            if (count_n == 0)
                return false;

            _column_aa = new List<List<string>>();

            int i = 0;
            for (; i < count_n; i++)
            {
                List<string> str_a = new List<string>();
                int index_n = get_index(_key, i);
                str_a.Add(get_type_name(index_n));
                str_a.Add(editor_string_get(_key, i));
                _column_aa.Add(str_a);
            }

            return true;
        }

        public string editor_string_get(int _key_n, int _seq_n)
        {
            if (m_pDst == IntPtr.Zero)
                return null;

            IntPtr pRet = StateInterface.dst_editor_string_get(m_pDst, _key_n, _seq_n);
            if (pRet == IntPtr.Zero)
                return null;

            return VLStateManager.MarshalToString(pRet);
        }

        public bool editor_string_add(int _key_n, int _index_n, string _str)
        {
            if (m_pDst == IntPtr.Zero)
                return false;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_str + "\0");
            return StateInterface.dst_editor_string_add(m_pDst, _key_n, _index_n, temp);
        }

        public bool release(int _key_n)
        {
            if (m_pDst == IntPtr.Zero)
                return false;

            return StateInterface.dst_release(m_pDst, _key_n);
        }

        public bool release_rows(int _key_n)
        {
            if (m_pDst == IntPtr.Zero)
                return false;

            return StateInterface.dst_release_rows(m_pDst, _key_n);
        }

        public string flag_string_get(int _flag_n)
        {
            if (m_pDst == IntPtr.Zero)
                return null;

            IntPtr pRet = StateInterface.dst_getstr_flag(m_pDst, _flag_n);
            return VLStateManager.MarshalToString(pRet);
        }

        public int flag_get(string _str)
        {
            if (m_pDst == IntPtr.Zero)
                return -1;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_str + "\0");
            return StateInterface.dst_getn_flag(m_pDst, temp);
        }
    }

    public class StateDStructureValue
    {
        //BaseLibrary
        //__Internal
        public IntPtr m_pDStructureValue;
        public IntPtr m_pManager;

        public StateDStructureValue(IntPtr _pDst)
        {
            m_pDStructureValue = _pDst;
            m_pManager = VLStateManager.get_manager();
        }

        public bool set_vector(string _column_str, Vector3 _v3)
        {
            float[] af = new float[3];
            af[0] = _v3.x;
            af[1] = _v3.y;
            af[2] = _v3.z;

            return set_float_arry(_column_str, af);
        }

        public bool set_vector(int _hash, Vector3 _v3)
        {
            float[] af = new float[3];
            af[0] = _v3.x;
            af[1] = _v3.y;
            af[2] = _v3.z;

            return set_float_arry(_hash, af);
        }

        public bool get_vector(int _hash, ref Vector3 _vec)
        {
            bool bRet = false;
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            IntPtr pnValue = IntPtr.Zero;
            int cnt = 0;
            bRet = StateInterface.dsv_get_ptr2(m_pDStructureValue, _hash, ref pnValue, ref cnt, 0);
            if (bRet)
            {
                float[] ret_a = new float[cnt];
                Marshal.Copy(pnValue, ret_a, 0, cnt);
                _vec.x = ret_a[0];
                _vec.y = ret_a[1];
                _vec.z = ret_a[2];
            }

            return true;
        }

        public bool get_vector(string _strColName, ref Vector3 _vec)
        {
            bool bRet = false;
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strColName + "\0");
            IntPtr pnValue = IntPtr.Zero;
            int cnt = 0;
            bRet = StateInterface.dsv_get_float_array(m_pDStructureValue, temp, ref pnValue, ref cnt);
            if (bRet)
            {
                float[] ret_a = new float[cnt];
                Marshal.Copy(pnValue, ret_a, 0, cnt);
                _vec.x = ret_a[0];
                _vec.y = ret_a[1];
                _vec.z = ret_a[2];
            }

            return true;
        }

        public bool get_vector_a(string _strColName, ref List<Vector3> _vec_a)
        {
            bool bRet = false;
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strColName + "\0");
            IntPtr pnValue = IntPtr.Zero;
            int cnt = 0;
            bRet = StateInterface.dsv_get_float_array(m_pDStructureValue, temp, ref pnValue, ref cnt);
            if (bRet)
            {
                float[] ret_a = new float[cnt];
                Marshal.Copy(pnValue, ret_a, 0, cnt);
                for (int i = 0; i < cnt / 3; i++)
                {
                    Vector3 vec = new Vector3(ret_a[i * 3 + 0], ret_a[i * 3 + 1], ret_a[i * 3 + 2]);
                    _vec_a.Add(vec);
                }
            }

            return true;
        }

        public bool get_float(string _strColName, ref float _fValue)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            bool bRet = false;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strColName + "\0");
            bRet = StateInterface.dsv_get_float(m_pDStructureValue, temp, ref _fValue);
            return bRet;
        }

        public bool get_float(int _hash, ref float _fValue)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            bool bRet = false;
            bRet = StateInterface.dsv_get_hash_float(m_pDStructureValue, _hash, ref _fValue);
            return bRet;
        }
        public bool get_int(int _nHash, ref int _nValue)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            bool bRet = false;
            bRet = StateInterface.dsv_get_int2(m_pDStructureValue, _nHash, ref _nValue);
            return bRet;
        }

        public bool get_int(string _strColName, ref int _nValue)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            bool bRet = false;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strColName + "\0");
            bRet = StateInterface.dsv_get_int(m_pDStructureValue, temp, ref _nValue);
            return bRet;
        }

        public float[] get_float_arry(string _strColName, ref int _nCnt)
        {
            bool bRet = false;
            if (m_pDStructureValue == IntPtr.Zero)
                return null;

            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strColName + "\0");
            IntPtr pnValue = IntPtr.Zero;
            _nCnt = 0;
            bRet = StateInterface.dsv_get_float_array(m_pDStructureValue, temp, ref pnValue, ref _nCnt);
            float[] aRet = null;
            if (bRet)
            {
                aRet = new float[_nCnt];
                Marshal.Copy(pnValue, aRet, 0, _nCnt);
            }

            return aRet;
        }

        public int[] get_int_arry(int _hash, int _seq)
        {
            bool bRet = false;
            if (m_pDStructureValue == IntPtr.Zero)
                return null;

            IntPtr pnValue = IntPtr.Zero;
            int cnt = 0;
            bRet = StateInterface.dsv_get_ptr2(m_pDStructureValue, _hash, ref pnValue, ref cnt, _seq);
            int[] aRet = null;
            if (bRet)
            {
                aRet = new int[cnt];
                Marshal.Copy(pnValue, aRet, 0, cnt);
            }

            return aRet;
        }

        public int[] get_int_arry(string _strColName)
        {
            bool bRet = false;
            if (m_pDStructureValue == IntPtr.Zero)
                return null;

            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strColName + "\0");
            IntPtr pnValue = IntPtr.Zero;
            int cnt = 0;
            bRet = StateInterface.dsv_get_int_array(m_pDStructureValue, temp, ref pnValue, ref cnt);
            int[] aRet = null;
            if (bRet)
            {
                aRet = new int[cnt];
                Marshal.Copy(pnValue, aRet, 0, cnt);
            }

            return aRet;
        }

        public bool add_aaVec(string _strColName, List<List<Vector3>> aavec3)
        {
            for (int i = 0; i < aavec3.Count; i++)
            {
                float[] afloat = new float[aavec3[i].Count * 3];
                for (int j = 0; j < aavec3[i].Count; j++)
                {
                    afloat[j * 3 + 0] = aavec3[i][j].x;
                    afloat[j * 3 + 1] = aavec3[i][j].y;
                    afloat[j * 3 + 2] = aavec3[i][j].z;
                }

                if (!add_float_arry(_strColName, afloat))
                    return false;
            }

            return true;
        }

        public bool add_float_arry(string _strColName, float[] _afValues)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;
            int cnt = _afValues.Length;
            bool bRet = false;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strColName + "\0");
            bRet = StateInterface.dsv_add_float_array(m_pDStructureValue, temp, _afValues, cnt);
            return bRet;
        }

        public bool set_float_arry(int _hash, float[] _value_af)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            bool bRet = false;
            int size_n = Marshal.SizeOf(typeof(float)) * _value_af.Length;
            IntPtr ptr = Marshal.AllocHGlobal(size_n);
            Marshal.Copy(_value_af, 0, ptr, _value_af.Length);
            bRet = StateInterface.dsv_set_void(m_pDStructureValue, _hash, ptr, _value_af.Length);
            return bRet;
        }
        public bool set_float(int _hash, float _value_f)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            bool bRet = false;
            bRet = StateInterface.dsv_set_hash_float(m_pDStructureValue, _hash, _value_f);
            return bRet;
        }

        public bool set_float_arry(string _strColName, float[] _afValues)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            bool bRet = false;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strColName + "\0");
            bRet = StateInterface.dsv_set_float_array(m_pDStructureValue, temp, _afValues, _afValues.Length);
            return bRet;
        }

        public bool add_int_arry(string _strColName, int[] _anValues)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            bool bRet = false;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strColName + "\0");
            bRet = StateInterface.dsv_add_int_array(m_pDStructureValue, temp, _anValues, _anValues.Length);
            return bRet;
        }

        public bool set_float(string _strColName, float _f)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            bool bRet = false;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strColName + "\0");
            bRet = StateInterface.dsv_set_float(m_pDStructureValue, temp, _f);
            return bRet;
        }

        public bool set_int(string _strColName, int _nValue)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            bool bRet = false;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strColName + "\0");
            bRet = StateInterface.dsv_set_int(m_pDStructureValue, temp, _nValue);
            return bRet;
        }

        public bool set_int(int _col_hash, int _nValue)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            bool bRet = false;
            bRet = StateInterface.dsv_set_hash_int(m_pDStructureValue, _col_hash, _nValue);
            return bRet;
        }

        public bool add_int(string _strColName, int _nValue)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            bool bRet = false;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strColName + "\0");
            bRet = StateInterface.dsv_add_int(m_pDStructureValue, temp, _nValue);
            return bRet;
        }

        public bool set_variable_string(int _nHash, string _strValue)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            bool bRet = false;
            byte[] temp2 = System.Text.Encoding.UTF8.GetBytes(_strValue + "\0");
            bRet = StateInterface.dsv_set_variable_string2(m_pManager, m_pDStructureValue, _nHash, temp2);
            return bRet;
        }

        public bool set_variable_string(string _strColName, string _strValue)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            bool bRet = false;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strColName + "\0");
            byte[] temp2 = System.Text.Encoding.UTF8.GetBytes(_strValue + "\0");
            bRet = StateInterface.dsv_set_variable_string(m_pManager, m_pDStructureValue, temp, temp2);
            return bRet;
        }

        public bool set_variable(int _nHash, int _nValue)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            bool bRet = false;
            bRet = StateInterface.dsv_set_variable2(m_pManager, m_pDStructureValue, _nHash, _nValue);
            return bRet;
        }

        public bool set_variable(string _strColName, int _nValue)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            bool bRet = false;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strColName + "\0");
            bRet = StateInterface.dsv_set_variable(m_pManager, m_pDStructureValue, temp, _nValue);
            return bRet;
        }

        public bool add_variable(string _col, string _str)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            bool bRet = false;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_col + "\0");
            byte[] temp2 = System.Text.Encoding.UTF8.GetBytes(_str + "\0");
            bRet = StateInterface.dsv_add_variable_string(m_pManager, m_pDStructureValue, temp, temp2);
            return bRet;
        }

        public bool add_string(string _col, string _str)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            bool bRet = false;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_col + "\0");
            byte[] temp2 = System.Text.Encoding.UTF8.GetBytes(_str + "\0");
            bRet = StateInterface.dsv_add_string(m_pDStructureValue, temp, temp2);
            return bRet;
        }

        public bool add_variable(string _strColName, int _nValue)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            bool bRet = false;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strColName + "\0");
            bRet = StateInterface.dsv_add_variable(m_pManager, m_pDStructureValue, temp, _nValue);
            return bRet;
        }

        public bool set_string(string _strColName, string _strValue)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            bool bRet = false;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strColName + "\0");
            byte[] temp2 = System.Text.Encoding.UTF8.GetBytes(_strValue + "\0");
            bRet = StateInterface.dsv_set_string(m_pDStructureValue, temp, temp2);
            return bRet;
        }

        public bool set_string(int _nHash, string _strValue)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            bool bRet = false;
            byte[] temp2 = System.Text.Encoding.UTF8.GetBytes(_strValue + "\0");
            bRet = StateInterface.dsv_set_string2(m_pDStructureValue, _nHash, temp2);
            return bRet;
        }

        public bool set_variable(string _column_str, float[] _variable_af)
        {
            bool bRet = false;
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            //IntPtr float_paf = StateInterface.base_malloc(_variable_af.Length * 4);
            //Marshal.Copy(_variable_af, 0, float_paf, _variable_af.Length);
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_column_str + "\0");
            bRet = StateInterface.dsv_set_float_array(m_pDStructureValue, temp, _variable_af, _variable_af.Length);
            //StateInterface.base_free(float_paf);
            return bRet;
        }

        public bool set_variable(string _strColName, GCHandle _handle)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            bool bRet = false;

            IntPtr ptr = (IntPtr)_handle;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strColName + "\0");
            bRet = StateInterface.dsv_set_variable_ptr(m_pManager, m_pDStructureValue, temp, ptr);

            //handle.Free ();
            return bRet;
        }

        public bool set_ptr(int _key, IntPtr _ptr, int _cnt)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            bool bRet = false;
            bRet = StateInterface.dsv_set_ptr(m_pDStructureValue, _key, _ptr, _cnt);

            return bRet;
        }

        public bool get_ptr(int _hash, ref GCHandle _handle, ref int _cnt)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            IntPtr ptr = IntPtr.Zero;

            bool bRet = false;
            bRet = StateInterface.dsv_get_ptr2(m_pDStructureValue, _hash, ref ptr, ref _cnt, 0);
            if (bRet == false || ptr == IntPtr.Zero)
                return false;

            _handle = (GCHandle)ptr;
            return true;
        }
        public bool get_ptr(string _strColName, ref GCHandle _handle)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            IntPtr ptr = IntPtr.Zero;

            bool bRet = false;
            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strColName + "\0");
            bRet = StateInterface.dsv_get_ptr(m_pDStructureValue, temp, ref ptr);
            if (bRet == false || ptr == IntPtr.Zero)
                return false;

            _handle = (GCHandle)ptr;
            return true;
        }

        public bool set_variable(string _strColName, MonoBehaviour _Obj)
        {
            GCHandle handle = GCHandle.Alloc(_Obj);
            return set_variable(_strColName, handle);
        }

        public bool get_ptr(string _strColName, ref MonoBehaviour _Obj)
        {
            GCHandle handle = new GCHandle();
            bool bRet = get_ptr(_strColName, ref handle);
            if (bRet)
                _Obj = (handle.Target as MonoBehaviour);
            return bRet;
        }

        public bool set_variable(string _strColName, GameObject _Obj)
        {
            GCHandle handle = GCHandle.Alloc(_Obj);
            return set_variable(_strColName, handle);
        }

        public bool get_ptr(string _strColName, ref GameObject _Obj)
        {
            GCHandle handle = new GCHandle();
            bool bRet = get_ptr(_strColName, ref handle);
            if (!bRet)
                return false;
            if (handle == null)
                return false;

            _Obj = (handle.Target as GameObject);
            return bRet;
        }

        public bool get_string(int _nHash, ref string _strValue)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            IntPtr pRet = StateInterface.dsv_get_string2(m_pDStructureValue, _nHash);
            if (pRet == IntPtr.Zero)
                return false;
            _strValue = VLStateManager.MarshalToString(pRet);
            return true;
        }

        public bool get_strings(int _hash, ref List<string> _str_a)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            IntPtr pRet = StateInterface.dsv_get_string2(m_pDStructureValue, _hash);
            if (pRet == IntPtr.Zero)
                return false;

            string source;
            source = VLStateManager.MarshalToString(pRet);
            List<List<string>> list_aa = BaseFile.paser_list(source, ",");
            if (list_aa.Count == 0)
                return false;
            _str_a = list_aa[0];
            return true;
        }

        public bool get_strings(string _column_name, ref List<string> _string_a)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_column_name + "\0");
            IntPtr pRet = StateInterface.dsv_get_string(m_pDStructureValue, temp);
            if (pRet == IntPtr.Zero)
                return false;

            string source;
            source = VLStateManager.MarshalToString(pRet);
            List<List<string>> list_aa = BaseFile.paser_list(source, ",");
            if (list_aa.Count == 0)
                return false;
            _string_a = list_aa[0];
            return true;
        }

        public bool get_string(string _strColName, ref string _strValue)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strColName + "\0");
            IntPtr pRet = StateInterface.dsv_get_string(m_pDStructureValue, temp);
            if (pRet == IntPtr.Zero)
                return false;
            _strValue = VLStateManager.MarshalToString(pRet);
            return true;
        }

        public bool param_get(IntPtr _event, IntPtr _context, int _seq, ref int _value)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            _value = StateInterface.static_variable_param_int_get(m_pDStructureValue, _event, _context, _seq,
                IntPtr.Zero);

            if (_value == -999999)
                return false;
            return true;
        }
        
        public bool param_get(IntPtr _event, IntPtr _context, int _seq, ref Vector3 _value)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            return VLStateManager.param_variable_get_vector(m_pDStructureValue, _event, _context, _seq,
                IntPtr.Zero, ref _value);
        }

        public bool param_get(IntPtr _event, IntPtr _context, int _seq, ref string _value)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            IntPtr str =
                StateInterface.static_variable_param_string_get(m_pDStructureValue, _event, _context, _seq,
                    IntPtr.Zero);

            if (str == IntPtr.Zero)
                return false;

            _value = VLStateManager.MarshalToString(str);
            return true;
        }
        
        public bool param_set(IntPtr _event, IntPtr _context, int _seq, string _value)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            return VLStateManager.param_variable_set_string(m_pDStructureValue, _event, _context, _seq,
                IntPtr.Zero, _value);
        }
        
        public bool param_set(IntPtr _event, IntPtr _context, int _seq, int _value)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            return VLStateManager.param_variable_set_int(m_pDStructureValue, _event, _context, _seq,
                IntPtr.Zero, _value);
        }
        
        public bool param_set(IntPtr _event, IntPtr _context, int _seq, Vector3 _value)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            return VLStateManager.param_variable_set_vector(m_pDStructureValue, _event, _context, _seq,
                IntPtr.Zero, _value);
        }

        public bool param_get(string _strHash, int _pnSeq, ref int _pnValue)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return false;

            byte[] temp = System.Text.Encoding.UTF8.GetBytes(_strHash + "\0");
            return StateInterface.dsv_param_get(m_pDStructureValue, temp, ref _pnSeq, ref _pnValue);
        }

        public int serial_get()
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return 0;

            return StateInterface.dsv_serial_get(m_pDStructureValue);
        }

        public StateDStructureValue event_cast_get(int _event, bool _new)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return null;

            return new StateDStructureValue(StateInterface.dsv_event_cast_get(m_pDStructureValue, _event, _new));
        }

        public StateDStructureValue event_group_make(int _evt, int _groupId, int _id)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return null;

            return new StateDStructureValue(
                StateInterface.dsv_event_group_make(m_pDStructureValue, _evt, _groupId, _id));
        }

        public void group_id_set(int _groupId, int _id)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return;

            StateInterface.dsv_group_id_set(m_pDStructureValue, _groupId, _id);
        }

        public void event_cast_reset()
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return;

            StateInterface.dsv_event_cast_reset(m_pDStructureValue);
        }

        public int state_function_hash_get()
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return 0;

            return StateInterface.dsv_state_function_hash(m_pDStructureValue);
        }

        public StateDStructureValue state_variable_get()
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return null;

            return new StateDStructureValue(StateInterface.dsv_state_variable(m_pDStructureValue));
        }
        
        public int state_localtime_get()
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return -1;

            return StateInterface.dsv_state_timelocal_get(m_pDStructureValue);
        }

        public IntPtr column_get_(int _column_n, ref int _hash_n, ref int _count_pn)
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return IntPtr.Zero;

            return StateInterface.dsv_column_get(m_pDStructureValue, _column_n, ref _hash_n, ref _count_pn);
        }

        static public int get_hash(int _index_n)
        {
            return StateInterface.dst_get_hash(_index_n);
        }

        public bool column_get(int _column_n, ref int _value_n, ref int _hash_n)
        {
            IntPtr ptr;
            int count_n = 0;
            ptr = column_get_(_column_n, ref _hash_n, ref count_n);
            if (ptr == IntPtr.Zero)
                return false;

            int index_n = StateDStructure.get_index(_hash_n);
            int type_n = StateDStructure.get_type(index_n);

            if (type_n != (int)StateDStructure.type.TYPE_INT32)
                return false;

            int[] value_an = new int[count_n];
            Marshal.Copy(ptr, value_an, 0, count_n);

            _value_n = value_an[0];
            return true;
        }

        public bool column_get(int _column_n, ref float _value_f)
        {
            IntPtr ptr;
            int hash_n = 0, count_n = 0;
            ptr = column_get_(_column_n, ref hash_n, ref count_n);
            if (ptr == IntPtr.Zero)
                return false;

            float[] value_af = new float[count_n];
            Marshal.Copy(ptr, value_af, 0, count_n);

            _value_f = value_af[0];
            return true;
        }

        public bool column_get(int _column_n, ref int[] _value_an, ref int _hash_n)
        {
            IntPtr ptr;
            int count_n = 0;
            ptr = column_get_(_column_n, ref _hash_n, ref count_n);
            if (ptr == IntPtr.Zero)
                return false;

            int index_n = StateDStructure.get_index(_hash_n);
            int type_n = StateDStructure.get_type(index_n);

            if (type_n != (int)StateDStructure.type.TYPE_ARRAY_INT32)
                return false;

            _value_an = new int[count_n];
            Marshal.Copy(ptr, _value_an, 0, count_n);
            return true;
        }

        public bool column_get(int _column_n, ref float[] _value_af)
        {
            IntPtr ptr;
            int hash_n = 0, count_n = 0;
            ptr = column_get_(_column_n, ref hash_n, ref count_n);
            if (ptr == IntPtr.Zero)
                return false;

            _value_af = new float[count_n];
            Marshal.Copy(ptr, _value_af, 0, count_n);
            return true;
        }

        public bool column_get(int _column_n, ref string _value_str)
        {
            IntPtr ptr;
            int hash_n = 0, count_n = 0;
            ptr = column_get_(_column_n, ref hash_n, ref count_n);
            if (ptr == IntPtr.Zero)
                return false;

            _value_str = VLStateManager.MarshalToString(ptr);
            return true;
        }

        public StateDStructureValue copy_get()
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return null;

            StateDStructureValue dsv = new StateDStructureValue(StateInterface.dsv_copy_get(m_pDStructureValue));
            return dsv;
        }

        public void delete()
        {
            if (m_pDStructureValue == IntPtr.Zero)
                return;

            StateInterface.dsv_delete(m_pDStructureValue);

            m_pDStructureValue = IntPtr.Zero;
        }

        public Vector3 mouse_pos_get(StateDStructureValue _evt)
        {
            int[] mouse_pos_an = _evt.get_int_arry("BasePos_anV");
            if (mouse_pos_an == null)
            {
                StateDStructureValue var_state = state_variable_get();
                mouse_pos_an = var_state.get_int_arry("BasePos_anV");
            }

            if (mouse_pos_an == null)
                return Vector3.zero;

            if (mouse_pos_an.Length == 4)
                mouse_pos_an[1] = (mouse_pos_an[1] + mouse_pos_an[3]) / 2;

            Vector3 pos = new Vector3(mouse_pos_an[0], mouse_pos_an[1], 0);
            return pos;
        }

    }
}