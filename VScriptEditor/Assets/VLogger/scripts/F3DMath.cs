using UnityEngine;
using System.Collections;
using System.Collections.Generic;

namespace StateSystem
{
	public class F3DMath
	{
		static public float ms_fE = 2.7182818284f; // natural constant
		static public float ms_fG = -9.8f * 2; // gravity

		static public float get_angle(Vector3 _v3Start, Vector3 _v3End, Vector3 _v3Up)
		{
			if (_v3Start == _v3End)
				return 0;

			Vector3 v3Normal = Vector3.Cross(_v3Start, _v3End);
			float fAngle = 0;

			fAngle = Mathf.Acos(Vector3.Dot(_v3Start, _v3End));
			fAngle = 180.0f * fAngle / Mathf.PI;
			if (Vector3.Dot(v3Normal, _v3Up) < 0)
			{
				fAngle = 360.0f - fAngle;
			}

			return fAngle;
		}

		static public float get_angle2(Vector3 _v3Start, Vector3 _v3End, Vector3 _v3Up)
		{
			if (_v3Start == _v3End)
				return 0;

			Vector3 v3Normal = Vector3.Cross(_v3Start, _v3End);
			float fAngle = 0;

			fAngle = Mathf.Acos(Vector3.Dot(_v3Start, _v3End));
			fAngle = 180.0f * fAngle / Mathf.PI;
			return fAngle;
		}

		static public float MOD(float _up, float _dn)
		{
			float fSol = 0, fRes = 0;

			fSol = _up / _dn;
			int nSol = (int)fSol;
			fSol = nSol * _dn;
			fRes = _up - fSol;

			return fRes;
		}

		static public float line_point_dist(Vector3 _v3Pos, Vector3 _v3Line, Vector3 _vPoint)
		{
			Vector3 v3Cross, v3Dir;
			v3Dir = _vPoint - _v3Pos;
			v3Cross = Vector3.Cross(v3Dir, _v3Line);
			return v3Cross.magnitude;
		}

		static public bool line_colision(Vector3[] _line, Vector3 _pos, float _range)
		{
			Vector3 dir, nor, vec;
			_line[0].z = 0;
			_line[1].z = 0;

			vec = _pos - _line[0];
			dir = _line[1] - _line[0];

			if (dir.sqrMagnitude == 0)
			{
				if (vec.magnitude < _range)
					return true;
				return false;
			}

			nor = dir.normalized;

			if (Vector3.Cross(nor, vec).magnitude <= _range)
			{
				float dot = Vector3.Dot(nor, vec);
				if (dot > -_range && dot < dir.magnitude + _range)
					return true;
			}

			return false;
		}

		static public Vector3 project(Vector3 _v3Dir, Vector3 _v3Target)
		{
			float fDot;

			fDot = Vector3.Dot(_v3Dir, _v3Target);
			Vector3 v3Projected = _v3Dir * fDot;
			v3Projected = _v3Target - v3Projected;
			return v3Projected;
		}

		static public Vector3 project(Vector4 _v4Dir, Vector4 _v4Target)
		{
			Vector3 v3Dir, v3Target;

			v3Dir = new Vector3();
			v3Target = new Vector3();
			v3Dir.Set(_v4Dir.x, _v4Dir.y, _v4Dir.z);
			v3Target.Set(_v4Target.x, _v4Target.y, _v4Target.z);
			return project(v3Dir, v3Target);
		}

		static public float BezierCurve(float[] ctr, float t)
		{
			int NumCtlPoints = 4;
			float[] c = new float[4];
			int k;
			int n, choose = 0;
			float result;

			n = NumCtlPoints - 1;

			for (k = 0; k <= n; k++)
			{
				if (k == 0) choose = 1;
				else if (k == 1) choose = n;
				else choose = choose * (n - k + 1) / k;

				c[k] = ctr[k] * (float)choose;
			}

			float t1, tt, u;
			float[] b = new float[4];

			n = NumCtlPoints - 1;
			u = t;

			b[0] = c[0];

			for (k = 1; k <= n; k++)
			{
				b[k] = c[k] * u;
				u = u * t;
			}

			result = b[n];
			t1 = (float)1 - t;
			tt = t1;

			for (k = (int)n - 1; k >= 0; k--)
			{
				result += b[k] * tt;
				tt = tt * t1;
			}

			return result;
		}

