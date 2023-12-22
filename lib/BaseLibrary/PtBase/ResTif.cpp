#include "stdafx.h"
#include "ResTif.h"

//#include "4DYUCHIGX_RENDER/image.h"
#include "lodepng.h"

ResTif	ResTif::sm_sample;

ResTif::ResTif(void)
{
	m_nSerialNumber	= serial_create_();	// Unique number for each objects
	m_nObjectId		= ECARE_RES_TEXTURE;
	m_bReturnLoad1	= false;
	m_pData			= NULL;
}

ResTif::~ResTif(void)
{
	Clear();
}

bbyte*ResTif::get_data()
{
	return m_pData;
}

bool ResTif::Load()
{
	//// Load file and decode image.
	//CImage	image;
	//
	//if(image.LoadTIF(get_name().c_str(),4))
	//{	
	//	m_nWidth	= image.GetWidth();
	//	m_nHeight	= image.GetHeight();

	//	bbyte *pData	= (bbyte*)image.GetRawImage();
	//	bbyte *pImg = new bbyte[m_nWidth*m_nHeight*4];
	//	memset(pImg, 0, m_nWidth*m_nHeight*4);

	//	for(int y=0; y<m_nHeight; y++)
	//		for(int x=0; x<m_nWidth; x++)
	//		{
	//			int nPos = (y*m_nWidth+x)*4;
	//			int nPos2 = ((m_nHeight-y-1)*m_nWidth+x)*4;

	//			*(pImg+nPos+0)	= *(pData+nPos2+0);// B
	//			*(pImg+nPos+1)	= *(pData+nPos2+1);// G
	//			*(pImg+nPos+2)	= *(pData+nPos2+2);// R
	//			*(pImg+nPos+3)	= *(pData+nPos2+3);// A
	//		}

	//	m_bReturnLoad1	= true;

	//	m_pData	= pImg;
	//	return true;
	//}else
	{
		STLVcChar image;
		unsigned width, height;
		std::string	filename;
		filename	= get_name().c_str();
		unsigned error = LodePNG::decode(image, width, height, filename);

		if(error)
		{
			g_SendMessage(LOG_MSG_POPUP, "Fail to open file '%s'.", filename.c_str());
            return false;
		}
		m_nWidth	= width;
		m_nHeight	= height;

		bbyte*pImg = new bbyte[m_nWidth*m_nHeight*4];
		memset(pImg, 0, m_nWidth*m_nHeight*4);

		for(int y=0; y<m_nHeight; y++)
			for(int x=0; x<m_nWidth; x++)
			{
				int nPos = (y*m_nWidth+x)*4;
				int nPos2 = (y*m_nWidth+x)*4;

//				*(pImg+nPos+0)	= image[nPos2+2];// B
//				*(pImg+nPos+1)	= image[nPos2+1];// G
//				*(pImg+nPos+2)	= image[nPos2+0];// R
//				*(pImg+nPos+3)	= image[nPos2+3];// A

				*(pImg+nPos+0)	= image[nPos2+0];// R
				*(pImg+nPos+1)	= image[nPos2+1];// G
				*(pImg+nPos+2)	= image[nPos2+2];// B
				*(pImg+nPos+3)	= image[nPos2+3];// A
			}

		m_bReturnLoad1	= true;

		m_pData	= pImg;
		return true;
	}

	return false;
}

bool ResTif::Load2()
{
	if(!m_bReturnLoad1)
		return false;
	return true;
}

void ResTif::Clear()
{
	delete[] m_pData;
	m_pData	= NULL;
}
