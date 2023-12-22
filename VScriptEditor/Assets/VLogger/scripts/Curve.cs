using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace StateSystem
{
    public class Curve : MonoBehaviour
    {
        // Start is called before the first frame update
        public List<Material> m_line_material_a;
        static public float m_width_f = 0;
        public int m_step_n = 50;
        public float m_wing_f = 300;
        public float m_wing_start = 0;
        public float m_wing_end = 0;
        public float m_wing_skip_f = 50;
        public float m_select_range = 5;
        List<Vector3> m_lines_av3;
        int m_type;
        bool m_update_b = false;
        CanvasRenderer m_canRender;

        int m_key_n = 0;
        //Canvas m_mainCanvas;

        public int key_get()
        {
            return m_key_n;
        }

        public void line_set(int _key_n, Vector3 _from_v3, Vector3 _to_v3, int _type)
        {
            m_key_n = _key_n;
            m_type = _type;
            if (m_type < 0 || m_type >= m_line_material_a.Count)
                m_type = 0;

            if (m_canRender == null)
                m_canRender = GetComponent<CanvasRenderer>();
            if (m_lines_av3 == null)
                m_lines_av3 = new List<Vector3>();

            float wing_f = m_wing_skip_f;

            //if(m_mainCanvas != null)
            //    wing_f = m_wing_skip_f * m_mainCanvas.scaleFactor;

            if (m_wing_start > 0)
                _from_v3.x += m_wing_start;
            else
                _from_v3.x += wing_f;

            if (m_wing_end > 0)
                _to_v3.x -= m_wing_end;
            else
                _to_v3.x -= wing_f;

            float dist_f = (_from_v3 - _to_v3).magnitude;
            wing_f = m_wing_f;
            if (dist_f < wing_f)
                wing_f = dist_f;

            m_lines_av3.Clear();

            m_lines_av3.Add(new Vector3(_from_v3.x, _from_v3.z, _from_v3.y));
            m_lines_av3.Add(new Vector3(_from_v3.x + wing_f, _from_v3.z, _from_v3.y));
            m_lines_av3.Add(new Vector3(_to_v3.x - wing_f, _to_v3.z, _to_v3.y));
            m_lines_av3.Add(new Vector3(_to_v3.x, _to_v3.z, _to_v3.y));

            m_update_b = true;
        }

        void Start()
        {
            //m_mainCanvas = GameObject.Find("Canvas").GetComponent<Canvas>();
        }

        public bool select(Vector3 _pos_v3)
        {
            _pos_v3.z = _pos_v3.y;
            _pos_v3.y = 0;
            Vector3[] points_av3 = new Vector3[4];

            for (int i = 0; i < m_lines_av3.Count; i++)
            {
                points_av3[i] = m_lines_av3[i];
            }

            F3DCurve4 curve4 = new F3DCurve4(points_av3);

            float len_f = curve4.length_rough();
            int cnt_n = (int)len_f / 2;

            if (cnt_n < 2)
                cnt_n = 2;

            float inc_f = 1.0f / (float)cnt_n;

            Vector3 pos_line_v3, diff_v3, left_v3 = new Vector3(0, 0, 0);
            for (float r = 0; r <= 1; r += inc_f)
            {
                pos_line_v3 = curve4.get_rate(r, ref left_v3);
                diff_v3 = pos_line_v3 - _pos_v3;

                if (diff_v3.sqrMagnitude <= m_select_range * m_select_range)
                    return true;
            }

            return false;
        }

        static private Camera m_cam = null;
        void UpdateLine()
        {
            if (m_cam == null)
            {
                m_cam = Camera.main;
                GameObject obj = GameObject.Find("State/StateViewCamera");
                m_cam = obj.GetComponent<Camera>();
            }
            
            float heightWorld = m_cam.orthographicSize;
            m_width_f = heightWorld / Screen.height;
            
            Vector3[] points_av3 = new Vector3[4];

            for (int i = 0; i < m_lines_av3.Count; i++)
            {
                points_av3[i] = m_lines_av3[i];
            }

            F3DCurve4 curve4 = new F3DCurve4(points_av3);

            Mesh mesh = new Mesh(); //GetComponent<MeshFilter>().mesh;
            mesh.Clear();

            int VecNum_n = m_step_n * 2 + 2;

            Vector3[] vec = new Vector3[VecNum_n];
            Vector3[] nor = new Vector3[VecNum_n];
            Vector2[] uv = new Vector2[VecNum_n];
            int[] tri = new int[m_step_n * 3 * 2];

            Vector3 left_v3, pos_v3, pos1_v3, pos2_v3;
            float rate_f;
            left_v3 = new Vector3();

            for (int i = 0;
                 i <=
                 m_step_n;
                 i++)
            {
                rate_f = (float)i / (float)m_step_n;
                pos_v3 = curve4.get_rate(rate_f, ref left_v3);
                pos1_v3 = pos_v3 + left_v3 * m_width_f;
                pos2_v3 = pos_v3 - left_v3 * m_width_f;

                //vec[i * 2 + 0] = Camera.main.ScreenToWorldPoint(new Vector3(pos1_v3.x, pos1_v3.z, Camera.main.nearClipPlane));
                //vec[i * 2 + 1] = Camera.main.ScreenToWorldPoint(new Vector3(pos2_v3.x, pos2_v3.z, Camera.main.nearClipPlane));
                vec[i * 2 + 0] = new Vector3(pos1_v3.x, pos1_v3.z, Camera.main.nearClipPlane);
                vec[i * 2 + 1] = new Vector3(pos2_v3.x, pos2_v3.z, Camera.main.nearClipPlane);

                nor[i * 2 + 0] = new Vector3(0, 0, -1);
                nor[i * 2 + 1] = new Vector3(0, 0, -1);

                uv[i * 2 + 0] = new Vector2(0, 0);
                uv[i * 2 + 1] = new Vector2(0, 0);

                if (i < m_step_n)
                {
                    tri[i * 6 + 0] = i * 2 + 0;
                    tri[i * 6 + 1] = i * 2 + 3;
                    tri[i * 6 + 2] = i * 2 + 1;
                    tri[i * 6 + 3] = i * 2 + 0;
                    tri[i * 6 + 4] = i * 2 + 2;
                    tri[i * 6 + 5] = i * 2 + 3;
                }
            }

            mesh.vertices = vec;
            mesh.normals = nor;
            mesh.uv = uv;
            mesh.triangles = tri;

            m_canRender.Clear();
            m_canRender.SetMaterial(m_line_material_a[m_type], null);
            m_canRender.SetMesh(mesh);
        }

        // Update is called once per frame
        void Update()
        {
            if (m_update_b)
            {
                m_update_b = false;
                UpdateLine();
            }
        }
    }
}