		static public void GetExtPoint(Vector3[] _v3Poss, float[] _fTimes, ref Vector3[] _v3PosExts, float _fQuickness)
		{
			// quickness is a time for adaption
			Vector3[] v3Dir = new Vector3[2];

			v3Dir[0] = _v3Poss[1] - _v3Poss[0];
			v3Dir[1] = _v3Poss[1] - _v3Poss[2];
			float[] afV = new float[2];
			afV[0] = v3Dir[0].magnitude / (_fTimes[1] - _fTimes[0]);
			afV[1] = v3Dir[1].magnitude / (_fTimes[2] - _fTimes[1]);
			for (int i = 0; i < 2; i++)
			{
				v3Dir[i].Normalize();
				_v3PosExts[i] = v3Dir[i] * afV[i] * _fQuickness + _v3Poss[i + 1];
			}
		}
		/*
		motionlist
		1 stand	walk	run				fastrun
		2 stand	left	runleft			fastrun
		3 stand	back	runl or runr	fastrun
		4 stand right	runright		fastrun
		//*/
	}

	public class F3DMotion
	{
		public string m_strName;
		public float m_fSpeed;
		public float m_fLength;
		public float m_fDir;

		public F3DMotion()
		{
			m_strName = "";
		}
	};

	public class F3DLocomotion
	{
		List<F3DMotion>[] m_astlVMotionSet;
		float m_fWDirection, m_fWSpeed, m_fWSpeed2;

