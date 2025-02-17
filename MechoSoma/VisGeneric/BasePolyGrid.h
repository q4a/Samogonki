#ifndef __BASEPOLYGRID_H__
#define __BASEPOLYGRID_H__

#include "BaseObject.h"

#define ATTRIBUTE_POLYGRID_COLOR_ADD					(1<<0)

struct sBaseColor4c
{
	unsigned char r,g,b,a;
	inline void Set(int rc,int gc,int bc)							{ r=rc; g=gc; b=bc; }
	inline void Set(int rc,int gc,int bc,int ac)					{ r=rc; g=gc; b=bc; a=ac; }
	inline sBaseColor4c operator + (sBaseColor4c &p)				{ sBaseColor4c tmp={r+p.r,g+p.g,b+p.b,a+p.a}; return tmp; }
	inline sBaseColor4c& operator += (sBaseColor4c &p)				{ r+=p.r; g+=p.g; b+=p.b; a+=p.a; return *this; }
};
struct sBasePoint3c
{
	char x,y,z;
	inline void Set(int xw,int yw,int zw)							{ x=xw; y=yw; z=zw; }
	inline sBasePoint3c operator + (sBasePoint3c &p)				{ sBasePoint3c tmp={x+p.x,y+p.y,z+p.z}; return tmp; }
	inline sBasePoint3c& operator += (sBasePoint3c &p)				{ x+=p.x; y+=p.y; z+=p.z; return *this; }
};
struct sBasePoint2c
{
	char x,y;
	inline void Set(int xw,int yw)									{ x=xw; y=yw; }
	inline sBasePoint2c operator + (sBasePoint2c &p)				{ sBasePoint2c tmp={x+p.x,y+p.y}; return tmp; }
	inline sBasePoint2c& operator += (sBasePoint2c &p)				{ x+=p.x; y+=p.y; return *this; }
};
// ��������� ����������� ����������
struct sBaseWarpTime
{
	float time;
};
struct sBaseWarpWave
{
	short w[2];
	short v[2];
};

class cBasePolyGrid : public cBaseObject, public cBaseDrawObject
{	// ���������� ������������� �����
public:	
	cMatrix				Matrix;
	sBaseColor4c		*pColor;							// ���� ������� ������������� �����
	sBasePoint2c		*pUVWarp;							// ���������� ���������� ��������� ������ ������������� �����
	sBasePoint3c		*pXYZWarp;							// ���������� ���������������� ��������� ������ ������������� �����
	void				*pStructWarp;						// ��������� ����������� ����������
	int					xsize,ysize;						// ����������� ������������� �����	
	float				xstep,ystep;						// ��� ���������� �����
	float				u,v,du,dv,uofs,vofs;				// ���������� ���������� �� ����� uofs+du .. u+uofs+du , vofs+dv .. v+vofs+dv (du � dv - ������������ ��������)
	int					Attribute;

	cBasePolyGrid(unsigned int type,unsigned int kind) : cBaseObject(type,kind)	{ pColor=0; pXYZWarp=0; pUVWarp=0; pStructWarp=0; Matrix.NewMatrix(); }
	~cBasePolyGrid()												
	{ 
		FREE_MEM(pColor); 
		FREE_MEM(pXYZWarp); 
		FREE_MEM(pUVWarp); 
		FREE_MEM(pStructWarp); 
	}
	void New(int xSize,int ySize,float xStep,float yStep);
//	int Draw(cCamera *Camera);
	void SetPosition(float x,float y,float z);
	inline int GetNumber(int i,int j)						{ return (i%xsize)+(j%ysize)*xsize; }
	inline float& x()										{ return Matrix.trans().x; }
	inline float& y()										{ return Matrix.trans().y; }
	inline float& z()										{ return Matrix.trans().z; }
protected:
//	int DrawSpecialCycl(cCamera *Camera,cMaterial &Material);
//	int DrawCycl(cCamera *Camera,int i1,int j1,int i2,int j2,cMaterial &Material);
//	int DrawTilePolyGrid(cCamera *Camera,int i1,int j1,int i2,int j2,cMaterial &Material);
};

