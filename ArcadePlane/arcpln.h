

struct Vertex
{
	float u, v;
	unsigned int color;
	float x,y,z;
};

struct HcArcplnLightInfo {
  float posx;
  float posy;
  float posz;
  int color;
  float att_const;
  float att_linear;
  float att_quad;
  float specular;
};

struct HcArcplnObjectInfo {
	int id;
  int originX;
  int originY;
  int originZ;
  int targetX;
  int targetY;
  int targetZ;
  int rotX;
  int rotY;
  int rotZ;
  int speed;
  void* guModelData;
  int guModelNumTris;
  void* guTexData;
  int texBufWidth;
  int texWidth;
  int texHeight;
  int texSwizzled;
  
  int healthmax;
  int health;
  int killpoints;
  int wave;
  
  void* additionaldata;
};


void hcArcplnInitGU( void );