		public F3DLocomotion()
		{
			m_astlVMotionSet = new List<F3DMotion> [6];

			for (int i = 0; i < 6; i++)
			{
				m_astlVMotionSet[i] = new List<F3DMotion>();
			}

			F3DMotion m;
			m = new F3DMotion();
			m.m_strName = "Idle";
			m.m_fSpeed = 0;
			m.m_fLength = 2.0f;
			m.m_fDir = 0.0f;
			m_astlVMotionSet[0].Add(m);
			m = new F3DMotion();
			m.m_strName = "SlowForward";
			m.m_fSpeed = 0.6f;
			m.m_fLength = 1.0f;
			m.m_fDir = 0.0f;
			m_astlVMotionSet[0].Add(m);
			m = new F3DMotion();
			m.m_strName = "FastForward";
			m.m_fSpeed = 9.0f;
			m.m_fLength = 0.667f;
			m.m_fDir = 0.0f;
			m_astlVMotionSet[0].Add(m); // end of forward

			m = new F3DMotion();
			m.m_strName = "Idle";
			m.m_fSpeed = 0;
			m.m_fLength = 2.0f;
			m.m_fDir = 0.0f;
			m_astlVMotionSet[1].Add(m);
			m = new F3DMotion();
			m.m_strName = "SlowLeft";
			m.m_fSpeed = 0.6f;
			m.m_fLength = 1.0f;
			m.m_fDir = 0.0f;
			m_astlVMotionSet[1].Add(m);
			m = new F3DMotion();
			m.m_strName = "Left";
			m.m_fSpeed = 5;
			m.m_fLength = 0.457f;
			m.m_fDir = 0.0f;
			m_astlVMotionSet[1].Add(m);
			m = new F3DMotion();
			m.m_strName = "FastForward";
			m.m_fSpeed = 9.0f;
			m.m_fLength = 0.667f;
			m.m_fDir = -90.0f;
			m_astlVMotionSet[1].Add(m); // end of left

			m = new F3DMotion();
			m.m_strName = "Idle";
			m.m_fSpeed = 0;
			m.m_fLength = 2.0f;
			m.m_fDir = 0.0f;
			m_astlVMotionSet[2].Add(m);
			m = new F3DMotion();
			m.m_strName = "SlowBack";
			m.m_fSpeed = 0.6f;
			m.m_fLength = 1.0f;
			m.m_fDir = 0.0f;
			m_astlVMotionSet[2].Add(m);
			m = new F3DMotion();
			m.m_strName = "Backward";
			m.m_fSpeed = 4;
			m.m_fLength = 0.667f;
			m.m_fDir = 0.0f;
			m_astlVMotionSet[2].Add(m);
			m = new F3DMotion();
			m.m_strName = "Left";
			m.m_fSpeed = 5;
			m.m_fLength = 0.457f;
			m.m_fDir = -90.0f;
			m_astlVMotionSet[2].Add(m);
			m = new F3DMotion();
			m.m_strName = "FastForward";
			m.m_fSpeed = 9.0f;
			m.m_fLength = 0.667f;
			m.m_fDir = -180.0f;
			m_astlVMotionSet[2].Add(m); // end of back left

			m = new F3DMotion();
			m.m_strName = "Idle";
			m.m_fSpeed = 0;
			m.m_fLength = 2.0f;
			m.m_fDir = 0.0f;
			m_astlVMotionSet[3].Add(m);
			m = new F3DMotion();
			m.m_strName = "SlowBack";
			m.m_fSpeed = 0.6f;
			m.m_fLength = 1.0f;
			m.m_fDir = 0.0f;
			m_astlVMotionSet[3].Add(m);
			m = new F3DMotion();
			m.m_strName = "Backward";
			m.m_fSpeed = 4;
			m.m_fLength = 0.667f;
			m.m_fDir = 0.0f;
			m_astlVMotionSet[3].Add(m);
			m = new F3DMotion();
			m.m_strName = "Right";
			m.m_fSpeed = 5;
			m.m_fLength = 0.457f;
			m.m_fDir = 90.0f;
			m_astlVMotionSet[3].Add(m);
			m = new F3DMotion();
			m.m_strName = "FastForward";
			m.m_fSpeed = 9.0f;
			m.m_fLength = 0.667f;
			m.m_fDir = 180.0f;
			m_astlVMotionSet[3].Add(m); // end of back right

			m = new F3DMotion();
			m.m_strName = "Idle";
			m.m_fSpeed = 0;
			m.m_fLength = 2.0f;
			m.m_fDir = 0.0f;
			m_astlVMotionSet[4].Add(m);
			m = new F3DMotion();
			m.m_strName = "SlowRight";
			m.m_fSpeed = 0.6f;
			m.m_fLength = 1.0f;
			m.m_fDir = 0.0f;
			m_astlVMotionSet[4].Add(m);
			m = new F3DMotion();
			m.m_strName = "Right";
			m.m_fSpeed = 5;
			m.m_fLength = 0.457f;
			m.m_fDir = 0.0f;
			m_astlVMotionSet[4].Add(m);
			m = new F3DMotion();
			m.m_strName = "FastForward";
			m.m_fSpeed = 9.0f;
			m.m_fLength = 0.667f;
			m.m_fDir = 90.0f;
			m_astlVMotionSet[4].Add(m); // end of right

			m = new F3DMotion();
			m.m_strName = "Idle";
			m.m_fSpeed = 0;
			m.m_fLength = 2.0f;
			m.m_fDir = 0.0f;
			m_astlVMotionSet[5].Add(m);
			m = new F3DMotion();
			m.m_strName = "SlowForward";
			m.m_fSpeed = 0.6f;
			m.m_fLength = 1.0f;
			m.m_fDir = 0.0f;
			m_astlVMotionSet[5].Add(m);
			m = new F3DMotion();
			m.m_strName = "FastForward";
			m.m_fSpeed = 9.0f;
			m.m_fLength = 0.667f;
			m.m_fDir = 0.0f;
			m_astlVMotionSet[5].Add(m); // end of forward
		}