class cWorldPolyGrid : public cBasePolyGrid
{
public:
	float				CyclDist;

	cWorldPolyGrid(	unsigned int type=BASEOBJECT_TYPE_BASEPOLYGRID_WORLD,
					unsigned int kind=BASEOBJECT_KIND_DRAW_EFFECT) : cBasePolyGrid(type,kind)	{ BaseDrawObject()->SetAttribute(BASEOBJECT_ATTRIBUTE_DRAW_SPECIALCYCL|BASEOBJECT_ATTRIBUTE_DRAW_POLYGONCCW|BASEOBJECT_ATTRIBUTE_DRAW_COLOR); Attribute=0; }
};

extern void BaseObject_WorldPolyGrid_Draw(cCamera *Camera);

inline void ShareMapping1(cCamera *Camera,float &xPos,float &yPos,float &zw,float RadiusWorldShare,float SqrRadiusWorldShare,float SqrRadiusWorldShare2,float RadiusWorldShare2)
{// �� ����������� ���� ����
	if(Camera->GetAttribute(ATTRIBUTE_CAMERA_PERSPECTIVE_WORLD_SHARE)!=(ATTRIBUTE_CAMERA_PERSPECTIVE_WORLD_SHARE))
		return;
   	float l2=round(xPos*xPos+yPos*yPos);
	if(l2>SqrRadiusWorldShare2) l2=SqrRadiusWorldShare2;
	float dz=-l2/RadiusWorldShare2;
	float Lz=RadiusWorldShare+dz;
	float norma=sqrt(SqrRadiusWorldShare-Lz*Lz)/sqrt(l2);
	xPos*=norma; yPos*=norma;
	zw+=dz;
}
__forceinline void ShareMapping2(cCamera *Camera,Vect3f &pos,float RadiusWorldShare,float DivRadiusWorldShare)
{// �� ����� ����
	if(Camera->GetAttribute(ATTRIBUTE_CAMERA_PERSPECTIVE_WORLD_SHARE)!=(ATTRIBUTE_CAMERA_PERSPECTIVE_WORLD_SHARE))
		return;
   	float l2=pos.x*pos.x+pos.y*pos.y;
	if(fabs(l2)<1.f) return;
	float r=sqrt(l2);
	int angle=round(r*DivRadiusWorldShare);	// DivRadiusWorldShare=GRAD_TO_DGRAD(360)/(2*pi)/RadiusWorldShare
	assert((angle>=GRAD_TO_DGRAD(0))&&(angle<GRAD_TO_DGRAD(360)));
	float l=RadiusWorldShare*SIN_FLOAT_DGRAD(angle);
	float norma=l/r;
	float dz=-RadiusWorldShare*(1-COS_FLOAT_DGRAD(angle));
	pos.x*=norma; pos.y*=norma; pos.z+=dz;
}

class cBaseWaveProcess : public cBaseObject
{	// ����� ����� ����� Time=0 ��� Amplitude<=0, ������� ���������� ����� � ������ ���� ������
public:
	Vect3f			Pos;					// ����� ��������� 
	float			Time,dTime;				// ������� ����� �����, ��������� ������� �����
	float			Amplitude,dAmplitude;	// ������� ��������� ���������, � ��������� ��������� (���������)
	float			Phase,dPhase;			// ������� ���� � ��������� ����

	cBaseWaveProcess(unsigned int type=BASEOBJECT_TYPE_BASEWAVEPROCESS,unsigned int kind=BASEOBJECT_KIND_PHYSICPROCESS) : cBaseObject(type,kind) { Pos.set(0,0,0); Time=1.f; dTime=0.01f; Amplitude=1.f; dAmplitude=0.01f; Phase=0.f; dPhase=0.01f; }
	int Update()							{ Time-=dTime; Amplitude-=dAmplitude; Phase+=dPhase; if(Time>0) return 1; BaseObject()->Type|=BASEOBJECT_TYPE_DELETE; return 0; }
};

extern void* BaseObject_BaseWaveProcess_Create(float x,float y,float z,float TimeLife=1.f,float dPhase=0.3f);

#endif __BASEPOLYGRID_H__