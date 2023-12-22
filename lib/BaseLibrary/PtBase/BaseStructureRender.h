#pragma once

typedef struct _SPtGeometry_{
	Vector		*pxArrayVertex_;
	Vector		*pxArrayNormal_;
	Vector4		*pxArrayColor_;
	Vector2		*pxArrayUV_;
	unsigned short		*pxArrayIndex_;	///< if point is NULL, means that not a indexed polygon
	//char		*pxArrayMatrix_;
	//PT_FIXED	*pxArrayWeight_;

	int			nNumOfTexture_;	///< it's for multitexture. 1 means that single texture
	//int			nNumOfPolygon_;
	int			nNumOfVertex_;
	int			nType_;

	enum{// nType_
		TRIANGLES,
		POINTS,
		LINES
	};
} SPtGeometry;

typedef struct _SPtMaterial_{
	float		fShininess_;
	Vector4		v4Specular_;
	Vector4		v4Diffuse_;
	Vector4		v4Ambient_;
	Vector4		v4Emission_;
} SPtMaterial;

typedef struct _SPtLight_{
	Vector		v3Pos_;
	enum{
		LIGHT_SPOT,
		LIGHT_DIRECTION
	};
	int			nType_;
	Vector4		v4Specular_;
	Vector4		v4Diffuse_;
	Vector4		v4Ambient_;
} SPtLight;

typedef struct _SPtCamera_{
	float		m4Projection_[4][4];
	Matrix		m4Frame_;
} SPtCamera;

typedef struct _SPtTexture_{
	UINT32 nID_;
	Vector		v3UVOffset_;
	int			nWidth_;
	int			nHegith_;
	int			nFormat_;
	int			nType_;
	unsigned char *pData_;
} SPtTexture;

typedef struct _SPtShader_{
	int			 nID_;
} SPtShader;

typedef struct _SPtSound_{
	int			nID_;
	Vector		v3Pos_;
} SPtSound;

typedef struct _SPtAnimation{
	int			nID_;					// hash key for bone name
	int			nNumOfKey_;				// number of animation key
	int			*pxArrayKeys_;			// time for key
	Vector4		*pxArrayRotation_;		// orientation
	Vector		*pxArrayPositioin_;		// position
} SPtAnimation;