		public void get_param(float _fDirection, float _fSpeed, ref float _fLength, ref float _fRotate)
		{
			int nDir = 0;

			if (_fDirection > 360)
				_fDirection -= 360;

			nDir = (int)(_fDirection / 90.0f);
			if (_fDirection > 180)
				nDir += 1;

			if (nDir < 0)
				nDir = 0;
			if (nDir >= m_astlVMotionSet.Length)
				nDir = m_astlVMotionSet.Length - 1;

			int nSpeed = m_astlVMotionSet[nDir].Count - 1;
			for (int i = 1; i < m_astlVMotionSet[nDir].Count; i++)
			{
				if (_fSpeed < m_astlVMotionSet[nDir][i].m_fSpeed)
				{
					nSpeed = i - 1;
					break;
				}
			}

			int nDir2 = nDir + 1;

			if (nDir2 < 0)
				nDir2 = 0;
			if (nDir2 >= m_astlVMotionSet.Length)
				nDir2 = m_astlVMotionSet.Length - 1;

			int nSpeed2 = m_astlVMotionSet[nDir2].Count - 1;
			for (int i = 1; i < m_astlVMotionSet[nDir2].Count; i++)
			{
				if (_fSpeed < m_astlVMotionSet[nDir2][i].m_fSpeed)
				{
					nSpeed2 = i - 1;
					break;
				}
			}

			// calculate for weight of direction
			if (_fDirection <= 180)
				m_fWDirection = (_fDirection - (float)(nDir * 90)) / 90.0f;
			else
				m_fWDirection = (_fDirection - (float)((nDir - 1) * 90)) / 90.0f;

			// calculate for weight for each direction of motion
			if (nSpeed == m_astlVMotionSet[nDir].Count - 1)
				m_fWSpeed = 0;
			else
				m_fWSpeed = (_fSpeed - m_astlVMotionSet[nDir][nSpeed].m_fSpeed) /
				            (m_astlVMotionSet[nDir][nSpeed + 1].m_fSpeed - m_astlVMotionSet[nDir][nSpeed].m_fSpeed);

			if (nSpeed2 == m_astlVMotionSet[nDir2].Count - 1)
				m_fWSpeed2 = 0;
			else
				m_fWSpeed2 = (_fSpeed - m_astlVMotionSet[nDir2][nSpeed2].m_fSpeed) /
				             (m_astlVMotionSet[nDir2][nSpeed2 + 1].m_fSpeed -
				              m_astlVMotionSet[nDir2][nSpeed2].m_fSpeed);

			float fLength, fLength2; // calculate for motion length

			fLength = m_astlVMotionSet[nDir][nSpeed].m_fLength;
			if (m_fWSpeed > 0)
				fLength += (m_astlVMotionSet[nDir][nSpeed + 1].m_fLength - m_astlVMotionSet[nDir][nSpeed].m_fLength) *
				           m_fWSpeed;

			fLength2 = m_astlVMotionSet[nDir2][nSpeed2].m_fLength;
			if (m_fWSpeed2 > 0)
				fLength2 +=
					(m_astlVMotionSet[nDir2][nSpeed2 + 1].m_fLength - m_astlVMotionSet[nDir2][nSpeed2].m_fLength) *
					m_fWSpeed2;

			_fLength = fLength;
			if (m_fWDirection > 0)
				_fLength += (fLength2 - fLength) * m_fWDirection;

			float fDir, fDir2; // calculate for motion rotation

			fDir = m_astlVMotionSet[nDir][nSpeed].m_fDir;
			if (m_fWSpeed > 0)
				fDir += (m_astlVMotionSet[nDir][nSpeed + 1].m_fDir - m_astlVMotionSet[nDir][nSpeed].m_fDir) * m_fWSpeed;

			fDir2 = m_astlVMotionSet[nDir2][nSpeed2].m_fDir;
			if (m_fWSpeed2 > 0)
				fDir2 += (m_astlVMotionSet[nDir2][nSpeed2 + 1].m_fDir - m_astlVMotionSet[nDir2][nSpeed2].m_fDir) *
				         m_fWSpeed2;

			_fRotate = fDir;
			if (m_fWDirection > 0)
				_fRotate += (fDir2 - fDir) * m_fWDirection;

		}
	};

	public class F3DCurveRun
	{
		int m_nCount = 0;
		Vector3[] m_av3Pos;
		float[] m_afTime;
		float m_fQuickness = 1.0f;
		Vector3 m_v3Direction, m_v3Normal;

		public F3DCurveRun(int _nCount, Vector3[] _av3Pos, float[] _afTime, float _fQuickness)
		{
			m_nCount = _nCount;
			m_fQuickness = _fQuickness;
			m_v3Normal = new Vector3(0, 1, 0);
			if (_nCount == 2)
			{
				m_av3Pos = new Vector3[2];
				m_afTime = new float[2];
				for (int i = 0; i < _nCount; i++)
				{
					m_av3Pos[i] = _av3Pos[i];
					m_afTime[i] = _afTime[i];
				}

				m_v3Direction = m_av3Pos[1] - m_av3Pos[0];
			}
			else
			{
				m_av3Pos = new Vector3[4];
				m_afTime = new float[3];
				for (int i = 0; i < 3; i++)
				{
					m_afTime[i] = _afTime[i];
				}

				Vector3[] av3Ext = new Vector3[2];
				F3DMath.GetExtPoint(_av3Pos, _afTime, ref av3Ext, m_fQuickness);
				m_av3Pos[0] = _av3Pos[1];
				m_av3Pos[1] = av3Ext[0];
				m_av3Pos[2] = av3Ext[1];
				m_av3Pos[3] = _av3Pos[2];
				m_v3Direction = _av3Pos[2] - _av3Pos[1];
			}
		}

		public Vector3 get_pos_rate(float _fRate)
		{
			Vector3 v3Ret;

			if (_fRate > 1)
				_fRate = 1;

			if (m_nCount == 2)
			{
				v3Ret = m_av3Pos[0];
				Vector3 v3Dir = m_av3Pos[1] - v3Ret;

				v3Ret += v3Dir * _fRate;

				v3Ret.y = 0.2f;
				return v3Ret;
			}

			float[] fCtr = new float[4];

			for (int i = 0; i < 4; i++)
				fCtr[i] = m_av3Pos[i].x;
			v3Ret.x = F3DMath.BezierCurve(fCtr, _fRate);

			for (int i = 0; i < 4; i++)
				fCtr[i] = m_av3Pos[i].z;
			v3Ret.z = F3DMath.BezierCurve(fCtr, _fRate);

			v3Ret.y = 0.2f;
			return v3Ret;
		}

		static public float get_wing_with_param_(float _fLength)
		{
			if (_fLength < 15)
				_fLength = 15;
			if (_fLength > 25)
				_fLength = 25;
			return _fLength;
		}

		public Vector3 get_pos_wing_rate(float _fRate, float _fWingRate)
		{
			float fSpeed = 0;
			float fWing = get_wing_with_param_(m_v3Direction.magnitude) *
			              _fWingRate; //m_v3Direction.magnitude * _fWingRate;
			Vector3 v3Pos = get_pos_rate(_fRate) + get_left(_fRate, ref fSpeed) * fWing;
			return v3Pos;
		}

		public Vector3 get_left(float _fRate, ref float _fSpeed)
		{
			Vector3 v1, v2, vd;
			if (_fRate < 0.5f)
			{
				v1 = get_pos_rate(_fRate);
				v2 = get_pos_rate(_fRate + 0.01f);
			}
			else
			{
				if (_fRate > 1.0f)
					_fRate = 1.0f;

				v1 = get_pos_rate(_fRate - 0.01f);
				v2 = get_pos_rate(_fRate);
			}

			vd = v2 - v1;
			Vector3 v3Left;
			v3Left = Vector3.Cross(vd.normalized, m_v3Normal);

			float fDelta = 0.01f * (m_afTime[1] - m_afTime[0]);
			_fSpeed = (vd.magnitude / fDelta);
			return v3Left;
		}

		public Vector3 get_normal(float _fRate)
		{
			return m_v3Normal;
		}
	}

	public class F3DCurveThrow
	{

		static public float ms_fCurveThrowMinTime = 0.2f,
			ms_fCurveThrowMaxTime = 1.5f,
			ms_fCurveThrowMinLen = 2.0f,
			ms_fCurveThrowMaxLen = 35.0f;
		//ms_fCurveThrowMaxHeight = 3.0f;

		Vector3 m_v3V0;

		Vector3 m_v3Up,
			m_v3Left // left vector for shape
			,
			m_v3Side; // Side vector for side curve

		Vector3 m_v3Direction;
		Vector3 m_v3Start, m_v3End;
		float m_fTimeLen;
		float m_fCurveSide;
		static float m_fK = .2f; // friction value
		int m_nType = 0; // 0 is roll, 1 is throwing

		public F3DCurveThrow(Vector3 _v3Start, Vector3 _v3End, int _nType, float _fCurveSide, float _fTimeMin)
		{
			if (_nType < 2)
				m_nType = 0; // Rolling
			else
				m_nType = 1; // Flying
			m_v3Start = _v3Start;
			m_v3End = _v3End;
			//m_v3Start.y += 0.2f;
			//m_v3End.y += 0.2f;
			m_fCurveSide = _fCurveSide; // Side Curve accel
			m_v3Up = new Vector3(0, 1, 0);

			init(_fTimeMin);
		}

		void init(float _fTimeMin)
		{
			m_v3Direction = m_v3End - m_v3Start;

			float fBlend = 0;
			if (m_v3Direction.magnitude <= ms_fCurveThrowMinLen)
			{
				m_fTimeLen = ms_fCurveThrowMinTime;
				fBlend = 0;
			}
			else if (m_v3Direction.magnitude >= ms_fCurveThrowMaxLen)
			{
				m_fTimeLen = ms_fCurveThrowMaxTime;
				fBlend = 1;
			}
			else
			{
				fBlend = (m_v3Direction.magnitude - ms_fCurveThrowMinLen) /
				         (ms_fCurveThrowMaxLen - ms_fCurveThrowMinLen);
				m_fTimeLen = ms_fCurveThrowMinTime + (ms_fCurveThrowMaxTime - ms_fCurveThrowMinTime) * fBlend;
			}

			if (_fTimeMin > 0 && m_fTimeLen < _fTimeMin)
				m_fTimeLen = _fTimeMin;

			// initial velocity
			m_v3V0.y = -F3DMath.ms_fG * m_fTimeLen / 2.0f + (m_v3End.y - m_v3Start.y) / m_fTimeLen;
			m_v3V0.x = m_v3Direction.x * m_fK / (1.0f - Mathf.Pow(F3DMath.ms_fE, (-m_fK * m_fTimeLen)));
			m_v3V0.z = m_v3Direction.z * m_fK / (1.0f - Mathf.Pow(F3DMath.ms_fE, (-m_fK * m_fTimeLen)));

			// flat left vector
			m_v3Side = Vector3.Cross(m_v3Direction.normalized, m_v3Up);
			m_v3Side.Normalize();

			// real up vector
			if (m_nType == 1)
			{
				float fTimeMiddle = m_fTimeLen / 2.0f;
				Vector3 v3Middle = get_pos(fTimeMiddle);
				Vector3 v3Up = v3Middle - m_v3Start;
				Vector3 v3Project = F3DMath.project(m_v3Direction.normalized, v3Up);
				m_v3Up = v3Project.normalized;

				m_v3Left = Vector3.Cross(m_v3Direction.normalized, m_v3Up);
			}
			else
			{
				m_v3Left = m_v3Side;
			}
		}

		public float get_len()
		{
			float fLen = 0;
			Vector3 v3Dis;
			Vector3 v3Middle = get_pos(m_fTimeLen / 2.0f);

			v3Dis = v3Middle - m_v3Start;
			fLen = v3Dis.magnitude;
			v3Dis = m_v3End - v3Middle;
			fLen += v3Dis.magnitude;

			return fLen;
		}

		public Vector3 get_pos_rate(float _fRate)
		{
			if (_fRate > 1)
				_fRate = 1;

			float fTime = m_fTimeLen * _fRate;
			return get_pos(fTime);
		}

		public Vector3 get_pos(float _fTime)
		{
			// throwing ball's position
			Vector3 v3Pos;

			if (_fTime > m_fTimeLen)
				_fTime = m_fTimeLen;

			if (m_nType == 1)
				v3Pos.y = F3DMath.ms_fG * Mathf.Pow(_fTime, 2.0f) / 2.0f + m_v3V0.y * _fTime;
			else
				v3Pos.y = m_v3Start.y;
			v3Pos.x = m_v3V0.x * (1.0f - Mathf.Pow(F3DMath.ms_fE, (-m_fK * _fTime))) / m_fK;
			v3Pos.z = m_v3V0.z * (1.0f - Mathf.Pow(F3DMath.ms_fE, (-m_fK * _fTime))) / m_fK;

			float fSide = 0;
			fSide = -m_fCurveSide * Mathf.Pow(_fTime, 2.0f) / 2.0f + m_fCurveSide * _fTime;
			v3Pos = v3Pos + m_v3Side * fSide;

			v3Pos += m_v3Start;
			v3Pos.y += 0.2f;
			return v3Pos;
		}

		public Vector3 get_pos_wing_rate(float _fRate, float _fWingRate)
		{
			float fTime = m_fTimeLen * _fRate;

			return get_pos_wing(fTime, _fWingRate);
		}

		public Vector3 get_pos_wing(float _fTime, float _fWingRate)
		{
			// get wing of arrow
			float fWing = F3DCurveRun.get_wing_with_param_(m_v3Direction.magnitude) *
			              _fWingRate; //m_v3Direction.magnitude * _fWingRate;
			Vector3 v3Pos = get_pos(_fTime) + m_v3Left * fWing;
			v3Pos.y += 0.2f;
			return v3Pos;
		}

		public Vector3 get_normal(float _fRate)
		{
			if (m_nType == 0)
				return m_v3Up;

			Vector3 v1, v2, vd;
			if (_fRate < 0.5f)
			{
				v1 = get_pos_rate(_fRate);
				v2 = get_pos_rate(_fRate + 0.01f);
			}
			else
			{
				v1 = get_pos_rate(_fRate - 0.01f);
				v2 = get_pos_rate(_fRate);
			}

			vd = v2 - v1;
			Vector3 v3Nor;
			v3Nor = Vector3.Cross(m_v3Left, vd.normalized);
			return v3Nor.normalized;
		}

		public float get_time_len()
		{
			return m_fTimeLen;
		}
	}

	class F3DCurve4
	{
		Vector3[] m_point4_av3;

		public F3DCurve4(Vector3[] _points)
		{
			m_point4_av3 = _points;
		}

		public float length_rough()
		{
			float len_f = 0;

			for (int i = 0; i < 3; i++)
			{
				len_f += (m_point4_av3[i + 1] - m_point4_av3[i]).magnitude;
			}

			return len_f;
		}

		public Vector3 get_rate(float _rate_f, ref Vector3 _left_v3)
		{
			Vector3 ret_v3 = new Vector3();
			Vector3 next_v3 = new Vector3();
			Vector3 normal_v3 = new Vector3(0, 1, 0);
			float[] ctrX_f = new float[4];
			float[] ctrZ_f = new float[4];

			for (int i = 0; i < 4; i++)
			{
				ctrX_f[i] = m_point4_av3[i].x;
				ctrZ_f[i] = m_point4_av3[i].z;
			}

			ret_v3.x = F3DMath.BezierCurve(ctrX_f, _rate_f);
			ret_v3.y = 0;
			ret_v3.z = F3DMath.BezierCurve(ctrZ_f, _rate_f);

			_rate_f += 0.1f;

			next_v3.x = F3DMath.BezierCurve(ctrX_f, _rate_f);
			next_v3.y = 0;
			next_v3.z = F3DMath.BezierCurve(ctrZ_f, _rate_f);

			next_v3 = next_v3 - ret_v3;
			next_v3.Normalize();
			_left_v3 = Vector3.Cross(normal_v3, next_v3);
			return ret_v3;
		}
	}

	class F3DCurveMulti
	{
		Vector3[] m_points_av3;
		float[] m_times_af;

		F3DCurveRun m_curve;

		public F3DCurveMulti(List<Vector3> _points_av3, List<float> _times_af)
		{
			int cnt_n = _points_av3.Count;
			m_points_av3 = new Vector3[_points_av3.Count + 2];
			m_times_af = new float[_times_af.Count];

			Vector3 direction_v3;

			direction_v3 = _points_av3[0] - _points_av3[1];
			m_points_av3[0] = _points_av3[0] + direction_v3;

			for (int i = 0; i < cnt_n; i++)
			{
				m_points_av3[i + 1] = _points_av3[i];
				m_times_af[i] = _times_af[i];
			}

			direction_v3 = _points_av3[cnt_n - 2] - _points_av3[cnt_n - 1];
			m_points_av3[cnt_n] = _points_av3[cnt_n - 1] + direction_v3;
		}

		public void curve_set(int _nIndex)
		{
			Vector3[] point_ctr_av3 = new Vector3[4];
			float[] times_af = new float[3];

			for (int i = 0; i < 4; i++)
				point_ctr_av3[i] = m_points_av3[_nIndex + i];
			for (int i = 0; i < 3; i++)
				times_af[i] = m_times_af[_nIndex + i];

			m_curve = new F3DCurveRun(4, point_ctr_av3, times_af, 0.05f);
		}

		public Vector3 get_rate(float _rate_f)
		{
			return m_curve.get_pos_rate(_rate_f);
		}

		public Vector3 get_left(float _rate_f, ref float _speed_f)
		{
			return m_curve.get_left(_rate_f, ref _speed_f);
		}
	}
}